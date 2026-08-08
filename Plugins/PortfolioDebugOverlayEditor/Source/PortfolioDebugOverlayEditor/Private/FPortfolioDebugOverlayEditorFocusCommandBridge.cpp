#include "FPortfolioDebugOverlayEditorFocusCommandBridge.h"

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Selection.h"

#define LOCTEXT_NAMESPACE "FPortfolioDebugOverlayEditorFocusCommandBridge"

namespace
{
	// ===== Constants =====

	static constexpr const TCHAR* DebugOverlaySelectNearestFocusCommand = TEXT("DebugOverlaySelectNearestFocus");
	static constexpr const TCHAR* DebugOverlaySelectRecentCombatFocusCommand = TEXT("DebugOverlaySelectRecentCombatFocus");
	static constexpr const TCHAR* DebugOverlayClearFocusCommand = TEXT("DebugOverlayClearFocus");
	static constexpr const TCHAR* DebugOverlaySelectOutlinerFocusCommand = TEXT("DebugOverlaySelectOutlinerFocus");

	// ===== PIE World Access =====

	UWorld* FindDebugOverlayPIEWorld()
	{
		if (!GEngine) return nullptr;

		for (const FWorldContext& worldContext : GEngine->GetWorldContexts())
		{
			if (worldContext.WorldType != EWorldType::PIE) continue;

			UWorld* world = worldContext.World();
			if (IsValid(world)) return world;
		}

		return nullptr;
	}

	// ===== Editor Selection =====

	AActor* GetFirstSelectedEditorActor()
	{
		if (!GEditor) return nullptr;

		USelection* selectedActors = GEditor->GetSelectedActors();
		if (!IsValid(selectedActors) || selectedActors->Num() <= 0) return nullptr;

		for (FSelectionIterator it(*selectedActors); it; ++it)
		{
			AActor* actor = Cast<AActor>(*it);
			if (IsValid(actor)) return actor;
		}

		return nullptr;
	}

	// ===== Command Execution =====

	FText ExecuteDebugOverlayFocusCommand(const TCHAR* InCommand, const FText& InSuccessStatus)
	{
		UWorld* world = FindDebugOverlayPIEWorld();
		if (!IsValid(world))
		{
			return LOCTEXT("PIEWorldNotAvailable", "PIE world not available");
		}

		APlayerController* playerController = world->GetFirstPlayerController();
		if (!IsValid(playerController))
		{
			return LOCTEXT("PlayerControllerNotAvailable", "PlayerController not available");
		}

		playerController->ConsoleCommand(InCommand, true);
		return InSuccessStatus;
	}
}

// ===== Focus Commands =====

FText PortfolioDebugOverlayEditorFocusCommandBridge::ExecuteSelectNearestFocusCommand()
{
	return ExecuteDebugOverlayFocusCommand(
		DebugOverlaySelectNearestFocusCommand,
		LOCTEXT("SelectNearestFocusSent", "Last Command: SelectNearestFocus"));
}

FText PortfolioDebugOverlayEditorFocusCommandBridge::ExecuteSelectRecentCombatFocusCommand()
{
	return ExecuteDebugOverlayFocusCommand(
		DebugOverlaySelectRecentCombatFocusCommand,
		LOCTEXT("SelectRecentCombatFocusSent", "Last Command: SelectRecentCombatFocus"));
}

FText PortfolioDebugOverlayEditorFocusCommandBridge::ExecuteClearFocusCommand()
{
	return ExecuteDebugOverlayFocusCommand(
		DebugOverlayClearFocusCommand,
		LOCTEXT("ClearFocusSent", "Last Command: ClearFocus"));
}

FText PortfolioDebugOverlayEditorFocusCommandBridge::ExecuteSelectOutlinerFocusCommand()
{
	AActor* selectedActor = GetFirstSelectedEditorActor();
	if (!IsValid(selectedActor))
	{
		return LOCTEXT("NoEditorActorSelected", "No editor actor selected");
	}

	const FString actorName = selectedActor->GetName();
	const FString command = FString::Printf(TEXT("%s %s"), DebugOverlaySelectOutlinerFocusCommand, *actorName);
	return ExecuteDebugOverlayFocusCommand(
		*command,
		FText::Format(LOCTEXT("SelectOutlinerFocusSent", "Last Command: SelectOutlinerFocus | Actor: {0}"), FText::FromString(actorName)));
}

#undef LOCTEXT_NAMESPACE
