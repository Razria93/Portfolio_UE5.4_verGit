#pragma once

#include "CoreMinimal.h"
#include "Type/CStateStructure.h"
#include "Type/CMovementStructure.h"

class PORTFOLIO_API FMovementDebug
{
public:
	// Gate
	static bool ShouldAuditMovement();

public:
	// Runtime LOD Diagnostic Hook
	static void RecordRuntimeLODMovementModeAppliedForAudit(const AActor* InOwnerActor, const UObject* InComponent, int32 InPreviousMode, int32 InNewMode, bool bInTickEnabled, bool bInCanMove, bool bInIntentBlocked);
	static void RecordRuntimeLODMovementIntentAllowedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InEvent);
	static void RecordRuntimeLODMovementIntentBlockedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InEvent);

public:
	// Movement Gate Diagnostic Hook
	static void RecordMovementInputAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FVector2D& InAxis2D, EMovementGait InGait);
	static void RecordMovementInputRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FVector2D& InAxis2D, const TCHAR* InReason, EExecutionState InExecutionState, bool bInCanMove, bool bInIntentBlocked);

public:
	// Movement Data Diagnostic Hook
	static void RecordMovementGaitAppliedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EMovementGait InGait, float InSpeed);
	static void RecordMovementGaitRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EMovementGait InGait, const TCHAR* InReason);
};
