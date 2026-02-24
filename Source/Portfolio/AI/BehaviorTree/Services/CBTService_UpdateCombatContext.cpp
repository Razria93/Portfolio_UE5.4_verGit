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
	if (!IsValid(ownerPawn)) return;

	FVector ownerLocation = ownerPawn ? ownerPawn->GetActorLocation() : FVector::ZeroVector;

	AActor* target = Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor));
	if (!IsValid(target))
	{
		blackboardComp->ClearValue(CAIKey::Metric::DistanceToTarget);
		blackboardComp->ClearValue(CAIKey::Combat::bInRange);
		return;
	}
	
	FVector targetLocation = target->GetActorLocation();
	float dist_target = FVector::Dist(ownerLocation, targetLocation);

	blackboardComp->SetValueAsBool(CAIKey::Combat::bInRange, dist_target <= AttackRange);
	blackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToTarget, dist_target);

	FVector homeLocation = blackboardComp->GetValueAsVector(CAIKey::Navigation::HomeLocation);
	float dist_home = FVector::Dist(ownerLocation, homeLocation);

	blackboardComp->SetValueAsBool(CAIKey::Navigation::bReturnHome, dist_home > MovableRange);
	blackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToHome, dist_home);
}
