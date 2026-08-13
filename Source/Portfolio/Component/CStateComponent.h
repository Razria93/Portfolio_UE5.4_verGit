#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CStateTypes.h"
#include "CStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FExecutionStateChanged, class ACharacter*, InOwnerCharacter, EExecutionState, InPrevExecutionState, EExecutionState, InNewExecutionState);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCStateComponent();

private:
	UPROPERTY(Transient)
	EExecutionState CurrentExecutionState = EExecutionState::Idle;

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

public:
	FExecutionStateChanged OnExecutionStateChanged;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Query
	FORCEINLINE bool CheckCurExecutionState(EExecutionState InNewExecutionState) const { return CurrentExecutionState == InNewExecutionState; }
	FORCEINLINE EExecutionState GetCurrentExecutionState() const { return CurrentExecutionState; }

public:
	// Mutation
	void SetIdleState();
	void SetActionState();
	void SetReactionState();

private:
	void ChangeExecutionState(EExecutionState InNewExecutionState);
};
