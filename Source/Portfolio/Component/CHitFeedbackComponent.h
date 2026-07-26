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
	UPROPERTY(EditAnywhere, Category = "Feedback|HitStop")
	FHitStopFeedbackTuning HitStopTuning;

	UPROPERTY(EditAnywhere, Category = "Feedback|VFX")
	class UNiagaraSystem* HitVFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "Feedback|SFX")
	class USoundBase* HitSFX = nullptr;

	UPROPERTY(EditAnywhere, Category = "Feedback|CameraShake")
	FHitCameraShakeFeedbackTuning CameraShakeTuning;

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

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
