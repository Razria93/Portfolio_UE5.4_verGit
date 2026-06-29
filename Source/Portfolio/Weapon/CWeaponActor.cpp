#include "Weapon/CWeaponActor.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

#include "Component/CCombatSignalSourceComponent.h"

#include "Type/CWeaponStructure.h"

ACWeaponActor::ACWeaponActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>("RootScene");
	check(RootSceneComponent);

	SetRootComponent(RootSceneComponent);

	TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Trail"));
	check(TrailComponent);

	TrailComponent->SetupAttachment(RootSceneComponent);
	TrailComponent->bAutoActivate = false;
}

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

void ACWeaponActor::ApplyInitialWeaponState(EWeaponType InWeaponType)
{
	ChangeWeaponType(InWeaponType);
	AttachToHolsterSocket();
}

void ACWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	ConfigureCollisionComponents();
	ConfigureTrailInitialState();
}

void ACWeaponActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CollisionDisabled();
	ToggleTrailActive(false);
	ClearCollisionComponents();

	OwnerCharacter_Injected = nullptr;
	CombatSignalSourceComp_Injected = nullptr;

	Super::EndPlay(EndPlayReason);
}

void ACWeaponActor::ConfigureCollisionComponents()
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

void ACWeaponActor::ConfigureTrailInitialState()
{
	if (!IsValid(TrailComponent)) return;
	if (!bDisableTrailOnBeginPlay) return;

	ToggleTrailActive(false);
}

const FOverlapContext& ACWeaponActor::GetLastOverlapContext() const
{
	return LastOverlapContext_Cached;
}

const FWeaponContext& ACWeaponActor::GetLastWeaponContext() const
{
	return LastWeaponContext_Cached;
}

const FActionContext& ACWeaponActor::GetLastActionContext() const
{
	return LastActionContext_Cached;
}

void ACWeaponActor::SetLastOverlapContext(const FOverlapContext& InOverlapContext)
{
	LastOverlapContext_Cached = InOverlapContext;
}

void ACWeaponActor::SetLastWeaponContext(const FWeaponContext& InWeaponContext)
{
	LastWeaponContext_Cached = InWeaponContext;
}

void ACWeaponActor::SetLastActionContext(const FActionContext& InActionContext)
{
	LastActionContext_Cached = InActionContext;
}

void ACWeaponActor::ChangeWeaponType(EWeaponType InWeaponType)
{
	WeaponType = InWeaponType;
}

