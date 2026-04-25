#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FActionTypeChanged, class ACharacter*, InOwnerCharacter, EActionType, InPrevActionType, EActionType, InNewActionType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCActionComponent();

	// === ActionData ======================================= //
private:
	UPROPERTY(EditAnywhere, Category = "Action")
	TArray<FActionDefinition> ActionDefinitions;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	TMap<EActionType, class UCAction*> ActionContainer;

private:
	/* === State === */
	UPROPERTY(Transient)
	EActionType CurrentActionType = EActionType::Max;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

public:
	FActionTypeChanged OnActionTypeChanged;

protected:
	void BeginPlay() override;

public:
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurrentActionType(EActionType InNewActionType) const { return CurrentActionType == InNewActionType; }

public:
	/* === Getter === */
	FORCEINLINE EActionType GetCurrentActionType() const { return CurrentActionType; }

public:
	class UCAction* GetCurrentAction() const;

public:
	FActionExecutionResult ExecuteAction(EActionType IncomingActionType);

public:
	void CompleteCurrentAction();
	void AbortCurrentAction(EActionAbortReason InActionAbortReason);

private:
	bool StartAction(class UCAction* InAction, EActionType InActionType);
	bool ApplyActionChain(class UCAction* InAction, const FActionExecutionQuery& InActionExecuteQuery);

private:
	FActionExecutionQuery BuildActionExecutionQuery(EActionType InIncomingActionType, class UCAction* InIncomingAction) const;
	FActionExecutionResult BuildActionExecutionResult(EActionExecutionDecision InActionExecutionDecision, EActionType InActionType) const;

private:
	void EnterActionState(EActionType InActionType);
	void ExitActionState();

private:
	void ChangeActionType(EActionType InNewActionType);

private:
	bool CreateAction(ACharacter* InOwnerCharacter, const FActionDefinition& InActionDefinition);
};
