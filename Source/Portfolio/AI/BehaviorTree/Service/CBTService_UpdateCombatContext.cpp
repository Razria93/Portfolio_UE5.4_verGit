#include "AI/BehaviorTree/Service/CBTService_UpdateCombatContext.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"
#include "Type/CAIStructure.h"

UCBTService_UpdateCombatContext::UCBTService_UpdateCombatContext()
{
	NodeName = TEXT("Update Combat Context");
	bNotifyTick = true;

	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UCBTService_UpdateCombatContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* blackBoardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackBoardComp)) return;

	APawn* ownerPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!IsValid(ownerPawn))
	{
		ClearCombatContext(blackBoardComp);
		return;
	}

	FCombatContext combatContext; // OutParameter

	const EContextBuildResult BuildResult = BuildCombatContext(ownerPawn, blackBoardComp, combatContext);

	if (BuildResult != EContextBuildResult::Success)
	{
		ClearCombatContext(blackBoardComp);
		return;
	}

	const EContextBuildResult computeResult = ComputeCombatContext(ownerPawn, blackBoardComp, combatContext);

	if (computeResult == EContextBuildResult::Success)
	{
		UpdateCombatContext(blackBoardComp, combatContext);
	}
	else
	{
		ClearCombatContext(blackBoardComp);
	}
}

EContextBuildResult UCBTService_UpdateCombatContext::BuildCombatContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FCombatContext& OutCombatContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	OutCombatContext.TargetActor = Cast<AActor>(InBlackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor));
	if (!IsValid(OutCombatContext.TargetActor)) return EContextBuildResult::NoData;

	OutCombatContext.CombatOffsetRange = InBlackboardComp->GetValueAsFloat(CAIKey::Combat::CombatOffsetRange);
	OutCombatContext.CombatEnterBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Combat::CombatEnterBuffer);
	OutCombatContext.CombatExitBuffer = InBlackboardComp->GetValueAsFloat(CAIKey::Combat::CombatExitBuffer);
	OutCombatContext.AttackableTime = InBlackboardComp->GetValueAsFloat(CAIKey::Combat::AttackableTime);
	OutCombatContext.bPrevInAttackRange = InBlackboardComp->GetValueAsBool(CAIKey::Combat::bInAttackRange);

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdateCombatContext::ComputeCombatContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FCombatContext& InOutCombatContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;
	if (!IsValid(InOutCombatContext.TargetActor)) return EContextBuildResult::NoData;

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector targetLocation = InOutCombatContext.TargetActor->GetActorLocation();

	float dist_target = FVector::Dist(ownerLocation, targetLocation);

	float combatOuterRange = InOutCombatContext.CombatOffsetRange + InOutCombatContext.CombatEnterBuffer;
	float combatInnerRange = FMath::Max(0.f, InOutCombatContext.CombatOffsetRange - InOutCombatContext.CombatExitBuffer);

	bool bInAttackRange = InOutCombatContext.bPrevInAttackRange;

	if (bInAttackRange)
	{
		if (dist_target > combatOuterRange) bInAttackRange = false;
	}
	else
	{
		if (dist_target <= combatInnerRange) bInAttackRange = true;
	}

	float currentTime = InOwnerPawn->GetWorld()->GetTimeSeconds();

	InOutCombatContext.DistanceToTarget = dist_target;
	InOutCombatContext.bInAttackRange = bInAttackRange;
	InOutCombatContext.bCanAttack = currentTime >= InOutCombatContext.AttackableTime;

	// PrintCombatContext(InOwnerPawn, InOutCombatContext, currentTime);

	return EContextBuildResult::Success;
}

void UCBTService_UpdateCombatContext::UpdateCombatContext(UBlackboardComponent* InBlackboardComp, FCombatContext& InCombatContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsBool(CAIKey::Combat::bInAttackRange, InCombatContext.bInAttackRange);
	InBlackboardComp->SetValueAsBool(CAIKey::Combat::bCanAttack, InCombatContext.bCanAttack);
}

void UCBTService_UpdateCombatContext::ClearCombatContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Combat::bInAttackRange);
	InBlackboardComp->ClearValue(CAIKey::Combat::bCanAttack);
}

void UCBTService_UpdateCombatContext::PrintCombatContext(const APawn* InOwnerPawn, const FCombatContext& InCombatContext, const float InCurrentTime)
{
	if (!IsValid(InOwnerPawn)) return;

	const FVector ownerLocation = InOwnerPawn->GetActorLocation();
	const FVector targetLocation = IsValid(InCombatContext.TargetActor) ? InCombatContext.TargetActor->GetActorLocation() : FVector::ZeroVector;

	const float combatOuterRange = InCombatContext.CombatOffsetRange + InCombatContext.CombatEnterBuffer;
	const float combatInnerRange = FMath::Max(0.f, InCombatContext.CombatOffsetRange - InCombatContext.CombatExitBuffer);

	FLog::Log(TEXT("======== CombatContext ========"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerPawn"), *GetNameSafe(InOwnerPawn)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InCombatContext.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerLocation"), *ownerLocation.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetLocation"), *targetLocation.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("DistanceToTarget"), InCombatContext.DistanceToTarget));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CombatOffsetRange"), InCombatContext.CombatOffsetRange));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CombatEnterBuffer"), InCombatContext.CombatEnterBuffer));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CombatExitBuffer"), InCombatContext.CombatExitBuffer));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CombatOuterRange"), combatOuterRange));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CombatInnerRange"), combatInnerRange));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("PrevInAttackRange"), InCombatContext.bPrevInAttackRange ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bInAttackRange"), InCombatContext.bInAttackRange ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CurrentTime"), InCurrentTime));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("AttackableTime"), InCombatContext.AttackableTime));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bCanAttack"), InCombatContext.bCanAttack ? TEXT("true") : TEXT("false")));
	FLog::Log(TEXT("================================"));
}