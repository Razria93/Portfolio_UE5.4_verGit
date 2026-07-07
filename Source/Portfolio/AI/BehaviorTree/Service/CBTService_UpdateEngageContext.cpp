#include "AI/BehaviorTree/Service/CBTService_UpdateEngageContext.h"
#include "ProjectGlobal.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"
#include "Character/Enemy/CEnemy.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Blackboard/CAIKey.h"
#include "AI/Blackboard/CAIBlackboardValueHelper.h"
#include "Type/CAIStructure.h"

UCBTService_UpdateEngageContext::UCBTService_UpdateEngageContext()
{
	NodeName = TEXT("Update Engage Context");
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;

	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UCBTService_UpdateEngageContext::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	Interval = CBTServiceIntervalHelper::GetEngageContextInterval();
	RandomDeviation = 0.0f;
}

void UCBTService_UpdateEngageContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_BT_UpdateEngageContext);

	UBlackboardComponent* blackBoardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackBoardComp)) return;

	const AAIController* aiOwner = OwnerComp.GetAIOwner();
	APawn* ownerPawn = IsValid(aiOwner) ? aiOwner->GetPawn() : nullptr;
	if (!IsValid(ownerPawn))
	{
		ClearEngageContext(blackBoardComp);
		return;
	}

	FEngageContext engageContext; // OutParameter

	const EContextBuildResult buildResult = BuildEngageContext(ownerPawn, blackBoardComp, engageContext);

	if (buildResult != EContextBuildResult::Success)
	{
		ClearEngageContext(blackBoardComp);
		return;
	}

	const EContextBuildResult computeResult = ComputeEngageContext(ownerPawn, blackBoardComp, engageContext);

	if (computeResult != EContextBuildResult::Success)
	{
		ClearEngageContext(blackBoardComp);
		return;
	}

	UpdateEngageContext(blackBoardComp, engageContext);
}

EContextBuildResult UCBTService_UpdateEngageContext::BuildEngageContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FEngageContext& OutEngageContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	ACEnemy* enemy = Cast<ACEnemy>(InOwnerPawn);
	if (!IsValid(enemy)) return EContextBuildResult::Error;

	OutEngageContext.TargetActor = Cast<AActor>(InBlackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName));
	if (!IsValid(OutEngageContext.TargetActor)) return EContextBuildResult::NoData;

	OutEngageContext.EngageOffsetRange = enemy->GetEngageOffsetRange();
	OutEngageContext.EngageEnterBuffer = enemy->GetEngageEnterBuffer();
	OutEngageContext.EngageExitBuffer = enemy->GetEngageExitBuffer();

	OutEngageContext.bPrevInEngageRange = InBlackboardComp->GetValueAsBool(CAIKey::Engage::bInEngageRange.KeyName);
	OutEngageContext.NextCombatActionTime = InBlackboardComp->GetValueAsFloat(CAIKey::Engage::NextCombatActionTime.KeyName);

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateEngageContext::ComputeEngageContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FEngageContext& InOutEngageContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;
	if (!IsValid(InOutEngageContext.TargetActor)) return EContextBuildResult::NoData;

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector targetLocation = InOutEngageContext.TargetActor->GetActorLocation();

	float dist_target = FVector::Dist(ownerLocation, targetLocation);

	float engageOuterRange = InOutEngageContext.EngageOffsetRange + InOutEngageContext.EngageEnterBuffer;
	float engageInnerRange = FMath::Max(0.f, InOutEngageContext.EngageOffsetRange - InOutEngageContext.EngageExitBuffer);

	bool bInEngageRange = InOutEngageContext.bPrevInEngageRange;

	if (bInEngageRange)
	{
		if (dist_target > engageOuterRange) bInEngageRange = false;
	}
	else
	{
		if (dist_target <= engageInnerRange) bInEngageRange = true;
	}

	float currentTime = InOwnerPawn->GetWorld()->GetTimeSeconds();
	const bool bCooldownElapsed = currentTime >= InOutEngageContext.NextCombatActionTime;

	const bool bIsCombatAction = InBlackboardComp->GetValueAsBool(CAIKey::Engage::bIsCombatAction.KeyName);
	const bool bIsActiveReaction = InBlackboardComp->GetValueAsBool(CAIKey::Reaction::bIsActiveReaction.KeyName);

	InOutEngageContext.EngageOuterRange = engageOuterRange;
	InOutEngageContext.EngageInnerRange = engageInnerRange;
	InOutEngageContext.DistanceToTarget = dist_target;

	// Result
	InOutEngageContext.bInEngageRange = bInEngageRange;
	InOutEngageContext.bCanCombatAction = 
		bInEngageRange				// for ActionRange Check
		&& bCooldownElapsed			// for ActionCooldown Check
		&& !bIsCombatAction			// for ActionType Check
		&& !bIsActiveReaction;		// for ActiveReaction Check

	// PrintEngageContext(InOwnerPawn, InOutEngageContext, currentTime);

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

void UCBTService_UpdateEngageContext::PrintEngageContext(const APawn* InOwnerPawn, const FEngageContext& InEngageContext, const float InCurrentTime)
{
	if (!IsValid(InOwnerPawn)) return;

	const FVector ownerLocation = InOwnerPawn->GetActorLocation();
	const FVector targetLocation = IsValid(InEngageContext.TargetActor) ? InEngageContext.TargetActor->GetActorLocation() : FVector::ZeroVector;

	FLog::Log(TEXT("======== EngageContext ========"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerPawn"), *GetNameSafe(InOwnerPawn)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InEngageContext.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerLocation"), *ownerLocation.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetLocation"), *targetLocation.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("DistanceToTarget"), InEngageContext.DistanceToTarget));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("EngageOffsetRange"), InEngageContext.EngageOffsetRange));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("EngageEnterBuffer"), InEngageContext.EngageEnterBuffer));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("EngageExitBuffer"), InEngageContext.EngageExitBuffer));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("EngageOuterRange"), InEngageContext.EngageOuterRange));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("EngageInnerRange"), InEngageContext.EngageInnerRange));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bPrevInEngageRange"), InEngageContext.bPrevInEngageRange ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bInEngageRange"), InEngageContext.bInEngageRange ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bCanCombatAction"), InEngageContext.bCanCombatAction ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("NextCombatActionTime"), InEngageContext.NextCombatActionTime));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CurrentTime"), InCurrentTime));
	FLog::Log(TEXT("================================"));
}
