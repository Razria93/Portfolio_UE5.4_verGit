#pragma once

#include "Core/Debug/FCombatMotionDebugTypes.h"

class UCMovementComponent;
class UWorld;

class PORTFOLIO_API FCombatKnockbackDebug
{
public:
	// Presentation
	static bool IsEnabled();
	static TArray<FString> BuildOverlayLines(const AActor* InActor);
	static void DrawWorldDebug(UWorld* InWorld, const AActor* InActor);

	// Diagnostic Hooks
	static void Record(const AActor* InActor, const TCHAR* InStage, const TCHAR* InResult, const TCHAR* InReason,
		const FCombatKnockbackContext& InContext, uint64 InGeneration = 0, const AActor* InOtherActor = nullptr);
	static void RecordStarted(const UCMovementComponent* InMovement, const FCombatKnockbackContext& InContext);
	static void RecordStopped(const UCMovementComponent* InMovement, const TCHAR* InReason);
};
