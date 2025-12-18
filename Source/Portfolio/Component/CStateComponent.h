#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CStateStructure.h"
#include "CStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStateTypeChanged, class ACharacter*, InOwnerCharacter, EStateType, InPrevStateType, EStateType, InNewStateType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCStateComponent();

private:
	EStateType CurrentStateType;

private:
	// Cached
	class ACharacter* OwnerCharacter_Cached;

public:
	// Delegate
	FStateTypeChanged OnStateTypeChanged;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void SetIdleMode();
	void SetEquipMode();
	void SetUnequipMode();
	void SetActionMode();

public:
	FORCEINLINE bool CheckCurType(EStateType InNewStateType) { return CurrentStateType == InNewStateType; }

private:
	void ChangeStateType(EStateType InNewStateType);
	void ChangeStateMode(EStateType InNewStateType);
};
