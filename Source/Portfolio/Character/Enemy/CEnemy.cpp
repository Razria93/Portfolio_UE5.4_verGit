#include "Character/Enemy/CEnemy.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Component/CStateComponent.h"

#include "Type/CWeaponStructure.h"

ACEnemy::ACEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Init CapsuleComp
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);

	// Init SkeletalMeshComp
	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // FRotator: (Pitch, Yaw, Roll)

	// Init CharacterMovementComp
	UCharacterMovementComponent* characterMovementComp = GetCharacterMovement();
	check(characterMovementComp);
	characterMovementComp->bOrientRotationToMovement = true;
	characterMovementComp->MaxWalkSpeed = 600.0f;

	// Init StateComp
	StateComponent = CreateDefaultSubobject<UCStateComponent>(TEXT("State"));
	check(StateComponent);
}

void ACEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void ACEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ACEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageEvent.IsOfType(FDefaultDamageEvent::ClassID))
	{
		const FDefaultDamageEvent& damageEvent = static_cast<const FDefaultDamageEvent&>(DamageEvent);
		return HandleDefaultDamage(damageEvent, EventInstigator, DamageCauser);
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

float ACEnemy::HandleDefaultDamage(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	if (!IsValid(InDamageCauser)) return 0.f;

	const FDamageSpecKey& damageSpecKey = InDefaultDamageEvent.DamageSpecKey;
	const FDamageResult& damageResult = InDefaultDamageEvent.DamageResult;

	// Calculate Damage (Minimal)
	const float takedDamage = FMath::Max(0.f, damageResult.FinalDamage);

	// Print_HandleDamageResult
	FLog::Log(TEXT("[@ TAKE DAMAGE]"));
	FLog::Log(FString::Printf(TEXT("Victim: %s | Key: [AttachmentType: %s / EquipmentType: %s / ActionType: %s / ActionIndex: %d] | TakedDamage: %.3f | Causer: %s"),
		*GetNameSafe(this),
		*UEnum::GetValueAsString(damageSpecKey.AttachmentType),
		*UEnum::GetValueAsString(damageSpecKey.EquipmentType),
		*UEnum::GetValueAsString(damageSpecKey.ActionType),
		damageSpecKey.ActionIndex,
		takedDamage,
		*GetNameSafe(InDamageCauser)
	));

	// TODO: HP reduction / state transitions / hit reactions (montage, VFX/SFX) / knockback / hit stop (time dilation) / etc.

	return takedDamage;
}

