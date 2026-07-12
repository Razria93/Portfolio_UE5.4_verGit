#pragma once

#include "CoreMinimal.h"
#include "Type/CStateStructure.h"
#include "Type/CWorldSubSystemStructure.h"

enum class EAIStateRuntimeLODTier : uint8
{
	Engage,
	Alert,
	Observe,
	Idle,
	Dormant
};

struct FAIStateRuntimeLODContext
{
	EAIIntentState AIIntentState = EAIIntentState::Idle;
	ECombatRole CombatRole = ECombatRole::None;
	bool bHasTargetAwareness = false;
	bool bDormantCandidate = false;
};

class FAIStateRuntimeLODPolicy
{
public:
	static int32 GetStatePolicyMode();
	static bool IsStatePolicyEnabled();

	static EAIStateRuntimeLODTier ResolveTier(const FAIStateRuntimeLODContext& InContext);
	static void RecordResolvedTierForProfiling(EAIStateRuntimeLODTier InTier);

	static const TCHAR* LexToString(EAIStateRuntimeLODTier InTier);
};
