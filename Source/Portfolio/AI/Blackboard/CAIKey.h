#pragma once

#include "CoreMinimal.h"

namespace CAIKey
{
	namespace Targeting
	{
		static const FName TargetActor = "TargetActor";					// Object(Actor)
	}

	namespace StateType
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
		// State
		static const FName bIsEncounterActive = "bIsEncounterActive";	// Bool
		static const FName bIsEngagementActive = "bIsEngagementActive";	// Bool
		static const FName bIsInCombatRange = "bIsInCombatRange";		// Bool

		// Able
		static const FName bCanEngageTarget = "bCanEngageTarget";		// Bool
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
