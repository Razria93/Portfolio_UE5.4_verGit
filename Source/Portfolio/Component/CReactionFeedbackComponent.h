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
	UPROPERTY(EditAnywhere, Category = "FeedBack|HitStop")
	EFeedbackAudience HitStopAudience = EFeedbackAudience::Both;

	UPROPERTY(EditAnywhere, Category = "FeedBack|HitStop")
	float HitStopDuration = 0.04f;

	UPROPERTY(EditAnywhere, Category = "FeedBack|HitStop")
	float HitStopDilation = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Feedback|VFX")
	class UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "Feedback|SFX")
	class USoundBase* HitSFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "Feedback|CameraShake")
	EFeedbackAudience CameraShakeAudience = EFeedbackAudience::Both;

	UPROPERTY(EditAnywhere, Category = "Feedback|CameraShake")
	TSubclassOf<class UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere, Category = "Feedback|CameraShake")
	float CameraShakeBaseScale = 1.f;

private:
	UPROPERTY(EditAnywhere)
	bool bEnableCameraShake = true;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class AActor* OwnerActor_Cached = nullptr;

	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

protected:
	void BeginPlay() override;

public:
	void PlayDamageFeedback(const FTakeDamagePacket& InTakeDamagePacket);

private:
	void PlayHitStop(const FTakeDamagePacket& InTakeDamagePacket);
	void PlayHitVFX(const FTakeDamagePacket& InTakeDamagePacket);
	void PlayHitSFX(const FTakeDamagePacket& InTakeDamagePacket);
	void PlayCameraShake(const FTakeDamagePacket& InTakeDamagePacket);

private:
	bool CanPlayDamageFeedback(const FTakeDamagePacket& InTakeDamagePacket) const;
	bool CanPlayHitStop(const FTakeDamagePacket& InTakeDamagePacket) const;
	bool CanPlayCameraShake(const FTakeDamagePacket& InTakeDamagePacket) const;

private:
	FHitStopRequest BuildHitStopRequest(const FTakeDamagePacket& InTakeDamagePacket) const;
	FCameraShakeRequest BuildCameraShakeRequest(const FTakeDamagePacket& InTakeDamagePacket) const;

private:
	void PrintHitStopRequestInfo(const FHitStopRequest& InHitStopRequest) const;
	void PrintHitVFXRequestInfo(class UNiagaraSystem* InHitVFX, const FVector& InLocation, const FRotator& InRotation) const;
	void PrintHitSFXRequestInfo(USoundBase * InHitSFX, const FVector& InLocation) const;
	void PrintCameraShakeRequestInfo(const FCameraShakeRequest& InCameraShakeRequest) const;
};
