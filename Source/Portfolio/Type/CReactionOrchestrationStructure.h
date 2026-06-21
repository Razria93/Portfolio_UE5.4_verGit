#pragma once

#include "CoreMinimal.h"
#include "Type/CWeaponStructure.h"
#include "CReactionOrchestrationStructure.generated.h"

UENUM(BlueprintType)
enum class EReactionIntentSource : uint8
{
	None = 0,

	TakeDamage,
	CombatResult,

	Max,
};

USTRUCT(BlueprintType)
struct FReactionCandidate
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FReactionDataKey ReactionDataKey = FReactionDataKey();

public:
	bool IsValidMinimal() const
	{
		return ReactionDataKey.IsValidMinimal();
	}
};

USTRUCT(BlueprintType)
struct FDamageReactionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionIntentSource IntentSource = EReactionIntentSource::TakeDamage;

	UPROPERTY(Transient)
	FTakeDamagePacket TakeDamagePacket = FTakeDamagePacket();
};

USTRUCT(BlueprintType)
struct FCombatResultReactionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionIntentSource IntentSource = EReactionIntentSource::CombatResult;

	UPROPERTY(Transient)
	FCombatResultPacket CombatResultPacket = FCombatResultPacket();

	UPROPERTY(Transient)
	EReactionType ReactionType = EReactionType::None;
};

USTRUCT(BlueprintType)
struct FReactionRequestResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionRequestResultType ResultType = EReactionRequestResultType::None;

	UPROPERTY(Transient)
	EReactionRequestRejectReason RejectReason = EReactionRequestRejectReason::None;

public:
	bool IsAccepted() const
	{
		return ResultType == EReactionRequestResultType::Started
			|| ResultType == EReactionRequestResultType::Intervened;
	}
};
