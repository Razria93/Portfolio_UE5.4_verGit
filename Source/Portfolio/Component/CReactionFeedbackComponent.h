#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CReactionFeedbackTypes.h"
#include "Type/CCombatPresentationEnvelope.h"
#include "Type/CCombatOverlayPlayback.h"
#include "CReactionFeedbackComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCReactionFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class FCounterRushFOVTest;
	friend class FWeaponGlowOwnershipTest;
	friend class FBodyGlowOwnershipTest;

public:
	UCReactionFeedbackComponent();

private:
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|Data")
	TArray<FReactionVFXFeedbackData> VFXFeedbackDatas;

	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|Data")
	TArray<FReactionSFXFeedbackData> SFXFeedbackDatas;

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	// Serialized migration input only. Runtime timing is authored by Body Glow point notifies.
	UPROPERTY()
	FCombatPresentationEnvelope PerfectBodyEnvelope;
	// Native mesh plus explicitly selected modular parts (name or tag).
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	TArray<FName> PerfectBodyParts;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	FName PerfectBodyPartTag = TEXT("PerfectDodgeGlow");
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	FName GlowWeightParameter = TEXT("Alpha");
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	class UMaterialInterface* PerfectBodyMaterial = nullptr;
	UPROPERTY(Transient)
	FCombatOverlayPlayback BodyOverlay;
	UPROPERTY(Transient)
	FCombatOverlayPlayback WeaponOverlay;
	UPROPERTY(Transient)
	FCombatPresentationEnvelopeState BodyEnvelopeState;
	uint64 BodyOwnerGeneration = 0;
	int32 BodyOwnerMontageInstanceId = INDEX_NONE;
	uint64 LastBodyStartGeneration = 0;
	int32 LastBodyStartMontageInstanceId = INDEX_NONE;
	double BodySafetyDeadline = 0.;
	float BodyPeakWeight = 1.f;
	UPROPERTY(Transient)
	FCombatPresentationEnvelopeState WeaponEnvelopeState;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	class UMaterialInterface* CounterWeaponMaterial = nullptr;
	TWeakObjectPtr<class ACWeaponActor> GlowWeapon;
	uint64 WeaponOpportunitySerial = 0;
	bool bWeaponTransferredToAttack = false;
	uint64 WeaponOwnerGeneration = 0;
	uint64 PendingWeaponTransfer = 0;
	bool bWeaponOwnerEnded = false;
	double WeaponSafetyDeadline = 0.;
	float WeaponPeakWeight = 1.f;
	TWeakObjectPtr<class UCameraComponent> PerfectCamera;
	float PreviousVignette = 0.f;
	bool bPreviousVignetteOverride = false;
	float LastWrittenVignette = 0.f;
	float PeakVignette = .85f;
	UPROPERTY(Transient)
	FCombatPresentationEnvelopeState ScreenEnvelopeState;
	TWeakObjectPtr<AActor> PerfectAttacker;
	uint64 PerfectSlowSerial = 0;
	uint64 LastPerfectResultSerial = 0;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|CounterRush", meta = (ClampMin = "0", ClampMax = "20"))
	float RushFOVOffset = 5.f;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|CounterRush", meta = (ClampMin = "0.001", Units = "s"))
	float RushFOVBlendIn = .04f;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|CounterRush", meta = (ClampMin = "0.001", Units = "s"))
	float RushFOVBlendOut = .12f;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|CounterRush")
	class UNiagaraSystem* RushMovementVFX = nullptr;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|CounterRush")
	FVector RushVFXOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|CounterRush", meta = (ClampMin = "0.01"))
	float RushVFXScale = 1.f;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|CounterRush", meta = (ClampMin = "0", ClampMax = "1"))
	float RushVFXMovementThreshold = .05f;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|CounterRush", meta = (ClampMin = "0", ClampMax = "2", Units = "s"))
	float RushVFXTail = .2f;
	UPROPERTY(Transient)
	class UNiagaraComponent* RushVFXComponent = nullptr;
	bool bRushVFXEmitting = false;
	double RushReleasedAt = 0.;
	TWeakObjectPtr<class UCameraComponent> RushCamera;
	uint64 RushPresentationSerial = 0;
	float RushBaseFOV = 0.f;
	float RushLastFOV = 0.f;
	float RushMotionWeight = 0.f;
	float RushFOVWeight = 0.f;
	double RushLastRealTime = 0.;
	bool bRushPresentationReleased = false;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Entry
	void PlayFeedback(const FReactionFeedbackRequest& InReactionFeedbackRequest);
	void ClearRuntimeFeedback();
	void PlayPerfectDodgeSuccess(AActor* InAttacker, uint64 InResultSerial);
	void StartBodyGlow(uint64 InGeneration, int32 InMontageInstanceId, float InPeak, float InFadeIn, float InFadeOut, float InSafetySeconds);
	void EndBodyGlow(uint64 InGeneration, int32 InMontageInstanceId, float InFadeOut);
	void EndBodyGlowOwner(uint64 InGeneration);
	void TransferCounterWeaponToAttack(uint64 InOpportunitySerial);
	void ReleaseCounterWeapon(uint64 InOpportunitySerial);
	void StartWeaponGlow(bool bInAction, uint64 InGeneration, float InPeak, float InFadeIn, float InFadeOut, float InSafetySeconds);
	void EndWeaponGlow(bool bInAction, uint64 InGeneration, float InFadeOut);
	void EndWeaponGlowOwner(bool bInAction, uint64 InGeneration);
	void BeginWeaponGlowTransfer(uint64 InOpportunitySerial);
	void FinishWeaponGlowTransfer();
	void StartPerfectDodgeSlow(const FCombatPresentationEnvelope& InEnvelope, float InDilation, bool bInPlayer, bool bInAttacker);
	void StartPerfectDodgeVignette(const FCombatPresentationEnvelope& InEnvelope, float InPeakIntensity);
	void BeginRushPresentation(uint64 InSerial);
	void UpdateRushPresentation(uint64 InSerial, float InMovementRatio);
	void EndRushPresentation(uint64 InSerial);

