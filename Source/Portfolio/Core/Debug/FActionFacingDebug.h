#pragma once

#include "Core/Debug/FCombatMotionDebugTypes.h"

class UCActionComponent;
class UWorld;

class PORTFOLIO_API FActionFacingDebug
{
public:
	// Presentation
	static bool IsEnabled();
	static TArray<FString> BuildOverlayLines(const AActor* InActor);
	static void DrawWorldDebug(UWorld* InWorld, const AActor* InActor);

	// Diagnostic Hooks
	static void Record(const UCActionComponent* InAction, uint64 InGeneration, const AActor* InTarget,
		const TCHAR* InResult, const TCHAR* InReason, const FVector* InOrigin = nullptr, const float* InBeforeYaw = nullptr);
};
