#include "Weapon/CWeaponActor.h"

#include "ProjectGlobal.h"

#include "Component/CCombatSignalSourceComponent.h"
#include "Core/Debug/FCombatSignalDebug.h"
#include "Core/Profiling/CCombatCollisionProfilingCounters.h"
#include "Type/CWeaponTypes.h"
#include "Type/CCombatHitTypes.h"
#include "Type/CCombatSignalTypes.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"
#include "NiagaraComponent.h"

ACWeaponActor::ACWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	check(RootSceneComponent);

	SetRootComponent(RootSceneComponent);

	TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trail"));
	check(TrailComponent);

	TrailComponent->SetupAttachment(RootSceneComponent);
	TrailComponent->bAutoActivate = false;
}

// Component Reference

void ACWeaponActor::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	CombatSignalSourceComp_Injected = InReferences.CombatSignalSourceComponent;

	ValidateRequiredReferences();
}

bool ACWeaponActor::ValidateRequiredReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ CombatSignalSourceComp_Injected, TEXT("UCCombatSignalSourceComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Initial State

void ACWeaponActor::ApplyInitialWeaponState(EWeaponType InWeaponType)
{
	ChangeWeaponType(InWeaponType);
	AttachToHolsterSocket();
}

// Lifecycle

void ACWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	InitializeCollisionComponents();
	InitializeTrailState();
}

void ACWeaponActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CollisionDisabled();

	ClearTrailState();
	ClearCollisionComponents();

	OwnerCharacter_Injected = nullptr;
	CombatSignalSourceComp_Injected = nullptr;

	Super::EndPlay(EndPlayReason);
}

// Collision Component

void ACWeaponActor::InitializeCollisionComponents()
{
	if (!IsValid(RootSceneComponent)) return;

	ClearCollisionComponents();

	TArray<USceneComponent*> children;
	RootSceneComponent->GetChildrenComponents(true, children);

	for (USceneComponent* child : children)
	{
		UShapeComponent* shape = Cast<UShapeComponent>(child); // UShapeComponent base for shape collision components (Sphere / Box / Capsule)
		if (!IsValid(shape)) continue;

		shape->OnComponentBeginOverlap.AddDynamic(this, &ACWeaponActor::OnComponentBeginOverlap);
		shape->OnComponentEndOverlap.AddDynamic(this, &ACWeaponActor::OnComponentEndOverlap);
		shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		Collisions_Cached.Add(shape);
	}
}

