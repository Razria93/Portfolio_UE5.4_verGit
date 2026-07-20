#pragma once

#include "CoreMinimal.h"
#include "Type/CWeaponStructure.h"

class PORTFOLIO_API FObservableOverlayDebug
{
public:
	// Gate
	static bool ShouldAuditObservableOverlay();

public:
	// Overlay Handling Diagnostic Hook
	static void RecordOverlayHandlingRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EObservableOverlayHandling InHandling, const TCHAR* InReason);
	static void RecordOverlayHandlingsRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TArray<EObservableOverlayHandling>& InHandlings, EObservableOverlayHandling InFailedHandling, const TCHAR* InReason);
};
