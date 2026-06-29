#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CActionOrchestrationStructure.h"
#include "Type/CCharacterComponentReferenceStructure.h"
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
	/* === Injected Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCObservableOverlayComponent* ObservableOverlayComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCCombatSignalSourceComponent* CombatSignalSourceComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionOrchestratorComponent* ActionOrchestratorComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionFeedbackComponent* ActionFeedbackComp_Injected = nullptr;

public:
	/* === Delegate === */
	FActionTypeChanged OnActionTypeChanged;
	FActionEventSignature OnActionEvent;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

protected:
	// Lifecycle
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Query
	FORCEINLINE bool IsActiveActionType(EActionType InType) const { return ActiveActionType == InType; }
	bool IsActive() const;

	EActionType GetActiveActionType() const;
	int32 GetActiveActionIndex() const;
	bool GetActiveActionData(FActionData& OutData) const;
	class UCAction* GetActiveActionExecutor() const;

public:
	// Data Resolve
	bool ResolveActionData(const FActionDataKey& InDataKey, FActionData& OutData);
	class UCAction* ResolveActionExecutor(const FActionData& InData);

	bool CanCommitChain(const UCAction* InAction, const FActionData& InData) const;

public:
	// Execution Entry
	bool ApplyActionDecision(const FActionExecutionResult& InResult);
	bool RequestInterruptActiveAction(const FExecutionInterventionDirective& InDirective);

public:
	// Execution Result Hooks
	bool HandleApplyActionConsumed(const UCAction* InAction, const FActionData& InData);
	void HandleApplyActionFinished(const class UCAction* InAction, EActionFinishReason InFinishReason);

public:
	// Notify Routing
	void HandleActionNotifyCommand(EActionNotifyCommand InNotifyCommand);

	void HandleActionAllowInterventionWindowBegin(FName InWindowKey);
	void HandleActionAllowInterventionWindowEnd(FName InWindowKey);

	void HandleActionFeedback(FName InTriggerKey);
	void HandleActionFeedbackWindowBegin(FName InTriggerKey);
	void HandleActionFeedbackWindowEnd(FName InTriggerKey);

	bool HandleActionCombatSignalCue(FName InCueTag);

public:
	// Cross-System Dispatch
	bool ApplyOverlayEvent(const FObservableOverlayEventContext& InContext);

	FActionRequestResult ConsumeDeferredAction(EDeferredActionConsumeKey InConsumeKey);
	void ClearDeferredActions(EDeferredActionConsumeKey InConsumeKey);

public:
	// Event Broadcast
	void BroadcastActionEvent(EActionType InType, int32 InIndex, EActionEventType InEventType);

private:
	// Data Build (temporary: move to DataAsset)
	void BuildActionDataMap(bool bRebuildAll);
	void BuildActionExecutorMap(bool bRebuildAll);

	FCharacterComponentReferences BuildActionExecutorReferences();

	UCAction* AddActionExecutor(const TSubclassOf<class UCAction> InSubClass);
	UCAction* FindActionExecutor(const UClass* InClass);

private:
	// Decision Apply
	bool ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InDirective);
	bool ApplyOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings);

private:
	// Execution Operations
	bool StartAction(const FActionExecutionContext& InContext);
	bool ReserveAction(const FActionExecutionContext& InContext);
	bool InterruptActiveAction(const FExecutionInterventionDirective& InDirective);
	bool EndActiveAction(EActionFinishReason InFinishReason);

private:
	// Active Context
	void SetActiveActionContext(const FActionExecutionContext& InContext);
	void ClearActiveActionContext();

private:
	// State Transition
	void EnterActionState(const FActionData& InData);
	void ExitActionState(const FActionData& InData);

private:
	// Conversion
	EActionFinishReason ConvertExecutionStopReasonToActionFinishReason(EExecutionStopReason InStopReason) const;
};
