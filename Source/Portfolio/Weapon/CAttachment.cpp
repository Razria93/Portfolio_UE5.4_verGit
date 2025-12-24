#include "Weapon/CAttachment.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

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

		Collisions_Cached.Add(shape);

		CollisionDisabled();
	}
}

void ACAttachment::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACAttachment::InitializeAttachment()
{
	AttachToOwnerSocket(SocketName_Holster);
}

void ACAttachment::AttachToOwnerSocket(FName InSocketName)
{
	AttachToComponent(OwnerCharacter_Cached->GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), InSocketName);
}

void ACAttachment::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Attack
	ACharacter* attacker = OwnerCharacter_Cached;
	AActor* damageCauser = this;
	UShapeComponent* attackCollision = Cast<UShapeComponent>(OverlappedComponent);
	
	// Hit (keep Actor/Component to support non-character objects)
	AActor* targetActor = OtherActor;
	UPrimitiveComponent* hitComponent = OtherComp;

	if (!IsValid(attacker)	||
		!IsValid(damageCauser) ||
		!IsValid(attackCollision) ||
		!IsValid(targetActor) ||
		!IsValid(hitComponent)) return;

	if (attacker == targetActor) return;

	if (OnAttachmentBeginOverlap.IsBound())
		OnAttachmentBeginOverlap.Broadcast(attacker, damageCauser, attackCollision, targetActor, hitComponent);

	Print_BeginOverlapEventInfo(attacker, damageCauser, attackCollision, targetActor, hitComponent, OtherBodyIndex, bFromSweep);
}

void ACAttachment::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == OwnerCharacter_Cached) return;

	ACharacter* attacker = OwnerCharacter_Cached;
	AActor* targetActor = OtherActor;

	if (!IsValid(attacker) ||
		!IsValid(targetActor)) return;

	if (attacker == targetActor) return;

	if (OnAttachmentEndOverlap.IsBound())
		OnAttachmentEndOverlap.Broadcast(attacker, targetActor);

	Print_EndOverlapEventInfo(attacker, targetActor);
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
	if (OnAttachmentCollisionEnabled.IsBound())
		OnAttachmentCollisionEnabled.Broadcast();

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
}

void ACAttachment::CollisionDisabled()
{
	if (OnAttachmentCollisionDisabled.IsBound())
		OnAttachmentCollisionDisabled.Broadcast();

	for (UShapeComponent* collision : Collisions_Cached)
		collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACAttachment::Print_BeginOverlapEventInfo(ACharacter* attacker, AActor* damageCauser, UShapeComponent* attackCollision, AActor* targetActor, UPrimitiveComponent* hitComponent, int32 OtherBodyIndex, bool bFromSweep)
{
	FLog::Log(TEXT("========== Begin Overlap =========="));
	FLog::Log(FString::Printf(TEXT("Attacker        : %s"), attacker ? *attacker->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("DamageCauser    : %s"), damageCauser ? *damageCauser->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("AttackCollision : %s"), attackCollision ? *attackCollision->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("TargetActor     : %s"), targetActor ? *targetActor->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("HitComponent    : %s"), hitComponent ? *hitComponent->GetName() : TEXT("NULL")));
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