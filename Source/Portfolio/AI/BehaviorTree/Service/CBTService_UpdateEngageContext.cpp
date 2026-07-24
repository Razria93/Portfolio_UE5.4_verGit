#include "AI/BehaviorTree/Service/CBTService_UpdateEngageContext.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "AIController.h"
#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"
#include "GameFramework/Pawn.h"
#include "Character/Enemy/CEnemy.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Blackboard/CAIKey.h"
#include "AI/Blackboard/CAIBlackboardValueHelper.h"
#include "Core/Debug/FAICombatBTDebug.h"
#include "Core/Profiling/CAIBehaviorTreeProfiling.h"
#include "Type/CAITypes.h"

UCBTService_UpdateEngageContext::UCBTService_UpdateEngageContext()
{
	NodeName = TEXT("Update Engage Context");
	bNotifyTick = true;

	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UCBTService_UpdateEngageContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_BT_UpdateEngageContext);
	FAIBehaviorTreeProfiling::RecordUpdateEngageContextTick();
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		FAICombatBTDebug::RecordEngageContextRejectedForAudit(nullptr, FEngageContext(), TEXT("Tick"), TEXT("MissingBlackboard"));
		return;
	}

	const AAIController* aiOwner = OwnerComp.GetAIOwner();
	APawn* ownerPawn = IsValid(aiOwner) ? aiOwner->GetPawn() : nullptr;
	if (!IsValid(ownerPawn))
	{
		FAICombatBTDebug::RecordEngageContextRejectedForAudit(ownerPawn, FEngageContext(), TEXT("Tick"), TEXT("MissingOwnerPawn"));
		ClearEngageContext(blackboardComp);
		return;
	}

	FEngageContext engageContext;

	const EContextBuildResult buildResult = BuildEngageContext(ownerPawn, blackboardComp, engageContext);

	if (buildResult != EContextBuildResult::Success)
	{
		ClearEngageContext(blackboardComp);
		return;
	}

	const EContextBuildResult computeResult = ComputeEngageContext(ownerPawn, blackboardComp, engageContext);

	if (computeResult != EContextBuildResult::Success)
	{
		ClearEngageContext(blackboardComp);
		return;
	}

	UpdateEngageContext(blackboardComp, engageContext);
}

void UCBTService_UpdateEngageContext::ScheduleNextTick(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	SetNextTickTime(NodeMemory, CBTServiceIntervalHelper::GetEngageContextInterval());
}

