#include "AI/RuntimeLOD/CAIMovementRuntimeLODPolicy.h"

#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"
#include "Controller/CAIController.h"
#include "Character/Enemy/CEnemy.h"

#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
	enum class EAIMovementRuntimeLODMode : int32
	{
		Default = 0,
		BlockMovementIntent = 1,
	};

	constexpr int32 ToMovementRuntimeLODModeValue(EAIMovementRuntimeLODMode InMode)
	{
		return static_cast<int32>(InMode);
	}

	TAutoConsoleVariable<int32> CVarAIRuntimeLODEnemyMovementMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyMovementMode"),
		ToMovementRuntimeLODModeValue(EAIMovementRuntimeLODMode::Default),
		TEXT("Controls ACEnemy movement runtime LOD mode. 0: default, 1: block movement intent."),
		ECVF_Default);
}

int32 FAIMovementRuntimeLODPolicy::GetEnemyMovementMode()
{
	return FMath::Clamp(
		CVarAIRuntimeLODEnemyMovementMode.GetValueOnGameThread(),
		ToMovementRuntimeLODModeValue(EAIMovementRuntimeLODMode::Default),
		ToMovementRuntimeLODModeValue(EAIMovementRuntimeLODMode::BlockMovementIntent));
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

bool FAIMovementRuntimeLODPolicy::ShouldBlockMovementIntent(int32 InMovementMode)
{
	return InMovementMode == ToMovementRuntimeLODModeValue(EAIMovementRuntimeLODMode::BlockMovementIntent);
}

int32 FAIMovementRuntimeLODPolicy::GetStateBasedMovementMode(const AActor* InOwner)
{
	const APawn* ownerPawn = Cast<APawn>(InOwner);
	const ACAIController* aiController = IsValid(ownerPawn) ? Cast<ACAIController>(ownerPawn->GetController()) : nullptr;
	if (!IsValid(aiController)) return ToMovementRuntimeLODModeValue(EAIMovementRuntimeLODMode::Default);

	switch (aiController->GetCurrentRuntimeLODTier())
	{
	case EAIRuntimeLODTier::Awareness:
	case EAIRuntimeLODTier::Dormant:
		return ToMovementRuntimeLODModeValue(EAIMovementRuntimeLODMode::BlockMovementIntent);

	case EAIRuntimeLODTier::CombatCritical:
	case EAIRuntimeLODTier::CombatSupport:
	case EAIRuntimeLODTier::Background:
	default:
		return ToMovementRuntimeLODModeValue(EAIMovementRuntimeLODMode::Default);
	}
}
