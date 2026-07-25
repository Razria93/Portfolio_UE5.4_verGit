#pragma once

#include "CoreMinimal.h"
#include "Type/CActionTypes.h"
#include "CActionKeyTypes.generated.h"

// Key / Identifier

USTRUCT(BlueprintType)
struct FActionDataKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Key")
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere, Category = "Key")
	int32 ActionIndex = INDEX_NONE;

public:
	bool IsValidMinimal() const;

public:
	bool operator==(const FActionDataKey& InOther) const
	{
		return ActionType == InOther.ActionType
			&& ActionIndex == InOther.ActionIndex;
	}
};

// Helper API

FORCEINLINE uint32 GetTypeHash(const FActionDataKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionType)));
	H = HashCombine(H, GetTypeHash(InKey.ActionIndex));

	return H;
}

FORCEINLINE int32 GetGuardActionPhaseIndex(EGuardActionPhase InPhase)
{
	switch (InPhase)
	{
	case EGuardActionPhase::In:
		return 1;

	case EGuardActionPhase::Out:
		return 2;

	case EGuardActionPhase::Hold:
		return 3;

	case EGuardActionPhase::Hit:
		return 4;

	case EGuardActionPhase::Parry:
		return 5;

	default:
		return INDEX_NONE;
	}
}

FORCEINLINE EGuardActionPhase ResolveGuardActionPhase(const FActionDataKey& InKey)
{
	if (InKey.ActionType != EActionType::Guard) return EGuardActionPhase::None;

	switch (InKey.ActionIndex)
	{
	case 1:
		return EGuardActionPhase::In;

	case 2:
		return EGuardActionPhase::Out;

	case 3:
		return EGuardActionPhase::Hold;

	case 4:
		return EGuardActionPhase::Hit;

	case 5:
		return EGuardActionPhase::Parry;

	default:
		return EGuardActionPhase::None;
	}
}
