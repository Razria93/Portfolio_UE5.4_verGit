#include "AI/RuntimeLOD/CAIMovementRuntimeLODPolicy.h"

#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"
#include "Controller/CAIController.h"
#include "Character/Enemy/CEnemy.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarAIRuntimeLODEnemyMovementMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyMovementMode"),
		0,
		TEXT("Controls ACEnemy movement runtime LOD mode. 0: default, 1: disable movement state refresh, 2: block movement intent."),
		ECVF_Default);
}

int32 FAIMovementRuntimeLODPolicy::GetEnemyMovementMode()
{
	return FMath::Clamp(CVarAIRuntimeLODEnemyMovementMode.GetValueOnGameThread(), 0, 2);
}

int32 FAIMovementRuntimeLODPolicy::GetEnemyMovementMode(const AActor* InOwner)
{
	if (FAIStateRuntimeLODPolicy::ShouldUseStateBasedPolicy())
	{
		return GetStateBasedMovementMode(InOwner);
	}

	return GetEnemyMovementMode();
}

bool FAIMovementRuntimeLODPolicy::IsEnemyMovementRuntimeLODTarget(const AActor* InOwner)
{
	return IsValid(InOwner) && InOwner->IsA<ACEnemy>();
}

bool FAIMovementRuntimeLODPolicy::ShouldDisableMovementStateRefresh(int32 InMovementMode)
{
	return InMovementMode == 1;
}

bool FAIMovementRuntimeLODPolicy::ShouldBlockMovementIntent(int32 InMovementMode)
{
	return InMovementMode == 2;
}

int32 FAIMovementRuntimeLODPolicy::GetStateBasedMovementMode(const AActor* InOwner)
{
	const APawn* ownerPawn = Cast<APawn>(InOwner);
	const ACAIController* aiController = IsValid(ownerPawn) ? Cast<ACAIController>(ownerPawn->GetController()) : nullptr;
	if (!IsValid(aiController)) return 0;

	switch (aiController->GetCurrentRuntimeLODTier())
	{
	case EAIRuntimeLODTier::Awareness:
	case EAIRuntimeLODTier::Dormant:
		return 2;

	case EAIRuntimeLODTier::CombatCritical:
	case EAIRuntimeLODTier::CombatSupport:
	case EAIRuntimeLODTier::Background:
	default:
		return 0;
	}
}
