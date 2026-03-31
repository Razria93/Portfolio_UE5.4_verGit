#include "AI/BehaviorTree/Service/CBTService_UpdateAIState.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Component/CMovementComponent.h"

#include "Type/CStateStructure.h"
#include "Type/CHealthStructure.h"
#include "AI/BlackBoard/CAIKey.h"

UCBTService_UpdateAIState::UCBTService_UpdateAIState()
{
	NodeName = "Update AIState";
	bNotifyTick = true;

	Interval = 0.1f;
	RandomDeviation = 0.02f;
}

void UCBTService_UpdateAIState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	const float currentTime = world->GetTimeSeconds();

	const EAIStateType nextAIStateType = DecideNextAIStateType(blackboardComp, currentTime);

	ChangeAIStateType(blackboardComp, nextAIStateType);
}

EAIStateType UCBTService_UpdateAIState::DecideNextAIStateType(UBlackboardComponent* InBlackboard, float InCurrentTime)
{
	// -----------------------------------------------------------------------------
	// 1. Absolute States
	// -----------------------------------------------------------------------------
	const EDeadState deadState = static_cast<EDeadState>(InBlackboard->GetValueAsEnum(CAIKey::Dead::DeadState));
	const bool bHasPendingReaction = InBlackboard->GetValueAsBool(CAIKey::Reaction::bHasPendingReaction);
	const bool bHasActiveReaction = InBlackboard->GetValueAsBool(CAIKey::Reaction::bHasActiveReaction);
	const bool bIsAttacking = InBlackboard->GetValueAsBool(CAIKey::Engage::bIsAttacking);

	if (deadState != EDeadState::Alive)
		return EAIStateType::Dead;

	if (bHasPendingReaction || bHasActiveReaction)
		return EAIStateType::HitReact;

	// Keep Engage while current attack action is still active.
	if (bIsAttacking)
		return EAIStateType::Engage;

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
	// 3) Decide Next AIStateType
	// -----------------------------------------------------------------------------
	// 3-1. Invalid Target -> Idle.
	if (!bHasTarget && !bIsInvestigating) return EAIStateType::Idle;

	// 3-2. Valid target But Invalid LOS.
	if (!bHasLOS) return EAIStateType::Investigate;

	// 3-3. Valid Target and LOS But Out of Range.
	if (!bInAlertRange) return EAIStateType::Chase;

	// 3-4. in Range But attack disable.
	if (!bShouldEngage) return EAIStateType::Alert;

	// 3-5. in Range and Attackable.
	return EAIStateType::Engage;
}

bool UCBTService_UpdateAIState::ChangeAIStateType(UBlackboardComponent* InBlackboardComp, EAIStateType InNextAIStateType)
{
	const uint8 currentAIStateType = static_cast<uint8>(InBlackboardComp->GetValueAsEnum(CAIKey::State::AIStateType));
	const uint8 nextAIStateType = static_cast<uint8>(InNextAIStateType);

	if (currentAIStateType == nextAIStateType) return false;

	InBlackboardComp->SetValueAsEnum(CAIKey::State::AIStateType, nextAIStateType);

	UpdateAIStateTransition(InBlackboardComp, static_cast<EAIStateType>(currentAIStateType), static_cast<EAIStateType>(nextAIStateType));
	return true;
}

void UCBTService_UpdateAIState::UpdateAIStateTransition(UBlackboardComponent* InBlackboardComp, EAIStateType InCurrentAIStateType, EAIStateType InNextAIStateType)
{
	if (!IsValid(InBlackboardComp)) return;

	// Engage -> Non-Engage
	if (InCurrentAIStateType == EAIStateType::Engage && InNextAIStateType != EAIStateType::Engage)
	{
		InBlackboardComp->SetValueAsBool(CAIKey::Engage::bInEngageRange, false);
		InBlackboardComp->SetValueAsBool(CAIKey::Engage::bCanAttack, false);
		InBlackboardComp->SetValueAsBool(CAIKey::Engage::bIsAttacking, false);
		InBlackboardComp->SetValueAsInt(CAIKey::Engage::AttackIndex, INDEX_NONE);

		if (InNextAIStateType == EAIStateType::Dead || InNextAIStateType == EAIStateType::Idle)
		{
			InBlackboardComp->ClearValue(CAIKey::Engage::AttackableTime);
		}

		if (AAIController* aIController = Cast<AAIController>(InBlackboardComp->GetOwner()))
			if (APawn* pawn = aIController->GetPawn())
				if (UCMovementComponent* movementComp = pawn->FindComponentByClass<UCMovementComponent>())
				{
					movementComp->SetMove();
				}
	} 
}