#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CReactionComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCReactionComponent : public UActorComponent
{
	GENERATED_BODY()

	// === ReactionData ===================================== //
private:
	UPROPERTY(EditAnywhere)
	TArray<FReactionData> ReactionDatas;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	TMap<FReactionKey, FReactionData> ReactionContainer;

private:
	/* === State === */
	EReactionType CurrentReactionType_Cached;

private:
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;

private:
	bool bIsReaction = false;

public:
	UCReactionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Entry API
	void RequestReaction(const FTakeDamageResult& InTakeDamageResult);

private:
	// Pipeline
	void ProcessReaction(const FTakeDamageResult& InTakeDamageResult);

private:
	bool ValidateRequest(const FTakeDamageResult& takeDamageResult) const;
	EReactionType ResolveReactionType(const FTakeDamageResult& takeDamageResult) const;
	void BuildCandidateSpecKeys(const FApplyDamageSpecKey& InApplyDamageSpecKey, TArray<FApplyDamageSpecKey>& OutApplyDamageSpecKeys) const;
	bool FindReaction(const FApplyDamageSpecKey& InApplyDamageSpecKey, EReactionType InReactionType, FReactionData& OutReactionData) const;
	void CommitReaction(const FReactionData& reactionData);

private:
	void BuildReactionContainer();

private:
	void PrintReactionContainerInfo() const;

	void PrintApplyDamageSpecKeyInfo(const FApplyDamageSpecKey& InApplyDamageSpecKey) const;
	void PrintReactionKeyInfo(const FReactionKey& InReactionKey) const;
	void PrintReactionDataInfo(const FReactionData& InReactionData) const;
};
