#include "AI/BehaviorTree/Service/CBTService_UpdateAIContext.h"

#include "ProjectGlobal.h"

#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"
#include "Controller/CAIController.h"
#include "Component/CReactionComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CCombatTargetComponent.h"
#include "Character/Enemy/CEnemy.h"
#include "System/Combat/CWorldSubsystem_CombatEngage.h"
#include "AI/Blackboard/CAIKey.h"
#include "AI/Blackboard/CAIBlackboardValueHelper.h"
#include "Core/Debug/FAICombatBTDebug.h"
#include "Core/Profiling/CAIBehaviorTreeProfiling.h"
#include "Type/CAITypes.h"
#include "Type/CEngageAssignmentTypes.h"

#include "ProfilingDebugging/CsvProfiler.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

// Lifecycle

UCBTService_UpdateAIContext::UCBTService_UpdateAIContext()
{
	NodeName = "Update AIContext";
	bNotifyTick = true;

	Interval = CBTServiceIntervalHelper::GetDefaultAIContextInterval();
	RandomDeviation = CBTServiceIntervalHelper::GetDefaultRandomDeviation();
}

void UCBTService_UpdateAIContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_BT_UpdateAIContext);
	FAIBehaviorTreeProfiling::RecordUpdateAIContextTick();
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		FAICombatBTDebug::RecordAIContextClearedForAudit(OwnerComp.GetAIOwner(), nullptr, nullptr, TEXT("AIContext"), TEXT("MissingBlackboard"));
		return;
	}

	ACAIController* aiOwner = Cast<ACAIController>(OwnerComp.GetAIOwner());
	APawn* ownerPawn = IsValid(aiOwner) ? aiOwner->GetPawn() : nullptr;
	if (!IsValid(ownerPawn))
	{
		AActor* targetActor = Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName));
		FAICombatBTDebug::RecordAIContextClearedForAudit(aiOwner, ownerPawn, targetActor, TEXT("AIContext"), TEXT("MissingOwnerPawn"));

		ClearDeadContext(blackboardComp);
		ClearReactionContext(blackboardComp);
		ClearHomeMetricContext(blackboardComp);

		ClearPerceptionContext(blackboardComp);
		ClearCombatTargetProjection(blackboardComp);

		ClearAlertRangeContext(blackboardComp);
		ClearEngageAssignmentContext(blackboardComp);

		if (IsValid(aiOwner))
		{
			aiOwner->RefreshRuntimeLODTierFromBlackboard();
		}

		return;
	}

	FAIBlackboardUpdateContext aiContext;

	// Compute owner-pawn based context first.
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

	// Perception only produces a candidate; it never commits Combat Target state.
	EContextBuildResult buildResult = BuildPerceptionContext(ownerPawn, aiContext);
	if (buildResult == EContextBuildResult::Success)
	{
		UpdatePerceptionContext(blackboardComp, aiContext);
	}
	else
	{
		FAICombatBTDebug::RecordAIContextClearedForAudit(aiOwner, ownerPawn, nullptr, TEXT("PerceptionContext"), *UEnum::GetValueAsString(buildResult));
		ClearPerceptionContext(blackboardComp);
	}

	const EContextBuildResult combatTargetResult = BuildCombatTargetContext(ownerPawn, aiContext);
	if (combatTargetResult == EContextBuildResult::Success)
	{
		UpdateCombatTargetProjection(blackboardComp, aiContext);
		aiOwner->RecordBlackboardTargetSetForAudit(aiContext.CombatTargetActor);
	}
	else
	{
		ClearCombatTargetProjection(blackboardComp);
	}

	// Alert remains perception-based; Engage uses only committed Combat Target state.
	EContextBuildResult engageMetricResult = ComputeAlertRangeContext(ownerPawn, blackboardComp, aiContext);

	if (engageMetricResult == EContextBuildResult::Success)
		UpdateAlertRangeContext(blackboardComp, aiContext);
	else
		ClearAlertRangeContext(blackboardComp);

	EContextBuildResult engageAssignmentResult = ComputeEngageAssignmentContext(ownerPawn, blackboardComp, aiContext);

	if (engageAssignmentResult == EContextBuildResult::Success)
		UpdateEngageAssignmentContext(blackboardComp, aiContext);
	else
	{
		ClearEngageAssignmentContext(blackboardComp);
	}

	aiOwner->RefreshRuntimeLODTierFromBlackboard();
}

