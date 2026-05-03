#pragma once

#include "CoreMinimal.h"

namespace CAIKey
{
	namespace Targeting
	{
		static const FName TargetActor = "TargetActor";							// Object(Actor)
		static const FName TargetPriority = "TargetPriority";					// Float
	}

	namespace State
	{
		static const FName AIIntentState = "AIIntentState";						// Enum(EAIIntentState)
	}

	namespace Perception
	{
		static const FName bHasLOS = "bHasLOS";									// Bool
		static const FName LastSeenTime = "LastSeenTime";						// Float
		static const FName LastKnownLocation = "LastKnownLocation";				// Vector
	}

	namespace Metric
	{
		static const FName DistanceToTarget = "DistanceToTarget";				// Float
		static const FName DistanceToHome = "DistanceToHome";					// Float
	}

	namespace Navigation
	{
		static const FName bReturnHome = "bReturnHome";							// Bool
		static const FName HomeLocation = "HomeLocation";						// Vector
	}

	namespace Patrol
	{
		static const FName bUsePatrol = "bUsePatrol";							// Bool
		static const FName PatrolPath = "PatrolPath";							// Object(ACPatrolPath)
		static const FName PatrolMode = "PatrolMode";							// Enum(EPatrolMode)

		static const FName bPatrolReverse = "bPatrolReverse";					// Bool
		static const FName PatrolLocation = "PatrolLocation";					// Vector
		static const FName PatrolIndex = "PatrolIndex";							// Int
	}

	namespace Investigate
	{
		static const FName bUseInvestigate = "bUseInvestigate";					// Bool
		static const FName InvestigateDuration = "InvestigateDuration";			// Float
		static const FName InvestigateMaxIndex = "InvestigateMaxIndex";			// Int

		static const FName bCanInvestigate = "bCanInvestigate";					// Bool
		static const FName bIsInvestigating = "bIsInvestigating";				// Bool
		static const FName InvestigateLocation = "InvestigateLocation";			// Vector
		static const FName InvestigateIndex = "InvestigateIndex";				// Int
	}

	namespace Chase
	{
		static const FName ChaseOffsetRange = "ChaseOffsetRange";				// Float
		static const FName ChaseEnterBuffer = "ChaseEnterBuffer";				// Float
		static const FName ChaseExitBuffer = "ChaseExitBuffer";					// Float
	}

	namespace Alert
	{
		static const FName bUseAlertStep = "bUseAlertStep";						// Bool
		static const FName StepForwardDistance = "StepForwardDistance";			// Float
		static const FName StepSideDistance = "StepSideDistance";				// Float

		static const FName bInAlertRange = "bInAlertRange";						// Bool
		static const FName AlertStepLocation = "AlertStepLocation";				// Vector
	}

	namespace Engage
	{
		static const FName bShouldEngage = "bShouldEngage";						// Bool	 (UpdateAIContext)
		static const FName bCanCombatAction = "bCanCombatAction";				// Bool	 (UpdateEngageContext)
		
		static const FName bIsCombatAction = "bIsCombatAction";					// Bool	 (OnActionTypeChanged)
		static const FName bInEngageRange = "bInEngageRange";					// Bool	 (UpdateAIContext)
		static const FName NextCombatActionTime = "NextCombatActionTime";		// Float (StartCombatAction)
	}
	
	namespace Reaction
	{
		static const FName bIsActiveReaction = "bIsActiveReaction";			// Bool
	}

	namespace Dead
	{
		static const FName DeadState = "DeadState";								// Enum(EDeadState)
	}
}
