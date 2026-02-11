#include "AI/BehaviorTree/Services/CBTService_UpdateMetrics.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"
#include "GameFramework/Pawn.h"

UCBTService_UpdateMetrics::UCBTService_UpdateMetrics()
{
	NodeName = "Update Metrics";
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	TargetActorKey.SelectedKeyName = CAIKey::TargetActor;
	DistanceToTargetKey.SelectedKeyName = CAIKey::DistanceToTarget;
	IsInCombatKey.SelectedKeyName = CAIKey::IsInCombat;
}

void UCBTService_UpdateMetrics::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* blackboard = OwnerComp.GetBlackboardComponent();
	if (!blackboard) return;
	
	AActor* target = Cast<AActor>(blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	APawn* ownerPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	
	if (!target || !ownerPawn)
	{
		blackboard->SetValueAsBool(IsInCombatKey.SelectedKeyName, false);
		blackboard->ClearValue(DistanceToTargetKey.SelectedKeyName);
		return;
	}
	
	const float distanceToTarget = FVector::Dist(ownerPawn->GetActorLocation(), target->GetActorLocation());
	blackboard->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, distanceToTarget);
	blackboard->SetValueAsBool(IsInCombatKey.SelectedKeyName, true);
}