// Context Build

EContextBuildResult UCBTService_UpdateAIContext::BuildPerceptionContext(APawn* InOwnerPawn, FAIBlackboardUpdateContext& OutAIContext)
{
	ACAIController* aiController = Cast<ACAIController>(InOwnerPawn->GetController());
	if (!IsValid(aiController)) return EContextBuildResult::Error;

	FTargetPerceptionState topState;
	const EPerceptionBuildResult Result = aiController->BuildPerceptionContext(topState);

	if (Result == EPerceptionBuildResult::Error) return EContextBuildResult::Error;
	if (Result == EPerceptionBuildResult::NoData) return EContextBuildResult::NoData;

	OutAIContext.PerceivedTargetActor = topState.TargetActor;
	OutAIContext.TargetPriority = topState.TargetPriority;
	OutAIContext.bHasLOS = topState.bHasLOS;
	OutAIContext.LastSeenTime = topState.LastSeenTime;
	OutAIContext.LastKnownLocation = topState.LastKnownLocation;

	aiController->RecordPerceptionContextBuiltForAudit(topState.TargetActor);

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::BuildCombatTargetContext(APawn* InOwnerPawn, FAIBlackboardUpdateContext& OutAIContext) const
{
	const ACEnemy* enemy = Cast<ACEnemy>(InOwnerPawn);
	const UCCombatTargetComponent* combatTargetComp = IsValid(enemy) ? enemy->GetCombatTargetComp() : nullptr;
	if (!IsValid(combatTargetComp)) return EContextBuildResult::Error;

	const FCombatTargetSnapshot snapshot = combatTargetComp->GetCombatTargetSnapshot();
	if (!IsValid(snapshot.TargetActor)) return EContextBuildResult::NoData;

	OutAIContext.CombatTargetActor = snapshot.TargetActor;
	OutAIContext.CombatTargetRevision = snapshot.Revision;
	return EContextBuildResult::Success;
}

// Context Compute

EContextBuildResult UCBTService_UpdateAIContext::ComputeHomeMetricContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext) const
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector homeLocation = InBlackboardComp->GetValueAsVector(CAIKey::Navigation::HomeLocation.KeyName);

	float distanceToHome = FVector::Dist(ownerLocation, homeLocation);

	InOutAIContext.DistanceToHome = distanceToHome;
	InOutAIContext.bReturnHome = distanceToHome > MovableRange;

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeAlertRangeContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext) const
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;
	if (!IsValid(InOutAIContext.PerceivedTargetActor)) return EContextBuildResult::NoData;

	float chaseOffsetRange = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseOffsetRange.KeyName);
	float chaseEnterBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseEnterBuffer.KeyName);
	float chaseExitBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseExitBuffer.KeyName);

	bool bInAlertRange = InBlackboardComp->GetValueAsBool(CAIKey::Alert::bInAlertRange.KeyName);

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector targetLocation = InOutAIContext.PerceivedTargetActor->GetActorLocation();

	float distanceToTarget = FVector::Dist(ownerLocation, targetLocation);

	float alertOuterRange = chaseOffsetRange + chaseEnterBuffer;
	float alertInnerRange = FMath::Max(0.f, chaseOffsetRange - chaseExitBuffer);

	if (bInAlertRange)
	{
		if (distanceToTarget > alertOuterRange) bInAlertRange = false;
	}
	else
	{
		if (distanceToTarget <= alertInnerRange) bInAlertRange = true;
	}

	InOutAIContext.DistanceToTarget = distanceToTarget;
	InOutAIContext.bInAlertRange = bInAlertRange;

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeEngageAssignmentContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp))
	{
		FAICombatBTDebug::RecordAIContextClearedForAudit(nullptr, InOwnerPawn, InOutAIContext.CombatTargetActor, TEXT("EngageAssignmentContext"), TEXT("InvalidInput"));
		return EContextBuildResult::Error;
	}
	if (!IsValid(InOutAIContext.CombatTargetActor))
	{
		FAICombatBTDebug::RecordAIContextClearedForAudit(nullptr, InOwnerPawn, InOutAIContext.CombatTargetActor, TEXT("EngageAssignmentContext"), TEXT("MissingTarget"));
		return EContextBuildResult::NoData;
	}

	ACAIController* aiController = Cast<ACAIController>(InOwnerPawn->GetController());
	if (!IsValid(aiController))
	{
		FAICombatBTDebug::RecordAIContextClearedForAudit(aiController, InOwnerPawn, InOutAIContext.CombatTargetActor, TEXT("EngageAssignmentContext"), TEXT("MissingAIController"));
		return EContextBuildResult::Error;
	}

	UWorld* world = InOwnerPawn->GetWorld();
	UCWorldSubsystem_CombatEngage* subsystem = IsValid(world) ? world->GetSubsystem<UCWorldSubsystem_CombatEngage>() : nullptr;
	if (!IsValid(subsystem))
	{
		FAICombatBTDebug::RecordAIContextClearedForAudit(aiController, InOwnerPawn, InOutAIContext.CombatTargetActor, TEXT("EngageAssignmentContext"), TEXT("MissingEngageSubsystem"));
		return EContextBuildResult::Error;
	}

	const FEngageAssignmentContext previousAssignmentContext = subsystem->GetAssignment(aiController); // Previous Context

	FEngageRequestContext requestContext;
	requestContext.RequestController = aiController;
	requestContext.TargetActor = InOutAIContext.CombatTargetActor;
	requestContext.TargetPriority = InOutAIContext.TargetPriority;
	requestContext.DistanceToTarget = InOutAIContext.DistanceToTarget;
	requestContext.bWasEngaged = previousAssignmentContext.IsValidAssignment() && previousAssignmentContext.CombatRole == ECombatRole::Engage;

	subsystem->SubmitRequest(requestContext);
	aiController->RecordEngageRequestSubmittedForAudit(InOutAIContext.CombatTargetActor);

	const FEngageAssignmentContext curAssignmentContext = subsystem->GetAssignment(aiController); // Current Context

	InOutAIContext.CombatRole = curAssignmentContext.IsValidAssignment() ? curAssignmentContext.CombatRole : ECombatRole::None;
	InOutAIContext.bShouldEngage = InOutAIContext.CombatRole == ECombatRole::Engage;
	if (InOutAIContext.bShouldEngage)
	{
		aiController->RecordEngageAssignmentResolvedForAudit(InOutAIContext.CombatTargetActor);
	}
	else
	{
		FAICombatBTDebug::RecordAIContextEngageAssignmentForAudit(aiController, InOwnerPawn, InOutAIContext.CombatTargetActor, InOutAIContext.CombatRole, InOutAIContext.bShouldEngage, TEXT("EngageAssignmentPending"));
	}

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeReactionContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext) const
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	UCReactionComponent* reactionComp = InOwnerPawn->FindComponentByClass<UCReactionComponent>();
	if (!IsValid(reactionComp)) return EContextBuildResult::NoData;

	InOutAIContext.bIsActiveReaction = reactionComp->IsActive();

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeDeadContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext) const
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	UCHealthComponent* healthComp = InOwnerPawn->FindComponentByClass<UCHealthComponent>();
	if (!IsValid(healthComp)) return EContextBuildResult::NoData;

	InOutAIContext.DeadState = healthComp->GetDeadState();

	return EContextBuildResult::Success;
}

