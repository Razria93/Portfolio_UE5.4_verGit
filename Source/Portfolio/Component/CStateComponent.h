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
	/* === Getter === */
	FORCEINLINE EStateType GetCurStateType() { return CurrentStateType; }

public:
	/* === Setter === */
	void SetIdleMode();
	void SetEquipMode();
	void SetUnequipMode();
	void SetActionMode();

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurStateType(EStateType InNewStateType) { return CurrentStateType == InNewStateType; }

private:
	void ChangeStateType(EStateType InNewStateType);
	void ChangeStateMode(EStateType InNewStateType);
};
