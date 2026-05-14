#include "AI/BehaviorTree/Service/CBTService_UpdateAIContext.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Controller/CAIController.h"
#include "Component/CReactionComponent.h"
#include "Component/CHealthComponent.h"
#include "System/Combat/CWorldSubsystem_CombatEngage.h"

#include "AI/BlackBoard/CAIKey.h"
#include "Type/CAIStructure.h"
#include "Type/CWorldSubSystemStructure.h"

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
		ClearDeadContext(blackboardComp);
		ClearReactionContext(blackboardComp);
		ClearHomeMetricContext(blackboardComp);

		ClearPerceptionContext(blackboardComp);

		ClearAlertRangeContext(blackboardComp);
		ClearEngageAssignmentContext(blackboardComp);

		return;
	}

	FAIContext aiContext; // OutParameter

	// Based OwnerPawn
	EContextBuildResult deadResult = ComputeDeadContext(ownerPawn, blackboardComp, aiContext);

	if (deadResult == EContextBuildResult::Success)
		UpdateDeadContext(blackboardComp, aiContext);
	else
		ClearDeadContext(blackboardComp);

	EContextBuildResult reactionResult = ComputeReactionContext(ownerPawn, blackboardComp, aiContext);

	if (reactionResult == EContextBuildResult::Success)
		UpdateReactionContext(blackboardComp, aiContext);
	else
		ClearReactionContext(blackboardComp);


	EContextBuildResult homeResult = ComputeHomeMetricContext(ownerPawn, blackboardComp, aiContext);

	if (homeResult == EContextBuildResult::Success)
		UpdateHomeMetricContext(blackboardComp, aiContext);
	else
		ClearHomeMetricContext(blackboardComp);

	// Based Perception
	EContextBuildResult buildResult = BuildPerceptionContext(ownerPawn, aiContext);

	if (buildResult != EContextBuildResult::Success)
	{
		ClearPerceptionContext(blackboardComp);

		ClearAlertRangeContext(blackboardComp);
		ClearEngageAssignmentContext(blackboardComp);

		return;
	}

	UpdatePerceptionContext(blackboardComp, aiContext);

	// Based TargetActor
	EContextBuildResult engageMetricResult = ComputeAlertRangeContext(ownerPawn, blackboardComp, aiContext);

	if (engageMetricResult == EContextBuildResult::Success)
		UpdateAlertRangeContext(blackboardComp, aiContext);
	else
		ClearAlertRangeContext(blackboardComp);

	EContextBuildResult engageAssignmentResult = ComputeEngageAssignmentContext(ownerPawn, blackboardComp, aiContext);

	if (engageAssignmentResult == EContextBuildResult::Success)
		UpdateEngageAssignmentContext(blackboardComp, aiContext);
	else
		ClearEngageAssignmentContext(blackboardComp);
}

