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
	void UpdateFocusDriverText(const UCDebugOverlayFocusComponent* InFocusComp, FDebugOverlayFocusViewData& InOutFocusViewData)
	{
		if (!IsValid(InFocusComp)) return;

		InOutFocusViewData.FocusDriverText = InFocusComp->GetDebugOverlayFocusDriverText();
		InOutFocusViewData.RecentFocusStateText = InFocusComp->GetDebugOverlayRecentFocusStateText();
	}

	const UCDebugOverlayFocusComponent* ResolveFocusComponent(const APlayerController* InOwningPlayerController)
	{
		if (!IsValid(InOwningPlayerController)) return nullptr;
		return InOwningPlayerController->FindComponentByClass<UCDebugOverlayFocusComponent>();
	}

	ACEnemy* ResolveFocusComponentTarget(const UCDebugOverlayFocusComponent* InFocusComp)
	{
		if (!IsValid(InFocusComp)) return nullptr;
		return Cast<ACEnemy>(InFocusComp->GetDebugOverlayFocusActor());
	}

	void UpdateTargetViewData(const UCDebugOverlayFocusComponent* InFocusComp, FDebugOverlayFocusViewData& OutFocusViewData)
	{
		if (!IsValid(InFocusComp)) return;

		OutFocusViewData.CurrentSourceText = InFocusComp->GetDebugOverlayFocusSourceText();
		OutFocusViewData.CurrentActorNameText = InFocusComp->GetDebugOverlayFocusActorNameText();
		UpdateFocusDriverText(InFocusComp, OutFocusViewData);
	}

	void ClearTargetViewData(const UCDebugOverlayFocusComponent* InFocusComp, FDebugOverlayFocusViewData& InOutFocusViewData)
	{
		InOutFocusViewData.CurrentSourceText = TEXT("None");
		InOutFocusViewData.CurrentActorNameText = TEXT("None");
		UpdateFocusDriverText(InFocusComp, InOutFocusViewData);
	}

	ACEnemy* ResolveDisplayTarget(const APlayerController* InOwningPlayerController, FDebugOverlayFocusViewData& OutFocusViewData)
	{
		const UCDebugOverlayFocusComponent* focusComp = ResolveFocusComponent(InOwningPlayerController);
		if (ACEnemy* focusComponentTarget = ResolveFocusComponentTarget(focusComp))
		{
			UpdateTargetViewData(focusComp, OutFocusViewData);
			return focusComponentTarget;
		}

		ClearTargetViewData(focusComp, OutFocusViewData);
		return nullptr;
	}
}
#endif

void ACDebugOverlayHUD::DrawHUD()
{
#if !UE_BUILD_SHIPPING
	Super::DrawHUD();

	if (!FDebugOverlaySnapshotStore::IsEnabled()) return;

	UWorld* world = GetWorld();
	const APlayerController* owningPlayerController = GetOwningPlayerController();
	FDebugOverlayFocusViewData targetFocus;
	const ACEnemy* target = ResolveDisplayTarget(owningPlayerController, targetFocus);

	const FDebugOverlayViewData viewData = FDebugOverlayViewDataBuilder::Build(world, GetOwningPawn(), target, targetFocus);
	const FDebugOverlayTextPanels textPanels = FDebugOverlayTextFormatter::Format(viewData);
	FDebugOverlayCanvasRenderer::Draw(*this, Canvas, textPanels);
#endif
}
