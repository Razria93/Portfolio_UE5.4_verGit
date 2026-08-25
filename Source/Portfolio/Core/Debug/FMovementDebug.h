#pragma once

#include "CoreMinimal.h"
#include "Type/CStateTypes.h"
#include "Type/CMovementTypes.h"

class APawn;
class UWorld;

struct FMovementDebugSnapshot
{
	bool bHasSnapshot = false;
	FVector VelocityWorld = FVector::ZeroVector;
	FVector VelocityLocal = FVector::ZeroVector;
	FVector LastInputWorld = FVector::ZeroVector;
	FVector LastInputLocal = FVector::ZeroVector;
	float Speed = 0.f;
	float Direction = 0.f;
};

struct FMovementDebugOverlayDetails
{
	bool bHasSnapshot = false;
	FString VelocityWorldText;
	FString VelocityLocalText;
	FString LastInputWorldText;
	FString LastInputLocalText;
	FString SpeedText;
	FString DirectionText;
};

class PORTFOLIO_API FMovementDebug
{
public:
	// ===== Display Gates =====

	static bool IsEnabled();
	static bool ShouldDrawVelocity();
	static bool ShouldDrawInput();
	static bool ShouldDrawFacing();
	static bool ShouldDrawDebugText();

	// ===== Audit Gate =====

	static bool ShouldAuditMovement();

public:
	// ===== Display Snapshot / Presentation =====

	static FMovementDebugSnapshot BuildSnapshot(const APawn* InPawn);
	static FMovementDebugOverlayDetails BuildOverlayDetails(const FMovementDebugSnapshot& InSnapshot);
	static void DrawWorldDebug(UWorld* InWorld, const APawn* InPawn, const FMovementDebugSnapshot& InSnapshot);

public:
	// ===== Runtime LOD Audit Hooks =====

	static void RecordRuntimeLODMovementModeAppliedForAudit(const AActor* InOwnerActor, const UObject* InComponent, int32 InPreviousMode, int32 InNewMode, bool bInMovementEnabled, bool bInIntentBlocked);
	static void RecordRuntimeLODMovementIntentAllowedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InEvent);
	static void RecordRuntimeLODMovementIntentBlockedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InEvent);

public:
	// ===== Movement Gate Audit Hooks =====

	static void RecordMovementInputAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FVector2D& InAxis2D, EMovementGait InGait);
	static void RecordMovementInputRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FVector2D& InAxis2D, const TCHAR* InReason, EExecutionState InExecutionState, bool bInMovementEnabled, bool bInIntentBlocked);

public:
	// ===== Movement Data Audit Hooks =====

	static void RecordMovementGaitAppliedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EMovementGait InGait, float InSpeed);
	static void RecordMovementGaitRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EMovementGait InGait, const TCHAR* InReason);
};
