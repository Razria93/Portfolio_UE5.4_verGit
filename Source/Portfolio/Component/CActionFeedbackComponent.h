#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CWeaponStructure.h"
#include "CActionFeedbackComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCActionFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCActionFeedbackComponent();

private:
	UPROPERTY(EditAnywhere, Category = "ActionFeedback|Data")
	TArray<FTrailFeedbackData> TrailFeedbackDatas;

	UPROPERTY(EditAnywhere, Category = "ActionFeedback|Data")
	TArray<FActionVFXFeedbackData> VFXFeedbackDatas;

	UPROPERTY(EditAnywhere, Category = "ActionFeedback|Data")
	TArray<FActionSFXFeedbackData> SFXFeedbackDatas;

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Injected = nullptr;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	void PlayFeedback(const FActionFeedbackRequest& InActionFeedbackRequest);
	void ClearRuntimeFeedback();

private:
	bool CanPlayActionFeedback(const FActionFeedbackRequest& InActionFeedbackRequest) const;

private:
	EActionFeedbackMatchTier CalculateMatchTier(const FActionFeedbackKey& InDataKey, EActionFeedbackTiming InDataTiming, FName InDataTriggerKey, const FActionFeedbackRequest& InActionFeedbackRequest) const;

private:
	FActionVFXExecutionKey BuildActionVFXExecutionKey(const FActionVFXFeedbackData& InActionVFXFeedbackData) const;
	FActionSFXExecutionKey BuildActionSFXExecutionKey(const FActionSFXFeedbackData& InActionSFXFeedbackData) const;

private:
	void ExecuteTrailFeedbacks(const FActionFeedbackRequest& InActionFeedbackRequest);
	void ExecuteVFXFeedbacks(const FActionFeedbackRequest& InActionFeedbackRequest);
	void ExecuteSFXFeedbacks(const FActionFeedbackRequest& InActionFeedbackRequest);

private:
	void PlayActionVFX(const FActionVFXFeedbackData& InActionVFXFeedbackData);
	void PlayActionSFX(const FActionSFXFeedbackData& InActionSFXFeedbackData);
	void ToggleTrailActive(bool bActive);
};
