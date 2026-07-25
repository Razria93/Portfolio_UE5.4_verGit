#pragma once

#include "CoreMinimal.h"
#include "Type/CActionTypes.h"
#include "Type/CReactionTypes.h"
#include "CExecutionRuleTypes.generated.h"

struct FExecutionParticipant;

// Enum

UENUM(BlueprintType)
enum class EExecutionDomain : uint8
{
	None = 0,

	Action,
	Reaction,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionInterventionTiming : uint8
{
	None = 0,

	Always,
	Window,

	Max,
};

// Key / Identifier

USTRUCT(BlueprintType)
struct FExecutionInterventionParticipantFilter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Filter")
	EExecutionDomain Domain = EExecutionDomain::None;

	UPROPERTY(EditAnywhere, Category = "Filter")
	EActionType ActionType = EActionType::None;

	UPROPERTY(EditAnywhere, Category = "Filter")
	EReactionType ReactionType = EReactionType::None;

	// INDEX_NONE means any index. Reaction currently ignores Index.
	UPROPERTY(EditAnywhere, Category = "Filter")
	int32 Index = INDEX_NONE;

public:
	bool IsValidMinimal() const;

	bool MatchesAction(EActionType InActionType, int32 InIndex = INDEX_NONE) const;
	bool MatchesReaction(EReactionType InReactionType) const;
	bool MatchesParticipant(const FExecutionParticipant& InParticipant) const;

public:
	bool operator==(const FExecutionInterventionParticipantFilter& InOther) const
	{
		return Domain == InOther.Domain
			&& ActionType == InOther.ActionType
			&& ReactionType == InOther.ReactionType
			&& Index == InOther.Index;
	}
};

// Data / Config

USTRUCT(BlueprintType)
struct FExecutionInterventionWantRule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Intervention")
	TArray<FExecutionInterventionParticipantFilter> ParticipantFilters;

public:
	bool IsValidMinimal() const
	{
		return !ParticipantFilters.IsEmpty();
	}
};

USTRUCT(BlueprintType)
struct FExecutionInterventionAllowRule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Intervention")
	EExecutionInterventionTiming Timing = EExecutionInterventionTiming::Always;

	UPROPERTY(EditAnywhere, Category = "Intervention")
	FName WindowKey = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Intervention")
	TArray<FExecutionInterventionParticipantFilter> ParticipantFilters;

public:
	bool IsValidMinimal() const
	{
		return Timing != EExecutionInterventionTiming::None
			&& Timing != EExecutionInterventionTiming::Max
			&& !ParticipantFilters.IsEmpty();
	}
};
