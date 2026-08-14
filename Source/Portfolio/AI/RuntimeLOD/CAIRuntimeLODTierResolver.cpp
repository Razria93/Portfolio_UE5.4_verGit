#include "AI/RuntimeLOD/CAIRuntimeLODTierResolver.h"

#include "AI/Blackboard/CAIKey.h"

#include "BehaviorTree/BlackboardComponent.h"

namespace
{
	// Helper
	bool IsAlwaysCombatCriticalIntentState(EAIIntentState InAIIntentState)
	{
		switch (InAIIntentState)
		{
		case EAIIntentState::Dead:
		case EAIIntentState::HitReact:
			return true;

		case EAIIntentState::Max:
		default:
			return false;
		}
	}
}

// Context Build

FAIRuntimeLODTierContext FAIRuntimeLODTierResolver::BuildContext(const UBlackboardComponent& InBlackboardComp)
{
	FAIRuntimeLODTierContext context;
	context.AIIntentState = static_cast<EAIIntentState>(InBlackboardComp.GetValueAsEnum(CAIKey::State::AIIntentState.KeyName));
	context.CombatRole = static_cast<ECombatRole>(InBlackboardComp.GetValueAsEnum(CAIKey::Engage::CombatRole.KeyName));

	const AActor* targetActor = Cast<AActor>(InBlackboardComp.GetValueAsObject(CAIKey::Perception::PerceivedTargetActor.KeyName));
	const bool bHasLOS = InBlackboardComp.GetValueAsBool(CAIKey::Perception::bHasLOS.KeyName);
	context.bHasTargetAwareness = IsValid(targetActor) || bHasLOS;

	return context;
}

// Tier Resolve

EAIRuntimeLODTier FAIRuntimeLODTierResolver::ResolveTier(const UBlackboardComponent& InBlackboardComp)
{
	return ResolveTier(BuildContext(InBlackboardComp));
}

EAIRuntimeLODTier FAIRuntimeLODTierResolver::ResolveTier(const FAIRuntimeLODTierContext& InContext)
{
	if (InContext.bDormantCandidate) return EAIRuntimeLODTier::Dormant;
	if (IsAlwaysCombatCriticalIntentState(InContext.AIIntentState)) return EAIRuntimeLODTier::CombatCritical;

	switch (InContext.CombatRole)
	{
	case ECombatRole::Engage:
		return EAIRuntimeLODTier::CombatCritical;

	case ECombatRole::Alert:
		return EAIRuntimeLODTier::CombatSupport;

	case ECombatRole::None:
	default:
		return InContext.bHasTargetAwareness ? EAIRuntimeLODTier::Awareness : EAIRuntimeLODTier::Background;
	}
}

// String Conversion

const TCHAR* FAIRuntimeLODTierResolver::LexToString(EAIRuntimeLODTier InTier)
{
	switch (InTier)
	{
	case EAIRuntimeLODTier::CombatCritical:
		return TEXT("CombatCritical");

	case EAIRuntimeLODTier::CombatSupport:
		return TEXT("CombatSupport");

	case EAIRuntimeLODTier::Awareness:
		return TEXT("Awareness");

	case EAIRuntimeLODTier::Background:
		return TEXT("Background");

	case EAIRuntimeLODTier::Dormant:
	default:
		return TEXT("Dormant");
	}
}
