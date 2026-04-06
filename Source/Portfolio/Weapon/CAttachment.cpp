#include "Weapon/CAttachment.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

#include "Component/CApplyDamageComponent.h"

#include "Type/CWeaponStructure.h"

ACAttachment::ACAttachment()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	check(Root);

	SetRootComponent(Root);
}

void ACAttachment::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());

	if (!IsValid(OwnerCharacter_Cached)) return;
	if (!IsValid(Root)) return;

	ApplyDamageComp_Cached = Cast<UCApplyDamageComponent>(OwnerCharacter_Cached->GetComponentByClass(UCApplyDamageComponent::StaticClass()));	// TODO: Refactor Interface
	check(ApplyDamageComp_Cached);

	TArray<USceneComponent*> children;
	Root->GetChildrenComponents(true, children);

	for (USceneComponent* child : children)
	{
		UShapeComponent* shape = Cast<UShapeComponent>(child); //  UShapeComponent base for shape collision components (Sphere / Box / Capsule)
		if (!IsValid(shape)) continue;

		shape->OnComponentBeginOverlap.AddDynamic(this, &ACAttachment::OnComponentBeginOverlap);
		shape->OnComponentEndOverlap.AddDynamic(this, &ACAttachment::OnComponentEndOverlap);
		shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// Collision_Disabled

		Collisions_Cached.Add(shape);
	}
}

void ACAttachment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACAttachment::InitializeAttachment(EAttachmentType InAttachmentType)
{
	SetAttachmentType(InAttachmentType);

	AttachToOwnerSocket(SocketName_Holster);
}

const FOverlapContext& ACAttachment::GetLastOverlapContext() const
{
	return LastOverlapContext;
}

const FAttachmentContext& ACAttachment::GetLastAttachmentContext() const
{
	return LastAttachmentContext;
}

const FEquipmentContext& ACAttachment::GetLastEquipmentContext() const
{
	return LastEquipmentContext;
}

const FActionContext& ACAttachment::GetLastActionContext() const
{
	return LastActionContext;
}

void ACAttachment::SetLastOverlapContext(const FOverlapContext& InOverlapContext)
{
	LastOverlapContext = InOverlapContext;
}

void ACAttachment::SetLastAttachmentContext(const FAttachmentContext& InAttachmentContext)
{
	LastAttachmentContext = InAttachmentContext;
}

void ACAttachment::SetLastEquipmentContext(const FEquipmentContext& InEquipmentContext)
{
	LastEquipmentContext = InEquipmentContext;
}

void ACAttachment::SetLastActionContext(const FActionContext& InActionContext)
{
	LastActionContext = InActionContext;
}

EAttachmentType ACAttachment::GetAttachmentType() const
{
	return AttachmentType;
}

void ACAttachment::SetAttachmentType(EAttachmentType InAttachmentType)
{
	AttachmentType = InAttachmentType;
}

void ACAttachment::AttachToOwnerSocket(FName InSocketName)
{
	AttachToComponent(OwnerCharacter_Cached->GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), InSocketName);
}

void ACAttachment::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UShapeComponent* overlapComp = Cast<UShapeComponent>(OverlappedComponent);
	if (!IsValid(overlapComp)) return;

	if (!IsValid(OwnerCharacter_Cached) || !IsValid(OtherActor)) return;
	if (OwnerCharacter_Cached == OtherActor) return;

	if (!IsValid(ApplyDamageComp_Cached)) return;

	FOverlapContext overlapContext = BuildOverlapContext(OwnerCharacter_Cached, this, OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	FHitContext hitContext = BuildHitContext(overlapContext);

	PrintBeginOverlapContextInfo(hitContext);

	// Legacy delegate
	if (OnAttachmentBeginOverlap.IsBound())
		OnAttachmentBeginOverlap.Broadcast(OwnerCharacter_Cached, this, overlapComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ApplyDamageComp_Cached->RequestApplyDamage(hitContext);
	LastOverlapContext = overlapContext;
}

void ACAttachment::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UShapeComponent* overlapComp = Cast<UShapeComponent>(OverlappedComponent);
	if (!IsValid(overlapComp)) return;

	if (!IsValid(OwnerCharacter_Cached) || !IsValid(OtherActor)) return;
	if (OwnerCharacter_Cached == OtherActor) return;

	if (!IsValid(ApplyDamageComp_Cached)) return;

	// Legacy delegate
	if (OnAttachmentEndOverlap.IsBound())
		OnAttachmentEndOverlap.Broadcast(OwnerCharacter_Cached, OtherActor);
}

