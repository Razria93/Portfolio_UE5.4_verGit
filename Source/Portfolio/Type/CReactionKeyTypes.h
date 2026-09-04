#pragma once

#include "CoreMinimal.h"
#include "Type/CReactionTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "CReactionKeyTypes.generated.h"

// Key / Identifier

UENUM(BlueprintType)
enum class EReactionDataMatchMode : uint8
{
	DamageSpec = 0,
	Global,

	Max,
};

USTRUCT(BlueprintType)
struct FReactionDataKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EReactionDataMatchMode MatchMode = EReactionDataMatchMode::DamageSpec;

	UPROPERTY(EditAnywhere)
	FDamageSpecKey DamageSpecKey = FDamageSpecKey();

	UPROPERTY(EditAnywhere)
	EReactionType ReactionType = EReactionType::None;

	UPROPERTY(EditAnywhere)
	int32 ReactionIndex = INDEX_NONE;

public:
	FReactionDataKey() = default;

public:
	bool IsValidMinimal() const;

public:
	bool operator==(const FReactionDataKey& InOther) const
	{
		if (MatchMode != InOther.MatchMode
			|| ReactionType != InOther.ReactionType
			|| ReactionIndex != InOther.ReactionIndex)
		{
			return false;
		}

		switch (MatchMode)
		{
		case EReactionDataMatchMode::Global:
			return true;

		case EReactionDataMatchMode::DamageSpec:
			return DamageSpecKey == InOther.DamageSpecKey;

		default:
			return false;
		}
	}
};

// Helper API

FORCEINLINE uint32 GetTypeHash(const FReactionDataKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.MatchMode)));
	if (InKey.MatchMode == EReactionDataMatchMode::DamageSpec)
	{
		H = HashCombine(H, GetTypeHash(InKey.DamageSpecKey));
	}
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ReactionType)));
	H = HashCombine(H, GetTypeHash(InKey.ReactionIndex));

	return H;
}
