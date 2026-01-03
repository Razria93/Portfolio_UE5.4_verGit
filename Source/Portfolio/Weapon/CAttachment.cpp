#include "Weapon/CAttachment.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

#include "Type/CWeaponStructure.h"

ACAttachment::ACAttachment()
{
	PrimaryActorTick.bCanEverTick = true;

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

FAttachmentContext ACAttachment::GetAttachmentContext() const
{
	return AttachmentContext;
}

FEquipmentContext ACAttachment::GetEquipmentContext() const
{
	return EquipmentContext;
}

FActionContext ACAttachment::GetActionContext() const
{
	return ActionContext;
}

void ACAttachment::SetAttachmentContext(FAttachmentContext InAttachmentContext)
{
	AttachmentContext = InAttachmentContext;
}

void ACAttachment::SetEquipmentContext(FEquipmentContext InEquipmentContext)
{
	EquipmentContext = InEquipmentContext;
}

void ACAttachment::SetActionContext(FActionContext InActionContext)
{
	ActionContext = InActionContext;
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

void ACAttachment::OnBeginPlayAction()
{
	FAttachmentContext attachmentContext;
	attachmentContext.CurrentAttachmentType = AttachmentType;

	PushAttachmentContext(attachmentContext);
}

void ACAttachment::OnEndPlayAction()
{
	FAttachmentContext attachmentContext = FAttachmentContext();

	PushAttachmentContext(attachmentContext);
}

void ACAttachment::OnNextPlayAction()
{
	FAttachmentContext attachmentContext;
	attachmentContext.CurrentAttachmentType = AttachmentType;

	PushAttachmentContext(attachmentContext);
}

void ACAttachment::PushAttachmentContext(const FAttachmentContext& InAttachmentContext)
{
	AttachmentContext = InAttachmentContext;
}

void ACAttachment::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UShapeComponent* overlapComp = Cast<UShapeComponent>(OverlappedComponent);
	if (!IsValid(overlapComp)) return;

	if (!IsValid(OwnerCharacter_Cached) || !IsValid(OtherActor)) return;
	if (OwnerCharacter_Cached == OtherActor) return;

	// Regacy
	if (OnAttachmentBeginOverlap.IsBound())
		OnAttachmentBeginOverlap.Broadcast(OwnerCharacter_Cached, this, overlapComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	Print_BeginOverlapEventInfo(OwnerCharacter_Cached, this, overlapComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep);
	Print_ActionContextInfo(ActionContext);
}

void ACAttachment::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UShapeComponent* overlapComp = Cast<UShapeComponent>(OverlappedComponent);
	if (!IsValid(overlapComp)) return;

	if (!IsValid(OwnerCharacter_Cached) || !IsValid(OtherActor)) return;
	if (OwnerCharacter_Cached == OtherActor) return;

	// Regacy
	if (OnAttachmentEndOverlap.IsBound())
		OnAttachmentEndOverlap.Broadcast(OwnerCharacter_Cached, OtherActor);

	Print_EndOverlapEventInfo(OwnerCharacter_Cached, OtherActor);
	Print_ActionContextInfo(ActionContext);
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
	if (!InName.IsNone())
	{
		for (UShapeComponent* collision : Collisions_Cached)
		{
			if (collision->GetFName() == InName)
			{
				collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				return;
			}
		}
	}
	else // InName == None: OnCollision_ALL
	{
		for (UShapeComponent* collision : Collisions_Cached)
			collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// Regacy
	if (OnAttachmentCollisionEnabled.IsBound())
		OnAttachmentCollisionEnabled.Broadcast();
}

void ACAttachment::CollisionDisabled()
{
	for (UShapeComponent* collision : Collisions_Cached)
		collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Regacy
	if (OnAttachmentCollisionDisabled.IsBound())
		OnAttachmentCollisionDisabled.Broadcast();
}

void ACAttachment::Print_BeginOverlapEventInfo(AActor* InAttackerActor, AActor* InDamageCauser, UShapeComponent* InAttackCollision, AActor* InTargetActor, UPrimitiveComponent* InHitComponent, int32 OtherBodyIndex, bool bFromSweep)
{
	FLog::Log(TEXT("========== Begin Overlap =========="));
	FLog::Log(FString::Printf(TEXT("Attacker        : %s"), InAttackerActor ? *InAttackerActor->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("DamageCauser    : %s"), InDamageCauser ? *InDamageCauser->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("AttackCollision : %s"), InAttackCollision ? *InAttackCollision->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("TargetActor     : %s"), InTargetActor ? *InTargetActor->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("HitComponent    : %s"), InHitComponent ? *InHitComponent->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("OtherBodyIndex	: %d"), OtherBodyIndex));
	FLog::Log(FString::Printf(TEXT("bFromSweep		: %s"), bFromSweep ? TEXT("true") : TEXT("false")));
	FLog::Log(TEXT("==================================="));
}

void ACAttachment::Print_EndOverlapEventInfo(ACharacter* attacker, AActor* targetActor)
{
	FLog::Log(TEXT("=========== End Overlap ==========="));
	FLog::Log(FString::Printf(TEXT("Attacker    : %s"), attacker ? *attacker->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("TargetActor : %s"), targetActor ? *targetActor->GetName() : TEXT("NULL")));
	FLog::Log(TEXT("==================================="));
}

void ACAttachment::Print_ActionContextInfo(FActionContext InActionContext)
{
	FLog::Log(TEXT("========== Action Context =========="));
	FLog::Log(FString::Printf(TEXT("CurrentActionType : %s"), *UEnum::GetValueAsString(InActionContext.CurrentActionType)));
	FLog::Log(FString::Printf(TEXT("Index             : %s"), (InActionContext.Index == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(InActionContext.Index)));
	FLog::Log(TEXT("===================================="));
}
