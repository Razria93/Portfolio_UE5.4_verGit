#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceStructure.h"
#include "Type/CStateStructure.h"
#include "Type/CHealthStructure.h"
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
	void OnDeadStateChanged(EDeadState InPrevDeadState, EDeadState InNewDeadState);

public:
	// Check / Query
	FORCEINLINE bool CheckCurExecutionState(EExecutionState InNewExecutionState) const { return CurrentExecutionState == InNewExecutionState; }

public:
	// Query
	FORCEINLINE EExecutionState GetCurrentExecutionState() const { return CurrentExecutionState; }

public:
	// Mutation
	void SetIdleState();
	void SetActionState();
	void SetReactionState();
	void SetDeadState();

private:
	void ChangeExecutionState(EExecutionState InNewExecutionState);
};
