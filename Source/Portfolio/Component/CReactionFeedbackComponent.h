#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CReactionFeedbackTypes.h"
#include "CReactionFeedbackComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCReactionFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

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

	// Provisional presentation tuning; independent real-time lifetimes.
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	float PerfectSlowDuration = 0.22f;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	float PerfectSlowDilation = 0.15f;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	float PerfectBodyDuration = 0.7f;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	float PerfectScreenDuration = 0.4f;
	UPROPERTY(EditAnywhere, Category = "ReactionFeedback|PerfectDodge")
	class UMaterialInterface* PerfectBodyMaterial = nullptr;
	UPROPERTY(Transient)
	class UMaterialInterface* PreviousOverlay = nullptr;
	TWeakObjectPtr<class UCameraComponent> PerfectCamera;
	float PreviousVignette = 0.f;
	bool bPreviousVignetteOverride = false;
	bool bOwnsBody = false;
	double BodyExpiresAt = 0.;
	double ScreenExpiresAt = 0.;
	uint64 PerfectSlowSerial = 0;
	uint64 LastPerfectResultSerial = 0;

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

protected:
	void TickComponent(float InDeltaTime, ELevelTick InTickType, FActorComponentTickFunction* InTickFunction) override;
	void EndPlay(const EEndPlayReason::Type InReason) override;

private:
	void ClearPerfectDodgePresentation();
	void RestorePerfectBody();
	void RestorePerfectScreen();

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
