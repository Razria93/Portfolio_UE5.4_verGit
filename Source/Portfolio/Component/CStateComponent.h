#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	/* === State === */
	UPROPERTY(Transient)
	EExecutionState CurrentExecutionState = EExecutionState::Idle;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

public:
	/* === [Out] Custom Delgate Events === */
	FExecutionStateChanged OnExecutionStateChanged;

protected:
	void BeginPlay() override;

public:
	void OnDeadStateChanged(EDeadState InPrevDeadState, EDeadState InNewDeadState);

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurExecutionState(EExecutionState InNewExecutionState) const { return CurrentExecutionState == InNewExecutionState; }

public:
	/* === Getter === */
	FORCEINLINE EExecutionState GetCurrentExecutionState() const { return CurrentExecutionState; }

public:
	/* === Setter === */
	void SetIdleState();
	void SetActionState();
	void SetReactionState();
	void SetDeadState();

private:
	void ChangeExecutionState(EExecutionState InNewExecutionState);

private:
	void PrintExecutionStateChangedInfo(EExecutionState InPrevExecutionState, EExecutionState InNewExecutionState) const;
	
};
