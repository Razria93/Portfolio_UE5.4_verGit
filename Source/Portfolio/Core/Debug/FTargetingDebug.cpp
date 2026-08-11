#include "Core/Debug/FTargetingDebug.h"

#include "Type/CTargetingTypes.h"

#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarTargetingDebugEnabled(
		TEXT("Portfolio.DebugOverlay.Targeting.Enabled"),
		0,
		TEXT("Enable player targeting debug visualization. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarTargetingDrawRangeSphere(
		TEXT("Portfolio.DebugOverlay.Targeting.DrawRangeSphere"),
		1,
		TEXT("Draw the player targeting maximum range sphere. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarTargetingDrawSelectedTargetSphere(
		TEXT("Portfolio.DebugOverlay.Targeting.DrawSelectedTargetSphere"),
		1,
		TEXT("Draw the current player target sphere. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarTargetingDrawViewLine(
		TEXT("Portfolio.DebugOverlay.Targeting.DrawViewLine"),
		1,
		TEXT("Draw a line from the player viewpoint to the current target. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarTargetingDrawDebugText(
		TEXT("Portfolio.DebugOverlay.Targeting.DrawDebugText"),
		1,
		TEXT("Draw player targeting debug text at the current target. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarTargetingShowOverlayDetails(
		TEXT("Portfolio.DebugOverlay.Targeting.ShowOverlayDetails"),
		1,
		TEXT("Show player targeting score details in Debug Overlay. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

}

bool FTargetingDebug::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarTargetingDebugEnabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FTargetingDebug::ShouldDrawRangeSphere()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarTargetingDrawRangeSphere.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FTargetingDebug::ShouldDrawSelectedTargetSphere()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarTargetingDrawSelectedTargetSphere.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FTargetingDebug::ShouldDrawViewLine()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarTargetingDrawViewLine.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FTargetingDebug::ShouldDrawDebugText()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarTargetingDrawDebugText.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FTargetingDebug::ShouldShowOverlayDetails()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarTargetingShowOverlayDetails.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

FTargetingDebugOverlayDetails FTargetingDebug::BuildOverlayDetails(const FTargetingDebugSnapshot& InSnapshot)
{
	FTargetingDebugOverlayDetails details;
	if (!ShouldShowOverlayDetails()) return details;

	details.bHasSnapshot = true;
	AActor* targetActor = InSnapshot.TargetActor.Get();
	details.RuntimeTargetText = IsValid(targetActor) ? GetNameSafe(targetActor) : TEXT("None");

	details.DistanceText = FString::Printf(TEXT("%.1f / %.1f"), InSnapshot.Distance, InSnapshot.MaxTargetDistance);
	details.DotText = FString::Printf(TEXT("%.3f / Min %.3f"), InSnapshot.Dot, InSnapshot.MinDot);

	details.AngleScoreText = FString::Printf(TEXT("%.3f"), InSnapshot.AngleScore);
	details.DistanceScoreText = FString::Printf(TEXT("%.3f"), InSnapshot.DistanceScore);
	details.FinalScoreText = FString::Printf(TEXT("%.3f"), InSnapshot.FinalScore);

	details.RangeText = InSnapshot.bWithinRange ? TEXT("true") : TEXT("false");
	details.ViewConeText = InSnapshot.bWithinViewCone ? TEXT("true") : TEXT("false");
	return details;
}

void FTargetingDebug::DrawWorldDebug(UWorld* InWorld, const FTargetingDebugSnapshot& InSnapshot)
{
#if !UE_BUILD_SHIPPING
	if (!IsValid(InWorld)) return;
	if (ShouldDrawRangeSphere())
	{
		DrawDebugSphere(InWorld, InSnapshot.ViewLocation, InSnapshot.MaxTargetDistance, 24, FColor::Cyan, false, 0.f);
	}

	AActor* targetActor = InSnapshot.TargetActor.Get();
	if (!IsValid(targetActor)) return;

	if (ShouldDrawSelectedTargetSphere())
	{
		DrawDebugSphere(InWorld, InSnapshot.TargetLocation, 100.f, 16, FColor::Green, false, 0.f, 0, 3.f);
	}

	if (ShouldDrawViewLine())
	{
		DrawDebugLine(InWorld, InSnapshot.ViewLocation, InSnapshot.TargetLocation, FColor::Green, false, 0.f, 0, 1.5f);
	}

	if (ShouldDrawDebugText())
	{
		const FString text = FString::Printf(
			TEXT("Target: %s | Dist: %.1f | Dot: %.3f | Score: %.3f"),
			*GetNameSafe(targetActor),
			InSnapshot.Distance,
			InSnapshot.Dot,
			InSnapshot.FinalScore);
		DrawDebugString(InWorld, InSnapshot.TargetLocation + FVector(0.f, 0.f, 130.f), text, nullptr, FColor::Green, 0.f, false, 1.25f);
	}
#endif
}
