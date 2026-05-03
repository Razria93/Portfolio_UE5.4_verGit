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
	const FReactionContext& GetActiveReactionContext() const { return ActiveReactionContext_Cached; }

public:
	// Orchestrator Decision Apply API
	bool ApplyReactionDecision(const FReactionOrchestrationResult& InReactionOrchestrationResult);

public:
	bool StartReaction(const FReactionContext& InReactionContext);
	bool InterruptReaction(const FReactionContext& InReactionContext);
	void EndReaction();

public:
	// Temporary data provider API (Move to DataAsset).
	bool ResolveReactionData(const FApplyDamageSpecKey& InApplyDamageSpecKey, EReactionType InReactionType, FReactionData& OutReactionData);
	UCReaction* ResolveReactionExecutor(const FReactionData& InReactionData);

public:
	// Notify Call API
	void OnReactionBegin();
	void OnReactionEnd(const UCReaction* InReaction, bool bInterrupted);
	void OnReactionWindowBegin(EReactionWindowType InReactionWindowType, UAnimSequenceBase* InAnimation);
	void OnReactionWindowEnd(EReactionWindowType InReactionWindowType, UAnimSequenceBase* InAnimation);

private:
	bool ReplaceActiveReaction(const FReactionContext& InContext, EReactionStopReason InStopReason);

private:
	bool StartActiveReactionInternal(const FReactionContext& InReactionContext);
	void StopActiveReactionInternal(EReactionStopReason InStopReason);
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

private:
	void PrintComponentStateInfo() const;
	void PrintApplyDamageSpecKeyInfo(const FApplyDamageSpecKey& InApplyDamageSpecKey) const;
	void PrintReactionDataKeyInfo(const FReactionDataKey& InReactionDataKey) const;
	void PrintReactionDataInfo(const FReactionData& InReactionData) const;
	void PrintReactionExcutorInfo(const UCReaction* InReaction) const;
	void PrintReactionExecutorRuntimeInfo(const UCReaction* InReaction) const;
};