EContextBuildResult UCBTService_UpdateEngageContext::BuildEngageContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FEngageContext& OutEngageContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp))
	{
		FAICombatBTDebug::RecordEngageContextRejectedForAudit(InOwnerPawn, OutEngageContext, TEXT("Build"), TEXT("InvalidInput"));
		return EContextBuildResult::Error;
	}

	ACEnemy* enemy = Cast<ACEnemy>(InOwnerPawn);
	if (!IsValid(enemy))
	{
		FAICombatBTDebug::RecordEngageContextRejectedForAudit(InOwnerPawn, OutEngageContext, TEXT("Build"), TEXT("InvalidEnemyPawn"));
		return EContextBuildResult::Error;
	}

	OutEngageContext.TargetActor = Cast<AActor>(InBlackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName));
	if (!IsValid(OutEngageContext.TargetActor))
	{
		FAICombatBTDebug::RecordEngageContextRejectedForAudit(InOwnerPawn, OutEngageContext, TEXT("Build"), TEXT("MissingTarget"));
		return EContextBuildResult::NoData;
	}

	OutEngageContext.EngageOffsetRange = enemy->GetEngageOffsetRange();
	OutEngageContext.EngageEnterBuffer = enemy->GetEngageEnterBuffer();
	OutEngageContext.EngageExitBuffer = enemy->GetEngageExitBuffer();

	OutEngageContext.bPrevInEngageRange = InBlackboardComp->GetValueAsBool(CAIKey::Engage::bInEngageRange.KeyName);
	OutEngageContext.NextCombatActionTime = InBlackboardComp->GetValueAsFloat(CAIKey::Engage::NextCombatActionTime.KeyName);

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateEngageContext::ComputeEngageContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FEngageContext& InOutEngageContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp))
	{
		FAICombatBTDebug::RecordEngageContextRejectedForAudit(InOwnerPawn, InOutEngageContext, TEXT("Compute"), TEXT("InvalidInput"));
		return EContextBuildResult::Error;
	}
	if (!IsValid(InOutEngageContext.TargetActor))
	{
		FAICombatBTDebug::RecordEngageContextRejectedForAudit(InOwnerPawn, InOutEngageContext, TEXT("Compute"), TEXT("MissingTarget"));
		return EContextBuildResult::NoData;
	}

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector targetLocation = InOutEngageContext.TargetActor->GetActorLocation();

	float distanceToTarget = FVector::Dist(ownerLocation, targetLocation);

	float engageOuterRange = InOutEngageContext.EngageOffsetRange + InOutEngageContext.EngageEnterBuffer;
	float engageInnerRange = FMath::Max(0.f, InOutEngageContext.EngageOffsetRange - InOutEngageContext.EngageExitBuffer);

	bool bInEngageRange = InOutEngageContext.bPrevInEngageRange;

	if (bInEngageRange)
	{
		if (distanceToTarget > engageOuterRange) bInEngageRange = false;
	}
	else
	{
		if (distanceToTarget <= engageInnerRange) bInEngageRange = true;
	}

	float currentTime = InOwnerPawn->GetWorld()->GetTimeSeconds();
	const bool bCooldownElapsed = currentTime >= InOutEngageContext.NextCombatActionTime;

	const bool bIsCombatAction = InBlackboardComp->GetValueAsBool(CAIKey::Engage::bIsCombatAction.KeyName);
	const bool bIsActiveReaction = InBlackboardComp->GetValueAsBool(CAIKey::Reaction::bIsActiveReaction.KeyName);

	InOutEngageContext.EngageOuterRange = engageOuterRange;
	InOutEngageContext.EngageInnerRange = engageInnerRange;
	InOutEngageContext.DistanceToTarget = distanceToTarget;

	// Result
	InOutEngageContext.bInEngageRange = bInEngageRange;
	InOutEngageContext.bCanCombatAction = 
		bInEngageRange				// for ActionRange Check
		&& bCooldownElapsed			// for ActionCooldown Check
		&& !bIsCombatAction			// for ActionType Check
		&& !bIsActiveReaction;		// for ActiveReaction Check

	FAICombatBTDebug::RecordEngageContextComputedForAudit(InOwnerPawn, InOutEngageContext, bCooldownElapsed, bIsCombatAction, bIsActiveReaction);

	if (!InOutEngageContext.bCanCombatAction)
	{
		const TCHAR* reason = TEXT("Unknown");
		if (!bInEngageRange)
		{
			reason = TEXT("OutOfEngageRange");
		}
		else if (!bCooldownElapsed)
		{
			reason = TEXT("Cooldown");
		}
		else if (bIsCombatAction)
		{
			reason = TEXT("AlreadyCombatAction");
		}
		else if (bIsActiveReaction)
		{
			reason = TEXT("ActiveReaction");
		}

		FAICombatBTDebug::RecordEngageContextRejectedForAudit(InOwnerPawn, InOutEngageContext, TEXT("Gate"), reason);
	}

	return EContextBuildResult::Success;
}

void UCBTService_UpdateEngageContext::UpdateEngageContext(UBlackboardComponent* InBlackboardComp, FEngageContext& InEngageContext)
{
	if (!IsValid(InBlackboardComp)) return;

	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Engage::bInEngageRange.KeyName, InEngageContext.bInEngageRange);
	CAIBlackboardValueHelper::SetBoolIfChanged(InBlackboardComp, CAIKey::Engage::bCanCombatAction.KeyName, InEngageContext.bCanCombatAction);
}

void UCBTService_UpdateEngageContext::ClearEngageContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Engage::bInEngageRange.KeyName);
	InBlackboardComp->ClearValue(CAIKey::Engage::bCanCombatAction.KeyName);
}
