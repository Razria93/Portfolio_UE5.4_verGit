#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "Type/CWorldSubSystemStructure.h"
#include "CReactionFeedbackComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCReactionFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCReactionFeedbackComponent();

private:
	UPROPERTY(EditAnywhere)
	EHitStopType HitStopType = EHitStopType::Mutual;

	UPROPERTY(EditAnywhere)
	float HitStopDuration = 0.04f;

	UPROPERTY(EditAnywhere)
	float HitStopDilation = 0.05f;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere)
	class USoundBase* HitSound = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere)
	float CameraShakeBaseScale = 1.f;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class AActor* OwnerActor_Cached = nullptr;

	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	bool CanPlayDamageFeedback(const FTakeDamagePacket& InTakeDamagePacket) const;
	void PlayDamageFeedback(const FTakeDamagePacket& InTakeDamagePacket);

private:
	void PlayHitStop(const FTakeDamagePacket& InTakeDamagePacket);
	void PlayHitVFX(const FTakeDamagePacket& InTakeDamagePacket);
	void PlayHitSound(const FTakeDamagePacket& InTakeDamagePacket);
	void PlayCameraShake(const FTakeDamagePacket& InTakeDamagePacket);

private:
	void PrintHitStopRequestInfo(const FHitStopRequest& InHitStopRequest) const;
	void PrintHitVFXRequestInfo(class UNiagaraSystem* InHitVFX, const FVector& InLocation, const FRotator& InRotation) const;
	void PrintHitSoundRequestInfo(class USoundBase* InHitSound, const FVector& InLocation) const;
	void PrintCameraShakeRequestInfo(const FCameraShakeRequest& InCameraShakeRequest) const;
};
