#include "Core/Debug/CDebugOverlayHUD.h"

#include "Character/Enemy/CEnemy.h"
#include "Core/Debug/CDebugOverlayTargetComponent.h"
#include "Core/Debug/FDebugOverlayCanvasRenderer.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FDebugOverlayTextFormatter.h"
#include "Core/Debug/FDebugOverlayViewDataBuilder.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#if !UE_BUILD_SHIPPING
namespace
{
	void UpdateLastFocusCommand(FDebugOverlayFocusViewData& InOutFocusViewData, const UCDebugOverlayTargetComponent* InTargetComp)
	{
		if (!IsValid(InTargetComp)) return;
		if (!InTargetComp->HasDebugOverlayFocusCommandResult()) return;

		InOutFocusViewData.LastCommandText = InTargetComp->GetDebugOverlayFocusCommandResultText();
	}
}
#endif

#if !UE_BUILD_SHIPPING
ACEnemy* ACDebugOverlayHUD::ResolveDisplayEnemy(FDebugOverlayFocusViewData& OutFocusViewData)
{
	if (ACEnemy* targetComponentEnemy = ResolveTargetComponentEnemy(OutFocusViewData))
	{
		return targetComponentEnemy;
	}

	OutFocusViewData.CurrentModeText = TEXT("None");
	OutFocusViewData.CurrentActorNameText = TEXT("None");
	if (const APlayerController* owningPlayerController = GetOwningPlayerController())
	{
		const UCDebugOverlayTargetComponent* targetComp = owningPlayerController->FindComponentByClass<UCDebugOverlayTargetComponent>();
		UpdateLastFocusCommand(OutFocusViewData, targetComp);
	}

	return nullptr;
}

ACEnemy* ACDebugOverlayHUD::ResolveTargetComponentEnemy(FDebugOverlayFocusViewData& OutFocusViewData) const
{
	const APlayerController* owningPlayerController = GetOwningPlayerController();
	if (!IsValid(owningPlayerController)) return nullptr;

	const UCDebugOverlayTargetComponent* targetComp = owningPlayerController->FindComponentByClass<UCDebugOverlayTargetComponent>();
	if (!IsValid(targetComp)) return nullptr;

	ACEnemy* targetEnemy = Cast<ACEnemy>(targetComp->GetDebugOverlayFocusActor());
	if (!IsValid(targetEnemy)) return nullptr;

	OutFocusViewData.CurrentModeText = targetComp->GetDebugOverlayFocusModeText();
	OutFocusViewData.CurrentActorNameText = GetNameSafe(targetEnemy);
	UpdateLastFocusCommand(OutFocusViewData, targetComp);
	return targetEnemy;
}
#endif

void ACDebugOverlayHUD::DrawHUD()
{
#if !UE_BUILD_SHIPPING
	Super::DrawHUD();

	if (!FDebugOverlaySnapshotStore::IsEnabled()) return;

	UWorld* world = GetWorld();
	FDebugOverlayFocusViewData enemyFocus;
	const ACEnemy* enemy = ResolveDisplayEnemy(enemyFocus);

	FDebugOverlayViewDataBuildContext context;
	context.WorldContextObject = world;
	context.World = world;
	context.ViewerPawn = GetOwningPawn();
	context.DisplayEnemy = enemy;
	context.EnemyFocus = enemyFocus;

	const FDebugOverlayViewData viewData = FDebugOverlayViewDataBuilder::Build(context);
	const FDebugOverlayTextPanels textPanels = FDebugOverlayTextFormatter::Format(viewData);
	FDebugOverlayCanvasRenderer::Draw(*this, Canvas, textPanels);
#endif
}
