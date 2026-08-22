#include "AI/BehaviorTree/Service/CBTService_UpdateAIContext.h"

#include "ProjectGlobal.h"

#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"
#include "Controller/CAIController.h"
#include "Component/CReactionComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CEnemyCombatParticipationComponent.h"
#include "Character/Enemy/CEnemy.h"
#include "System/Combat/CWorldSubsystem_CombatParticipation.h"
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
		AActor* combatTargetActor = Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::CombatTarget::Actor.KeyName));
		FAICombatBTDebug::RecordAIContextClearedForAudit(aiOwner, ownerPawn, combatTargetActor, TEXT("AIContext"), TEXT("MissingOwnerPawn"));

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

	const bool bWasReturningHome = blackboardComp->GetValueAsBool(CAIKey::Navigation::bReturnHome.KeyName);
	EContextBuildResult homeResult = ComputeHomeMetricContext(ownerPawn, blackboardComp, aiContext);

	if (homeResult == EContextBuildResult::Success)
	{
		UpdateHomeMetricContext(blackboardComp, aiContext);

		if (ACEnemy* enemy = Cast<ACEnemy>(ownerPawn))
		{
			if (UCEnemyCombatParticipationComponent* participationComp = enemy->GetEnemyCombatParticipationComp())
			{
				if (!bWasReturningHome && aiContext.Home.bReturnHome)
				{
					participationComp->SetParticipationSuppressed(true);

					CAIBlackboardValueHelper::SetBoolIfChanged(blackboardComp, CAIKey::Investigate::bShouldInvestigate.KeyName, false);
					CAIBlackboardValueHelper::SetBoolIfChanged(blackboardComp, CAIKey::Investigate::bIsInvestigating.KeyName, false);
					CAIBlackboardValueHelper::SetBoolIfChanged(blackboardComp, CAIKey::Investigate::bShouldEndInvestigate.KeyName, false);
				}
				else if (bWasReturningHome && !aiContext.Home.bReturnHome)
				{
					participationComp->SetParticipationSuppressed(false);
				}
			}
		}
	}
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
		FAICombatBTDebug::RecordBlackboardTargetSetForAudit(aiOwner, aiContext.CombatParticipation.CombatTargetActor);
	}
	else
	{
		ClearCombatTargetProjection(blackboardComp);
	}

	// Participation Target takes priority for combat movement metrics; Perception remains the fallback without assignment.
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

	FPerceptionTargetContext topContext;
	const EPerceptionBuildResult Result = aiController->BuildPerceptionContext(topContext);

	if (Result == EPerceptionBuildResult::Error) return EContextBuildResult::Error;
	if (Result == EPerceptionBuildResult::NoData) return EContextBuildResult::NoData;

	OutAIContext.Perception = topContext;

	aiController->RecordPerceptionContextBuiltForAudit(topContext.TargetActor);

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::BuildCombatTargetContext(APawn* InOwnerPawn, FAIBlackboardUpdateContext& OutAIContext) const
{
	const ACEnemy* enemy = Cast<ACEnemy>(InOwnerPawn);
	const UCCombatTargetComponent* combatTargetComp = IsValid(enemy) ? enemy->GetCombatTargetComp() : nullptr;
	const UCEnemyCombatParticipationComponent* participationComp = IsValid(enemy) ? enemy->GetEnemyCombatParticipationComp() : nullptr;
	if (!IsValid(combatTargetComp) || !IsValid(participationComp)) return EContextBuildResult::Error;

	const FCombatParticipationAppliedSnapshot appliedSnapshot = participationComp->GetAppliedSnapshot();
	const FCombatTargetSnapshot combatTargetSnapshot = combatTargetComp->GetCombatTargetSnapshot();

	if (!appliedSnapshot.bIsApplied) return EContextBuildResult::NoData;
	if (appliedSnapshot.TargetActor != combatTargetSnapshot.TargetActor) return EContextBuildResult::NoData;
	if (appliedSnapshot.CombatTargetRevision != combatTargetSnapshot.Revision) return EContextBuildResult::NoData;

	OutAIContext.CombatParticipation.CombatTargetActor = appliedSnapshot.TargetActor;
	OutAIContext.CombatParticipation.CombatTargetRevision = appliedSnapshot.CombatTargetRevision;
	OutAIContext.CombatParticipation.CombatRole = appliedSnapshot.CombatRole;
	OutAIContext.CombatParticipation.CombatParticipationRevision = appliedSnapshot.AssignmentRevision;
	OutAIContext.CombatParticipation.bHasCombatParticipationProjection = true;

	return EContextBuildResult::Success;
}

