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
	if (!IsValid(ownerPawn))
	{
		ClearPerceptionContext(blackboardComp);
		ClearCombatMetricContext(blackboardComp);
		ClearHomeMetricContext(blackboardComp);

		return;
	}

	FAIContext aiContext; // OutParameter
	EContextBuildResult buildResult = BuildPerceptionContext(ownerPawn, aiContext);

	if (buildResult == EContextBuildResult::Error)
	{
		ClearPerceptionContext(blackboardComp);
		ClearCombatMetricContext(blackboardComp);
		ClearHomeMetricContext(blackboardComp);

		return;
	}

	if (buildResult == EContextBuildResult::NoData)
	{
		ClearPerceptionContext(blackboardComp);
		ClearCombatMetricContext(blackboardComp);

		EContextBuildResult homeResult = ComputeHomeMetricContext(ownerPawn, blackboardComp, aiContext);

		if (homeResult == EContextBuildResult::Success)
			UpdateHomeMetricContext(blackboardComp, aiContext);
		else
			ClearHomeMetricContext(blackboardComp);
		
		return;
	}

	UpdatePerceptionContext(blackboardComp, aiContext);

	EContextBuildResult combatResult = ComputeCombatMetricContext(ownerPawn, blackboardComp, aiContext);

	if (combatResult == EContextBuildResult::Success)
		UpdateCombatMetricContext(blackboardComp, aiContext);
	else
		ClearCombatMetricContext(blackboardComp);
	
	EContextBuildResult homeResult = ComputeHomeMetricContext(ownerPawn, blackboardComp, aiContext);

	if (homeResult == EContextBuildResult::Success)
		UpdateHomeMetricContext(blackboardComp, aiContext);
	else
		ClearHomeMetricContext(blackboardComp);
}

EContextBuildResult UCBTService_UpdateAIContext::BuildPerceptionContext(APawn* InOwnerPawn, FAIContext& OutAIContext)
{
	ACAIController* aiController = Cast<ACAIController>(InOwnerPawn->GetController());
	if (!IsValid(aiController)) return EContextBuildResult::Error;

	FTargetData topData;

	const EPerceptionBuildResult Result = aiController->BuildPerceptionContext(topData);

	if (Result == EPerceptionBuildResult::Error) return EContextBuildResult::Error;
	if (Result == EPerceptionBuildResult::NoData) return EContextBuildResult::NoData;

	OutAIContext.TargetActor = topData.TargetActor;
	OutAIContext.TargetPriority = topData.TargetPriority;
	OutAIContext.bHasLOS = topData.bHasLOS;
	OutAIContext.LastSeenTime = topData.LastSeenTime;
	OutAIContext.LastKnownLocation = topData.LastKnownLocation;

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeCombatMetricContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;
	if (!IsValid(InOutAIContext.TargetActor)) return EContextBuildResult::NoData;

	float chaseOffsetRange = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseOffsetRange);
	float chaseEnterBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseEnterBuffer);
	float chaseExitBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseExitBuffer);

	bool bCanChase = InBlackboardComp->GetValueAsBool(CAIKey::Chase::bCanChase);

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector targetLocation = InOutAIContext.TargetActor->GetActorLocation();

	float dist_target = FVector::Dist(ownerLocation, targetLocation);

	float chaseEnterDist = chaseOffsetRange + chaseEnterBuffer;
	float chaseExitDist = FMath::Max(0.f, chaseOffsetRange - chaseExitBuffer);

	if (!bCanChase)
	{
		if (dist_target > chaseEnterDist) bCanChase = true;
	}
	else
	{
		if (dist_target <= chaseExitDist) bCanChase = false;
	}

	InOutAIContext.DistanceToTarget = dist_target;
	InOutAIContext.bCanChase = bCanChase;
	InOutAIContext.bInRange = dist_target <= AttackRange;

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeHomeMetricContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector homeLocation = InBlackboardComp->GetValueAsVector(CAIKey::Navigation::HomeLocation);

	float dist_home = FVector::Dist(ownerLocation, homeLocation);

	InOutAIContext.DistanceToHome = dist_home;
	InOutAIContext.bReturnHome = dist_home > MovableRange;

	return EContextBuildResult::Success;
}

void UCBTService_UpdateAIContext::UpdatePerceptionContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;
	if (!IsValid(InAIContext.TargetActor)) return; 

	InBlackboardComp->SetValueAsObject(CAIKey::Targeting::TargetActor, InAIContext.TargetActor);
	InBlackboardComp->SetValueAsInt(CAIKey::Targeting::TargetPriority, InAIContext.TargetPriority);
	InBlackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, InAIContext.bHasLOS);
	InBlackboardComp->SetValueAsFloat(CAIKey::Perception::LastSeenTime, InAIContext.LastSeenTime);
	InBlackboardComp->SetValueAsVector(CAIKey::Perception::LastKnownLocation, InAIContext.LastKnownLocation);
}

void UCBTService_UpdateAIContext::UpdateCombatMetricContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!InAIContext.IsValidContext()) return;
	if (!IsValid(InAIContext.TargetActor)) return;

	InBlackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToTarget, InAIContext.DistanceToTarget);
	InBlackboardComp->SetValueAsBool(CAIKey::Chase::bCanChase, InAIContext.bCanChase);
	InBlackboardComp->SetValueAsBool(CAIKey::Combat::bInRange, InAIContext.bInRange);
}

void UCBTService_UpdateAIContext::UpdateHomeMetricContext(class UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	InBlackboardComp->SetValueAsBool(CAIKey::Navigation::bReturnHome, InAIContext.bReturnHome);
	InBlackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToHome, InAIContext.DistanceToHome);
}

void UCBTService_UpdateAIContext::ClearPerceptionContext(UBlackboardComponent* InBlackboardComp)
{
	InBlackboardComp->ClearValue(CAIKey::Targeting::TargetActor);
	InBlackboardComp->ClearValue(CAIKey::Targeting::TargetPriority);
	InBlackboardComp->ClearValue(CAIKey::Perception::bHasLOS);
}

void UCBTService_UpdateAIContext::ClearCombatMetricContext(UBlackboardComponent* InBlackboardComp)
{
	InBlackboardComp->ClearValue(CAIKey::Metric::DistanceToTarget);
	InBlackboardComp->ClearValue(CAIKey::Chase::bCanChase);
	InBlackboardComp->ClearValue(CAIKey::Combat::bInRange);
}

void UCBTService_UpdateAIContext::ClearHomeMetricContext(UBlackboardComponent* InBlackboardComp)
{
	InBlackboardComp->ClearValue(CAIKey::Metric::DistanceToHome);
	InBlackboardComp->ClearValue(CAIKey::Navigation::bReturnHome);
}