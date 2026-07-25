#pragma once

#include "CoreMinimal.h"
#include "Type/CStateTypes.h"
#include "Type/CEngageAssignmentTypes.h"

class UBlackboardComponent;

enum class EAIRuntimeLODTier : uint8
{
	CombatCritical,
	CombatSupport,
	Awareness,
	Background,
	Dormant
};

struct FAIRuntimeLODTierContext
{
	EAIIntentState AIIntentState = EAIIntentState::Idle;
	ECombatRole CombatRole = ECombatRole::None;
	bool bHasTargetAwareness = false;
	bool bDormantCandidate = false;
};

class FAIRuntimeLODTierResolver
{
public:
	// Context Build
	static FAIRuntimeLODTierContext BuildContext(const UBlackboardComponent& InBlackboardComp);

	// Tier Resolve
	static EAIRuntimeLODTier ResolveTier(const UBlackboardComponent& InBlackboardComp);
	static EAIRuntimeLODTier ResolveTier(const FAIRuntimeLODTierContext& InContext);

	// String Conversion
	static const TCHAR* LexToString(EAIRuntimeLODTier InTier);
};