// Context Compute

EContextBuildResult UCBTService_UpdateAIContext::ComputeHomeMetricContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext) const
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector homeLocation = InBlackboardComp->GetValueAsVector(CAIKey::Navigation::HomeLocation.KeyName);

	float distanceToHome = FVector::Dist(ownerLocation, homeLocation);

	InOutAIContext.Home.DistanceToHome = distanceToHome;
	InOutAIContext.Home.bReturnHome = distanceToHome > MovableRange;

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeAlertRangeContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext) const
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	AActor* rangeTarget = InOutAIContext.CombatParticipation.bHasCombatParticipationProjection
		? InOutAIContext.CombatParticipation.CombatTargetActor
		: InOutAIContext.Perception.TargetActor;
	if (!IsValid(rangeTarget)) return EContextBuildResult::NoData;

	float chaseOffsetRange = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseOffsetRange.KeyName);
	float chaseEnterBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseEnterBuffer.KeyName);
	float chaseExitBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Chase::ChaseExitBuffer.KeyName);

	bool bInAlertRange = InBlackboardComp->GetValueAsBool(CAIKey::Alert::bInAlertRange.KeyName);

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector targetLocation = rangeTarget->GetActorLocation();

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

	InOutAIContext.TargetRange.TargetActor = rangeTarget;
	InOutAIContext.TargetRange.DistanceToTarget = distanceToTarget;
	InOutAIContext.TargetRange.bInAlertRange = bInAlertRange;

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeEngageAssignmentContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp))
	{
		FAICombatBTDebug::RecordAIContextClearedForAudit(nullptr, InOwnerPawn, InOutAIContext.CombatParticipation.CombatTargetActor, TEXT("EngageAssignmentContext"), TEXT("InvalidInput"));
		return EContextBuildResult::Error;
	}
	ACAIController* aiController = Cast<ACAIController>(InOwnerPawn->GetController());
	if (!IsValid(aiController))
	{
		FAICombatBTDebug::RecordAIContextClearedForAudit(aiController, InOwnerPawn, InOutAIContext.CombatParticipation.CombatTargetActor, TEXT("EngageAssignmentContext"), TEXT("MissingAIController"));
		return EContextBuildResult::Error;
	}

	if (!InOutAIContext.CombatParticipation.bHasCombatParticipationProjection)
	{
		FAICombatBTDebug::RecordAIContextClearedForAudit(aiController, InOwnerPawn, InOutAIContext.CombatParticipation.CombatTargetActor, TEXT("EngageAssignmentContext"), TEXT("MissingCoherentParticipationProjection"));
		return EContextBuildResult::NoData;
	}

	if (InOutAIContext.CombatParticipation.ShouldEngage())
	{
		aiController->RecordEngageAssignmentResolvedForAudit(InOutAIContext.CombatParticipation.CombatTargetActor);
	}
	else
	{
		FAICombatBTDebug::RecordAIContextEngageAssignmentForAudit(aiController, InOwnerPawn, InOutAIContext.CombatParticipation.CombatTargetActor, InOutAIContext.CombatParticipation.CombatRole, InOutAIContext.CombatParticipation.ShouldEngage(), TEXT("EngageAssignmentPending"));
	}

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeReactionContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext) const
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	UCReactionComponent* reactionComp = InOwnerPawn->FindComponentByClass<UCReactionComponent>();
	if (!IsValid(reactionComp)) return EContextBuildResult::NoData;

	InOutAIContext.Reaction.bIsActiveReaction = reactionComp->IsActive();

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateAIContext::ComputeDeadContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InOutAIContext) const
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	UCHealthComponent* healthComp = InOwnerPawn->FindComponentByClass<UCHealthComponent>();
	if (!IsValid(healthComp)) return EContextBuildResult::NoData;

	InOutAIContext.Lifecycle.DeadState = healthComp->GetDeadState();

	return EContextBuildResult::Success;
}

// Blackboard Update

