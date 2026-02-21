#pragma once

#include "CoreMinimal.h"

namespace CAIKey
{
	namespace Targeting
	{
		static const FName TargetActor = "TargetActor";					// Object(Actor)
	}

	namespace State
	{
		static const FName AIStateType = "AIStateType";					// Enum(EAIStateType)
	}

	namespace Perception
	{
		static const FName bHasLOS = "bHasLOS";							// Bool
		static const FName LastSeenTime = "LastSeenTime";				// Float
		static const FName LastKnownLocation = "LastKnownLocation";		// Vector
		static const FName HomeLocation = "HomeLocation";				// Vector
	}

	namespace Metric
	{
		static const FName DistanceToTarget = "DistanceToTarget";		// Float
	}

	namespace Combat
	{
		// Condition
		static const FName bIsInCombatRange = "bIsInCombatRange";		// Bool
		static const FName bCanEngageTarget = "bCanEngageTarget";		// Bool

		// State
		static const FName bIsEncounterActive = "bIsEncounterActive";	// Bool
		static const FName bIsEngagementActive = "bIsEngagementActive";	// Bool
	}

	namespace Reaction
	{
		// State
		static const FName bIsHitReacting = "bIsHitReacting";			// Bool
	}

	namespace Lifecycle
	{
		// State
		static const FName bIsDead = "bIsDead";							// Bool
	}
}
