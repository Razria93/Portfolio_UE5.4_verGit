#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CStateStructure.h"
#include "Type/CHealthStructure.h"
#include "CStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStateTypeChanged, class ACharacter*, InOwnerCharacter, EStateType, InPrevStateType, EStateType, InNewStateType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCStateComponent();

private:
	/* === State === */
	EStateType CurrentStateType;

private:
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;

public:
	/* === [Out] Custom Delgate Events === */
	FStateTypeChanged OnStateTypeChanged;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void OnDeadStateChanged(EDeadState InPrevDeadState, EDeadState InNewDeadState);

public:
	/* === Getter === */
	FORCEINLINE EStateType GetCurStateType() const { return CurrentStateType; }

public:
	/* === Setter === */
	void SetIdleState();
	void SetEquipState();
	void SetUnequipState();
	void SetActionState();
	void SetReactionState();
	void SetDeadState();

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurStateType(EStateType InNewStateType) { return CurrentStateType == InNewStateType; }

private:
	void ChangeStateType(EStateType InNewStateType);

private:
	void PrintStateChangedInfo(EStateType InPrevStateType, EStateType InNewStateType) const;
	
};