void ACWeaponActor::ClearCollisionComponents()
{
	for (UShapeComponent* collision : Collisions_Cached)
	{
		if (!IsValid(collision)) continue;

		collision->OnComponentBeginOverlap.RemoveDynamic(this, &ACWeaponActor::OnComponentBeginOverlap);
		collision->OnComponentEndOverlap.RemoveDynamic(this, &ACWeaponActor::OnComponentEndOverlap);
		collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Collisions_Cached.Empty();
}

// Trail

void ACWeaponActor::InitializeTrailState()
{
	if (!bDisableTrailOnBeginPlay) return;

	ToggleTrailActive(false);
}

void ACWeaponActor::ClearTrailState()
{
	ToggleTrailActive(false);
}

// Hit Context Provider Query

const FOverlapContext& ACWeaponActor::GetLastOverlapContext() const
{
	return LastOverlapContext_Cached;
}

const FWeaponContext& ACWeaponActor::GetLastWeaponContext() const
{
	return LastWeaponContext_Cached;
}

const FActionDataKey& ACWeaponActor::GetLastActionDataKey() const
{
	return LastActionDataKey_Cached;
}

// Hit Context Provider Mutation

void ACWeaponActor::SetLastOverlapContext(const FOverlapContext& InOverlapContext)
{
	LastOverlapContext_Cached = InOverlapContext;
}

void ACWeaponActor::SetLastWeaponContext(const FWeaponContext& InWeaponContext)
{
	LastWeaponContext_Cached = InWeaponContext;
}

void ACWeaponActor::SetLastActionDataKey(const FActionDataKey& InActionDataKey)
{
	LastActionDataKey_Cached = InActionDataKey;
}

// Mutation

void ACWeaponActor::ChangeWeaponType(EWeaponType InWeaponType)
{
	WeaponType = InWeaponType;
}

void ACWeaponActor::ToggleTrailActive(bool bEnable)
{
	if (!IsValid(TrailComponent)) return;

	if (bEnable)
	{
		TrailComponent->SetVisibility(true);
		TrailComponent->Activate(true);
	}
	else
	{
		TrailComponent->Deactivate();
		TrailComponent->SetVisibility(false);
	}
}

// Equip Notify Events

void ACWeaponActor::AttachToHandSocket()
{
	AttachToOwnerSocket(SocketName_Hand);
}

void ACWeaponActor::AttachToHolsterSocket()
{
	AttachToOwnerSocket(SocketName_Holster);
}

bool ACWeaponActor::GetAttachmentRelativeTransform(FTransform& OutRelativeTransform) const
{
	if (!IsValid(RootSceneComponent)) return false;

	OutRelativeTransform = RootSceneComponent->GetRelativeTransform();
	return true;
}

bool ACWeaponActor::SetAttachmentRelativeTransform(const FTransform& InRelativeTransform)
{
	if (!IsValid(RootSceneComponent)) return false;

	RootSceneComponent->SetRelativeTransform(InRelativeTransform);
	return true;
}

// Collision Notify Events

void ACWeaponActor::CollisionEnabled(FName InName)
{
	TArray<UShapeComponent*> collisionsToEnable;

	if (!InName.IsNone())
	{
		for (UShapeComponent* collision : Collisions_Cached)
		{
			if (collision->GetFName() == InName)
			{
				collisionsToEnable.Add(collision);
				break;
			}
		}
	}
	else
	{
		for (UShapeComponent* collision : Collisions_Cached)
		{
			collisionsToEnable.Add(collision);
		}
	}

	// Reject empty collision enable requests.
	if (collisionsToEnable.IsEmpty())
	{
		FCombatSignalDebug::RecordWeaponCollisionWindowForAudit(
			OwnerCharacter_Injected,
			this,
			InName,
			CurrentHitWindowId,
			Collisions_Cached.Num(),
			TEXT("CollisionEnableRejected"),
			InName.IsNone() ? TEXT("NoCollisionComponents") : TEXT("CollisionNameNotFound"));
		return;
	}

	if (!bHitWindowOpened)
	{
		++CurrentHitWindowId;
		bHitWindowOpened = true;

		FCombatCollisionProfilingCounters::RecordHitWindowOpen();

		if (IsValid(CombatSignalSourceComp_Injected))
		{
			CombatSignalSourceComp_Injected->NotifyHitWindowOpened(this, CurrentHitWindowId);
		}
		else
		{
			FCombatSignalDebug::RecordWeaponCollisionWindowForAudit(
				OwnerCharacter_Injected,
				this,
				InName,
				CurrentHitWindowId,
				collisionsToEnable.Num(),
				TEXT("HitWindowOpenWarning"),
				TEXT("MissingCombatSignalSourceComponent"));
		}
	}

	for (UShapeComponent* collision : collisionsToEnable)
	{
		collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (OnWeaponActorCollisionEnabled.IsBound())
		OnWeaponActorCollisionEnabled.Broadcast();

	FCombatSignalDebug::RecordWeaponCollisionWindowForAudit(
		OwnerCharacter_Injected,
		this,
		InName,
		CurrentHitWindowId,
		collisionsToEnable.Num(),
		TEXT("CollisionEnabled"));
}

void ACWeaponActor::CollisionDisabled()
{
	if (!bHitWindowOpened)
	{
		FCombatSignalDebug::RecordWeaponCollisionWindowForAudit(
			OwnerCharacter_Injected,
			this,
			NAME_None,
			CurrentHitWindowId,
			Collisions_Cached.Num(),
			TEXT("CollisionDisableIgnored"),
			TEXT("HitWindowNotOpened"));
		return;
	}

	FCombatCollisionProfilingCounters::RecordHitWindowClose();

	for (UShapeComponent* collision : Collisions_Cached)
	{
		collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	bHitWindowOpened = false;

	if (IsValid(CombatSignalSourceComp_Injected) && CurrentHitWindowId != INDEX_NONE)
	{
		CombatSignalSourceComp_Injected->NotifyHitWindowClosed(this, CurrentHitWindowId);
	}
	else
	{
		FCombatSignalDebug::RecordWeaponCollisionWindowForAudit(
			OwnerCharacter_Injected,
			this,
			NAME_None,
			CurrentHitWindowId,
			Collisions_Cached.Num(),
			TEXT("HitWindowCloseWarning"),
			TEXT("MissingCombatSignalSourceComponentOrInvalidHitWindow"));
	}

	// Notify legacy collision disabled listeners.
	if (OnWeaponActorCollisionDisabled.IsBound())
		OnWeaponActorCollisionDisabled.Broadcast();

	FCombatSignalDebug::RecordWeaponCollisionWindowForAudit(
		OwnerCharacter_Injected,
		this,
		NAME_None,
		CurrentHitWindowId,
		Collisions_Cached.Num(),
		TEXT("CollisionDisabled"));
}

// Engine Delegate Events

void ACWeaponActor::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UShapeComponent* overlapComp = Cast<UShapeComponent>(OverlappedComponent);
	if (!IsValid(overlapComp))
	{
		FCombatSignalDebug::RecordWeaponOverlapRejectedForAudit(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, CurrentHitWindowId, TEXT("BeginOverlap"), TEXT("InvalidOverlapComponent"));
		return;
	}

	if (!IsValid(OwnerCharacter_Injected) || !IsValid(OtherActor))
	{
		FCombatSignalDebug::RecordWeaponOverlapRejectedForAudit(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, CurrentHitWindowId, TEXT("BeginOverlap"), TEXT("InvalidOwnerOrOtherActor"));
		return;
	}

	if (OwnerCharacter_Injected == OtherActor)
	{
		FCombatSignalDebug::RecordWeaponOverlapIgnoredForAudit(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, CurrentHitWindowId, TEXT("BeginOverlap"), TEXT("SelfOverlap"));
		return;
	}

	if (!IsValid(CombatSignalSourceComp_Injected))
	{
		FCombatSignalDebug::RecordWeaponOverlapRejectedForAudit(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, CurrentHitWindowId, TEXT("BeginOverlap"), TEXT("MissingCombatSignalSourceComponent"));
		return;
	}

	FCombatCollisionProfilingCounters::RecordHitWindowOverlap();

	FOverlapContext overlapContext = BuildOverlapContext(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	FHitContext hitContext = BuildHitContext(overlapContext);

	FCombatSignalDebug::RecordWeaponOverlapAcceptedForAudit(hitContext, TEXT("BeginOverlap"));
	FCombatSignalDebug::PrintWeaponHitContextDebug(hitContext);

	// Notify legacy overlap listeners before forwarding the combat signal.
	if (OnWeaponActorBeginOverlap.IsBound())
		OnWeaponActorBeginOverlap.Broadcast(OwnerCharacter_Injected, this, overlapComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	CombatSignalSourceComp_Injected->RequestCombatSignalSource(hitContext);
	LastOverlapContext_Cached = overlapContext;
}

void ACWeaponActor::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UShapeComponent* overlapComp = Cast<UShapeComponent>(OverlappedComponent);
	if (!IsValid(overlapComp))
	{
		FCombatSignalDebug::RecordWeaponOverlapIgnoredForAudit(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, CurrentHitWindowId, TEXT("EndOverlap"), TEXT("InvalidOverlapComponent"));
		return;
	}

	if (!IsValid(OwnerCharacter_Injected) || !IsValid(OtherActor))
	{
		FCombatSignalDebug::RecordWeaponOverlapIgnoredForAudit(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, CurrentHitWindowId, TEXT("EndOverlap"), TEXT("InvalidOwnerOrOtherActor"));
		return;
	}

	if (OwnerCharacter_Injected == OtherActor)
	{
		FCombatSignalDebug::RecordWeaponOverlapIgnoredForAudit(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, CurrentHitWindowId, TEXT("EndOverlap"), TEXT("SelfOverlap"));
		return;
	}

	if (!IsValid(CombatSignalSourceComp_Injected))
	{
		FCombatSignalDebug::RecordWeaponOverlapIgnoredForAudit(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, CurrentHitWindowId, TEXT("EndOverlap"), TEXT("MissingCombatSignalSourceComponent"));
		return;
	}

	// Notify legacy overlap listeners after validating the end overlap.
	if (OnWeaponActorEndOverlap.IsBound())
		OnWeaponActorEndOverlap.Broadcast(OwnerCharacter_Injected, OtherActor);
}

// Helper

FOverlapContext ACWeaponActor::BuildOverlapContext(AActor* InOwnerActor, AActor* InDamageCauser, UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) const
{
	FOverlapContext overlapContext;

	overlapContext.OwnerActor = InOwnerActor;
	overlapContext.DamageCauser = InDamageCauser;
	overlapContext.OverlappedComponent = OverlappedComponent;
	overlapContext.OverlapShape = Cast<UShapeComponent>(OverlappedComponent);
	overlapContext.OtherActor = OtherActor;
	overlapContext.OtherComponent = OtherComp;
	overlapContext.OtherBodyIndex = OtherBodyIndex;
	overlapContext.bFromSweep = bFromSweep;
	overlapContext.SweepResult = bFromSweep ? SweepResult : FHitResult();
	overlapContext.HitWindowId = CurrentHitWindowId;

	return overlapContext;
}

FHitImpactContext ACWeaponActor::BuildHitImpactContext(const FOverlapContext& InOverlapContext) const
{
	FHitImpactContext hitImpactContext;

	if (!IsValid(InOverlapContext.OverlappedComponent) || !IsValid(InOverlapContext.OtherComponent))
	{
		// Invalid overlap context cannot produce hit impact data.
		return hitImpactContext;
	}

	if (InOverlapContext.bFromSweep)
	{
		hitImpactContext.bHasHitResult = true;
		hitImpactContext.Source = EHitImpactContextSource::SweepResult;
		hitImpactContext.HitResult = InOverlapContext.SweepResult;

		return hitImpactContext;
	}

	const FVector queryLocation = InOverlapContext.OverlappedComponent->GetComponentLocation();

	FVector closestPoint = FVector::ZeroVector;
	const float distance = InOverlapContext.OtherComponent->GetClosestPointOnCollision(queryLocation, closestPoint);

	if (distance < 0.f)
	{
		// Invalid closest-point query cannot produce hit impact data.
		return hitImpactContext;
	}

	FHitResult hitResult;

	hitResult.bBlockingHit = false;
	hitResult.ImpactPoint = closestPoint;
	hitResult.Location = closestPoint;

	if (IsValid(InOverlapContext.OtherActor))
	{
		hitResult.HitObjectHandle = FActorInstanceHandle(InOverlapContext.OtherActor);
	}

	if (IsValid(InOverlapContext.OtherComponent))
	{
		hitResult.Component = InOverlapContext.OtherComponent;
	}

	FVector impactNormal = (queryLocation - closestPoint).GetSafeNormal();

	hitResult.ImpactNormal = impactNormal;
	hitResult.Normal = impactNormal;

	hitImpactContext.bHasHitResult = true;
	hitImpactContext.Source = EHitImpactContextSource::ClosestPoint;
	hitImpactContext.HitResult = hitResult;

	return hitImpactContext;
}

FHitContext ACWeaponActor::BuildHitContext(const FOverlapContext& InOverlapContext) const
{
	FHitContext hitContext;

	hitContext.OverlapContext = InOverlapContext;
	hitContext.WeaponContext = LastWeaponContext_Cached;
	hitContext.ActionDataKey = LastActionDataKey_Cached;
	hitContext.HitImpactContext = BuildHitImpactContext(InOverlapContext);

	return hitContext;
}

void ACWeaponActor::AttachToOwnerSocket(FName InSocketName)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return;

	AttachToComponent(meshComp, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), InSocketName);
}