// Blackboard Update

void UCBTService_UpdateAIContext::UpdatePerceptionContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;
	if (!IsValid(InAIContext.PerceivedTargetActor)) return;

	CAIBlackboardValueHelper::SetObjectIfChanged(InBlackboardComp, CAIKey::Perception::PerceivedTargetActor.KeyName, InAIContext.PerceivedTargetActor);
	CAIBlackboardValueHelper::SetIntIfChanged(InBlackboardComp, CAIKey::Targeting::TargetPriority.KeyName, InAIContext.TargetPriority);
	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Perception::bHasLOS.KeyName, InAIContext.bHasLOS);
	CAIBlackboardValueHelper::SetFloatIfChanged(InBlackboardComp, CAIKey::Perception::LastSeenTime.KeyName, InAIContext.LastSeenTime);
	CAIBlackboardValueHelper::SetVectorIfChanged(InBlackboardComp, CAIKey::Perception::LastKnownLocation.KeyName, InAIContext.LastKnownLocation);
}

void UCBTService_UpdateAIContext::UpdateCombatTargetProjection(UBlackboardComponent* InBlackboardComp, const FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;
	CAIBlackboardValueHelper::SetObjectIfChanged(InBlackboardComp, CAIKey::Targeting::TargetActor.KeyName, InAIContext.CombatTargetActor);
	CAIBlackboardValueHelper::SetIntIfChanged(InBlackboardComp, CAIKey::Targeting::CombatTargetRevision.KeyName, InAIContext.CombatTargetRevision);
}

