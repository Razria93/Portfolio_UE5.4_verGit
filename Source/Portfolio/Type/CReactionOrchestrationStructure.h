#pragma once

#include "CoreMinimal.h"
#include "Type/CWeaponStructure.h"
#include "CReactionOrchestrationStructure.generated.h"

UENUM(BlueprintType)
enum class EReactionIntentSource : uint8
{
	None = 0,

	TakeDamage,

	Max,
};

UENUM(BlueprintType)
enum class EReactionRequestResultType : uint8
{
	None = 0,

	Rejected,
	Ignored,

	Started,
	Interrupted,

	Max,
};

UENUM(BlueprintType)
enum class EReactionRequestRejectReason : uint8
{
	None = 0,

	InvalidOwner,
	InvalidRequest,
	InvalidComponent,

	InvalidDamageResult,

	ReactionTypeNotFound,
	ReactionDataNotFound,
	ReactionExecutorNotFound,

	LowerPriority,
	CurrentNotInterruptible,
	IncomingCannotInterrupt,

	Max,
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
struct FReactionRequestResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionRequestResultType ResultType = EReactionRequestResultType::None;

	UPROPERTY(Transient)
	EReactionRequestRejectReason RejectReason = EReactionRequestRejectReason::None;

	UPROPERTY(Transient)
	EReactionType ResolvedReactionType = EReactionType::None;

public:
	bool IsAccepted() const
	{
		return ResultType == EReactionRequestResultType::Started
			|| ResultType == EReactionRequestResultType::Interrupted;
	}
};


UENUM(BlueprintType)
enum class EReactionOrchestrationDecision : uint8
{
	None = 0,

	Reject,
	Ignore,

	Start,
	Interrupt,

	Max,
};

USTRUCT(BlueprintType)
struct FReactionExecutionPolicy
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bCanInterruptActive = false;
	
	UPROPERTY(Transient)
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct FReactionOrchestrationQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionIntentSource IntentSource = EReactionIntentSource::None;

	UPROPERTY(Transient)
	EReactionType IncomingType = EReactionType::None;

	UPROPERTY(Transient)
	FReactionExecutionPolicy IncomingPolicy;

	UPROPERTY(Transient)
	FReactionContext IncomingContext;

	UPROPERTY(Transient)
	FReactionContext ActiveContext;
};

USTRUCT(BlueprintType)
struct FReactionOrchestrationResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionOrchestrationDecision Decision = EReactionOrchestrationDecision::Reject;

	UPROPERTY(Transient)
	EReactionRequestRejectReason RejectReason = EReactionRequestRejectReason::None;

	UPROPERTY(Transient)
	EReactionType ReactionType = EReactionType::None;

	UPROPERTY(Transient)
	FReactionContext ReactionContext;

public:
	bool IsAccepted() const
	{
		return Decision == EReactionOrchestrationDecision::Start
			|| Decision == EReactionOrchestrationDecision::Interrupt;
	}
};
