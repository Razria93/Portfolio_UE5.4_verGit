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
	EReactionType CurrentReactionType_Cached;

	UPROPERTY(Transient)
	bool bHasActive = false;

private:
	/* === ActiveReaction State === */
	UPROPERTY(Transient)
	FReactionData ActiveReactionData_Cached;

	UPROPERTY(Transient)
	class UCReaction* ActiveReactionExcutor_Cached;

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
	// Entry API
	void RequestReaction(const FTakeDamageResult& InTakeDamageResult);

public:
	// CReaction API
	void OnReactionBegin();
	void OnReactionEnd(const UCReaction* InReaction, bool bInterrupted);

public:
	// AnimNotify API
	void OnReactionWindowBegin(EReactionWindowType InReactionWindowType, UAnimSequenceBase* InAnimation);
	void OnReactionWindowEnd(EReactionWindowType InReactionWindowType, UAnimSequenceBase* InAnimation);

private:
	// Pipeline
	void ProcessReaction(const FTakeDamageResult& InTakeDamageResult);

private:
	bool ValidateRequest(const FTakeDamageResult& takeDamageResult) const;
	EReactionType ResolveReactionType(const FTakeDamageResult& takeDamageResult);
	bool ResolveReactionData(const FApplyDamageSpecKey& InApplyDamageSpecKey, EReactionType InReactionType, FReactionData& OutReactionData);
	UCReaction* ResolveReaction(const FReactionData& InReactionData);
	bool QueryAcceptNewReaction(UCReaction* InActiveReaction, UCReaction* InNewReaction, const FReactionData& InActiveReactionData, const FReactionData& InNewReactionData);
	void PlayReaction(UCReaction* InNewReaction, const FReactionData& InReactionData);

private:
	void BuildReactionDataMap(bool bRebuildAll);	// true: Rebuild | false: Append
	void BuildReactionMap(bool bRebuildAll);		// true: Rebuild | false: Append
	void BuildCandidateSpecKeys(const FApplyDamageSpecKey& InApplyDamageSpecKey, TArray<FApplyDamageSpecKey>& OutApplyDamageSpecKeys) const;

private:
	UCReaction* CreateReaction(const TSubclassOf<class UCReaction> InSubClass);
	UCReaction* FindReaction(const UClass* InClass);

private:
	void ChangeActiveReaction(UCReaction* InNewReaction, const FReactionData& InReactionData);
	void ClearActiveReaction();

private:
	void RestoreMovementToMovable();
	void RestoreStateToIdle();

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
