#include "AI/BehaviorTree/Service/CBTService_UpdateAIState.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

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

EAIStateType UCBTService_UpdateAIState::DecideNextAIStateType(UBlackboardComponent* InBlackboard, float InCurrentTime) const
{
	// -----------------------------------------------------------------------------
	// 1. Absolute States
	// -----------------------------------------------------------------------------
	if (InBlackboard->GetValueAsBool(CAIKey::Dead::bIsDead))
		return EAIStateType::Dead;

	if (InBlackboard->GetValueAsBool(CAIKey::Reaction::bIsHitReacting))
		return EAIStateType::HitReact;

	// -----------------------------------------------------------------------------
	// 2.  Context
	// -----------------------------------------------------------------------------
	AActor* target = Cast<AActor>(InBlackboard->GetValueAsObject(CAIKey::Targeting::TargetActor));

	const bool bHasTarget = IsValid(target);
	const bool bHasLOS = InBlackboard->GetValueAsBool(CAIKey::Perception::bHasLOS);
	const bool bIsInvestigating = InBlackboard->GetValueAsBool(CAIKey::Investigate::bIsInvestigating);

	const bool bInAlertRange = InBlackboard->GetValueAsBool(CAIKey::Alert::bInAlertRange);
	const bool bShouldEngage = InBlackboard->GetValueAsBool(CAIKey::Combat::bShouldEngage);

	// -----------------------------------------------------------------------------
	// 3) Decide Next AIStateType
	// -----------------------------------------------------------------------------
	// 3-1. Invalid Target -> Idle
	if (!bHasTarget && !bIsInvestigating) return EAIStateType::Idle;

	// 3-2. Valid target But Invalid LOS
	if (!bHasLOS) return EAIStateType::Investigate;

	// 3-3. Valid Target and LOS But Out of Range
	if (!bInAlertRange) return EAIStateType::Chase;

	// 3-4. in Range But attack disable
	if (!bShouldEngage) return EAIStateType::Alert;

	// 3-5. in Range and Attackable
	return EAIStateType::Combat;
}

bool UCBTService_UpdateAIState::ChangeAIStateType(UBlackboardComponent* InBlackboardComp, EAIStateType InNextAIStateType) const
{
	const uint8 currentAIStateType = static_cast<uint8>(InBlackboardComp->GetValueAsEnum(CAIKey::State::AIStateType));
	const uint8 nextAIStateType = static_cast<uint8>(InNextAIStateType);

	if (currentAIStateType == nextAIStateType) return false;

	InBlackboardComp->SetValueAsEnum(CAIKey::State::AIStateType, nextAIStateType);
	return true;
}