protected:
	void TickComponent(float InDeltaTime, ELevelTick InTickType, FActorComponentTickFunction* InTickFunction) override;
	void EndPlay(const EEndPlayReason::Type InReason) override;

private:
	void ClearPerfectDodgePresentation();
	void RestorePerfectBody();
	void RestoreCounterWeapon();
	void RestorePerfectScreen();
	void RestoreRushPresentation();
	void TickRushPresentation(double InNow);

private:
	// Query
	bool CanPlayReactionFeedback(const FReactionFeedbackRequest& InReactionFeedbackRequest) const;

private:
	// Matching
	bool TryCalculateMatchScore(const FReactionFeedbackMatchKey& InDataKey, EReactionFeedbackTiming InDataTiming, FName InDataTriggerKey, const FReactionFeedbackRequest& InReactionFeedbackRequest, int32& OutScore) const;

private:
	// Runtime Key / Playback Key
	FReactionVFXPlaybackKey BuildReactionVFXPlaybackKey(const FReactionVFXFeedbackData& InReactionVFXFeedbackData) const;
	FReactionSFXPlaybackKey BuildReactionSFXPlaybackKey(const FReactionSFXFeedbackData& InReactionSFXFeedbackData) const;

private:
	// Execution
	void ExecuteVFXFeedbacks(const FReactionFeedbackRequest& InReactionFeedbackRequest);
	void ExecuteSFXFeedbacks(const FReactionFeedbackRequest& InReactionFeedbackRequest);

private:
	// Playback
	void PlayReactionVFX(const FReactionVFXFeedbackData& InReactionVFXFeedbackData);
	void PlayReactionSFX(const FReactionSFXFeedbackData& InReactionSFXFeedbackData);
};
