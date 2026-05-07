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

	UPROPERTY(EditAnywhere, Category = "Reaction|Excutor")
	TArray<TSubclassOf<class UCReaction>> ReactionClasses;

	UPROPERTY(EditAnywhere, Category = "Reaction|Data")
	TArray<FReactionData> ReactionDatas;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	TMap<FReactionDataKey, FReactionData> ReactionDataMap;

	UPROPERTY(Transient)
	TMap<class UClass*, class UCReaction*> ReactionExcutorMap;

private:
	/* === Component State === */
	UPROPERTY(Transient)
	EReactionType ActiveReactionType_Cached = EReactionType::None;

private:
	/* === ReactionContext State === */
	UPROPERTY(Transient)
	FReactionContext ActiveReactionContext_Cached;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached = nullptr;

public:
	/* === Delegate === */
	FReactionTypeChanged OnReactionTypeChanged;

protected:
	void BeginPlay() override;

public:
	// Query API
	bool IsActiveReaction() const;

public:
	// Get API
	bool GetActiveReactionContext(FReactionContext& OutReactionContext) const;
	UCReaction* GetActiveReactionExecutor() const;

public:
	// Temporary data provider API (Move to DataAsset).
	bool ResolveReactionData(const FApplyDamageSpecKey& InApplyDamageSpecKey, EReactionType InReactionType, FReactionData& OutReactionData);
	UCReaction* ResolveReactionExecutor(const FReactionData& InReactionData);

public:
	/* === EntryPoint API === */
	// Entry points used by orchestration/external systems to drive reaction execution.
	bool ApplyReactionDecision(const FReactionOrchestrationResult& InReactionOrchestrationResult);

public:
	void HandleReactionFinished(const UCReaction* InReaction, EReactionFinishReason InReactionFinishReason);

public:
	void HandleReactionControlWindowBegin(EReactionControlWindowType InReactionWindowType);
	void HandleReactionControlWindowEnd(EReactionControlWindowType InReactionWindowType);

	void HandleReactionFeedbackWindowBegin(FName InTriggerKey);
	void HandleReactionFeedbackWindowEnd(FName InTriggerKey);
	void HandleReactionFeedback(FName InTriggerKey);

private:
	bool TryStartReaction(const FReactionContext& InReactionContext);
	bool TryInterruptReaction(const FReactionContext& InReactionContext);
	bool TryCancelReaction(const FReactionContext& InReactionContext);

private:
	bool StartActiveReactionInternal(const FReactionContext& InReactionContext);
	bool StopActiveReactionInternal(EReactionStopReason InStopReason);
	void EndActiveReactionInternal();

private:
	void SetActiveReaction(const FReactionContext& InReactionContext);
	void ClearActiveReaction();

private:
	void EnterReactionState(const FReactionData& InReactionData);
	void ExitReactionState(const FReactionData& InReactionData);

private:
	void AbortActiveActionForReaction();

private:
	// Temporary data build API (Move to DataAsset).
	void BuildReactionDataMap(bool bRebuildAll);
	void BuildReactionExecutorMap(bool bRebuildAll);
	void BuildCandidateSpecKeys(const FApplyDamageSpecKey& InApplyDamageSpecKey, TArray<FApplyDamageSpecKey>& OutApplyDamageSpecKeys) const;

private:
	UCReaction* AddReactionExecutor(const TSubclassOf<class UCReaction> InSubClass);
	UCReaction* FindReactionExecutor(const UClass* InClass);

private:
	void PrintReactionInfoSummary() const;
	void PrintReactionDataMap() const;

	void PrintComponentStateInfo() const;
	void PrintApplyDamageSpecKeyInfo(const FApplyDamageSpecKey& InApplyDamageSpecKey) const;
	void PrintReactionDataKeyInfo(const FReactionDataKey& InReactionDataKey) const;
	void PrintReactionDataInfo(const FReactionData& InReactionData) const;
	void PrintReactionExcutorInfo(const UCReaction* InReaction) const;
	void PrintReactionExecutorRuntimeInfo(const UCReaction* InReaction) const;
};
