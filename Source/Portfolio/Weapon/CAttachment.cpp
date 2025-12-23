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
		UShapeComponent* shape = Cast<UShapeComponent>(child); // UShapeComponent base for shape collision components (Sphere / Box / Capsule)
		if (!IsValid(shape)) continue;

		shape->OnComponentBeginOverlap.AddDynamic(this, &ACAttachment::OnComponentBeginOverlap);
		shape->OnComponentEndOverlap.AddDynamic(this, &ACAttachment::OnComponentEndOverlap);

		Collisions_Cached.Add(shape);

		OffCollision();
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

void ACAttachment::OnEquipmentBeginEquip()
{
	AttachToOwnerSocket(SocketName_Hand);
}

void ACAttachment::OnEquipmentBeginUnequip()
{
	AttachToOwnerSocket(SocketName_Holster);
}

void ACAttachment::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == OwnerCharacter_Cached) return;

	Print_BeginOverlapEventInfo(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void ACAttachment::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == OwnerCharacter_Cached) return;

	Print_EndOverlapEventInfo(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void ACAttachment::OnCollision(FName InName)
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
}

void ACAttachment::OffCollision()
{
	for (UShapeComponent* collision : Collisions_Cached)
		collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACAttachment::AttachToOwnerSocket(FName InSocketName)
{
	AttachToComponent(OwnerCharacter_Cached->GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), InSocketName);
}

void ACAttachment::Print_BeginOverlapEventInfo(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FLog::Log(TEXT("========== Begin Overlap =========="));
	FLog::Log(FString::Printf(TEXT("OverlappedComponent : %s"), OverlappedComponent ? *OverlappedComponent->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("OtherActor          : %s"), OtherActor ? *OtherActor->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("OtherComp           : %s"), OtherComp ? *OtherComp->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("OtherBodyIndex      : %d"), OtherBodyIndex));
	FLog::Log(FString::Printf(TEXT("bFromSweep          : %s"), bFromSweep ? TEXT("true") : TEXT("false")));
	FLog::Log(TEXT("==================================="));
}

void ACAttachment::Print_EndOverlapEventInfo(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	FLog::Log(TEXT("=========== End Overlap ==========="));
	FLog::Log(FString::Printf(TEXT("OverlappedComponent : %s"), OverlappedComponent ? *OverlappedComponent->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("OtherActor          : %s"), OtherActor ? *OtherActor->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("OtherComp           : %s"), OtherComp ? *OtherComp->GetName() : TEXT("NULL")));
	FLog::Log(FString::Printf(TEXT("OtherBodyIndex      : %d"), OtherBodyIndex));
	FLog::Log(TEXT("==================================="));
}