void UCBTService_UpdateAIContext::UpdateHomeMetricContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Navigation::bReturnHome.KeyName, InAIContext.bReturnHome);
	CAIBlackboardValueHelper::SetFloatIfChanged(InBlackboardComp, CAIKey::Metric::DistanceToHome.KeyName, InAIContext.DistanceToHome);
}

void UCBTService_UpdateAIContext::UpdateAlertRangeContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetFloatIfChanged(InBlackboardComp, CAIKey::Metric::DistanceToTarget.KeyName, InAIContext.DistanceToTarget);
	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Alert::bInAlertRange.KeyName, InAIContext.bInAlertRange);

}

void UCBTService_UpdateAIContext::UpdateEngageAssignmentContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::Engage::CombatRole.KeyName, static_cast<uint8>(InAIContext.CombatRole));
	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Engage::bShouldEngage.KeyName, InAIContext.bShouldEngage);
}

void UCBTService_UpdateAIContext::UpdateReactionContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Reaction::bIsActiveReaction.KeyName, InAIContext.bIsActiveReaction);
}

void UCBTService_UpdateAIContext::UpdateDeadContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::Dead::DeadState.KeyName, static_cast<uint8>(InAIContext.DeadState));
}

// Blackboard Clear

void UCBTService_UpdateAIContext::ClearPerceptionContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Perception::PerceivedTargetActor.KeyName);
	InBlackboardComp->ClearValue(CAIKey::Targeting::TargetPriority.KeyName);
	InBlackboardComp->ClearValue(CAIKey::Perception::bHasLOS.KeyName);
}

void UCBTService_UpdateAIContext::ClearCombatTargetProjection(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;
	InBlackboardComp->ClearValue(CAIKey::Targeting::TargetActor.KeyName);
	CAIBlackboardValueHelper::SetIntIfChanged(InBlackboardComp, CAIKey::Targeting::CombatTargetRevision.KeyName, 0);
}

void UCBTService_UpdateAIContext::ClearHomeMetricContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Metric::DistanceToHome.KeyName);
	InBlackboardComp->ClearValue(CAIKey::Navigation::bReturnHome.KeyName);
}

void UCBTService_UpdateAIContext::ClearAlertRangeContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Metric::DistanceToTarget.KeyName);
	InBlackboardComp->ClearValue(CAIKey::Alert::bInAlertRange.KeyName);
}

void UCBTService_UpdateAIContext::ClearEngageAssignmentContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::Engage::CombatRole.KeyName, static_cast<uint8>(ECombatRole::None));
	InBlackboardComp->ClearValue(CAIKey::Engage::bShouldEngage.KeyName);
}

void UCBTService_UpdateAIContext::ClearReactionContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Reaction::bIsActiveReaction.KeyName);
}

void UCBTService_UpdateAIContext::ClearDeadContext(UBlackboardComponent* InBlackboardComp)
{
	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::Dead::DeadState.KeyName, static_cast<uint8>(EDeadState::Alive));
}

// Lifecycle

void UCBTService_UpdateAIContext::ScheduleNextTick(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	SetNextTickTime(NodeMemory, CBTServiceIntervalHelper::GetAIContextInterval(OwnerComp));
}