void ACAttachment::OnEquipmentBeginEquip()
{
	AttachToOwnerSocket(SocketName_Hand);
}

void ACAttachment::OnEquipmentBeginUnequip()
{
	AttachToOwnerSocket(SocketName_Holster);
}

void ACAttachment::CollisionEnabled(FName InName)
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

		if (IsValid(ApplyDamageComp_Cached))
		{
			ApplyDamageComp_Cached->NotifyHitWindowOpened(this, CurrentHitWindowId);
		}
	}

	for (UShapeComponent* collision : collisionsToEnable)
	{
		collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	if (OnAttachmentCollisionEnabled.IsBound())
		OnAttachmentCollisionEnabled.Broadcast();
}

void ACAttachment::CollisionDisabled()
{
	if (!bHitWindowOpened) return;

	for (UShapeComponent* collision : Collisions_Cached)
	{
		collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	bHitWindowOpened = false;

	if (IsValid(ApplyDamageComp_Cached) && CurrentHitWindowId != INDEX_NONE)
	{
		ApplyDamageComp_Cached->NotifyHitWindowClosed(this, CurrentHitWindowId);
	}

	// Legacy delegate
	if (OnAttachmentCollisionDisabled.IsBound())
		OnAttachmentCollisionDisabled.Broadcast();
}

FOverlapContext ACAttachment::BuildOverlapContext(AActor* InOwnerActor, AActor* InDamageCauser, UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) const
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

FHitContext ACAttachment::BuildHitContext(const FOverlapContext& InOverlapContext) const
{
	FHitContext hitContext;

	hitContext.OverlapContext = InOverlapContext;
	hitContext.AttachmentContext = LastAttachmentContext;
	hitContext.EquipmentContext = LastEquipmentContext;
	hitContext.ActionContext = LastActionContext;

	return hitContext;
}

void ACAttachment::PrintBeginOverlapContextInfo(const FHitContext& InHitContext)
{
	FLog::Log(TEXT("========= Begin Overlap ========="));
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	PrintHitContextInfo(InHitContext.AttachmentContext, InHitContext.EquipmentContext, InHitContext.ActionContext);
	FLog::Log(TEXT("================================="));
}

void ACAttachment::PrintEndOverlapContextInfo(const FHitContext& InHitContext)
{
	FLog::Log(TEXT("========== End Overlap =========="));
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	PrintHitContextInfo(InHitContext.AttachmentContext, InHitContext.EquipmentContext, InHitContext.ActionContext);
	FLog::Log(TEXT("================================="));
}

void ACAttachment::PrintOverlapContextInfo(const FOverlapContext& InOverlapContext)
{
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(InOverlapContext.OwnerActor)));

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("DamageCauser"), *GetNameSafe(InOverlapContext.DamageCauser)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OverlappedComponent"), *GetNameSafe(InOverlapContext.OverlappedComponent)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OverlapShape"), *GetNameSafe(InOverlapContext.OverlapShape))); // cast result (can be NULL)
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OtherActor"), *GetNameSafe(InOverlapContext.OtherActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OtherComponent"), *GetNameSafe(InOverlapContext.OtherComponent)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("OtherBodyIndex"), InOverlapContext.OtherBodyIndex));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bFromSweep"), InOverlapContext.bFromSweep ? TEXT("true") : TEXT("false")));

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

void ACAttachment::PrintHitContextInfo(const FAttachmentContext& InAttachmentContext, const FEquipmentContext& InEquipmentContext, const FActionContext& InActionContext)
{
	FLog::Log(TEXT("---------- Hit Context ----------"));
	FLog::Log(TEXT("[AttachmentContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CurrentAttachmentType"), *UEnum::GetValueAsString(InAttachmentContext.CurrentAttachmentType)));

	FLog::Log(TEXT("[EquipmentContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CurrentEquipmentType"), *UEnum::GetValueAsString(InEquipmentContext.CurrentEquipmentType)));

	FLog::Log(TEXT("[ActionContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CurrentActionType"), *UEnum::GetValueAsString(InActionContext.CurrentActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Index"), (InActionContext.ActionIndex == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(InActionContext.ActionIndex)));
	FLog::Log(TEXT("---------------------------------"));
}
