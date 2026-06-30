#pragma once

#include "CoreMinimal.h"

#include "AI/BlackBoard/CAIKey.h"

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

namespace CAIKeyRegistry
{
	static const TCHAR* GetValueTypeName(EAIBlackboardKeyValueType InValueType)
	{
		switch (InValueType)
		{
		case EAIBlackboardKeyValueType::Bool:
			return TEXT("Bool");

		case EAIBlackboardKeyValueType::Int:
			return TEXT("Int");

		case EAIBlackboardKeyValueType::Float:
			return TEXT("Float");

		case EAIBlackboardKeyValueType::Enum:
			return TEXT("Enum");

		case EAIBlackboardKeyValueType::Object:
			return TEXT("Object");

		case EAIBlackboardKeyValueType::Vector:
			return TEXT("Vector");

		default:
			return TEXT("Unknown");
		}
	}

	static const TArray<FAIBlackboardKeySpec>& GetKeySpecs()
	{
		static const TArray<FAIBlackboardKeySpec> keySpecs =
		{
			// Targeting
			{ CAIKey::Targeting::TargetActor, EAIBlackboardKeyValueType::Object },
			{ CAIKey::Targeting::TargetPriority, EAIBlackboardKeyValueType::Int },

			// State
			{ CAIKey::State::AIIntentState, EAIBlackboardKeyValueType::Enum },

			// Perception
			{ CAIKey::Perception::bHasLOS, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Perception::LastSeenTime, EAIBlackboardKeyValueType::Float },
			{ CAIKey::Perception::LastKnownLocation, EAIBlackboardKeyValueType::Vector },

			// Metric
			{ CAIKey::Metric::DistanceToTarget, EAIBlackboardKeyValueType::Float },
			{ CAIKey::Metric::DistanceToHome, EAIBlackboardKeyValueType::Float },

			// Navigation
			{ CAIKey::Navigation::bReturnHome, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Navigation::HomeLocation, EAIBlackboardKeyValueType::Vector },

			// Patrol
			{ CAIKey::Patrol::bUsePatrol, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Patrol::PatrolPath, EAIBlackboardKeyValueType::Object },
			{ CAIKey::Patrol::PatrolMode, EAIBlackboardKeyValueType::Enum },
			{ CAIKey::Patrol::bPatrolReverse, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Patrol::PatrolLocation, EAIBlackboardKeyValueType::Vector },
			{ CAIKey::Patrol::PatrolIndex, EAIBlackboardKeyValueType::Int },

			// Investigate
			{ CAIKey::Investigate::bUseInvestigate, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Investigate::InvestigateDuration, EAIBlackboardKeyValueType::Float },
			{ CAIKey::Investigate::InvestigateMaxIndex, EAIBlackboardKeyValueType::Int },
			{ CAIKey::Investigate::bCanInvestigate, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Investigate::bIsInvestigating, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Investigate::InvestigateLocation, EAIBlackboardKeyValueType::Vector },
			{ CAIKey::Investigate::InvestigateIndex, EAIBlackboardKeyValueType::Int },

			// Chase
			{ CAIKey::Chase::ChaseOffsetRange, EAIBlackboardKeyValueType::Float },
			{ CAIKey::Chase::ChaseEnterBuffer, EAIBlackboardKeyValueType::Float },
			{ CAIKey::Chase::ChaseExitBuffer, EAIBlackboardKeyValueType::Float },

			// Alert
			{ CAIKey::Alert::bUseAlertStep, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Alert::StepForwardDistance, EAIBlackboardKeyValueType::Float },
			{ CAIKey::Alert::StepSideDistance, EAIBlackboardKeyValueType::Float },
			{ CAIKey::Alert::bInAlertRange, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Alert::AlertStepLocation, EAIBlackboardKeyValueType::Vector },

			// Engage
			{ CAIKey::Engage::bShouldEngage, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Engage::bCanCombatAction, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Engage::bIsCombatAction, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Engage::bInEngageRange, EAIBlackboardKeyValueType::Bool },
			{ CAIKey::Engage::NextCombatActionTime, EAIBlackboardKeyValueType::Float },

			// Reaction
			{ CAIKey::Reaction::bIsActiveReaction, EAIBlackboardKeyValueType::Bool },

			// Dead
			{ CAIKey::Dead::DeadState, EAIBlackboardKeyValueType::Enum },
		};

		return keySpecs;
	}
}
