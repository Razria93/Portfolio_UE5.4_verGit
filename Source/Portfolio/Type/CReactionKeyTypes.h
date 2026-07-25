#pragma once

#include "CoreMinimal.h"
#include "Type/CReactionTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "CReactionKeyTypes.generated.h"

// Key / Identifier

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

// Helper API

FORCEINLINE uint32 GetTypeHash(const FReactionDataKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(InKey.DamageSpecKey));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ReactionType)));

	return H;
}
