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
	bool bIsReaction = false;

private:
	/* === State === */
	UPROPERTY(Transient)
	FReactionDataKey ActiveReactionDataKey_Cached;

	UPROPERTY(Transient)
	FReactionData ActiveReactionData_Cached;

private:
	UPROPERTY(Transient)
	class UClass* ActiveReactionExcutorKey_Cached;

	UPROPERTY(Transient)
	class UCReaction* ActiveReactionExcutor_Cached;



private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached;

public:
	/* === Delegate === */
	FReactionTypeChanged OnReactionTypeChanged;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// AnimNotify Entry API
	void AnimNotify_ReactionBegin();
	void AnimNotify_ReactionEnd();

public:
	// Entry API
	void RequestReaction(const FTakeDamageResult& InTakeDamageResult);

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
	void ChangeActiveReaction(UCReaction* InNewReaction, const FReactionData& InReactionData);

private:
	void BuildReactionDataMap(bool bRebuildAll);	// true: Rebuild | false: Append
	void BuildReactionMap(bool bRebuildAll);		// true: Rebuild | false: Append
	void BuildCandidateSpecKeys(const FApplyDamageSpecKey& InApplyDamageSpecKey, TArray<FApplyDamageSpecKey>& OutApplyDamageSpecKeys) const;

private:
	UCReaction* CreateReaction(const TSubclassOf<class UCReaction> InSubClass);
	UCReaction* FindReaction(const UClass* InClass);

private:
	void PrintReactionDataMapInfo() const;

	void PrintApplyDamageSpecKeyInfo(const FApplyDamageSpecKey& InApplyDamageSpecKey) const;
	void PrintReactionDataKeyInfo(const FReactionDataKey& InReactionDataKey) const;
	void PrintReactionDataInfo(const FReactionData& InReactionData) const;
};
