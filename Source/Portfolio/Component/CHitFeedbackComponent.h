#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCombatSignalTargetTypes.h"
#include "Type/CCombatFeedbackTypes.h"
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
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	void PlayHitFeedback(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);

private:
	void PlayHitStop(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);
	void PlayHitVFX(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);
	void PlayHitSFX(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);
	void PlayCameraShake(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);

private:
	bool CanPlayHitFeedback(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	bool CanPlayHitStop(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	bool CanPlayCameraShake(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;

private:
	FVector ResolveHitFeedbackLocation(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	FRotator ResolveHitFeedbackRotation(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;

private:
	FHitStopRequest BuildHitStopRequest(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	FCameraShakeRequest BuildCameraShakeRequest(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
};
