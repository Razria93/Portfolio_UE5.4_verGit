#pragma once

#include "CoreMinimal.h"

namespace CAIKey
{
	namespace Targeting
	{
		static const FName TargetActor = "TargetActor";						// Object(Actor)
		static const FName TargetPriority = "TargetPriority";				// Float
	}

	namespace State
	{
		static const FName AIStateType = "AIStateType";						// Enum(EAIStateType)
	}

	namespace Perception
	{
		static const FName bHasLOS = "bHasLOS";								// Bool
		static const FName LastSeenTime = "LastSeenTime";					// Float
		static const FName LastKnownLocation = "LastKnownLocation";			// Vector
	}

	namespace Navigation
	{
		static const FName bReturnHome = "bReturnHome";						// Bool
		static const FName HomeLocation = "HomeLocation";					// Vector
	}

	namespace Patrol
	{
		static const FName bUsePatrol = "bUsePatrol";						// Bool
		static const FName bPatrolReverse = "bPatrolReverse";				// Bool
		static const FName PatrolPathActor = "PatrolPathActor";				// Object(ACPatrolPath)
		static const FName PatrolMode = "PatrolMode";						// Enum(EPatrolMode)
		static const FName PatrolLocation = "PatrolLocation";				// Vector
		static const FName PatrolIndex = "PatrolIndex";						// Int
	}

	namespace Metric
	{
		static const FName DistanceToTarget = "DistanceToTarget";			// Float
		static const FName DistanceToHome = "DistanceToHome";				// Float
	}

	namespace Combat
	{
		static const FName bInRange = "bInRange";							// Bool
		static const FName bCanAttack = "bCanAttack";						// Bool
	}

	namespace Reaction
	{
		static const FName bIsHitReacting = "bIsHitReacting";				// Bool
	}

	namespace Lifecycle
	{
		static const FName bIsDead = "bIsDead";								// Bool
	}
}
