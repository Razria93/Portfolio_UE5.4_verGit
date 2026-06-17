#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "Type/CReactionOrchestrationStructure.h"
#include "CReactionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReactionTypeChanged, class ACharacter*, InOwnerCharacter, EReactionType, InPrevReactionType, EReactionType, InNewReactionType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCReactionComponent();

	// === ReactionData ===================================== //
private:
	UPROPERTY(EditAnywhere, Category = "Reaction|Data")
	TArray<FReactionData> ReactionDatas;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	TMap<FReactionDataKey, FReactionData> ReactionDataMap;

	UPROPERTY(Transient)
	TMap<class UClass*, class UCReaction*> ReactionExecutorMap;

private:
	UPROPERTY(Transient)
	EReactionType ActiveReactionType = EReactionType::Max;

	UPROPERTY(Transient)
	FReactionData ActiveReactionData = FReactionData();

	UPROPERTY(Transient)
	class UCReaction* ActiveReactionExecutor = nullptr;

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
	class UCDefenseComponent* DefenseComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCObservableOverlayComponent* ObservableOverlayComp_Cached = nullptr;

public:
	/* === Delegate === */
	FReactionTypeChanged OnReactionTypeChanged;

protected:
	// Lifecycle
	void BeginPlay() override;

public:
	// Query
	FORCEINLINE bool IsActiveReactionType(EReactionType InType) const { return ActiveReactionType == InType; }
	bool IsActive() const;

	EReactionType GetActiveReactionType() const;
	bool GetActiveReactionData(FReactionData& OutData) const;
	UCReaction* GetActiveReactionExecutor() const;

public:
	// Data Resolve
	bool ResolveReactionData(const FReactionDataKey& InDataKey, FReactionData& OutData);
	UCReaction* ResolveReactionExecutor(const FReactionData& InData);

public:
	// Execution Entry
	bool ApplyReactionDecision(const FReactionExecutionResult& InResult);
	bool RequestStopActiveReaction(const FExecutionInterventionDirective& InDirective);

public:
	// Execution Result Hooks
	void HandleApplyReactionFinished(const UCReaction* InReaction, EReactionFinishReason InFinishReason);

public:
	// Notify Routing
	void HandleReactionNotifyCommand(EReactionNotifyCommand InNotifyCommand);

	void HandleReactionAllowInterventionWindowBegin(FName InWindowKey);
	void HandleReactionAllowInterventionWindowEnd(FName InWindowKey);

	void HandleReactionFeedback(FName InTriggerKey);
	void HandleReactionFeedbackWindowBegin(FName InTriggerKey);
	void HandleReactionFeedbackWindowEnd(FName InTriggerKey);

private:
	// Data Build (temporary: move to DataAsset)
	void BuildReactionDataMap(bool bRebuildAll);
	void BuildReactionExecutorMap(bool bRebuildAll);

	UCReaction* AddReactionExecutor(const TSubclassOf<class UCReaction> InSubClass);
	UCReaction* FindReactionExecutor(const UClass* InClass);

private:
	// Data Resolve Helpers
	void BuildCandidateSpecKeys(const FApplyDamageSpecKey& InSpecKey, TArray<FApplyDamageSpecKey>& OutSpecKeys) const;

private:
	// Decision Apply
	bool ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InDirective);
	bool ApplyObservableOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings);

private:
	// Execution Operations
	bool StartReaction(const FReactionExecutionContext& InContext);
	bool StopActiveReaction(const FExecutionInterventionDirective& InDirective);
	bool EndActiveReaction(EReactionFinishReason InFinishReason);

private:
	// Active Context
	void SetActiveReactionContext(const FReactionExecutionContext& InContext);
	void ClearActiveReactionContext();

private:
	// State Transition
	void EnterReactionState(const FReactionData& InData);
	void ExitReactionState(const FReactionData& InData);

private:
	// Conversion
	EReactionStopReason ConvertExecutionStopReasonToReactionStopReason(EExecutionStopReason InStopReason) const;
	EReactionFinishReason ConvertExecutionStopReasonToReactionFinishReason(EExecutionStopReason InStopReason) const;

private:
	// Debug
	void PrintReactionInfoSummary() const;
	void PrintReactionDataMap() const;
	void PrintComponentStateInfo() const;
	void PrintApplyDamageSpecKeyInfo(const FApplyDamageSpecKey& InSpecKey) const;
	void PrintReactionDataKeyInfo(const FReactionDataKey& InDataKey) const;
	void PrintReactionDataInfo(const FReactionData& InData) const;
	void PrintReactionExcutorInfo(const UCReaction* InReaction) const;
	void PrintReactionExecutorRuntimeInfo(const UCReaction* InReaction) const;
};
