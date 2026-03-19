#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
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
	UPROPERTY(EditAnywhere)
	TArray<FReactionData> ReactionDatas;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class UCReaction>> ReactionClasses;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	TMap<FReactionDataKey, FReactionData> ReactionDataMap;

	UPROPERTY(Transient)
	TMap<class UClass*, class UCReaction*> ReactionExcutorMap; // TODO: Refactor from UCReaction* to TObjectPtr<UCReaction>

private:
	/* === Component State === */
	UPROPERTY(Transient)
	EReactionType ActiveReactionType_Cached;

private:
	/* === ReactionContext State === */
	UPROPERTY(Transient)
	FReactionContext PendingReactionContext_Cached;

	UPROPERTY(Transient)
	FReactionContext ActiveReactionContext_Cached;

	UPROPERTY(Transient)
	int32 PendingReactionVersion_Cached = INDEX_NONE;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Cached;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached;

public:
	/* === Delegate === */
	FReactionTypeChanged OnReactionTypeChanged;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Getter API
	int32 GetPendingReactionVersion() const;

public:
	// Query API
	bool HasPendingReactionContext() const;
	bool HasActiveReactionContext() const;

public:
	// Pending API
	bool TryRequestPendingReaction(const FTakeDamageResult& InTakeDamageResult);
	bool TryConsumePendingReaction(FReactionContext& OutReactionContext);
	bool TryExecuteReaction(const FReactionContext& InReactionContext);
	void FinishReaction();

public:
	// Call API
	void OnReactionBegin();
	void OnReactionEnd(const UCReaction* InReaction, bool bInterrupted);
	void OnReactionWindowBegin(EReactionWindowType InReactionWindowType, UAnimSequenceBase* InAnimation);
	void OnReactionWindowEnd(EReactionWindowType InReactionWindowType, UAnimSequenceBase* InAnimation);

private:
	// Bulid Pipeline
	bool TryBuildReactionContext(const FTakeDamageResult& InTakeDamageResult, FReactionContext& OutReactionContext);

private:
	bool ValidateRequest(const FTakeDamageResult& takeDamageResult) const;
	EReactionType ResolveReactionType(const FTakeDamageResult& takeDamageResult);
	bool ResolveReactionData(const FApplyDamageSpecKey& InApplyDamageSpecKey, EReactionType InReactionType, FReactionData& OutReactionData);
	UCReaction* ResolveReactionExecutor(const FReactionData& InReactionData);
	bool QueryReplaceReaction(UCReaction * InCurrentReactionExecutor, UCReaction * InIncomingReactionExecutor, const FReactionData & InCurrentReactionData, const FReactionData & InIncomingReactionData);

private:
	// Bulid Function
	void BuildReactionDataMap(bool bRebuildAll);			// true: Rebuild | false: Append
	void BuildReactionExecutorMap(bool bRebuildAll);		// true: Rebuild | false: Append
	void BuildCandidateSpecKeys(const FApplyDamageSpecKey& InApplyDamageSpecKey, TArray<FApplyDamageSpecKey>& OutApplyDamageSpecKeys) const;

private:
	UCReaction* AddReactionExecutor(const TSubclassOf<class UCReaction> InSubClass);
	UCReaction* FindReactionExecutor(const UClass* InClass);

private:
	// Execute Function
	void ChangeActiveReaction(const FReactionContext& InReactionContext);
	void ClearActiveReaction();

private:
	void UpdateMovementToImmovable(const FReactionData& InReactionData);
	void UpdateMovementToMovable(const FReactionData& InReactionData);
	void UpdateStateToReaction();
	void UpdateStateToIdle();

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
