#include "AI/BehaviorTree/Service/CBTService_UpdateAIContext.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Controller/CAIController.h"

#include "AI/BlackBoard/CAIKey.h"
#include "Type/CAIStructure.h"

UCBTService_UpdateAIContext::UCBTService_UpdateAIContext()
{
	NodeName = "Update AIContext";
	bNotifyTick = true;

	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UCBTService_UpdateAIContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	APawn* ownerPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!IsValid(ownerPawn)) return;

	FAIContext aiContext; // OutParameter
	if (!BuildPerceptionContext(ownerPawn, aiContext)) return;
	if (!ComputeMetricContext(ownerPawn, blackboardComp, aiContext)) return;

	UpdatePerceptionContext(blackboardComp, aiContext);
	UpdateCombatContext(blackboardComp, aiContext);
	UpdateNavigationContext(blackboardComp, aiContext);
}

bool UCBTService_UpdateAIContext::BuildPerceptionContext(APawn* InOwnerPawn, FAIContext& OutAIContext)
{
	ACAIController* aiController = Cast<ACAIController>(InOwnerPawn->GetController());
	if (!IsValid(aiController)) return false;

	FTargetData topData;
	if (!aiController->BuildPerceptionContext(topData) || !topData.IsValidData()) return false;

	OutAIContext.TargetActor = topData.TargetActor;
	OutAIContext.TargetPriority = topData.TargetPriority;
	OutAIContext.bHasLOS = topData.bHasLOS;
	OutAIContext.LastSeenTime = topData.LastSeenTime;
	OutAIContext.LastKnownLocation = topData.LastKnownLocation;

	return true;
}

bool UCBTService_UpdateAIContext::ComputeMetricContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
{
	if (!InOutAIContext.IsValidContext()) return false;
	if (!IsValid(InOutAIContext.TargetActor)) return false;

	FVector ownerLocation = InOwnerPawn ? InOwnerPawn->GetActorLocation() : FVector::ZeroVector;
	FVector targetLocation = InOutAIContext.TargetActor->GetActorLocation();
	FVector homeLocation = InBlackboardComp->GetValueAsVector(CAIKey::Navigation::HomeLocation);
	
	float dist_target = FVector::Dist(ownerLocation, targetLocation);
	float dist_home = FVector::Dist(ownerLocation, homeLocation);

	InOutAIContext.DistanceToTarget = dist_target;
	InOutAIContext.bInRange = dist_target <= AttackRange;
	InOutAIContext.DistanceToHome = dist_home;
	InOutAIContext.bReturnHome = dist_home > MovableRange;

	return true;
}

void UCBTService_UpdateAIContext::UpdatePerceptionContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	AActor* currentTarget = Cast<AActor>(InBlackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor));

	if (InAIContext.IsValidContext())
	{
		// Change TargetActor (Current != New)
		if (currentTarget != InAIContext.TargetActor)
		{
			InBlackboardComp->SetValueAsObject(CAIKey::Targeting::TargetActor, InAIContext.TargetActor);
			InBlackboardComp->SetValueAsInt(CAIKey::Targeting::TargetPriority, InAIContext.TargetPriority);
		}

		if (InAIContext.bHasLOS)
		{
			InBlackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, InAIContext.bHasLOS);
			InBlackboardComp->SetValueAsFloat(CAIKey::Perception::LastSeenTime, InAIContext.LastSeenTime);
			InBlackboardComp->SetValueAsVector(CAIKey::Perception::LastKnownLocation, InAIContext.LastKnownLocation);
		}
		else // bHasLOS == false
		{
			InBlackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, InAIContext.bHasLOS);
		}

		return;
	}
	else
	{
		InBlackboardComp->ClearValue(CAIKey::Targeting::TargetActor);
		InBlackboardComp->ClearValue(CAIKey::Targeting::TargetPriority);
		InBlackboardComp->ClearValue(CAIKey::Perception::bHasLOS);
		InBlackboardComp->ClearValue(CAIKey::Perception::LastSeenTime);
		InBlackboardComp->ClearValue(CAIKey::Perception::LastKnownLocation);

		return;
	}

}

void UCBTService_UpdateAIContext::UpdateCombatContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!InAIContext.IsValidContext()) return;
	if (!IsValid(InAIContext.TargetActor))
	{
		InBlackboardComp->ClearValue(CAIKey::Metric::DistanceToTarget);
		InBlackboardComp->ClearValue(CAIKey::Combat::bInRange);
		return;
	}

	InBlackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToTarget, InAIContext.DistanceToTarget);
	InBlackboardComp->SetValueAsBool(CAIKey::Combat::bInRange, InAIContext.bInRange);
}

void UCBTService_UpdateAIContext::UpdateNavigationContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	InBlackboardComp->SetValueAsBool(CAIKey::Navigation::bReturnHome, InAIContext.bReturnHome);
	InBlackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToHome, InAIContext.DistanceToHome);
}
