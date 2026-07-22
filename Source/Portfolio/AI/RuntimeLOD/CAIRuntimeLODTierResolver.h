#pragma once

#include "CoreMinimal.h"
#include "Type/CStateStructure.h"
#include "Type/CWorldSubsystemStructure.h"

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
	static FAIRuntimeLODTierContext BuildContext(const UBlackboardComponent& InBlackboardComp);
	static EAIRuntimeLODTier ResolveTier(const UBlackboardComponent& InBlackboardComp);
	static EAIRuntimeLODTier ResolveTier(const FAIRuntimeLODTierContext& InContext);

	static const TCHAR* LexToString(EAIRuntimeLODTier InTier);
};
