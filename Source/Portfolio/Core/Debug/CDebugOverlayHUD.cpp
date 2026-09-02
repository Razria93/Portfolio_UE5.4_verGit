#include "Core/Debug/CDebugOverlayHUD.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CPlayerTargetSelectionComponent.h"
#include "Core/Debug/FBalanceDebug.h"
#include "Core/Debug/FCombatParticipationDebug.h"
#include "Core/Debug/FDebugOverlayDisplayConfig.h"
#include "Core/Debug/FEnemyCombatTargetFacingDebug.h"
#include "Core/Debug/FExecutionCollaborationDebug.h"
#include "Core/Debug/FMovementDebug.h"
#include "Core/Debug/CDebugOverlayFocusComponent.h"
#include "Core/Debug/FDebugOverlayCanvasRenderer.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FDebugOverlayTextFormatter.h"
#include "Core/Debug/FDebugOverlayViewDataBuilder.h"
#include "Core/Debug/FTargetingDebug.h"
#include "System/Combat/CWorldSubsystem_CombatParticipation.h"
#include "Type/CTargetingTypes.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

#if !UE_BUILD_SHIPPING
namespace
{
	// ===== Focus View Data =====

	const UCDebugOverlayFocusComponent* ResolveFocusComponent(const APlayerController* InOwningPlayerController)
	{
		if (!IsValid(InOwningPlayerController)) return nullptr;
		return InOwningPlayerController->FindComponentByClass<UCDebugOverlayFocusComponent>();
	}

	ACEnemy* ResolveFocusComponentEnemy(const UCDebugOverlayFocusComponent* InFocusComp)
	{
		if (!IsValid(InFocusComp)) return nullptr;
		return Cast<ACEnemy>(InFocusComp->GetDebugOverlayFocusActor());
	}

	void UpdateFocusDriverText(const UCDebugOverlayFocusComponent* InFocusComp, FDebugOverlayFocusViewData& InOutFocusViewData)
	{
		if (!IsValid(InFocusComp)) return;

		InOutFocusViewData.FocusDriverText = InFocusComp->GetDebugOverlayFocusDriverText();
		InOutFocusViewData.RecentFocusStateText = InFocusComp->GetDebugOverlayRecentFocusStateText();
	}

	void UpdateFocusViewData(const UCDebugOverlayFocusComponent* InFocusComp, FDebugOverlayFocusViewData& OutFocusViewData)
	{
		if (!IsValid(InFocusComp)) return;

		OutFocusViewData.CurrentSourceText = InFocusComp->GetDebugOverlayFocusSourceText();
		OutFocusViewData.CurrentActorNameText = InFocusComp->GetDebugOverlayFocusActorNameText();
		UpdateFocusDriverText(InFocusComp, OutFocusViewData);
	}

	void ClearFocusViewData(const UCDebugOverlayFocusComponent* InFocusComp, FDebugOverlayFocusViewData& InOutFocusViewData)
	{
		InOutFocusViewData.CurrentSourceText = TEXT("None");
		InOutFocusViewData.CurrentActorNameText = TEXT("None");
		UpdateFocusDriverText(InFocusComp, InOutFocusViewData);
	}

	ACEnemy* ResolveDisplayFocusEnemy(const APlayerController* InOwningPlayerController, FDebugOverlayFocusViewData& OutFocusViewData)
	{
		const UCDebugOverlayFocusComponent* focusComp = ResolveFocusComponent(InOwningPlayerController);
		if (ACEnemy* focusComponentEnemy = ResolveFocusComponentEnemy(focusComp))
		{
			UpdateFocusViewData(focusComp, OutFocusViewData);
			return focusComponentEnemy;
		}

		ClearFocusViewData(focusComp, OutFocusViewData);
		return nullptr;
	}

	// ===== Targeting Debug =====

	void DrawTargetingDebug(const APlayerController* InOwningPlayerController, UWorld* InWorld)
	{
		if (!FTargetingDebug::IsEnabled()) return;
		if (!IsValid(InOwningPlayerController)) return;

		const UCPlayerTargetSelectionComponent* targetingComp = InOwningPlayerController->FindComponentByClass<UCPlayerTargetSelectionComponent>();
		if (!IsValid(targetingComp)) return;

		FTargetingEvaluation targetingEvaluation;
		if (!targetingComp->BuildSelectionDebugSnapshot(targetingEvaluation)) return;

		FTargetingDebug::DrawWorldDebug(InWorld, targetingEvaluation);
	}

	FMovementDebugSnapshot BuildPlayerLocomotionSnapshot(const APlayerController* InOwningPlayerController)
	{
		if (!FMovementDebug::IsEnabled()) return FMovementDebugSnapshot();
		const APawn* playerPawn = IsValid(InOwningPlayerController) ? InOwningPlayerController->GetPawn() : nullptr;
		return IsValid(playerPawn) ? FMovementDebug::BuildSnapshot(playerPawn) : FMovementDebugSnapshot();
	}

