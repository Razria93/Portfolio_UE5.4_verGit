#include "AI/BehaviorTree/Service/CBTService_UpdateAIIntentState.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"

#include "Type/CStateStructure.h"
#include "Type/CWeaponStructure.h"
#include "Type/CHealthStructure.h"
#include "AI/BlackBoard/CAIKey.h"

UCBTService_UpdateAIIntentState::UCBTService_UpdateAIIntentState()
{
	NodeName = "Update AI Intent State";
	bNotifyTick = true;

	Interval = 0.2f;
	RandomDeviation = 0.f;
}

void UCBTService_UpdateAIIntentState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
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
	const EDeadState deadState = static_cast<EDeadState>(InBlackboard->GetValueAsEnum(CAIKey::Dead::DeadState));
	const bool bIsActiveReaction = InBlackboard->GetValueAsBool(CAIKey::Reaction::bIsActiveReaction);
	const bool bIsCombatAction = InBlackboard->GetValueAsBool(CAIKey::Engage::bIsCombatAction);

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
	AActor* target = Cast<AActor>(InBlackboard->GetValueAsObject(CAIKey::Targeting::TargetActor));

	const bool bHasTarget = IsValid(target);
	const bool bHasLOS = InBlackboard->GetValueAsBool(CAIKey::Perception::bHasLOS);
	const bool bIsInvestigating = InBlackboard->GetValueAsBool(CAIKey::Investigate::bIsInvestigating);

	const bool bInAlertRange = InBlackboard->GetValueAsBool(CAIKey::Alert::bInAlertRange);
	const bool bShouldEngage = InBlackboard->GetValueAsBool(CAIKey::Engage::bShouldEngage);

	// -----------------------------------------------------------------------------
	// 3) Decide Next AIIntentState
	// -----------------------------------------------------------------------------
	// 3-1. Invalid Target -> Idle.
	if (!bHasTarget && !bIsInvestigating) return EAIIntentState::Idle;

	// 3-2. Valid target But Invalid LOS.
	if (!bHasLOS) return EAIIntentState::Investigate;

	// 3-3. Valid Target and LOS But Out of Range.
	if (!bInAlertRange) return EAIIntentState::Chase;

	// 3-4. in Range But attack disable.
	if (!bShouldEngage) return EAIIntentState::Alert;

	// 3-5. in Range and Attackable.
	return EAIIntentState::Engage;
}

bool UCBTService_UpdateAIIntentState::ChangeAIIntentState(UBlackboardComponent* InBlackboardComp, EAIIntentState InNextAIIntentState)
{
	const uint8 currentAIIntentState = static_cast<uint8>(InBlackboardComp->GetValueAsEnum(CAIKey::State::AIIntentState));
	const uint8 nextAIIntentState = static_cast<uint8>(InNextAIIntentState);

	if (currentAIIntentState == nextAIIntentState) return false;

	InBlackboardComp->SetValueAsEnum(CAIKey::State::AIIntentState, nextAIIntentState);

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
		InBlackboardComp->SetValueAsBool(CAIKey::Engage::bInEngageRange, false);
		InBlackboardComp->SetValueAsBool(CAIKey::Engage::bCanCombatAction, false);

		if (InNextAIIntentState == EAIIntentState::Dead || InNextAIIntentState == EAIIntentState::Idle)
		{
			InBlackboardComp->ClearValue(CAIKey::Engage::NextCombatActionTime);
		}
	} 
}