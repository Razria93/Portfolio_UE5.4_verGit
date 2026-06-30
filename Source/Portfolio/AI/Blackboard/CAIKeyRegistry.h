#pragma once

#include "CoreMinimal.h"

#include "BehaviorTree/BlackboardData.h"

#include "AI/BlackBoard/CAIKey.h"

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
			CAIKey::Targeting::TargetActor,
			CAIKey::Targeting::TargetPriority,

			// State
			CAIKey::State::AIIntentState,

			// Perception
			CAIKey::Perception::bHasLOS,
			CAIKey::Perception::LastSeenTime,
			CAIKey::Perception::LastKnownLocation,

			// Metric
			CAIKey::Metric::DistanceToTarget,
			CAIKey::Metric::DistanceToHome,

			// Navigation
			CAIKey::Navigation::bReturnHome,
			CAIKey::Navigation::HomeLocation,

			// Patrol
			CAIKey::Patrol::bUsePatrol,
			CAIKey::Patrol::PatrolPath,
			CAIKey::Patrol::PatrolMode,
			CAIKey::Patrol::bPatrolReverse,
			CAIKey::Patrol::PatrolLocation,
			CAIKey::Patrol::PatrolIndex,

			// Investigate
			CAIKey::Investigate::bUseInvestigate,
			CAIKey::Investigate::InvestigateDuration,
			CAIKey::Investigate::InvestigateMaxIndex,
			CAIKey::Investigate::bCanInvestigate,
			CAIKey::Investigate::bIsInvestigating,
			CAIKey::Investigate::InvestigateLocation,
			CAIKey::Investigate::InvestigateIndex,

			// Chase
			CAIKey::Chase::ChaseOffsetRange,
			CAIKey::Chase::ChaseEnterBuffer,
			CAIKey::Chase::ChaseExitBuffer,

			// Alert
			CAIKey::Alert::bUseAlertStep,
			CAIKey::Alert::StepForwardDistance,
			CAIKey::Alert::StepSideDistance,
			CAIKey::Alert::bInAlertRange,
			CAIKey::Alert::AlertStepLocation,

			// Engage
			CAIKey::Engage::bShouldEngage,
			CAIKey::Engage::bCanCombatAction,
			CAIKey::Engage::bIsCombatAction,
			CAIKey::Engage::bInEngageRange,
			CAIKey::Engage::NextCombatActionTime,

			// Reaction
			CAIKey::Reaction::bIsActiveReaction,

			// Dead
			CAIKey::Dead::DeadState,
		};

		return keySpecs;
	}

	static bool ValidateKey(const UBlackboardData* InBlackboardAsset, const FAIBlackboardKeySpec& InKeySpec)
	{
		if (!IsValid(InBlackboardAsset)) return false;

		return InBlackboardAsset->GetKeyID(InKeySpec.KeyName) != FBlackboard::InvalidKey;
	}

	static bool ValidateRequiredKeys(const UBlackboardData* InBlackboardAsset)
	{
		if (!IsValid(InBlackboardAsset)) return false;

		TArray<FString> missingKeyMessages;

		for (const FAIBlackboardKeySpec& keySpec : GetKeySpecs())
		{
			if (!keySpec.bRequired) continue;

			const bool bValidKey = ValidateKey(InBlackboardAsset, keySpec);
			if (bValidKey) continue;

			missingKeyMessages.Add(FString::Printf(
				TEXT("%s:%s"),
				*keySpec.KeyName.ToString(),
				GetValueTypeName(keySpec.ValueType)));
		}

		if (missingKeyMessages.IsEmpty()) return true;

		const FString missingKeys = FString::Join(missingKeyMessages, TEXT(", "));
		ensureMsgf(false,
			TEXT("[AIKeyRegistry] Missing required Blackboard keys | Blackboard=%s | Missing=%s"),
			*GetNameSafe(InBlackboardAsset),
			*missingKeys);

		return false;
	}
}
