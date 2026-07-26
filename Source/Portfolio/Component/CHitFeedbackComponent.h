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

	virtual void PostLoad() override;

private:
	UPROPERTY(EditAnywhere, Category = "Feedback|HitStop")
	FHitStopFeedbackTuning HitStopTuning;

	UPROPERTY(EditAnywhere, Category = "Feedback|VFX")
	class UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "Feedback|SFX")
	class USoundBase* HitSFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "Feedback|CameraShake")
	FHitCameraShakeFeedbackTuning CameraShakeTuning;

private:
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use HitStopTuning.Audience."))
	EFeedbackAudience HitStopAudience;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use HitStopTuning.Duration."))
	float HitStopDuration;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use HitStopTuning.Dilation."))
	float HitStopDilation;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use CameraShakeTuning.Audience."))
	EFeedbackAudience CameraShakeAudience;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use CameraShakeTuning.CameraShakeClass."))
	TSubclassOf<class UCameraShakeBase> CameraShakeClass;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use CameraShakeTuning.BaseScale."))
	float CameraShakeBaseScale;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use CameraShakeTuning.bEnabled."))
	bool bEnableCameraShake;

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

private:
	void MigrateDeprecatedFeedbackTuning();

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Entry
	void PlayHitFeedback(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);

private:
	// Playback
	void PlayHitStop(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);
	void PlayHitVFX(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);
	void PlayHitSFX(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);
	void PlayCameraShake(const FCombatSignalTargetPacket& InCombatSignalTargetPacket);

private:
	// Query
	bool CanPlayHitFeedback(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	bool CanPlayHitStop(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	bool CanPlayCameraShake(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;

private:
	// Resolve
	FVector ResolveHitFeedbackLocation(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	FRotator ResolveHitFeedbackRotation(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;

private:
	// Request
	FHitStopRequest BuildHitStopRequest(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
	FCameraShakeRequest BuildCameraShakeRequest(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const;
};
