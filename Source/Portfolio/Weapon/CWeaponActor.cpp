#include "Weapon/CWeaponActor.h"

#include "ProjectGlobal.h"

#include "Component/CCombatSignalSourceComponent.h"
#include "Core/Debug/FCombatSignalDebug.h"
#include "Core/Profiling/CCombatCollisionProfilingCounters.h"
#include "Type/CWeaponTypes.h"
#include "Type/CCombatHitTypes.h"
#include "Type/CCombatSignalTypes.h"
#include "Type/CActionFeedbackTypes.h"
#include "Component/CWeaponTrailComponent.h"
#include "DataAsset/CWeaponTrailDataAsset.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NiagaraSystem.h"

ACWeaponActor::ACWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ActorRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	check(ActorRootComponent);
	SetRootComponent(ActorRootComponent);

	PivotComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PresentationPivot"));
	check(PivotComponent);
	PivotComponent->SetupAttachment(ActorRootComponent);

	ContentRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PresentationContentRoot"));
	check(ContentRootComponent);
	ContentRootComponent->SetupAttachment(PivotComponent);

	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	check(WeaponMeshComponent);
	WeaponMeshComponent->SetupAttachment(ContentRootComponent);
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

void ACWeaponActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	InitializePivotHierarchy();
}

void ACWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	InitializePivotHierarchy();
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

// Weapon State

void ACWeaponActor::ChangeWeaponType(EWeaponType InWeaponType)
{
	WeaponType = InWeaponType;
}

// Weapon Attachment

void ACWeaponActor::AttachToHandSocket()
{
	AttachToSocketSlot(EWeaponSocketSlot::Hand);
}

void ACWeaponActor::AttachToHolsterSocket()
{
	AttachToSocketSlot(EWeaponSocketSlot::Holster);
}

bool ACWeaponActor::AttachToSocketSlot(EWeaponSocketSlot InSlot)
{
	FName socketName;
	if (!ResolveSocketName(InSlot, socketName)) return false;

	return AttachToOwnerSocket(socketName);
}

bool ACWeaponActor::AttachToOwnerSocket(FName InSocketName)
{
	if (InSocketName.IsNone() || !IsValid(OwnerCharacter_Injected) || !IsValid(ActorRootComponent)) return false;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp) || !meshComp->DoesSocketExist(InSocketName)) return false;

	const bool bAttached = AttachToComponent(meshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, InSocketName);
	if (!bAttached) return false;

	ActorRootComponent->SetRelativeTransform(FTransform::Identity);
	return ActorRootComponent->GetAttachParent() == meshComp && ActorRootComponent->GetAttachSocketName() == InSocketName;
}

bool ACWeaponActor::ResolveSocketName(EWeaponSocketSlot InSlot, FName& OutSocketName) const
{
	OutSocketName = NAME_None;

	switch (InSlot)
	{
	case EWeaponSocketSlot::Hand:
		OutSocketName = SocketName_Hand;
		break;

	case EWeaponSocketSlot::Holster:
		OutSocketName = SocketName_Holster;
		break;

	default:
		return false;
	}

	return !OutSocketName.IsNone();
}

bool ACWeaponActor::GetCurrentOwnerSocketName(FName& OutSocketName) const
{
	OutSocketName = NAME_None;

	if (!IsValid(ActorRootComponent) || !IsValid(OwnerCharacter_Injected)) return false;

	const USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp) || ActorRootComponent->GetAttachParent() != meshComp) return false;

	OutSocketName = ActorRootComponent->GetAttachSocketName();
	return !OutSocketName.IsNone() && meshComp->DoesSocketExist(OutSocketName);
}

bool ACWeaponActor::GetOwnerSocketWorldTransform(FName InSocketName, FTransform& OutWorldTransform) const
{
	if (InSocketName.IsNone() || !IsValid(OwnerCharacter_Injected)) return false;

	const USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp) || !meshComp->DoesSocketExist(InSocketName)) return false;

	OutWorldTransform = meshComp->GetSocketTransform(InSocketName, RTS_World);
	return true;
}

bool ACWeaponActor::SetActorRootWorldTransform(const FTransform& InWorldTransform)
{
	if (!IsValid(ActorRootComponent)) return false;

	ActorRootComponent->SetWorldTransform(InWorldTransform);
	return true;
}

// Weapon Pivot

bool ACWeaponActor::ApplyPivotRotation(const FQuat& InRotation)
{
	if (!bHasValidPivot || !IsValid(PivotComponent)) return false;

	FQuat rotation = InRotation;
	rotation.Normalize();

	const FTransform rotationOffset(rotation);
	PivotComponent->SetRelativeTransform(rotationOffset * PivotRestTransform);
	return true;
}

