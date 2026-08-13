#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCharacterFeedbackTypes.h"
#include "CCharacterFeedbackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathPresentationRequested, EDeathPresentationReason, InReason);
DECLARE_MULTICAST_DELEGATE_OneParam(FDeathPresentationEvent, EDeathPresentationEventType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCCharacterFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCharacterFeedbackComponent();

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	EDeathPresentationRuntimeState DeathPresentationState = EDeathPresentationRuntimeState::Inactive;

public:
	// Event
	UPROPERTY(BlueprintAssignable, Category = "CharacterFeedback|Death")
	FDeathPresentationRequested OnDeathPresentationRequested;

	// Native Event
	FDeathPresentationEvent OnDeathPresentationEvent;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Death Presentation Command
	bool RequestDeathPresentation(EDeathPresentationReason InReason);

	UFUNCTION(BlueprintCallable, Category = "CharacterFeedback|Death")
	void NotifyDeathPresentationStarted();

	UFUNCTION(BlueprintCallable, Category = "CharacterFeedback|Death")
	void NotifyDeathPresentationUnavailable();

	UFUNCTION(BlueprintCallable, Category = "CharacterFeedback|Death")
	void NotifyDeathPresentationFinished();

	// Runtime Cleanup
	void ClearRuntimeFeedback();

public:
	// Query
	FORCEINLINE EDeathPresentationRuntimeState GetDeathPresentationState() const { return DeathPresentationState; }
	FORCEINLINE bool IsDeathPresentationRequested() const { return DeathPresentationState == EDeathPresentationRuntimeState::Requested; }
	FORCEINLINE bool IsDeathPresentationActive() const { return DeathPresentationState == EDeathPresentationRuntimeState::Active; }
};
