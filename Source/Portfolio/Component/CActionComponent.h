#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CActionTypes.h"
#include "Type/CActionDataTypes.h"
#include "Type/CActionOrchestrationTypes.h"
#include "Type/CObservableOverlayTypes.h"
#include "Type/CExecutionTypes.h"
#include "CActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FActionTypeChanged, class ACharacter*, InOwnerCharacter, EActionType, InPrevActionType, EActionType, InNewActionType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FActionEventSignature, class ACharacter*, InOwnerCharacter, EActionType, InActionType, int32, InActionIndex, uint32, InActionRequestSerial, EActionEventType, InActionEventType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Construction
	UCActionComponent();

private:
	// Data Configuration
	UPROPERTY(EditAnywhere, Category = "Action|Data")
	TArray<FActionData> ActionDatas;

private:
	// Runtime Map
	UPROPERTY(Transient)
	TMap<FActionDataKey, FActionData> ActionDataMap;

	UPROPERTY(Transient)
	TMap<class UClass*, class UCAction*> ActionExecutorMap;

private:
	// Active Runtime State
	UPROPERTY(Transient)
	EActionType ActiveActionType = EActionType::Max;

	UPROPERTY(Transient)
	int32 ActiveActionIndex = INDEX_NONE;

	UPROPERTY(Transient)
	uint32 ActiveActionRequestSerial = 0;

	UPROPERTY(Transient)
	FActionData ActiveActionData = FActionData();

	UPROPERTY(Transient)
	class UCAction* ActiveActionExecutor = nullptr;

	// Execution Generation
	uint64 ActiveActionGeneration = 0;
	uint64 NextActionGeneration = 0;

	// Start Facing Runtime
	uint64 PendingStartFacingGeneration = 0;
	TWeakObjectPtr<AActor> PendingStartFacingTarget;

	// Action Pose Scope
	UPROPERTY(Transient)
	uint32 ActiveWeaponActionPoseScopeHandle = 0;

	UPROPERTY(Transient)
	uint32 NextWeaponActionPoseScopeHandle = 1;

private:
	// Component References
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
	class UCCombatTargetComponent* CombatTargetComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCExecutionCollaborationComponent* ExecutionCollaborationComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCCombatSignalSourceComponent* CombatSignalSourceComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionOrchestratorComponent* ActionOrchestratorComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionFeedbackComponent* ActionFeedbackComp_Injected = nullptr;

public:
	// Event
	FActionTypeChanged OnActionTypeChanged;
	FActionEventSignature OnActionEvent;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Lifecycle
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Query
	FORCEINLINE bool IsActiveActionType(EActionType InType) const { return ActiveActionType == InType; }
	bool IsActive() const;

	EActionType GetActiveActionType() const;
	int32 GetActiveActionIndex() const;
	uint32 GetActiveActionRequestSerial() const;
	uint64 GetActiveActionGeneration() const { return ActiveActionGeneration; }
	uint32 GetActiveWeaponActionPoseScopeHandle() const;
	bool GetActiveActionData(FActionData& OutData) const;
	class UCAction* GetActiveActionExecutor() const;

	bool FindPreparedActionContext(const FActionDataKey& InKey, FActionExecutionContext& OutContext) const;
	bool CanCommitChain(const UCAction* InAction, const FActionData& InData) const;

public:
	// Data Resolve
	bool ResolveActionData(const FActionDataKey& InDataKey, FActionData& OutData);
	class UCAction* ResolveActionExecutor(const FActionData& InData);

public:
	// Execution Entry
	bool ApplyActionDecision(const FActionExecutionResult& InResult);
	bool RequestInterruptActiveAction(const FExecutionInterventionDirective& InDirective);
	bool CancelActiveActionForSystem();

public:
	// Execution Result Hooks
	void HandleApplyActionStarted(const UCAction* InAction, uint64 InActionGeneration);
	bool HandleApplyActionConsumed(const UCAction* InAction, const FActionData& InData, uint32 InActionRequestSerial);
	void HandleApplyActionFinished(const class UCAction* InAction, EActionFinishReason InFinishReason);

public:
	// Execution Collaboration Bridge
	bool TryCommitActiveExecution(const class UCAction* InAction, uint32 InActionRequestSerial);

public:
	// Notify Routing
	void HandleActionNotifyCommand(EActionNotifyCommand InNotifyCommand);

	void HandleActionAllowInterventionWindowBegin(FName InWindowKey);
	void HandleActionAllowInterventionWindowEnd(FName InWindowKey);

	void HandleActionFeedback(FName InTriggerKey);
	void HandleActionFeedbackWindowBegin(FName InTriggerKey);
	void HandleActionFeedbackWindowEnd(FName InTriggerKey);

	void HandleActionCollisionWindowBegin(FName InCollisionName);
	void HandleActionCollisionWindowEnd();

	bool HandleActionCombatSignalCue(FName InCueTag);

public:
	// Cross-System Dispatch
	bool ApplyOverlayEvent(const FObservableOverlayEventContext& InContext);

	FActionRequestResult ConsumeDeferredAction(EDeferredActionConsumeKey InConsumeKey);
	void ClearDeferredActions(EDeferredActionConsumeKey InConsumeKey);

public:
	// Event Broadcast
	void BroadcastActionEvent(EActionType InType, int32 InIndex, uint32 InActionRequestSerial, EActionEventType InEventType);

private:
	// Component Reference Validation
	bool ValidateRequiredComponentReferences() const;

private:
	// Runtime Lifecycle
	void InitializeActionRuntime();
	void UninitializeActionRuntime();

private:
	// Runtime Map
	void BuildActionRuntimeMaps();
	void ClearActionRuntimeMaps();

	// Active Runtime State
	void SetInitialActiveActionRuntimeState();
	void ResetActiveActionRuntimeState();

private:
	// Data Build
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
	// Weapon Action Pose Scope
	uint32 AllocateWeaponActionPoseScopeHandle();
	void ReleaseActiveWeaponActionPoseScope();

private:
	// Active Context
	void AdvanceActionGeneration();
	void SetActiveActionContext(const FActionExecutionContext& InContext);
	void ClearActiveActionContext();

private:
	// Start Facing
	void ApplyPendingStartFacing();
	void ClearPendingStartFacing();

private:
	// State Transition
	void EnterActionState(const FActionData& InData);
	void ExitActionState(const FActionData& InData);

private:
	// Conversion
	EActionFinishReason ConvertExecutionStopReasonToActionFinishReason(EExecutionStopReason InStopReason) const;
};
