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

	if (InKey.ActionIndex == GetGuardActionPhaseIndex(EGuardActionPhase::In)) return EGuardActionPhase::In;
	if (InKey.ActionIndex == GetGuardActionPhaseIndex(EGuardActionPhase::Out)) return EGuardActionPhase::Out;
	if (InKey.ActionIndex == GetGuardActionPhaseIndex(EGuardActionPhase::Hold)) return EGuardActionPhase::Hold;
	if (InKey.ActionIndex == GetGuardActionPhaseIndex(EGuardActionPhase::Hit)) return EGuardActionPhase::Hit;
	if (InKey.ActionIndex == GetGuardActionPhaseIndex(EGuardActionPhase::Parry)) return EGuardActionPhase::Parry;

	return EGuardActionPhase::None;
}
