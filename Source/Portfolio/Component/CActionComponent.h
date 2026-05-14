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
	class UCReactionComponent* ReactionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached = nullptr;

public:
	/* === Delegate === */
	FActionTypeChanged OnActionTypeChanged;
	FActionEventSignature OnActionEvent;

protected:
	void BeginPlay() override;

public:
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	FORCEINLINE bool CheckActiveActionType(EActionType InNewActionType) const { return ActiveActionType == InNewActionType; }

public:
	bool IsActive() const;

public:
	EActionType GetActiveActionType() const;
	int32 GetActiveActionIndex() const;
	bool GetActiveActionData(FActionData& OutActionData) const;
	class UCAction* GetActiveActionExecutor() const;

public:
	bool ResolveActionData(const FActionDataKey& InActionDataKey, FActionData& OutActionData);
	class UCAction* ResolveActionExecutor(const FActionData& InActionData);

public:
	bool ApplyActionDecision(const FActionOrchestrationLevelResult& InActionOrchestrationResult);
	bool RequestStopActiveAction(EActionStopReason InActionStopReason);

public:
	bool HandleActionChained(const UCAction* InAction, const FActionData& InActionData);
	void HandleActionFinished(const class UCAction* InAction, EActionFinishReason InActionFinishReason);

public:
	void BroadcastActionEvent(EActionType InActionType, int32 InActionIndex, EActionEventType InActionEventType);

public:
	void HandleActionNotifyCommand(EActionNotifyCommand InNotifyCommand);

	void HandleActionFeedback(FName InTriggerKey);
	void HandleActionFeedbackWindowBegin(FName InTriggerKey);
	void HandleActionFeedbackWindowEnd(FName InTriggerKey);

private:
	// Temporary data build API (Move to DataAsset).
	void BuildActionDataMap(bool bRebuildAll);
	void BuildActionExecutorMap(bool bRebuildAll);

private:
	UCAction* AddActionExecutor(const TSubclassOf<class UCAction> InSubClass);
	UCAction* FindActionExecutor(const UClass* InClass);

private:
	bool TryStartAction(const FActionResolvedContext& InActionResolvedContext);
	bool TryChainAction(const FActionResolvedContext& InActionResolvedContext);
	bool TryEnqueueAction(const FActionResolvedContext& InActionResolvedContext);
	bool TryReplaceAction(const FActionResolvedContext& InActionResolvedContext, EActionStopReason InStopReason);
	bool TryStopActiveAction(EActionStopReason InStopReason);

private:
	bool StartActiveActionInternal(const FActionResolvedContext& InActionResolvedContext);
	bool ChainActiveActionInternal(const FActionResolvedContext& InActionResolvedContext);
	bool StopActiveActionInternal(EActionStopReason InStopReason);
	bool EndActiveActionInternal(EActionFinishReason InFinishReason);

private:
	void SetActiveActionContext(const FActionResolvedContext& InActionResolvedContext);
	void ClearActiveActionContext();

private:
	void EnterActionState(const FActionData& InActionData);
	void ExitActionState(const FActionData& InActionData);

private:
	bool ApplyReactionStopDirective(const FActionOrchestrationLevelResult& InActionOrchestrationResult);

private:
	EActionFinishReason ConvertStopReasonToFinishReason(EActionStopReason InStopReason) const;

private:
	void PrintActionLocalLevelQuery(const FActionLocalLevelQuery& InQuery) const;
};
