#pragma once

#include "CoreMinimal.h"
#include "AI/Blackboard/CAIKeyFactory.h"
#include "Type/CHealthTypes.h"
#include "Type/CStateTypes.h"
#include "Type/CEngageAssignmentTypes.h"

namespace CAIKey
{
	namespace Targeting
	{
		static const FAIBlackboardKeySpec TargetActor = CAIKeyFactory::FixedObjectNull(TEXT("TargetActor"));
		static const FAIBlackboardKeySpec TargetPriority = CAIKeyFactory::FixedInt(TEXT("TargetPriority"), INT_MAX);
	}

	namespace State
	{
		static const FAIBlackboardKeySpec AIIntentState = CAIKeyFactory::FixedEnum(TEXT("AIIntentState"), static_cast<uint8>(EAIIntentState::Idle));
	}

	namespace Perception
	{
		static const FAIBlackboardKeySpec bHasLOS = CAIKeyFactory::FixedBool(TEXT("bHasLOS"), false);
		static const FAIBlackboardKeySpec LastSeenTime = CAIKeyFactory::RuntimeFloat(TEXT("LastSeenTime"));
		static const FAIBlackboardKeySpec LastKnownLocation = CAIKeyFactory::RuntimeVector(TEXT("LastKnownLocation"));
	}

	namespace Metric
	{
		static const FAIBlackboardKeySpec DistanceToTarget = CAIKeyFactory::RuntimeFloat(TEXT("DistanceToTarget"));
		static const FAIBlackboardKeySpec DistanceToHome = CAIKeyFactory::RuntimeFloat(TEXT("DistanceToHome"));
	}

	namespace Navigation
	{
		static const FAIBlackboardKeySpec bReturnHome = CAIKeyFactory::FixedBool(TEXT("bReturnHome"), false);
		static const FAIBlackboardKeySpec HomeLocation = CAIKeyFactory::FromOwnerLocation(TEXT("HomeLocation"));
	}

	namespace Patrol
	{
		static const FAIBlackboardKeySpec bUsePatrol = CAIKeyFactory::CustomBool(TEXT("bUsePatrol"));
		static const FAIBlackboardKeySpec PatrolPath = CAIKeyFactory::CustomObject(TEXT("PatrolPath"));
		static const FAIBlackboardKeySpec PatrolMode = CAIKeyFactory::CustomEnum(TEXT("PatrolMode"));

		static const FAIBlackboardKeySpec bPatrolReverse = CAIKeyFactory::FixedBool(TEXT("bPatrolReverse"), false);
		static const FAIBlackboardKeySpec PatrolLocation = CAIKeyFactory::FromOwnerLocation(TEXT("PatrolLocation"));
		static const FAIBlackboardKeySpec PatrolIndex = CAIKeyFactory::FixedInt(TEXT("PatrolIndex"), -1);
	}

	namespace Investigate
	{
		static const FAIBlackboardKeySpec bUseInvestigate = CAIKeyFactory::CustomBool(TEXT("bUseInvestigate"));
		static const FAIBlackboardKeySpec InvestigateDuration = CAIKeyFactory::CustomFloat(TEXT("InvestigateDuration"));
		static const FAIBlackboardKeySpec InvestigateMaxIndex = CAIKeyFactory::CustomInt(TEXT("InvestigateMaxIndex"));

		static const FAIBlackboardKeySpec bShouldInvestigate = CAIKeyFactory::FixedBool(TEXT("bShouldInvestigate"), false);
		static const FAIBlackboardKeySpec bIsInvestigating = CAIKeyFactory::FixedBool(TEXT("bIsInvestigating"), false);
		static const FAIBlackboardKeySpec bShouldEndInvestigate = CAIKeyFactory::FixedBool(TEXT("bShouldEndInvestigate"), false);
		static const FAIBlackboardKeySpec InvestigateLocation = CAIKeyFactory::FromOwnerLocation(TEXT("InvestigateLocation"));
		static const FAIBlackboardKeySpec InvestigateIndex = CAIKeyFactory::FixedInt(TEXT("InvestigateIndex"), INDEX_NONE);
	}

	namespace Chase
	{
		static const FAIBlackboardKeySpec ChaseOffsetRange = CAIKeyFactory::CustomFloat(TEXT("ChaseOffsetRange"));
		static const FAIBlackboardKeySpec ChaseEnterBuffer = CAIKeyFactory::CustomFloat(TEXT("ChaseEnterBuffer"));
		static const FAIBlackboardKeySpec ChaseExitBuffer = CAIKeyFactory::CustomFloat(TEXT("ChaseExitBuffer"));
	}

	namespace Alert
	{
		static const FAIBlackboardKeySpec bUseAlertStep = CAIKeyFactory::CustomBool(TEXT("bUseAlertStep"));
		static const FAIBlackboardKeySpec StepForwardDistance = CAIKeyFactory::CustomFloat(TEXT("StepForwardDistance"));
		static const FAIBlackboardKeySpec StepSideDistance = CAIKeyFactory::CustomFloat(TEXT("StepSideDistance"));

		static const FAIBlackboardKeySpec bInAlertRange = CAIKeyFactory::FixedBool(TEXT("bInAlertRange"), false);
		static const FAIBlackboardKeySpec AlertStepLocation = CAIKeyFactory::FromOwnerLocation(TEXT("AlertStepLocation"));
	}

	namespace Engage
	{
		static const FAIBlackboardKeySpec CombatRole = CAIKeyFactory::FixedEnum(TEXT("CombatRole"), static_cast<uint8>(ECombatRole::None));
		static const FAIBlackboardKeySpec bShouldEngage = CAIKeyFactory::FixedBool(TEXT("bShouldEngage"), false);
		static const FAIBlackboardKeySpec bCanCombatAction = CAIKeyFactory::FixedBool(TEXT("bCanCombatAction"), false);
		
		static const FAIBlackboardKeySpec bIsCombatAction = CAIKeyFactory::FixedBool(TEXT("bIsCombatAction"), false);
		static const FAIBlackboardKeySpec bInEngageRange = CAIKeyFactory::FixedBool(TEXT("bInEngageRange"), false);
		static const FAIBlackboardKeySpec NextCombatActionTime = CAIKeyFactory::FixedFloat(TEXT("NextCombatActionTime"), -1.f);
	}
	
	namespace Reaction
	{
		static const FAIBlackboardKeySpec bIsActiveReaction = CAIKeyFactory::FixedBool(TEXT("bIsActiveReaction"), false);
	}

	namespace Dead
	{
		static const FAIBlackboardKeySpec DeadState = CAIKeyFactory::FixedEnum(TEXT("DeadState"), static_cast<uint8>(EDeadState::Alive));
	}
}
