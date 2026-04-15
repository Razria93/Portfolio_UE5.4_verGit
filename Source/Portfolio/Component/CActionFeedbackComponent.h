#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	FActionFeedbackData ActionStartFeedback;

	UPROPERTY(EditAnywhere, Category = "ActionFeedback|Data")
	FActionFeedbackData TrailWindowBeginFeedback;

	UPROPERTY(EditAnywhere, Category = "ActionFeedback|Data")
	FActionFeedbackData TrailWindowEndFeedback;

	UPROPERTY(EditAnywhere, Category = "ActionFeedback|Data")
	FActionFeedbackData ActionEndFeedback;

private:
	UPROPERTY(Transient)
	class AActor* OwnerActor_Cached = nullptr;

	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached = nullptr;

protected:
	virtual void BeginPlay() override;

public:
	void PlayActionFeedback(const FApplyDamageSpecKey& InApplyDamageSpecKey, EActionFeedbackPhase InActionFeedbackPhase);

private:
	void ExecuteActionFeedback(const FActionFeedbackData& InActionFeedbackData);
	void PlayActionVFX(class UNiagaraSystem* InActionVFX);
	void PlayActionSFX(class USoundBase* InActionSFX);

private:
	void SetActionTrailActive(bool bActive);

private:
	bool CanPlayActionFeedback(EActionFeedbackPhase InActionFeedbackPhase) const;
	bool ResolveActionFeedbackData(EActionFeedbackPhase InActionFeedbackPhase, FActionFeedbackData& OutActionFeedbackData) const;

private:
	void PrintActionFeedbackRequestInfo(const FApplyDamageSpecKey& InApplyDamageSpecKey, EActionFeedbackPhase InActionFeedbackPhase) const;
	void PrintActionFeedbackDataInfo(const FActionFeedbackData& InActionFeedbackData) const;
	void PrintActionTrailInfo(bool bActive, const class ACAttachment* InAttachment) const;
};
