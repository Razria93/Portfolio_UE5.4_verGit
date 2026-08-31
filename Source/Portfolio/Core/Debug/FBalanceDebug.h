#pragma once

#include "CoreMinimal.h"
#include "Type/CBalanceTypes.h"

class ACEnemy;
class UCBalanceComponent;
class UWorld;

struct FBalanceDebugSnapshot
{
	bool bHasSnapshot = false;
	int32 CurrentCount = 0;
	int32 Threshold = 0;
	uint32 LifecycleSerial = 0;
	EBalanceLifecycleState LifecycleState = EBalanceLifecycleState::Accumulating;
	EBalanceAbortReason LastAbortReason = EBalanceAbortReason::None;
	float LoopRemainingSeconds = 0.f;
	float LoopDurationSeconds = 0.f;
	float ExecutionDownRemainingSeconds = 0.f;
	float ExecutionDownDurationSeconds = 0.f;
	bool bIsCollapsePoseActive = false;
	bool bIsCollapseLoopActive = false;
	bool bIsExecutionDownPoseActive = false;
	bool bIsLifecycleBlocking = false;
	bool bIsFacingSuppressed = false;
};

struct FBalanceDebugOverlayDetails
{
	bool bHasSnapshot = false;
	FString CountText;
	FString LifecycleText;
	FString LifecycleSerialText;
	FString LoopLifetimeText;
	FString ExecutionDownLifetimeText;
	FString CollapsePoseText;
	FString CollapseLoopText;
	FString ExecutionDownPoseText;
	FString LifecycleBlockingText;
	FString FacingSuppressedText;
	FString LastAbortText;
};

class PORTFOLIO_API FBalanceDebug
{
public:
	// Display Gates
	static bool IsEnabled();
	static bool ShouldDrawWorldText();
	static bool ShouldDrawLifecycleBar();

	// Audit Gate
	static bool ShouldAuditBalance();

	// Display Snapshot / Presentation
	static FBalanceDebugSnapshot BuildSnapshot(const ACEnemy* InEnemy);
	static FBalanceDebugOverlayDetails BuildOverlayDetails(const FBalanceDebugSnapshot& InSnapshot);
	static void DrawWorldDebug(UWorld* InWorld, const ACEnemy* InEnemy, const FBalanceDebugSnapshot& InSnapshot);

	// Lifecycle Audit Hooks
	static void RecordLifecycleEvent(const UCBalanceComponent* InBalanceComp, const TCHAR* InEvent, const FString& InDetail = FString());
	static void RecordLifecycleStateChanged(const UCBalanceComponent* InBalanceComp, EBalanceLifecycleState InPreviousState, EBalanceLifecycleState InNewState);
};
