#include "AI/BehaviorTree/Services/CBTService_UpdateCombatContext.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTService_UpdateCombatContext::UCBTService_UpdateCombatContext()
{
	NodeName = "Update CombatContext";
	bNotifyTick = true;

	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UCBTService_UpdateCombatContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	APawn* ownerPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	AActor* target = Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor));

	if (IsValid(ownerPawn) && IsValid(target))
	{
		FVector ownerLocation = ownerPawn ? ownerPawn->GetActorLocation() : FVector::ZeroVector;
		FVector targetLocation = target ? target->GetActorLocation() : FVector::ZeroVector;
		
		float dist = FVector::Dist(ownerLocation, targetLocation);
		// TODO: DistanceToHome

		blackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToTarget, dist);
		blackboardComp->SetValueAsBool(CAIKey::Combat::bInRange, dist <= AttackRange);
		// TODO: bCanAttack

		return;
	}
	else // Invalid target
	{
		blackboardComp->ClearValue(CAIKey::Metric::DistanceToTarget);
		blackboardComp->SetValueAsBool(CAIKey::Combat::bInRange, false);
		// TODO: bCanAttack
	}
}
