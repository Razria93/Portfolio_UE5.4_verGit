#include "AI/BehaviorTree/Service/CBTService_UpdateAIIntentState.h"
#include "ProjectGlobal.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"

#include "Type/CStateStructure.h"
#include "Type/CWeaponStructure.h"
#include "Type/CHealthStructure.h"
#include "Type/CWorldSubSystemStructure.h"
#include "AI/Blackboard/CAIKey.h"
#include "AI/Blackboard/CAIBlackboardValueHelper.h"

UCBTService_UpdateAIIntentState::UCBTService_UpdateAIIntentState()
{
	NodeName = "Update AI Intent State";
	bNotifyTick = true;

	Interval = 0.2f;
	RandomDeviation = 0.f;
}

void UCBTService_UpdateAIIntentState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_BT_UpdateAIIntentState);
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	const float currentTime = world->GetTimeSeconds();

	const EAIIntentState nextAIIntentState = DecideNextAIIntentState(blackboardComp, currentTime);

	ChangeAIIntentState(blackboardComp, nextAIIntentState);
}

EAIIntentState UCBTService_UpdateAIIntentState::DecideNextAIIntentState(UBlackboardComponent* InBlackboard, float InCurrentTime)
{
	// -----------------------------------------------------------------------------
	// 1. Absolute States
	// -----------------------------------------------------------------------------
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

	// -----------------------------------------------------------------------------
	// 2.  Context
	// -----------------------------------------------------------------------------
	AActor* target = Cast<AActor>(InBlackboard->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName));

	const bool bHasTarget = IsValid(target);
	const bool bHasLOS = InBlackboard->GetValueAsBool(CAIKey::Perception::bHasLOS.KeyName);
	const bool bIsInvestigating = InBlackboard->GetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName);

	const bool bInAlertRange = InBlackboard->GetValueAsBool(CAIKey::Alert::bInAlertRange.KeyName);
	const ECombatRole combatRole = static_cast<ECombatRole>(InBlackboard->GetValueAsEnum(CAIKey::Engage::CombatRole.KeyName));

	// -----------------------------------------------------------------------------
	// 3) Decide Next AIIntentState
	// -----------------------------------------------------------------------------
	// 3-1. Invalid Target -> Idle.
	if (!bHasTarget && !bIsInvestigating) return EAIIntentState::Idle;

	// 3-2. Valid target But Invalid LOS.
	if (!bHasLOS) return EAIIntentState::Investigate;

	// 3-3. Valid Target and LOS But Out of Range.
	if (!bInAlertRange) return EAIIntentState::Chase;

	// 3-4. in Range and assigned by CombatEngage subsystem.
	if (combatRole == ECombatRole::Engage) return EAIIntentState::Engage;
	if (combatRole == ECombatRole::Alert) return EAIIntentState::Alert;

	// 3-5. Target is visible but this AI was not assigned to combat participation.
	return EAIIntentState::Idle;
}

bool UCBTService_UpdateAIIntentState::ChangeAIIntentState(UBlackboardComponent* InBlackboardComp, EAIIntentState InNextAIIntentState)
{
	const uint8 currentAIIntentState = static_cast<uint8>(InBlackboardComp->GetValueAsEnum(CAIKey::State::AIIntentState.KeyName));
	const uint8 nextAIIntentState = static_cast<uint8>(InNextAIIntentState);

	if (currentAIIntentState == nextAIIntentState) return false;

	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::State::AIIntentState.KeyName, nextAIIntentState);

	UpdateAIIntentStateTransition(InBlackboardComp, static_cast<EAIIntentState>(currentAIIntentState), static_cast<EAIIntentState>(nextAIIntentState));
	return true;
}

// [NOTE] Safety-net cleanup for unexpected State exit.
void UCBTService_UpdateAIIntentState::UpdateAIIntentStateTransition(UBlackboardComponent* InBlackboardComp, EAIIntentState InCurrentAIIntentState, EAIIntentState InNextAIIntentState)
{
	if (!IsValid(InBlackboardComp)) return;

	// Engage -> Non-Engage
	if (InCurrentAIIntentState == EAIIntentState::Engage && InNextAIIntentState != EAIIntentState::Engage)
	{
		CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Engage::bInEngageRange.KeyName, false);
		CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Engage::bCanCombatAction.KeyName, false);

		if (InNextAIIntentState == EAIIntentState::Dead || InNextAIIntentState == EAIIntentState::Idle)
		{
			InBlackboardComp->ClearValue(CAIKey::Engage::NextCombatActionTime.KeyName);
		}
	} 
}

void UCBTService_UpdateAIIntentState::ScheduleNextTick(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	SetNextTickTime(NodeMemory, CBTServiceIntervalHelper::GetAIIntentStateInterval(OwnerComp));
}
