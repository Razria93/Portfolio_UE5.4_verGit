#include "AI/BehaviorTree/Service/CBTService_UpdateAIIntentState.h"

#include "ProjectGlobal.h"

#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"
#include "Controller/CAIController.h"
#include "Core/Profiling/CAIBehaviorTreeProfiling.h"
#include "Type/CStateTypes.h"
#include "Type/CHealthTypes.h"
#include "Type/CEngageAssignmentTypes.h"
#include "AI/Blackboard/CAIKey.h"
#include "AI/Blackboard/CAIBlackboardValueHelper.h"

#include "ProfilingDebugging/CsvProfiler.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

// Lifecycle

UCBTService_UpdateAIIntentState::UCBTService_UpdateAIIntentState()
{
	NodeName = "Update AI Intent State";
	bNotifyTick = true;

	Interval = CBTServiceIntervalHelper::GetDefaultAIIntentStateInterval();
	RandomDeviation = CBTServiceIntervalHelper::GetDefaultRandomDeviation();
}

void UCBTService_UpdateAIIntentState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_BT_UpdateAIIntentState);
	FAIBehaviorTreeProfiling::RecordUpdateAIIntentStateTick();
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	const float currentTime = world->GetTimeSeconds();

	const EAIIntentState nextAIIntentState = DecideNextAIIntentState(blackboardComp, currentTime);

	if (ChangeAIIntentState(blackboardComp, nextAIIntentState))
	{
		ACAIController* aiOwner = Cast<ACAIController>(OwnerComp.GetAIOwner());
		if (IsValid(aiOwner))
		{
			aiOwner->RefreshRuntimeLODTierFromBlackboard();
		}
	}
}

// Intent Decision

EAIIntentState UCBTService_UpdateAIIntentState::DecideNextAIIntentState(UBlackboardComponent* InBlackboard, float InCurrentTime)
{
	// Absolute states override contextual intent decisions.
	const EDeadState deadState = static_cast<EDeadState>(InBlackboard->GetValueAsEnum(CAIKey::Dead::DeadState.KeyName));
	const bool bIsActiveReaction = InBlackboard->GetValueAsBool(CAIKey::Reaction::bIsActiveReaction.KeyName);
	const bool bIsCombatAction = InBlackboard->GetValueAsBool(CAIKey::Engage::bIsCombatAction.KeyName);

	if (deadState != EDeadState::Alive)
		return EAIIntentState::Dead;

	if (bIsActiveReaction)
		return EAIIntentState::HitReact;

	// Keep Engage while current attack action is still active.
	if (bIsCombatAction)
		return EAIIntentState::Engage;

	// Gather blackboard context for intent selection.
	AActor* target = Cast<AActor>(InBlackboard->GetValueAsObject(CAIKey::CombatTarget::Actor.KeyName));
	AActor* perceivedTarget = Cast<AActor>(InBlackboard->GetValueAsObject(CAIKey::Perception::PerceivedTargetActor.KeyName));

	const bool bHasTarget = IsValid(target);
	const bool bHasLOS = InBlackboard->GetValueAsBool(CAIKey::Perception::bHasLOS.KeyName);
	const bool bHasAwareness = IsValid(perceivedTarget) || bHasLOS;

	const bool bUseInvestigate = InBlackboard->GetValueAsBool(CAIKey::Investigate::bUseInvestigate.KeyName);
	const bool bShouldInvestigate = InBlackboard->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName);
	const bool bIsInvestigating = InBlackboard->GetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName);

	const bool bInAlertRange = InBlackboard->GetValueAsBool(CAIKey::Alert::bInAlertRange.KeyName);
	const ECombatRole combatRole = static_cast<ECombatRole>(InBlackboard->GetValueAsEnum(CAIKey::Engage::CombatRole.KeyName));

	// No awareness: investigate only when requested or already active.
	if (!bHasAwareness)
	{
		if (bUseInvestigate && (bShouldInvestigate || bIsInvestigating)) return EAIIntentState::Investigate;
		return EAIIntentState::Idle;
	}

	// Aware + no combat role: observe only.
	if (combatRole == ECombatRole::None) return EAIIntentState::Observe;

	// Aware + combat role: remember Engage as investigate candidate.
	if (combatRole == ECombatRole::Engage)
	{
		CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboard, CAIKey::Investigate::bShouldInvestigate.KeyName, true);
	}

	// Aware + combat role: movement/combat state.
	if (!bInAlertRange) return EAIIntentState::Chase;

	if (combatRole == ECombatRole::Engage) return EAIIntentState::Engage;
	if (combatRole == ECombatRole::Alert) return EAIIntentState::Alert;

	// Aware fallback.
	return EAIIntentState::Observe;
}

// Intent Transition

bool UCBTService_UpdateAIIntentState::ChangeAIIntentState(UBlackboardComponent* InBlackboardComp, EAIIntentState InNextAIIntentState)
{
	const uint8 currentAIIntentState = static_cast<uint8>(InBlackboardComp->GetValueAsEnum(CAIKey::State::AIIntentState.KeyName));
	const uint8 nextAIIntentState = static_cast<uint8>(InNextAIIntentState);

	if (currentAIIntentState == nextAIIntentState) return false;

	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::State::AIIntentState.KeyName, nextAIIntentState);

	UpdateAIIntentStateTransition(InBlackboardComp, static_cast<EAIIntentState>(currentAIIntentState), static_cast<EAIIntentState>(nextAIIntentState));
	return true;
}

void UCBTService_UpdateAIIntentState::UpdateAIIntentStateTransition(UBlackboardComponent* InBlackboardComp, EAIIntentState InCurrentAIIntentState, EAIIntentState InNextAIIntentState)
{
	if (!IsValid(InBlackboardComp)) return;

	// Clear Engage context when leaving Engage unexpectedly.
	if (InCurrentAIIntentState == EAIIntentState::Engage && InNextAIIntentState != EAIIntentState::Engage)
	{
		CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Engage::bInEngageRange.KeyName, false);
		CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Engage::bCanCombatAction.KeyName, false);

		if (InNextAIIntentState == EAIIntentState::Dead || InNextAIIntentState == EAIIntentState::Idle)
		{
			InBlackboardComp->ClearValue(CAIKey::Engage::NextCombatActionTime.KeyName);
		}
	} 

	// Clear Investigate context when leaving Investigate unexpectedly.
	if (InCurrentAIIntentState == EAIIntentState::Investigate && InNextAIIntentState != EAIIntentState::Investigate)
	{
		CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Investigate::bShouldInvestigate.KeyName, false);
		CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Investigate::bIsInvestigating.KeyName, false);
		CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Investigate::bShouldEndInvestigate.KeyName, false);

		InBlackboardComp->ClearValue(CAIKey::Investigate::InvestigateLocation.KeyName);
		CAIBlackboardValueHelper::SetIntIfChanged(InBlackboardComp, CAIKey::Investigate::InvestigateIndex.KeyName, INDEX_NONE);
	}
}

// Lifecycle

void UCBTService_UpdateAIIntentState::ScheduleNextTick(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	SetNextTickTime(NodeMemory, CBTServiceIntervalHelper::GetAIIntentStateInterval(OwnerComp));
}
