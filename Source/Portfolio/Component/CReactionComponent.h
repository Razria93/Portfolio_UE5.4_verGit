#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CWeaponStructure.h"
#include "Type/CActionOrchestrationStructure.h"
#include "Type/CReactionOrchestrationStructure.h"
#include "CReactionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReactionTypeChanged, class ACharacter*, InOwnerCharacter, EReactionType, InPrevReactionType, EReactionType, InNewReactionType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCReactionComponent();

private:
	UPROPERTY(EditAnywhere, Category = "Reaction|Data")
	TArray<FReactionData> ReactionDatas;

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
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCObservableOverlayComponent* ObservableOverlayComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionFeedbackComponent* ReactionFeedbackComp_Injected = nullptr;

public:
	FReactionTypeChanged OnReactionTypeChanged;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

protected:
	// Lifecycle
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
	bool RequestInterruptActiveReaction(const FExecutionInterventionDirective& InDirective);

public:
	// Execution Result Hooks
	void HandleApplyReactionFinished(const UCReaction* InReaction, EReactionFinishReason InFinishReason);

public:
	// Cross-System Dispatch
	void RequestConsumeDeferredAction(EDeferredActionConsumeKey InConsumeKey);

public:
	// Notify Routing
	void HandleReactionNotifyCommand(EReactionNotifyCommand InNotifyCommand);

	void HandleReactionAllowInterventionWindowBegin(FName InWindowKey);
	void HandleReactionAllowInterventionWindowEnd(FName InWindowKey);

	void HandleReactionFeedback(FName InTriggerKey);
	void HandleReactionFeedbackWindowBegin(FName InTriggerKey);
	void HandleReactionFeedbackWindowEnd(FName InTriggerKey);

private:
	// Runtime Lifecycle
	void InitializeReactionRuntime();
	void UninitializeReactionRuntime();

private:
	// Runtime Map
	void BuildReactionRuntimeMaps();
	void ClearReactionRuntimeMaps();

private:
	// Active Runtime State
	void SetInitialActiveReactionRuntimeState();
	void ResetActiveReactionRuntimeState();

private:
	// Data Build
	void BuildReactionDataMap(bool bRebuildAll);
	void BuildReactionExecutorMap(bool bRebuildAll);

	FCharacterComponentReferences BuildReactionExecutorReferences();
	UCReaction* AddReactionExecutor(const TSubclassOf<class UCReaction> InSubClass);
	UCReaction* FindReactionExecutor(const UClass* InClass);

private:
	// Data Resolve Helpers
	void BuildCandidateSpecKeys(const FDamageSpecKey& InSpecKey, TArray<FDamageSpecKey>& OutSpecKeys) const;

private:
	// Decision Apply
	bool ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InDirective);
	bool ApplyOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings);

private:
	// Execution Operations
	bool StartReaction(const FReactionExecutionContext& InContext);
	bool InterruptActiveReaction(const FExecutionInterventionDirective& InDirective);
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
	EReactionFinishReason ConvertExecutionStopReasonToReactionFinishReason(EExecutionStopReason InStopReason) const;
};
