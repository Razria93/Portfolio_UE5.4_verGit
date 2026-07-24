#include "Core/Debug/FObservableOverlayDebug.h"

#include "Core/Debug/FLog.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarObservableOverlayAudit(
		TEXT("Portfolio.Debug.ObservableOverlayAudit"),
		0,
		TEXT("Print observable overlay handling diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatObservableOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
	{
		if (InHandlings.IsEmpty()) return TEXT("None");

		TArray<FString> handlingNames;
		handlingNames.Reserve(InHandlings.Num());

		for (const EObservableOverlayHandling handling : InHandlings)
		{
			handlingNames.Add(UEnum::GetValueAsString(handling));
		}

		return FString::Join(handlingNames, TEXT(","));
	}
}

// Gate

bool FObservableOverlayDebug::ShouldAuditObservableOverlay()
{
#if !UE_BUILD_SHIPPING
	return CVarObservableOverlayAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Overlay Handling Diagnostic Hook

void FObservableOverlayDebug::RecordOverlayHandlingRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EObservableOverlayHandling InHandling, const TCHAR* InReason)
{
	if (!ShouldAuditObservableOverlay()) return;

	FLog::Log(FString::Printf(
		TEXT("[Overlay|Component|HandlingRejected] Reason=%s | Owner=%s | Component=%s | Handling=%s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*UEnum::GetValueAsString(InHandling)));
}

void FObservableOverlayDebug::RecordOverlayHandlingsRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TArray<EObservableOverlayHandling>& InHandlings, EObservableOverlayHandling InFailedHandling, const TCHAR* InReason)
{
	if (!ShouldAuditObservableOverlay()) return;

	FLog::Log(FString::Printf(
		TEXT("[Overlay|Component|HandlingsRejected] Reason=%s | Owner=%s | Component=%s | FailedHandling=%s | Handlings=%s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*UEnum::GetValueAsString(InFailedHandling),
		*FormatObservableOverlayHandlings(InHandlings)));
}
