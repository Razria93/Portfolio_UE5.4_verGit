#include "Core/Debug/CDebugOverlayHUD.h"

#include "Character/Enemy/CEnemy.h"
#include "Core/Debug/CDebugOverlayFocusComponent.h"
#include "Core/Debug/FDebugOverlayCanvasRenderer.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FDebugOverlayTextFormatter.h"
#include "Core/Debug/FDebugOverlayViewDataBuilder.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#if !UE_BUILD_SHIPPING
namespace
{
	void UpdateLastFocusCommand(FDebugOverlayFocusViewData& InOutFocusViewData, const UCDebugOverlayFocusComponent* InFocusComp)
	{
		if (!IsValid(InFocusComp)) return;
		if (!InFocusComp->HasDebugOverlayFocusCommandResult()) return;

		InOutFocusViewData.LastCommandText = InFocusComp->GetDebugOverlayFocusCommandResultText();
	}
}
#endif

#if !UE_BUILD_SHIPPING
ACEnemy* ACDebugOverlayHUD::ResolveDisplayEnemy(FDebugOverlayFocusViewData& OutFocusViewData)
{
	if (ACEnemy* focusComponentEnemy = ResolveFocusComponentEnemy(OutFocusViewData))
	{
		return focusComponentEnemy;
	}

	OutFocusViewData.CurrentModeText = TEXT("None");
	OutFocusViewData.CurrentActorNameText = TEXT("None");
	if (const APlayerController* owningPlayerController = GetOwningPlayerController())
	{
		const UCDebugOverlayFocusComponent* focusComp = owningPlayerController->FindComponentByClass<UCDebugOverlayFocusComponent>();
		UpdateLastFocusCommand(OutFocusViewData, focusComp);
	}

	return nullptr;
}

ACEnemy* ACDebugOverlayHUD::ResolveFocusComponentEnemy(FDebugOverlayFocusViewData& OutFocusViewData) const
{
	const APlayerController* owningPlayerController = GetOwningPlayerController();
	if (!IsValid(owningPlayerController)) return nullptr;

	const UCDebugOverlayFocusComponent* focusComp = owningPlayerController->FindComponentByClass<UCDebugOverlayFocusComponent>();
	if (!IsValid(focusComp)) return nullptr;

	ACEnemy* targetEnemy = Cast<ACEnemy>(focusComp->GetDebugOverlayFocusActor());
	if (!IsValid(targetEnemy)) return nullptr;

	OutFocusViewData.CurrentModeText = focusComp->GetDebugOverlayFocusModeText();
	OutFocusViewData.CurrentActorNameText = GetNameSafe(targetEnemy);
	UpdateLastFocusCommand(OutFocusViewData, focusComp);
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