	void DrawMovementDebug(UWorld* InWorld, const APawn* InPlayerPawn, const FMovementDebugSnapshot& InSnapshot)
	{
		if (!InSnapshot.bHasSnapshot) return;

		FMovementDebug::DrawWorldDebug(InWorld, InPlayerPawn, InSnapshot);
	}

	void UpdatePlayerTargetingViewData(const APlayerController* InOwningPlayerController, const FDebugOverlayPanelVisibility& InPanelVisibility, FDebugOverlayPlayerTargetingViewData& OutPlayerTargetingViewData)
	{
		if (!InPanelVisibility.bShowPlayer || !InPanelVisibility.bShowPlayerTargeting) return;
		if (!IsValid(InOwningPlayerController)) return;

		const UCPlayerTargetSelectionComponent* targetingComp = InOwningPlayerController->FindComponentByClass<UCPlayerTargetSelectionComponent>();
		if (!IsValid(targetingComp)) return;

		FTargetingEvaluation targetingEvaluation;
		if (!targetingComp->BuildSelectionDebugSnapshot(targetingEvaluation)) return;

		OutPlayerTargetingViewData.Details = FTargetingDebug::BuildOverlayDetails(targetingEvaluation);
	}

	void UpdatePlayerLocomotionViewData(const FMovementDebugSnapshot& InSnapshot, const FDebugOverlayPanelVisibility& InPanelVisibility, FDebugOverlayPlayerLocomotionViewData& OutPlayerLocomotionViewData)
	{
		if (!InPanelVisibility.bShowPlayer || !InPanelVisibility.bShowPlayerLocomotion) return;

		OutPlayerLocomotionViewData.Details = FMovementDebug::BuildOverlayDetails(InSnapshot);
	}

	FBalanceDebugSnapshot BuildBalanceCollapseSnapshot(const ACEnemy* InFocusedEnemy)
	{
		return FBalanceDebug::BuildSnapshot(InFocusedEnemy);
	}

	void UpdateBalanceCollapseViewData(const FBalanceDebugSnapshot& InSnapshot, const FDebugOverlayPanelVisibility& InPanelVisibility, FDebugOverlayBalanceCollapseViewData& OutBalanceCollapseViewData)
	{
		if (!InPanelVisibility.bShowEnemy || !InPanelVisibility.bShowEnemyBalanceCollapse) return;

		OutBalanceCollapseViewData.Details = FBalanceDebug::BuildOverlayDetails(InSnapshot);
	}

	FEnemyCombatTargetFacingDebugSnapshot BuildCombatTargetFacingSnapshot(const ACEnemy* InFocusedEnemy)
	{
		return FEnemyCombatTargetFacingDebug::BuildSnapshot(InFocusedEnemy);
	}

	void UpdateCombatTargetFacingViewData(const FEnemyCombatTargetFacingDebugSnapshot& InSnapshot, const FDebugOverlayPanelVisibility& InPanelVisibility, FDebugOverlayCombatTargetFacingViewData& OutCombatTargetFacingViewData)
	{
		if (!InPanelVisibility.bShowEnemy || !InPanelVisibility.bShowEnemyCombatTargetFacing) return;

		OutCombatTargetFacingViewData.Details = FEnemyCombatTargetFacingDebug::BuildOverlayDetails(InSnapshot);
	}

	FExecutionCollaborationDebugSnapshot BuildExecutionCollaborationSnapshot(const ACharacter* InCharacter)
	{
		return FExecutionCollaborationDebug::BuildSnapshot(InCharacter);
	}

	void UpdateExecutionCollaborationViewData(const FExecutionCollaborationDebugSnapshot& InSnapshot, const bool bInVisible, FDebugOverlayExecutionCollaborationViewData& OutExecutionCollaborationViewData)
	{
		if (!bInVisible) return;

		OutExecutionCollaborationViewData.Details = FExecutionCollaborationDebug::BuildOverlayDetails(InSnapshot);
	}

	FCombatParticipationDebugSnapshot BuildCombatParticipationSnapshot(UWorld* InWorld)
	{
		if (!FCombatParticipationDebug::IsEnabled() || !IsValid(InWorld)) return FCombatParticipationDebugSnapshot();

		const UCWorldSubsystem_CombatParticipation* participationSubsystem = InWorld->GetSubsystem<UCWorldSubsystem_CombatParticipation>();
		return IsValid(participationSubsystem) ? participationSubsystem->BuildDebugSnapshot() : FCombatParticipationDebugSnapshot();
	}