void ACWeaponActor::ResetPivotRotation()
{
	if (!IsValid(PivotComponent)) return;

	PivotComponent->SetRelativeTransform(PivotRestTransform);
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

// Collision Overlap Delegate Events

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

	if (OnWeaponActorEndOverlap.IsBound())
		OnWeaponActorEndOverlap.Broadcast(OwnerCharacter_Injected, OtherActor);
}

// Hit Context - Provider Query

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

// Hit Context - Provider Mutation

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

// Feedback - Trail

bool ACWeaponActor::HandleTrailFeedback(const FActionFeedbackRequest& InActionFeedbackRequest)
{
	if (InActionFeedbackRequest.ActionFeedbackTiming != EActionFeedbackTiming::TriggerWindowBegin
		&& InActionFeedbackRequest.ActionFeedbackTiming != EActionFeedbackTiming::TriggerWindowEnd)
	{
		return false;
	}

	if (!IsValid(WeaponTrailDataAsset) || InActionFeedbackRequest.TriggerKey.IsNone()) return false;

	const FWeaponTrailDefinition* definition = WeaponTrailDataAsset->FindTrailDefinition(InActionFeedbackRequest.TriggerKey);
	if (!definition) return false;

	if (definition->ActionFeedbackMatchKey.CalculateMatchTier(InActionFeedbackRequest.ActionFeedbackMatchKey) == EActionFeedbackMatchTier::None)
	{
		return false;
	}

	const TArray<TObjectPtr<UWeaponTrailComponent>>* trailComponents = FindTrailComponents(InActionFeedbackRequest.TriggerKey);
	if (!trailComponents || trailComponents->IsEmpty()) return false;

	const bool bEnable = InActionFeedbackRequest.ActionFeedbackTiming == EActionFeedbackTiming::TriggerWindowBegin;
	for (UWeaponTrailComponent* trailComponent : *trailComponents)
	{
		SetTrailComponentActive(trailComponent, bEnable);
	}

	return true;
}

void ACWeaponActor::DeactivateAllTrails()
{
	for (const TPair<FName, TArray<TObjectPtr<UWeaponTrailComponent>>>& trailComponentPair : TrailComponents_Cached)
	{
		for (UWeaponTrailComponent* trailComponent : trailComponentPair.Value)
		{
			SetTrailComponentActive(trailComponent, false);
		}
	}
}

// Weapon Pivot Initialization

bool ACWeaponActor::InitializePivotHierarchy()
{
	ResetPivotHierarchy();

	if (!IsValid(PivotComponent)
		|| !IsValid(ContentRootComponent)
		|| !IsValid(WeaponMeshComponent)
		|| PivotSocketName.IsNone())
	{
		return false;
	}

	if (WeaponMeshComponent->GetAttachParent() != ContentRootComponent)
	{
		ensureMsgf(false,
			TEXT("[Weapon|PivotInvalid] Reason=WeaponMeshMustBeUnderContentRoot | Weapon=%s | Mesh=%s"),
			*GetNameSafe(this),
			*GetNameSafe(WeaponMeshComponent));
		return false;
	}

	const FTransform weaponMeshRelativeToContentRoot = WeaponMeshComponent->GetRelativeTransform();
	if (!WeaponMeshComponent->DoesSocketExist(PivotSocketName))
	{
		ensureMsgf(false,
			TEXT("[Weapon|PivotInvalid] Reason=SocketMissing | Weapon=%s | Mesh=%s | Socket=%s"),
			*GetNameSafe(this),
			*GetNameSafe(WeaponMeshComponent->GetSkeletalMeshAsset()),
			*PivotSocketName.ToString());
		return false;
	}

	const FTransform pivotSocketRelativeToWeaponMesh = WeaponMeshComponent->GetSocketTransform(PivotSocketName, RTS_Component);
	FTransform pivotSocketRelativeToContentRoot = pivotSocketRelativeToWeaponMesh * weaponMeshRelativeToContentRoot;

	if (pivotSocketRelativeToContentRoot.ContainsNaN())
	{
		ensureMsgf(false,
			TEXT("[Weapon|PivotInvalid] Reason=PivotTransformContainsNaN | Weapon=%s | Socket=%s"),
			*GetNameSafe(this),
			*PivotSocketName.ToString());
		return false;
	}

	pivotSocketRelativeToContentRoot.SetScale3D(FVector::OneVector);

	PivotRestTransform = pivotSocketRelativeToContentRoot;
	PivotComponent->SetRelativeTransform(PivotRestTransform);
	ContentRootComponent->SetRelativeTransform(PivotRestTransform.Inverse());
	bHasValidPivot = true;
	return true;
}