EContextBuildResult UCBTService_UpdateAIContext::BuildPerceptionContext(APawn* InOwnerPawn, FAIContext& OutAIContext)
{
	ACAIController* aiController = Cast<ACAIController>(InOwnerPawn->GetController());
	if (!IsValid(aiController)) return EContextBuildResult::Error;

	FTargetData topData; // OutParameter
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

EContextBuildResult UCBTService_UpdateAIContext::ComputeAlertRangeContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;
	if (!IsValid(InOutAIContext.TargetActor)) return EContextBuildResult::NoData;

	float chaseOffsetRange = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseOffsetRange);
	float chaseEnterBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseEnterBuffer);
	float chaseExitBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseExitBuffer);

	bool bInAlertRange = InBlackboardComp->GetValueAsBool(CAIKey::Alert::bInAlertRange);

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector targetLocation = InOutAIContext.TargetActor->GetActorLocation();

	float dist_target = FVector::Dist(ownerLocation, targetLocation);

	float alertOuterRange = chaseOffsetRange + chaseEnterBuffer;
	float alertInnerRange = FMath::Max(0.f, chaseOffsetRange - chaseExitBuffer);

	if (bInAlertRange)
	{
		if (dist_target > alertOuterRange) bInAlertRange = false;
	}
	else
	{
		if (dist_target <= alertInnerRange) bInAlertRange = true;
	}

	InOutAIContext.DistanceToTarget = dist_target;
	InOutAIContext.bInAlertRange = bInAlertRange;

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeEngageAssignmentContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;
	if (!IsValid(InOutAIContext.TargetActor)) return EContextBuildResult::NoData;

	ACAIController* aiController = Cast<ACAIController>(InOwnerPawn->GetController());
	if (!IsValid(aiController)) return EContextBuildResult::Error;

	UCWorldSubsystem_CombatEngage* subsystem = InOwnerPawn->GetWorld()->GetSubsystem<UCWorldSubsystem_CombatEngage>();
	if (!IsValid(subsystem)) return EContextBuildResult::Error;

	// [TODO]
	// Change from 'UCWorldSubsystem_CombatEngage' to 'CWorldSubsystem_EngageCoordinator'
	const FEngageAssignmentContext previousAssignmentContext = subsystem->GetAssignment(aiController); // Previous Context

	FEngageRequestContext requestContext;
	requestContext.RequestController = aiController;
	requestContext.TargetActor = InOutAIContext.TargetActor;
	requestContext.TargetPriority = InOutAIContext.TargetPriority;
	requestContext.DistanceToTarget = InOutAIContext.DistanceToTarget;
	requestContext.bWasEngaged = previousAssignmentContext.IsValidAssignment() && previousAssignmentContext.CombatRole == ECombatRole::Engage;

	subsystem->SubmitRequest(requestContext);

	const FEngageAssignmentContext curAssignmentContext = subsystem->GetAssignment(aiController); // Current Context

	InOutAIContext.bShouldEngage = curAssignmentContext.IsValidAssignment() && curAssignmentContext.CombatRole == ECombatRole::Engage;

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeReactionContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	UCReactionComponent* reactionComp = Cast<UCReactionComponent>(InOwnerPawn->GetComponentByClass(UCReactionComponent::StaticClass()));
	if (!IsValid(reactionComp)) return EContextBuildResult::NoData;

	InOutAIContext.bIsActiveReaction = reactionComp->IsActive();

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeDeadContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	UCHealthComponent* healthComp = Cast<UCHealthComponent>(InOwnerPawn->GetComponentByClass(UCHealthComponent::StaticClass()));
	if (!IsValid(healthComp)) return EContextBuildResult::NoData;

	InOutAIContext.DeadState = healthComp->GetDeadState();

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

void UCBTService_UpdateAIContext::UpdateHomeMetricContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsBool(CAIKey::Navigation::bReturnHome, InAIContext.bReturnHome);
	InBlackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToHome, InAIContext.DistanceToHome);
}

void UCBTService_UpdateAIContext::UpdateAlertRangeContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToTarget, InAIContext.DistanceToTarget);
	InBlackboardComp->SetValueAsBool(CAIKey::Alert::bInAlertRange, InAIContext.bInAlertRange);

}

void UCBTService_UpdateAIContext::UpdateEngageAssignmentContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsBool(CAIKey::Engage::bShouldEngage, InAIContext.bShouldEngage);
}

void UCBTService_UpdateAIContext::UpdateReactionContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsBool(CAIKey::Reaction::bIsActiveReaction, InAIContext.bIsActiveReaction);
}

void UCBTService_UpdateAIContext::UpdateDeadContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsEnum(CAIKey::Dead::DeadState, static_cast<uint8>(InAIContext.DeadState));
}

void UCBTService_UpdateAIContext::ClearPerceptionContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Targeting::TargetActor);
	InBlackboardComp->ClearValue(CAIKey::Targeting::TargetPriority);
	InBlackboardComp->ClearValue(CAIKey::Perception::bHasLOS);
}

void UCBTService_UpdateAIContext::ClearHomeMetricContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Metric::DistanceToHome);
	InBlackboardComp->ClearValue(CAIKey::Navigation::bReturnHome);
}

void UCBTService_UpdateAIContext::ClearAlertRangeContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Metric::DistanceToTarget);
	InBlackboardComp->ClearValue(CAIKey::Alert::bInAlertRange);
}

void UCBTService_UpdateAIContext::ClearEngageAssignmentContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Engage::bShouldEngage);
}

void UCBTService_UpdateAIContext::ClearReactionContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Reaction::bIsActiveReaction);
}

void UCBTService_UpdateAIContext::ClearDeadContext(UBlackboardComponent* InBlackboardComp)
{
	InBlackboardComp->SetValueAsEnum(CAIKey::Dead::DeadState, static_cast<uint8>(EDeadState::Alive));
}
