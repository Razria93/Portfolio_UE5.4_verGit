#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "Type/CWorldSubSystemStructure.h"
#include "CHitFeedbackComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCHitFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCHitFeedbackComponent();

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
	void PlayHitFeedback(const FCombatSignalTargetPacket& InTakeDamagePacket);

private:
	void PlayHitStop(const FCombatSignalTargetPacket& InTakeDamagePacket);
	void PlayHitVFX(const FCombatSignalTargetPacket& InTakeDamagePacket);
	void PlayHitSFX(const FCombatSignalTargetPacket& InTakeDamagePacket);
	void PlayCameraShake(const FCombatSignalTargetPacket& InTakeDamagePacket);

private:
	bool CanPlayHitFeedback(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	bool CanPlayHitStop(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	bool CanPlayCameraShake(const FCombatSignalTargetPacket& InTakeDamagePacket) const;

private:
	FVector ResolveHitFeedbackLocation(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	FRotator ResolveHitFeedbackRotation(const FCombatSignalTargetPacket& InTakeDamagePacket) const;

private:
	FHitStopRequest BuildHitStopRequest(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
	FCameraShakeRequest BuildCameraShakeRequest(const FCombatSignalTargetPacket& InTakeDamagePacket) const;

private:
	void PrintHitStopRequestInfo(const FHitStopRequest& InHitStopRequest) const;
	void PrintHitVFXRequestInfo(class UNiagaraSystem* InHitVFX, const FVector& InLocation, const FRotator& InRotation) const;
	void PrintHitSFXRequestInfo(USoundBase * InHitSFX, const FVector& InLocation) const;
	void PrintCameraShakeRequestInfo(const FCameraShakeRequest& InCameraShakeRequest) const;
	void PrintHitInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const;
};