void ACWeaponActor::ResetPivotHierarchy()
{
	bHasValidPivot = false;
	PivotRestTransform = FTransform::Identity;

	if (IsValid(PivotComponent))
	{
		PivotComponent->SetRelativeTransform(FTransform::Identity);
	}

	if (IsValid(ContentRootComponent))
	{
		ContentRootComponent->SetRelativeTransform(FTransform::Identity);
	}
}

// Collision Lifecycle

void ACWeaponActor::InitializeCollisionComponents()
{
	if (!IsValid(ActorRootComponent)) return;

	ClearCollisionComponents();

	TArray<USceneComponent*> children;
	ActorRootComponent->GetChildrenComponents(true, children);

	for (USceneComponent* child : children)
	{
		UShapeComponent* shape = Cast<UShapeComponent>(child);
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

// Hit Context Helpers

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

// Feedback - Trail Lifecycle

void ACWeaponActor::InitializeTrailState()
{
	CreateTrailComponents();
	DeactivateAllTrails();
}

void ACWeaponActor::ClearTrailState()
{
	DeactivateAllTrails();
	DestroyTrailComponents();
}

// Feedback - Trail Runtime Helpers

void ACWeaponActor::CreateTrailComponents()
{
	DestroyTrailComponents();

	if (!IsValid(WeaponTrailDataAsset) || !IsValid(WeaponMeshComponent)) return;

	for (const FWeaponTrailDefinition& definition : WeaponTrailDataAsset->TrailDefinitions)
	{
		if (definition.TriggerKey.IsNone())
		{
			ensureMsgf(false,
				TEXT("[Weapon|Trail] TriggerKey is required. Weapon=%s | Data=%s"),
				*GetName(),
				*GetNameSafe(WeaponTrailDataAsset));
			continue;
		}

		if (TrailComponents_Cached.Contains(definition.TriggerKey))
		{
			ensureMsgf(false,
				TEXT("[Weapon|Trail] Duplicate TriggerKey. Weapon=%s | TriggerKey=%s"),
				*GetName(),
				*definition.TriggerKey.ToString());
			continue;
		}

		TArray<TObjectPtr<UWeaponTrailComponent>> trailComponents;

		for (UNiagaraSystem* system : definition.NiagaraSystems)
		{
			if (!IsValid(system))
			{
				ensureMsgf(false,
					TEXT("[Weapon|Trail] Niagara System is required. Weapon=%s | TriggerKey=%s"),
					*GetName(),
					*definition.TriggerKey.ToString());
				continue;
			}

			const FName componentName = MakeUniqueObjectName(
				this,
				UWeaponTrailComponent::StaticClass(),
				FName(*FString::Printf(TEXT("Trail_%s"), *definition.TriggerKey.ToString())));

			UWeaponTrailComponent* trailComponent = NewObject<UWeaponTrailComponent>(this, componentName);
			if (!IsValid(trailComponent)) continue;

			trailComponent->SetAsset(system);
			trailComponent->SetAutoActivate(false);
			trailComponent->SetupAttachment(WeaponMeshComponent);

			AddInstanceComponent(trailComponent);
			trailComponent->RegisterComponent();

			SetTrailComponentActive(trailComponent, false);
			trailComponents.Add(trailComponent);
		}

		if (!trailComponents.IsEmpty())
		{
			TrailComponents_Cached.Add(definition.TriggerKey, MoveTemp(trailComponents));
		}
	}
}

void ACWeaponActor::DestroyTrailComponents()
{
	for (const TPair<FName, TArray<TObjectPtr<UWeaponTrailComponent>>>& trailComponentPair : TrailComponents_Cached)
	{
		for (UWeaponTrailComponent* trailComponent : trailComponentPair.Value)
		{
			if (!IsValid(trailComponent)) continue;

			SetTrailComponentActive(trailComponent, false);
			trailComponent->DestroyComponent();
		}
	}

	TrailComponents_Cached.Empty();
}

const TArray<TObjectPtr<UWeaponTrailComponent>>* ACWeaponActor::FindTrailComponents(FName InTriggerKey) const
{
	return TrailComponents_Cached.Find(InTriggerKey);
}

void ACWeaponActor::SetTrailComponentActive(UWeaponTrailComponent* InTrailComponent, bool bEnable)
{
	if (!IsValid(InTrailComponent)) return;

	if (bEnable)
	{
		InTrailComponent->SetVisibility(true);
		InTrailComponent->Activate(true);
		return;
	}

	InTrailComponent->Deactivate();
	InTrailComponent->SetVisibility(false);
}
