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

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	void PlayFeedback(const FReactionFeedbackRequest& InReactionFeedbackRequest);
	void ClearRuntimeFeedback();

private:
	bool CanPlayReactionFeedback(const FReactionFeedbackRequest& InReactionFeedbackRequest) const;

private:
	bool TryCalculateMatchScore(const FReactionFeedbackKey& InDataKey, EReactionFeedbackTiming InDataTiming, FName InDataTriggerKey, const FReactionFeedbackRequest& InReactionFeedbackRequest, int32& OutScore) const;

private:
	FReactionVFXExecutionKey BuildReactionVFXExecutionKey(const FReactionVFXFeedbackData& InReactionVFXFeedbackData) const;
	FReactionSFXExecutionKey BuildReactionSFXExecutionKey(const FReactionSFXFeedbackData& InReactionSFXFeedbackData) const;

private:
	void ExecuteVFXFeedbacks(const FReactionFeedbackRequest& InReactionFeedbackRequest);
	void ExecuteSFXFeedbacks(const FReactionFeedbackRequest& InReactionFeedbackRequest);

private:
	void PlayReactionVFX(const FReactionVFXFeedbackData& InReactionVFXFeedbackData);
	void PlayReactionSFX(const FReactionSFXFeedbackData& InReactionSFXFeedbackData);
};
