#include "AI/RuntimeLOD/CAIRuntimeLODTierResolver.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Blackboard/CAIKey.h"

namespace
{
	EAIRuntimeLODTier ResolveTierFromIntentState(EAIIntentState InAIIntentState, bool bHasTargetAwareness)
	{
		switch (InAIIntentState)
		{
		case EAIIntentState::Dead:
		case EAIIntentState::HitReact:
		case EAIIntentState::Engage:
			return EAIRuntimeLODTier::CombatCritical;

		case EAIIntentState::Alert:
		case EAIIntentState::Chase:
		case EAIIntentState::Investigate:
			return EAIRuntimeLODTier::CombatSupport;

		case EAIIntentState::Observe:
			return EAIRuntimeLODTier::Awareness;

		case EAIIntentState::Patrol:
		case EAIIntentState::Idle:
			return EAIRuntimeLODTier::Background;

		case EAIIntentState::Max:
		default:
			return bHasTargetAwareness ? EAIRuntimeLODTier::Awareness : EAIRuntimeLODTier::Background;
		}
	}
}

FAIRuntimeLODTierContext FAIRuntimeLODTierResolver::BuildContext(const UBlackboardComponent& InBlackboardComp)
{
	FAIRuntimeLODTierContext context;
	context.AIIntentState = static_cast<EAIIntentState>(InBlackboardComp.GetValueAsEnum(CAIKey::State::AIIntentState.KeyName));
	context.CombatRole = static_cast<ECombatRole>(InBlackboardComp.GetValueAsEnum(CAIKey::Engage::CombatRole.KeyName));

	const AActor* targetActor = Cast<AActor>(InBlackboardComp.GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName));
	const bool bHasLOS = InBlackboardComp.GetValueAsBool(CAIKey::Perception::bHasLOS.KeyName);
	context.bHasTargetAwareness = IsValid(targetActor) || bHasLOS;

	return context;
}

EAIRuntimeLODTier FAIRuntimeLODTierResolver::ResolveTier(const UBlackboardComponent& InBlackboardComp)
{
	return ResolveTier(BuildContext(InBlackboardComp));
}

EAIRuntimeLODTier FAIRuntimeLODTierResolver::ResolveTier(const FAIRuntimeLODTierContext& InContext)
{
	if (InContext.bDormantCandidate) return EAIRuntimeLODTier::Dormant;

	switch (InContext.CombatRole)
	{
	case ECombatRole::Engage:
		return EAIRuntimeLODTier::CombatCritical;

	case ECombatRole::Alert:
		return EAIRuntimeLODTier::CombatSupport;

	case ECombatRole::None:
	default:
		return ResolveTierFromIntentState(InContext.AIIntentState, InContext.bHasTargetAwareness);
	}
}

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
