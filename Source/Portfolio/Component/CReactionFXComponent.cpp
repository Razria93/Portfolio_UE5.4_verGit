#include "Component/CReactionFXComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"

#include "Type/CWeaponStructure.h"

UCReactionFXComponent::UCReactionFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCReactionFXComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor_Cached = GetOwner();
	check(OwnerActor_Cached);

	OwnerCharacter_Cached = Cast<ACharacter>(OwnerActor_Cached);
	check(OwnerCharacter_Cached);
}

void UCReactionFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UCReactionFXComponent::CanPlayDamageFeedback(const FTakeDamageResult& InTakeDamageResult) const
{
	if (!IsValid(OwnerActor_Cached)) return false;
	if (!InTakeDamageResult.bAccepted) return false;
	if (InTakeDamageResult.CommittedDamage <= KINDA_SMALL_NUMBER) return false;

	return true;
}

void UCReactionFXComponent::PlayDamageFeedback(const FTakeDamageResult& InTakeDamageResult)
{
	if (!CanPlayDamageFeedback(InTakeDamageResult)) return;

	PlayHitStop(InTakeDamageResult);
	PlayHitVFX(InTakeDamageResult);
	PlayHitSound(InTakeDamageResult);
	PlayCameraShake(InTakeDamageResult);
}

void UCReactionFXComponent::PlayHitStop(const FTakeDamageResult& InTakeDamageResult)
{
	if (!IsValid(OwnerActor_Cached)) return;
	if (HitStopTime <= 0.f)
	{
		FLog::Log(TEXT("[UCReactionFXComponent] Invalid HitStopTime."));
		return;
	}
}

void UCReactionFXComponent::PlayHitVFX(const FTakeDamageResult& InTakeDamageResult)
{
	if (!IsValid(GetWorld())) return;
	if (!IsValid(OwnerActor_Cached)) return;

	if (!IsValid(HitVFX))
	{
		FLog::Log(TEXT("[UCReactionFXComponent] Invalid HitVFX."));
		return;
	}

	const FVector location = OwnerActor_Cached->GetActorLocation();
	const FRotator rotation = OwnerActor_Cached->GetActorRotation();

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitVFX, location, rotation);
}

void UCReactionFXComponent::PlayHitSound(const FTakeDamageResult& InTakeDamageResult)
{
	if (!IsValid(OwnerActor_Cached)) return;

	if (!IsValid(HitSound)) 
	{
		FLog::Log(TEXT("[UCReactionFXComponent] Invalid HitSound."));
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, HitSound, OwnerActor_Cached->GetActorLocation());
}

void UCReactionFXComponent::PlayCameraShake(const FTakeDamageResult& InTakeDamageResult)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	if (!IsValid(CameraShakeClass))
	{
		FLog::Log(TEXT("[UCReactionFXComponent] Invalid CameraShakeClass."));
		return;
	}

	APlayerController* playerController = Cast<APlayerController>(OwnerCharacter_Cached->GetController());
	
	if (!IsValid(playerController)) return;
	if (!playerController->IsLocalController()) return;

	playerController->ClientStartCameraShake(CameraShakeClass);
}
