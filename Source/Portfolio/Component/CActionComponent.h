#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CActionOrchestrationStructure.h"
#include "Type/CWeaponStructure.h"
#include "CActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FActionTypeChanged, class ACharacter*, InOwnerCharacter, EActionType, InPrevActionType, EActionType, InNewActionType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FActionEventSignature, class ACharacter*, InOwnerCharacter, EActionType, InActionType, int32, InActionIndex, EActionEventType, InActionEventType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCActionComponent();


	// === ActionData ======================================= //
private:
	UPROPERTY(EditAnywhere, Category = "Action|Data")
	TArray<FActionData> ActionDatas;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	TMap<FActionDataKey, FActionData> ActionDataMap;

	UPROPERTY(Transient)
	TMap<class UClass*, class UCAction*> ActionExecutorMap;

private:
	/* === Active Action Context === */
	UPROPERTY(Transient)
	EActionType ActiveActionType = EActionType::Max;

	UPROPERTY(Transient)
	int32 ActiveActionIndex = INDEX_NONE;

	UPROPERTY(Transient)
	FActionData ActiveActionData = FActionData();

	UPROPERTY(Transient)
	class UCAction* ActiveActionExecutor = nullptr;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Cached = nullptr;

public:
	/* === Delegate === */
	FActionTypeChanged OnActionTypeChanged;
	FActionEventSignature OnActionEvent;

protected:
	void BeginPlay() override;

public:
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FORCEINLINE bool IsActiveActionType(EActionType InType) const { return ActiveActionType == InType; }

public:
	bool CanCommitChain(const UCAction* InAction, const FActionData& InData) const;

public:
	bool IsActive() const;

public:
	EActionType GetActiveActionType() const;
	int32 GetActiveActionIndex() const;
	bool GetActiveActionData(FActionData& OutData) const;
	class UCAction* GetActiveActionExecutor() const;

public:
	bool ResolveActionData(const FActionDataKey& InDataKey, FActionData& OutData);
	class UCAction* ResolveActionExecutor(const FActionData& InData);

public:
	bool ApplyActionDecision(const FActionExecutionResult& InResult);
	bool RequestStopActiveAction(const FExecutionInterventionDirective& InDirective);

public:
	bool HandleApplyActionConsumed(const UCAction* InAction, const FActionData& InData);
	void HandleApplyActionFinished(const class UCAction* InAction, EActionFinishReason InFinishReason);

public:
	void HandleActionNotifyCommand(EActionNotifyCommand InNotifyCommand);

public:
	void HandleActionInterventionWindowBegin(FName InWindowKey);
	void HandleActionInterventionWindowEnd(FName InWindowKey);

public:
	void HandleActionFeedback(FName InTriggerKey);
	void HandleActionFeedbackWindowBegin(FName InTriggerKey);
	void HandleActionFeedbackWindowEnd(FName InTriggerKey);

public:
	void BroadcastActionEvent(EActionType InType, int32 InIndex, EActionEventType InEventType);

private:
	// Temporary data build API (Move to DataAsset).
	void BuildActionDataMap(bool bRebuildAll);
	void BuildActionExecutorMap(bool bRebuildAll);

private:
	UCAction* AddActionExecutor(const TSubclassOf<class UCAction> InSubClass);
	UCAction* FindActionExecutor(const UClass* InClass);

private:
	bool ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InDirective);

private:
	bool StartAction(const FActionExecutionContext& InContext);
	bool ReserveAction(const FActionExecutionContext& InContext);
	bool StopActiveAction(const FExecutionInterventionDirective& InDirective);
	bool EndActiveAction(EActionFinishReason InFinishReason);

private:
	void SetActiveActionContext(const FActionExecutionContext& InContext);
	void ClearActiveActionContext();

private:
	void EnterActionState(const FActionData& InData);
	void ExitActionState(const FActionData& InData);

private:
	EActionStopReason ConvertExecutionStopReasonToActionStopReason(EExecutionStopReason InStopReason) const;
	EActionFinishReason ConvertExecutionStopReasonToActionFinishReason(EExecutionStopReason InStopReason) const;
};
