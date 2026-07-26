#include "AI/RuntimeLOD/CAIAnimationRuntimeLODPolicy.h"

#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"
#include "Character/Enemy/CEnemy.h"
#include "Controller/CAIController.h"

#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
	enum class EAIAnimationRuntimeLODMode : int32
	{
		DefaultRefresh = 0,
		ReducedParameterRefresh = 1,
	};

	constexpr int32 ToAnimationRuntimeLODModeValue(EAIAnimationRuntimeLODMode InMode)
	{
		return static_cast<int32>(InMode);
	}

	TAutoConsoleVariable<int32> CVarEnemyAnimationMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyAnimationMode"),
		ToAnimationRuntimeLODModeValue(EAIAnimationRuntimeLODMode::DefaultRefresh),
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
	return FMath::Clamp(
		CVarEnemyAnimationMode.GetValueOnGameThread(),
		ToAnimationRuntimeLODModeValue(EAIAnimationRuntimeLODMode::DefaultRefresh),
		ToAnimationRuntimeLODModeValue(EAIAnimationRuntimeLODMode::ReducedParameterRefresh));
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

int32 FAIAnimationRuntimeLODPolicy::GetStateBasedAnimationMode(const AActor* InOwner)
{
	const APawn* ownerPawn = Cast<APawn>(InOwner);
	const ACAIController* aiController = IsValid(ownerPawn) ? Cast<ACAIController>(ownerPawn->GetController()) : nullptr;
	if (!IsValid(aiController)) return ToAnimationRuntimeLODModeValue(EAIAnimationRuntimeLODMode::DefaultRefresh);

	switch (aiController->GetCurrentRuntimeLODTier())
	{
	case EAIRuntimeLODTier::Awareness:
	case EAIRuntimeLODTier::Background:
	case EAIRuntimeLODTier::Dormant:
		return ToAnimationRuntimeLODModeValue(EAIAnimationRuntimeLODMode::ReducedParameterRefresh);

	case EAIRuntimeLODTier::CombatCritical:
	case EAIRuntimeLODTier::CombatSupport:
	default:
		return ToAnimationRuntimeLODModeValue(EAIAnimationRuntimeLODMode::DefaultRefresh);
	}
}