void UCBTService_UpdateAIContext::UpdatePerceptionContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;
	if (!IsValid(InAIContext.Perception.TargetActor)) return;

	CAIBlackboardValueHelper::SetObjectIfChanged(InBlackboardComp, CAIKey::Perception::PerceivedTargetActor.KeyName, InAIContext.Perception.TargetActor);
	CAIBlackboardValueHelper::SetIntIfChanged(InBlackboardComp, CAIKey::Perception::PerceivedTargetPriority.KeyName, InAIContext.Perception.TargetPriority);
	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Perception::bHasLOS.KeyName, InAIContext.Perception.bHasLOS);
	CAIBlackboardValueHelper::SetFloatIfChanged(InBlackboardComp, CAIKey::Perception::LastSeenTime.KeyName, InAIContext.Perception.LastSeenTime);
	CAIBlackboardValueHelper::SetVectorIfChanged(InBlackboardComp, CAIKey::Perception::LastKnownLocation.KeyName, InAIContext.Perception.LastKnownLocation);
}

void UCBTService_UpdateAIContext::UpdateCombatTargetProjection(UBlackboardComponent* InBlackboardComp, const FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;
	CAIBlackboardValueHelper::SetObjectIfChanged(InBlackboardComp, CAIKey::CombatTarget::Actor.KeyName, InAIContext.CombatParticipation.CombatTargetActor);
	CAIBlackboardValueHelper::SetIntIfChanged(InBlackboardComp, CAIKey::CombatTarget::CombatTargetRevision.KeyName, InAIContext.CombatParticipation.CombatTargetRevision);
}

void UCBTService_UpdateAIContext::UpdateHomeMetricContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Navigation::bReturnHome.KeyName, InAIContext.Home.bReturnHome);
	CAIBlackboardValueHelper::SetFloatIfChanged(InBlackboardComp, CAIKey::Metric::DistanceToHome.KeyName, InAIContext.Home.DistanceToHome);
}

void UCBTService_UpdateAIContext::UpdateAlertRangeContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetFloatIfChanged(InBlackboardComp, CAIKey::Metric::DistanceToTarget.KeyName, InAIContext.TargetRange.DistanceToTarget);
	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Alert::bInAlertRange.KeyName, InAIContext.TargetRange.bInAlertRange);

}

void UCBTService_UpdateAIContext::UpdateEngageAssignmentContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::Engage::CombatRole.KeyName, static_cast<uint8>(InAIContext.CombatParticipation.CombatRole));
	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::CombatParticipation::State.KeyName, static_cast<uint8>(InAIContext.CombatParticipation.CombatRole));
	CAIBlackboardValueHelper::SetIntIfChanged(InBlackboardComp, CAIKey::CombatParticipation::AssignmentRevision.KeyName, InAIContext.CombatParticipation.CombatParticipationRevision);
	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Engage::bShouldEngage.KeyName, InAIContext.CombatParticipation.ShouldEngage());
}

void UCBTService_UpdateAIContext::UpdateReactionContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Reaction::bIsActiveReaction.KeyName, InAIContext.Reaction.bIsActiveReaction);
}

void UCBTService_UpdateAIContext::UpdateDeadContext(UBlackboardComponent* InBlackboardComp, FAIBlackboardUpdateContext& InAIContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::Dead::DeadState.KeyName, static_cast<uint8>(InAIContext.Lifecycle.DeadState));
}

// Blackboard Clear

void UCBTService_UpdateAIContext::ClearPerceptionContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Perception::PerceivedTargetActor.KeyName);
	InBlackboardComp->ClearValue(CAIKey::Perception::PerceivedTargetPriority.KeyName);
	InBlackboardComp->ClearValue(CAIKey::Perception::bHasLOS.KeyName);
}

void UCBTService_UpdateAIContext::ClearCombatTargetProjection(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;
	InBlackboardComp->ClearValue(CAIKey::CombatTarget::Actor.KeyName);
	CAIBlackboardValueHelper::SetIntIfChanged(InBlackboardComp, CAIKey::CombatTarget::CombatTargetRevision.KeyName, 0);
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
	CAIBlackboardValueHelper::SetEnumIfChanged(InBlackboardComp, CAIKey::CombatParticipation::State.KeyName, static_cast<uint8>(ECombatRole::None));
	CAIBlackboardValueHelper::SetIntIfChanged(InBlackboardComp, CAIKey::CombatParticipation::AssignmentRevision.KeyName, 0);
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