	void UpdateCombatParticipationViewData(const FCombatParticipationDebugSnapshot& InSnapshot, const ACEnemy* InFocusedEnemy, const FDebugOverlayPanelVisibility& InPanelVisibility, FDebugOverlayCombatParticipationViewData& OutCombatParticipationViewData)
	{
		if (!FCombatParticipationDebug::IsEnabled()) return;

		if (InPanelVisibility.bShowEnemy && InPanelVisibility.bShowEnemyCombatParticipation)
		{
			OutCombatParticipationViewData.FocusedEnemyDetails = FCombatParticipationDebug::BuildOverlayDetails(InSnapshot, InFocusedEnemy);
		}

		if (InPanelVisibility.bShowWorldSummaryCombatParticipation)
		{
			OutCombatParticipationViewData.WorldSummaryLines = FCombatParticipationDebug::BuildWorldSummaryLines(InSnapshot);
		}
	}
}
#endif

// ===== HUD Rendering =====

void ACDebugOverlayHUD::DrawHUD()
{
#if !UE_BUILD_SHIPPING
	Super::DrawHUD();

	UWorld* world = GetWorld();
	const APlayerController* owningPlayerController = GetOwningPlayerController();
	const APawn* playerPawn = IsValid(owningPlayerController) ? owningPlayerController->GetPawn() : nullptr;
	const ACharacter* playerCharacter = Cast<ACharacter>(playerPawn);
	const FMovementDebugSnapshot playerLocomotionSnapshot = BuildPlayerLocomotionSnapshot(owningPlayerController);
	const FCombatParticipationDebugSnapshot combatParticipationSnapshot = BuildCombatParticipationSnapshot(world);
	FDebugOverlayFocusViewData enemyFocus;
	const ACEnemy* focusedEnemy = ResolveDisplayFocusEnemy(owningPlayerController, enemyFocus);
	const FBalanceDebugSnapshot balanceCollapseSnapshot = BuildBalanceCollapseSnapshot(focusedEnemy);
	const FEnemyCombatTargetFacingDebugSnapshot combatTargetFacingSnapshot = BuildCombatTargetFacingSnapshot(focusedEnemy);
	const FExecutionCollaborationDebugSnapshot playerExecutionCollaborationSnapshot = BuildExecutionCollaborationSnapshot(playerCharacter);
	const FExecutionCollaborationDebugSnapshot enemyExecutionCollaborationSnapshot = BuildExecutionCollaborationSnapshot(focusedEnemy);

	DrawTargetingDebug(owningPlayerController, world);
	DrawMovementDebug(world, playerPawn, playerLocomotionSnapshot);
	FCombatParticipationDebug::DrawWorldDebug(world, combatParticipationSnapshot);
	FBalanceDebug::DrawWorldDebug(world, focusedEnemy, balanceCollapseSnapshot);
	FExecutionCollaborationDebug::DrawWorldDebug(world, playerExecutionCollaborationSnapshot);

	if (!FDebugOverlaySnapshotStore::IsEnabled()) return;
	const FDebugOverlayPanelVisibility panelVisibility = DebugOverlayDisplayConfig::GetPanelVisibility();
	FDebugOverlayPlayerTargetingViewData playerTargeting;
	FDebugOverlayPlayerLocomotionViewData playerLocomotion;
	FDebugOverlayExecutionCollaborationViewData playerExecutionCollaboration;
	FDebugOverlayBalanceCollapseViewData balanceCollapse;
	FDebugOverlayCombatTargetFacingViewData combatTargetFacing;
	FDebugOverlayExecutionCollaborationViewData enemyExecutionCollaboration;
	FDebugOverlayCombatParticipationViewData combatParticipation;
	UpdatePlayerTargetingViewData(owningPlayerController, panelVisibility, playerTargeting);
	UpdatePlayerLocomotionViewData(playerLocomotionSnapshot, panelVisibility, playerLocomotion);
	UpdateExecutionCollaborationViewData(playerExecutionCollaborationSnapshot, panelVisibility.bShowPlayer && panelVisibility.bShowPlayerExecutionCollaboration, playerExecutionCollaboration);
	UpdateBalanceCollapseViewData(balanceCollapseSnapshot, panelVisibility, balanceCollapse);
	UpdateCombatTargetFacingViewData(combatTargetFacingSnapshot, panelVisibility, combatTargetFacing);
	UpdateExecutionCollaborationViewData(enemyExecutionCollaborationSnapshot, panelVisibility.bShowEnemy && panelVisibility.bShowEnemyExecutionCollaboration, enemyExecutionCollaboration);
	UpdateCombatParticipationViewData(combatParticipationSnapshot, focusedEnemy, panelVisibility, combatParticipation);

	const FDebugOverlayViewData viewData = FDebugOverlayViewDataBuilder::Build(world, GetOwningPawn(), focusedEnemy, enemyFocus, playerTargeting, playerLocomotion, playerExecutionCollaboration, balanceCollapse, combatTargetFacing, enemyExecutionCollaboration, combatParticipation, panelVisibility);
	const FDebugOverlayTextPanels textPanels = FDebugOverlayTextFormatter::Format(viewData);

	FDebugOverlayCanvasRenderer::Draw(*this, Canvas, textPanels);
#endif
}
