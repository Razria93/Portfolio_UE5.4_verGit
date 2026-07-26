#pragma once

#include "CoreMinimal.h"
#include "Type/CActionTypes.h"
#include "CActionKeyTypes.generated.h"

// Key / Identifier

namespace CActionIndexConstants
{
	constexpr int32 FirstActionIndex = 0;
	constexpr int32 NextSequentialActionOffset = 1;

	constexpr int32 GuardInActionIndex = 1;
	constexpr int32 GuardOutActionIndex = 2;
	constexpr int32 GuardHoldActionIndex = 3;
	constexpr int32 GuardHitActionIndex = 4;
	constexpr int32 GuardParryActionIndex = 5;
}

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
		return CActionIndexConstants::GuardInActionIndex;

	case EGuardActionPhase::Out:
		return CActionIndexConstants::GuardOutActionIndex;

	case EGuardActionPhase::Hold:
		return CActionIndexConstants::GuardHoldActionIndex;

	case EGuardActionPhase::Hit:
		return CActionIndexConstants::GuardHitActionIndex;

	case EGuardActionPhase::Parry:
		return CActionIndexConstants::GuardParryActionIndex;

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
