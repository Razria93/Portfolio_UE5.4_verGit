#include "AI/BehaviorTree/Service/CBTService_UpdateAIContext.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Controller/CAIController.h"
#include "Component/CReactionComponent.h"
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
		ClearPerceptionContext(blackboardComp);
		ClearHomeMetricContext(blackboardComp);
		ClearCombatMetricContext(blackboardComp);
		ClearCombatAssignmentContext(blackboardComp);
		ClearReactionContext(blackboardComp);

		return;
	}

	FAIContext aiContext; // OutParameter

	// Based OwnerPawn
	EContextBuildResult homeResult = ComputeHomeMetricContext(ownerPawn, blackboardComp, aiContext);

	if (homeResult == EContextBuildResult::Success)
		UpdateHomeMetricContext(blackboardComp, aiContext);
	else
		ClearHomeMetricContext(blackboardComp);

	EContextBuildResult reactionResult = ComputeReactionContext(ownerPawn, blackboardComp, aiContext);

	if (reactionResult == EContextBuildResult::Success)
		UpdateReactionContext(blackboardComp, aiContext);
	else
		ClearReactionContext(blackboardComp);

	// Based Perception
	EContextBuildResult buildResult = BuildPerceptionContext(ownerPawn, aiContext);

	if (buildResult != EContextBuildResult::Success)
	{
		ClearPerceptionContext(blackboardComp);
		ClearCombatMetricContext(blackboardComp);
		ClearCombatAssignmentContext(blackboardComp);

		return;
	}

	UpdatePerceptionContext(blackboardComp, aiContext);

	// Based TargetActor
	EContextBuildResult combatMetricResult = ComputeCombatMetricContext(ownerPawn, blackboardComp, aiContext);

	if (combatMetricResult == EContextBuildResult::Success)
		UpdateCombatMetricContext(blackboardComp, aiContext);
	else
		ClearCombatMetricContext(blackboardComp);

	EContextBuildResult combatAssignmentResult = ComputeCombatAssignmentContext(ownerPawn, blackboardComp, aiContext);

	if (combatAssignmentResult == EContextBuildResult::Success)
		UpdateCombatAssignmentContext(blackboardComp, aiContext);
	else
		ClearCombatAssignmentContext(blackboardComp);
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

EContextBuildResult UCBTService_UpdateAIContext::ComputeCombatMetricContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
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

EContextBuildResult UCBTService_UpdateAIContext::ComputeCombatAssignmentContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;
	if (!IsValid(InOutAIContext.TargetActor)) return EContextBuildResult::NoData;

	ACAIController* aiController = Cast<ACAIController>(InOwnerPawn->GetController());
	if (!IsValid(aiController)) return EContextBuildResult::Error;

	UCWorldSubsystem_CombatEngage* subsystem = InOwnerPawn->GetWorld()->GetSubsystem<UCWorldSubsystem_CombatEngage>();
	if (!IsValid(subsystem)) return EContextBuildResult::Error;

	// Previous Context
	const FCombatAssignmentContext prevAssignmentContext = subsystem->GetAssignment(aiController);

	FCombatRequestContext requestContext;
	requestContext.RequestController = aiController;
	requestContext.TargetActor = InOutAIContext.TargetActor;
	requestContext.TargetPriority = InOutAIContext.TargetPriority;
	requestContext.DistanceToTarget = InOutAIContext.DistanceToTarget;
	requestContext.bWasEngaged = prevAssignmentContext.IsValidAssignment() && prevAssignmentContext.CombatRole == ECombatRole::Engage;

	subsystem->SubmitRequest(requestContext);

	// Current Context
	const FCombatAssignmentContext curAssignmentContext = subsystem->GetAssignment(aiController);

	InOutAIContext.bShouldEngage = curAssignmentContext.IsValidAssignment() && curAssignmentContext.CombatRole == ECombatRole::Engage;

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeReactionContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	UCReactionComponent* reactionComp = Cast<UCReactionComponent>(InOwnerPawn->GetComponentByClass(UCReactionComponent::StaticClass()));

	if (!IsValid(reactionComp))
	{
		InOutAIContext.bHasPendingReaction = false;
		InOutAIContext.bHasActiveReaction = false;
		InOutAIContext.bIsHitReacting = false;

		return EContextBuildResult::NoData;
	}

	InOutAIContext.bHasPendingReaction = reactionComp->HasPendingReactionContext();
	InOutAIContext.bHasActiveReaction = reactionComp->HasActiveReactionContext();
	InOutAIContext.bIsHitReacting = InOutAIContext.bHasActiveReaction;	// [Policy]  AIState HitReact is driven by active reaction

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

void UCBTService_UpdateAIContext::UpdateCombatMetricContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsFloat(CAIKey::Metric::DistanceToTarget, InAIContext.DistanceToTarget);
	InBlackboardComp->SetValueAsBool(CAIKey::Alert::bInAlertRange, InAIContext.bInAlertRange);

}

void UCBTService_UpdateAIContext::UpdateCombatAssignmentContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsBool(CAIKey::Combat::bShouldEngage, InAIContext.bShouldEngage);
}

void UCBTService_UpdateAIContext::UpdateReactionContext(UBlackboardComponent* InBlackboardComp, FAIContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsBool(CAIKey::Reaction::bHasPendingReaction, InAIContext.bHasPendingReaction);
	InBlackboardComp->SetValueAsBool(CAIKey::Reaction::bHasActiveReaction, InAIContext.bHasActiveReaction);
	InBlackboardComp->SetValueAsBool(CAIKey::Reaction::bIsHitReacting, InAIContext.bIsHitReacting);
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

void UCBTService_UpdateAIContext::ClearCombatMetricContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Metric::DistanceToTarget);
	InBlackboardComp->ClearValue(CAIKey::Alert::bInAlertRange);
}

void UCBTService_UpdateAIContext::ClearCombatAssignmentContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Combat::bShouldEngage);
}

void UCBTService_UpdateAIContext::ClearReactionContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsBool(CAIKey::Reaction::bHasPendingReaction, false);
	InBlackboardComp->SetValueAsBool(CAIKey::Reaction::bHasActiveReaction, false);
	InBlackboardComp->SetValueAsBool(CAIKey::Reaction::bIsHitReacting, false);
}
