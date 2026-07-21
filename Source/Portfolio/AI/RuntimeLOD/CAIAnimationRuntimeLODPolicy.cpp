#include "AI/RuntimeLOD/CAIAnimationRuntimeLODPolicy.h"

#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"
#include "Character/Enemy/CEnemy.h"
#include "Controller/CAIController.h"
#include "Core/Profiling/CAIAnimationProfiling.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarEnemyAnimationMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyAnimationMode"),
		0,
		TEXT("Controls ACEnemy animation runtime LOD mode. 0: default refresh, 1: reduced parameter refresh."),
		ECVF_Default);

	TAutoConsoleVariable<float> CVarEnemyAnimationReducedRefreshInterval(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyAnimationReducedRefreshInterval"),
		0.1f,
		TEXT("Refresh interval for ACEnemy reduced animation parameter mode."),
		ECVF_Default);

}

int32 FAIAnimationRuntimeLODPolicy::GetEnemyAnimationMode()
{
	return FMath::Clamp(CVarEnemyAnimationMode.GetValueOnGameThread(), 0, 1);
}

int32 FAIAnimationRuntimeLODPolicy::GetEnemyAnimationMode(const AActor* InOwner)
{
	if (FAIStateRuntimeLODPolicy::ShouldUseStateBasedPolicy())
	{
		return GetStateBasedAnimationMode(InOwner);
	}

	return GetEnemyAnimationMode();
}

bool FAIAnimationRuntimeLODPolicy::IsEnemyAnimationRuntimeLODTarget(const AActor* InOwner)
{
	return IsValid(InOwner) && InOwner->IsA<ACEnemy>();
}

float FAIAnimationRuntimeLODPolicy::GetReducedAnimationRefreshInterval()
{
	return FMath::Max(CVarEnemyAnimationReducedRefreshInterval.GetValueOnGameThread(), KINDA_SMALL_NUMBER);
}

bool FAIAnimationRuntimeLODPolicy::ShouldAuditAnimationRefresh()
{
	return FAIAnimationProfiling::ShouldAuditAnimationRefresh();
}

int32 FAIAnimationRuntimeLODPolicy::GetStateBasedAnimationMode(const AActor* InOwner)
{
	const APawn* ownerPawn = Cast<APawn>(InOwner);
	const ACAIController* aiController = IsValid(ownerPawn) ? Cast<ACAIController>(ownerPawn->GetController()) : nullptr;
	if (!IsValid(aiController)) return 0;

	switch (aiController->GetCurrentRuntimeLODTier())
	{
	case EAIRuntimeLODTier::Awareness:
	case EAIRuntimeLODTier::Background:
	case EAIRuntimeLODTier::Dormant:
		return 1;

	case EAIRuntimeLODTier::CombatCritical:
	case EAIRuntimeLODTier::CombatSupport:
	default:
		return 0;
	}
}
