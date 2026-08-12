#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCharacterFeedbackTypes.h"
#include "CCharacterFeedbackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathPresentationRequested, EDeathPresentationReason, InReason);
DECLARE_MULTICAST_DELEGATE(FDeathPresentationFinished);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCCharacterFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCharacterFeedbackComponent();

private:
	UPROPERTY(EditDefaultsOnly, Category = "CharacterFeedback|Death", meta = (ClampMin = 0.0))
	float DeathPresentationExpectedDuration = 1.f;

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	bool bDeathPresentationActive = false;

public:
	UPROPERTY(BlueprintAssignable, Category = "CharacterFeedback|Death")
	FDeathPresentationRequested OnDeathPresentationRequested;
	FDeathPresentationFinished OnDeathPresentationFinished;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Death Presentation
	FDeathPresentationStartResult StartDeathPresentation(EDeathPresentationReason InReason);

	UFUNCTION(BlueprintCallable, Category = "CharacterFeedback|Death")
	void NotifyDeathPresentationFinished();

	void ClearRuntimeFeedback();

public:
	// Query
	FORCEINLINE bool IsDeathPresentationActive() const { return bDeathPresentationActive; }
};