void ACWeaponActor::ToggleTrailActive(bool bEnable)
{
	if (!IsValid(TrailComponent)) return;

	// PrintTrailInfo(bEnable);

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

void ACWeaponActor::AttachToHandSocket()
{
	AttachToOwnerSocket(SocketName_Hand);
}

void ACWeaponActor::AttachToHolsterSocket()
{
	AttachToOwnerSocket(SocketName_Holster);
}

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

	// Early-Return
	if (collisionsToEnable.IsEmpty()) return;

	if (!bHitWindowOpened)
	{
		++CurrentHitWindowId;
		bHitWindowOpened = true;

		if (IsValid(CombatSignalSourceComp_Injected))
		{
			CombatSignalSourceComp_Injected->NotifyHitWindowOpened(this, CurrentHitWindowId);
		}
	}

	for (UShapeComponent* collision : collisionsToEnable)
	{
		collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (OnWeaponActorCollisionEnabled.IsBound())
		OnWeaponActorCollisionEnabled.Broadcast();
}

void ACWeaponActor::CollisionDisabled()
{
	if (!bHitWindowOpened) return;

	for (UShapeComponent* collision : Collisions_Cached)
	{
		collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	bHitWindowOpened = false;

	if (IsValid(CombatSignalSourceComp_Injected) && CurrentHitWindowId != INDEX_NONE)
	{
		CombatSignalSourceComp_Injected->NotifyHitWindowClosed(this, CurrentHitWindowId);
	}

	// Legacy delegate
	if (OnWeaponActorCollisionDisabled.IsBound())
		OnWeaponActorCollisionDisabled.Broadcast();
}

void ACWeaponActor::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UShapeComponent* overlapComp = Cast<UShapeComponent>(OverlappedComponent);
	if (!IsValid(overlapComp)) return;

	if (!IsValid(OwnerCharacter_Injected) || !IsValid(OtherActor)) return;
	if (OwnerCharacter_Injected == OtherActor) return;

	if (!IsValid(CombatSignalSourceComp_Injected)) return;

	FOverlapContext overlapContext = BuildOverlapContext(OwnerCharacter_Injected, this, OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	FHitContext hitContext = BuildHitContext(overlapContext);

	// PrintBeginOverlapContextInfo(hitContext);

	// Legacy delegate
	if (OnWeaponActorBeginOverlap.IsBound())
		OnWeaponActorBeginOverlap.Broadcast(OwnerCharacter_Injected, this, overlapComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	CombatSignalSourceComp_Injected->RequestCombatSignalSource(hitContext);
	LastOverlapContext_Cached = overlapContext;
}

void ACWeaponActor::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UShapeComponent* overlapComp = Cast<UShapeComponent>(OverlappedComponent);
	if (!IsValid(overlapComp)) return;

	if (!IsValid(OwnerCharacter_Injected) || !IsValid(OtherActor)) return;
	if (OwnerCharacter_Injected == OtherActor) return;

	if (!IsValid(CombatSignalSourceComp_Injected)) return;

	// Legacy delegate
	if (OnWeaponActorEndOverlap.IsBound())
		OnWeaponActorEndOverlap.Broadcast(OwnerCharacter_Injected, OtherActor);
}

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

FDamageImpactInfo ACWeaponActor::BuildDamageImpactInfo(const FOverlapContext& InOverlapContext) const
{
	FDamageImpactInfo damageImpactInfo;

	if (!IsValid(InOverlapContext.OverlappedComponent) || !IsValid(InOverlapContext.OtherComponent))
	{
		// Invalid
		return damageImpactInfo;
	}

	if (InOverlapContext.bFromSweep)
	{
		damageImpactInfo.bHasHitResult = true;
		damageImpactInfo.Source = EDamageImpactInfoSource::SweepResult;
		damageImpactInfo.HitResult = InOverlapContext.SweepResult;

		return damageImpactInfo;
	}

	const FVector queryLocation = InOverlapContext.OverlappedComponent->GetComponentLocation();

	FVector closestPoint = FVector::ZeroVector;
	const float distance = InOverlapContext.OtherComponent->GetClosestPointOnCollision(queryLocation, closestPoint);

	if (distance < 0.f)
	{
		// Invalid
		return damageImpactInfo;
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

	damageImpactInfo.bHasHitResult = true;
	damageImpactInfo.Source = EDamageImpactInfoSource::ClosestPoint;
	damageImpactInfo.HitResult = hitResult;

	return damageImpactInfo;
}

FHitContext ACWeaponActor::BuildHitContext(const FOverlapContext& InOverlapContext) const
{
	FHitContext hitContext;

	hitContext.OverlapContext = InOverlapContext;
	hitContext.WeaponContext = LastWeaponContext_Cached;
	hitContext.ActionContext = LastActionContext_Cached;
	hitContext.DamageImpactInfo = BuildDamageImpactInfo(InOverlapContext);

	return hitContext;
}

void ACWeaponActor::AttachToOwnerSocket(FName InSocketName)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return;

	AttachToComponent(meshComp, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), InSocketName);
}

void ACWeaponActor::PrintBeginOverlapContextInfo(const FHitContext& InHitContext)
{
	FLog::Log(TEXT("========= Begin Overlap ========="));
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	PrintHitContextInfo(InHitContext.WeaponContext, InHitContext.ActionContext);
	FLog::Log(TEXT("================================="));
}

void ACWeaponActor::PrintEndOverlapContextInfo(const FHitContext& InHitContext)
{
	FLog::Log(TEXT("========== End Overlap =========="));
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	PrintHitContextInfo(InHitContext.WeaponContext, InHitContext.ActionContext);
	FLog::Log(TEXT("================================="));
}

void ACWeaponActor::PrintOverlapContextInfo(const FOverlapContext& InOverlapContext)
{
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(InOverlapContext.OwnerActor)));

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("DamageCauser"), *GetNameSafe(InOverlapContext.DamageCauser)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OverlappedComponent"), *GetNameSafe(InOverlapContext.OverlappedComponent)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OverlapShape"), *GetNameSafe(InOverlapContext.OverlapShape))); // cast result (can be NULL)
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OtherActor"), *GetNameSafe(InOverlapContext.OtherActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OtherComponent"), *GetNameSafe(InOverlapContext.OtherComponent)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("OtherBodyIndex"), InOverlapContext.OtherBodyIndex));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bFromSweep"), InOverlapContext.bFromSweep ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("HitWindowId"), InOverlapContext.HitWindowId));

	if (InOverlapContext.bFromSweep)
	{
		const FHitResult& hitResult = InOverlapContext.SweepResult;

		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.BlockingHit"), hitResult.bBlockingHit ? TEXT("true") : TEXT("false")));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.Actor"), *GetNameSafe(hitResult.GetActor())));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.Component"), *GetNameSafe(hitResult.GetComponent())));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.BoneName"), *hitResult.BoneName.ToString()));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.ImpactPoint"), *hitResult.ImpactPoint.ToString()));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.ImpactNormal"), *hitResult.ImpactNormal.ToString()));
	}
}

void ACWeaponActor::PrintHitContextInfo(const FWeaponContext& InWeaponContext, const FActionContext& InActionContext)
{
	FLog::Log(TEXT("---------- Hit Context ----------"));
	FLog::Log(TEXT("[WeaponContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(InWeaponContext.WeaponType)));

	FLog::Log(TEXT("[ActionContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InActionContext.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Index"), (InActionContext.ActionIndex == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(InActionContext.ActionIndex)));
	FLog::Log(TEXT("---------------------------------"));
}

void ACWeaponActor::PrintTrailInfo(bool bEnable) const
{
	if (!IsValid(TrailComponent)) return;

	UNiagaraSystem* trailAsset = TrailComponent->GetAsset();

	const FString trailCompName = TrailComponent->GetName();
	FString trailAssetName = IsValid(trailAsset) ? trailAsset->GetName() : TEXT("None");

	FLog::Log(TEXT("======= Weapon Trail Info ======="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("State"), bEnable ? TEXT("Active") : TEXT("Inactive")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TrailComponent"), *trailCompName));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TrailAsset"), *trailAssetName));
	FLog::Log(TEXT("================================="));
}
