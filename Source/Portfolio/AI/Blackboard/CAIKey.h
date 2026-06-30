#pragma once

#include "CoreMinimal.h"

enum class EAIBlackboardKeyValueType : uint8
{
	Bool,
	Int,
	Float,
	Enum,
	Object,
	Vector,
};

struct FAIBlackboardKeySpec
{
	FName KeyName = NAME_None;
	EAIBlackboardKeyValueType ValueType = EAIBlackboardKeyValueType::Bool;
	bool bRequired = true;
	bool bClearOnRuntimeTeardown = true;
};

namespace CAIKey
{
	namespace Targeting
	{
		static const FAIBlackboardKeySpec TargetActor = { TEXT("TargetActor"), EAIBlackboardKeyValueType::Object };
		static const FAIBlackboardKeySpec TargetPriority = { TEXT("TargetPriority"), EAIBlackboardKeyValueType::Int };
	}

	namespace State
	{
		static const FAIBlackboardKeySpec AIIntentState = { TEXT("AIIntentState"), EAIBlackboardKeyValueType::Enum };
	}

	namespace Perception
	{
		static const FAIBlackboardKeySpec bHasLOS = { TEXT("bHasLOS"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec LastSeenTime = { TEXT("LastSeenTime"), EAIBlackboardKeyValueType::Float };
		static const FAIBlackboardKeySpec LastKnownLocation = { TEXT("LastKnownLocation"), EAIBlackboardKeyValueType::Vector };
	}

	namespace Metric
	{
		static const FAIBlackboardKeySpec DistanceToTarget = { TEXT("DistanceToTarget"), EAIBlackboardKeyValueType::Float };
		static const FAIBlackboardKeySpec DistanceToHome = { TEXT("DistanceToHome"), EAIBlackboardKeyValueType::Float };
	}

	namespace Navigation
	{
		static const FAIBlackboardKeySpec bReturnHome = { TEXT("bReturnHome"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec HomeLocation = { TEXT("HomeLocation"), EAIBlackboardKeyValueType::Vector };
	}

	namespace Patrol
	{
		static const FAIBlackboardKeySpec bUsePatrol = { TEXT("bUsePatrol"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec PatrolPath = { TEXT("PatrolPath"), EAIBlackboardKeyValueType::Object };
		static const FAIBlackboardKeySpec PatrolMode = { TEXT("PatrolMode"), EAIBlackboardKeyValueType::Enum };

		static const FAIBlackboardKeySpec bPatrolReverse = { TEXT("bPatrolReverse"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec PatrolLocation = { TEXT("PatrolLocation"), EAIBlackboardKeyValueType::Vector };
		static const FAIBlackboardKeySpec PatrolIndex = { TEXT("PatrolIndex"), EAIBlackboardKeyValueType::Int };
	}

	namespace Investigate
	{
		static const FAIBlackboardKeySpec bUseInvestigate = { TEXT("bUseInvestigate"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec InvestigateDuration = { TEXT("InvestigateDuration"), EAIBlackboardKeyValueType::Float };
		static const FAIBlackboardKeySpec InvestigateMaxIndex = { TEXT("InvestigateMaxIndex"), EAIBlackboardKeyValueType::Int };

		static const FAIBlackboardKeySpec bCanInvestigate = { TEXT("bCanInvestigate"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec bIsInvestigating = { TEXT("bIsInvestigating"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec InvestigateLocation = { TEXT("InvestigateLocation"), EAIBlackboardKeyValueType::Vector };
		static const FAIBlackboardKeySpec InvestigateIndex = { TEXT("InvestigateIndex"), EAIBlackboardKeyValueType::Int };
	}

	namespace Chase
	{
		static const FAIBlackboardKeySpec ChaseOffsetRange = { TEXT("ChaseOffsetRange"), EAIBlackboardKeyValueType::Float };
		static const FAIBlackboardKeySpec ChaseEnterBuffer = { TEXT("ChaseEnterBuffer"), EAIBlackboardKeyValueType::Float };
		static const FAIBlackboardKeySpec ChaseExitBuffer = { TEXT("ChaseExitBuffer"), EAIBlackboardKeyValueType::Float };
	}

	namespace Alert
	{
		static const FAIBlackboardKeySpec bUseAlertStep = { TEXT("bUseAlertStep"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec StepForwardDistance = { TEXT("StepForwardDistance"), EAIBlackboardKeyValueType::Float };
		static const FAIBlackboardKeySpec StepSideDistance = { TEXT("StepSideDistance"), EAIBlackboardKeyValueType::Float };

		static const FAIBlackboardKeySpec bInAlertRange = { TEXT("bInAlertRange"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec AlertStepLocation = { TEXT("AlertStepLocation"), EAIBlackboardKeyValueType::Vector };
	}

	namespace Engage
	{
		static const FAIBlackboardKeySpec bShouldEngage = { TEXT("bShouldEngage"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec bCanCombatAction = { TEXT("bCanCombatAction"), EAIBlackboardKeyValueType::Bool };
		
		static const FAIBlackboardKeySpec bIsCombatAction = { TEXT("bIsCombatAction"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec bInEngageRange = { TEXT("bInEngageRange"), EAIBlackboardKeyValueType::Bool };
		static const FAIBlackboardKeySpec NextCombatActionTime = { TEXT("NextCombatActionTime"), EAIBlackboardKeyValueType::Float };
	}
	
	namespace Reaction
	{
		static const FAIBlackboardKeySpec bIsActiveReaction = { TEXT("bIsActiveReaction"), EAIBlackboardKeyValueType::Bool };
	}

	namespace Dead
	{
		static const FAIBlackboardKeySpec DeadState = { TEXT("DeadState"), EAIBlackboardKeyValueType::Enum };
	}
}
