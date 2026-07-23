#pragma once

#include "CoreMinimal.h"
#include "Type/CReactionTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "Type/CExecutionRuleTypes.h"
#include "CReactionDataTypes.generated.h"

USTRUCT(BlueprintType)
struct FReactionDataKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(EditAnywhere)
	EReactionType ReactionType = EReactionType::None;

public:
	FReactionDataKey() = default;

public:
	bool IsValidMinimal() const;

public:
	bool operator==(const FReactionDataKey& InOther) const
	{
		return ReactionType == InOther.ReactionType
			&& DamageSpecKey == InOther.DamageSpecKey;
	}
};

FORCEINLINE uint32 GetTypeHash(const FReactionDataKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(InKey.DamageSpecKey));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ReactionType)));

	return H;
}

USTRUCT(BlueprintType)
struct FReactionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Key")
	FReactionDataKey ReactionDataKey = FReactionDataKey();

	UPROPERTY(EditAnywhere, Category = "Key")
	TSubclassOf<class UCReaction> ReactionExecutorKey = nullptr;

	UPROPERTY(EditAnywhere, Category = "Priority")
	int32 Priority = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Data")
	class UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Data")
	float PlayRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Data")
	FName StartSectionName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Data")
	bool bCanMove = false;

	UPROPERTY(EditAnywhere, Category = "Intervention|Want")
	TArray<FExecutionInterventionWantRule> WantInterventionRules;

	UPROPERTY(EditAnywhere, Category = "Intervention|Allow")
	TArray<FExecutionInterventionAllowRule> AllowInterventionRules;

public:
	FReactionData() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FReactionExecutionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FReactionDataKey ReactionDataKey = FReactionDataKey();

	UPROPERTY(Transient)
	FReactionData ReactionData = FReactionData();

	UPROPERTY(Transient)
	class UCReaction* ReactionExecutor = nullptr;

public:
	FReactionExecutionContext() = default;

public:
	bool IsValidMinimal() const;
};
