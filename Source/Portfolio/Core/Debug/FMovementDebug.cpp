#include "Core/Debug/FMovementDebug.h"

#include "Component/CMovementComponent.h"
#include "Core/Debug/FLog.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	// ===== Display CVars =====

	TAutoConsoleVariable<int32> CVarMovementDebugEnabled(
		TEXT("Portfolio.DebugOverlay.Movement.Enabled"),
		0,
		TEXT("Enable movement debug data and world visualization. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarMovementDrawVelocity(
		TEXT("Portfolio.DebugOverlay.Movement.DrawVelocity"),
		1,
		TEXT("Draw the current movement velocity arrow. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarMovementDrawInput(
		TEXT("Portfolio.DebugOverlay.Movement.DrawInput"),
		1,
		TEXT("Draw the last movement input arrow. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarMovementDrawFacing(
		TEXT("Portfolio.DebugOverlay.Movement.DrawFacing"),
		1,
		TEXT("Draw the actor facing arrow. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarMovementDrawDebugText(
		TEXT("Portfolio.DebugOverlay.Movement.DrawDebugText"),
		1,
		TEXT("Draw movement speed and direction debug text. 0: disabled, 1: enabled."),
		ECVF_Default);

	// ===== Audit CVar =====

	TAutoConsoleVariable<int32> CVarMovementAudit(
		TEXT("Portfolio.Debug.MovementAudit"),
		0,
		TEXT("Print movement gate, gait, and RuntimeLOD diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	// ===== World Debug Layout =====

	constexpr float MovementDebugArrowBaseHeight = 10.f;
	constexpr float MovementDebugArrowVerticalSpacing = 30.f;
	constexpr float MovementDebugVelocityReferenceSpeed = 600.f;
	constexpr float MovementDebugArrowLength = 180.f;
	constexpr float MovementDebugArrowHeadSize = 24.f;

	// ===== Display Formatting =====

	FString FormatMovementDebugVector2D(const FVector& InVector)
	{
		return FString::Printf(TEXT("(%.1f, %.1f)"), InVector.X, InVector.Y);
	}

	FString FormatMovementDebugLocalVector2D(const FVector& InVector)
	{
		return FString::Printf(TEXT("(Forward: %.1f, Right: %.1f)"), InVector.X, InVector.Y);
	}

	// ===== World Debug Drawing =====

	FVector GetMovementDebugBaseLocation(const APawn* InPawn)
	{
		if (!IsValid(InPawn)) return FVector::ZeroVector;

		const ACharacter* character = Cast<ACharacter>(InPawn);
		const UCapsuleComponent* capsule = IsValid(character) ? character->GetCapsuleComponent() : nullptr;
		return IsValid(capsule)
			? InPawn->GetActorLocation() - FVector(0.f, 0.f, capsule->GetScaledCapsuleHalfHeight() - MovementDebugArrowBaseHeight)
			: InPawn->GetActorLocation() + FVector(0.f, 0.f, MovementDebugArrowBaseHeight);
	}

	void DrawMovementDebugArrow(UWorld* InWorld, const FVector& InStart, const FVector& InDirection, float InMagnitude, float InReferenceMagnitude, const FColor& InColor)
	{
		if (!IsValid(InWorld) || InMagnitude <= KINDA_SMALL_NUMBER) return;

		const FVector direction = InDirection.GetSafeNormal2D();
		if (direction.IsNearlyZero()) return;

		const float lengthScale = FMath::Clamp(InMagnitude / FMath::Max(InReferenceMagnitude, KINDA_SMALL_NUMBER), 0.2f, 1.f);
		DrawDebugDirectionalArrow(InWorld, InStart, InStart + (direction * MovementDebugArrowLength * lengthScale), MovementDebugArrowHeadSize, InColor, false, 0.f, 0, 2.f);
	}

	// ===== Audit Formatting =====

	FString FormatMovementGateState(EExecutionState InExecutionState, bool bInMovementEnabled, bool bInIntentBlocked)
	{
		return FString::Printf(
			TEXT("ExecutionState=%s | MovementEnabled=%s | RuntimeLODIntentBlocked=%s"),
			*UEnum::GetValueAsString(InExecutionState),
			bInMovementEnabled ? TEXT("true") : TEXT("false"),
			bInIntentBlocked ? TEXT("true") : TEXT("false"));
	}
}

// ===== Display Gates =====

bool FMovementDebug::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarMovementDebugEnabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FMovementDebug::ShouldDrawVelocity()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarMovementDrawVelocity.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FMovementDebug::ShouldDrawInput()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarMovementDrawInput.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FMovementDebug::ShouldDrawFacing()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarMovementDrawFacing.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FMovementDebug::ShouldDrawDebugText()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarMovementDrawDebugText.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// ===== Audit Gate =====

bool FMovementDebug::ShouldAuditMovement()
{
#if !UE_BUILD_SHIPPING
	return CVarMovementAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// ===== Display Snapshot / Presentation =====

FMovementDebugSnapshot FMovementDebug::BuildSnapshot(const APawn* InPawn)
{
	FMovementDebugSnapshot snapshot;
	const UCMovementComponent* movementComp = IsValid(InPawn) ? InPawn->FindComponentByClass<UCMovementComponent>() : nullptr;
	if (!IsValid(InPawn) || !IsValid(movementComp)) return snapshot;

	const FVector actorForward = InPawn->GetActorForwardVector().GetSafeNormal2D();
	const FVector actorRight = InPawn->GetActorRightVector().GetSafeNormal2D();
	snapshot.bHasSnapshot = true;
	snapshot.VelocityWorld = InPawn->GetVelocity();
	snapshot.LastInputWorld = InPawn->GetLastMovementInputVector();
	snapshot.VelocityLocal = FVector(
		FVector::DotProduct(snapshot.VelocityWorld, actorForward),
		FVector::DotProduct(snapshot.VelocityWorld, actorRight),
		0.f);
	snapshot.LastInputLocal = FVector(
		FVector::DotProduct(snapshot.LastInputWorld, actorForward),
		FVector::DotProduct(snapshot.LastInputWorld, actorRight),
		0.f);
	snapshot.Speed = movementComp->GetCurrentSpeed();
	snapshot.Direction = movementComp->GetCurrentDirection();
	return snapshot;
}

FMovementDebugOverlayDetails FMovementDebug::BuildOverlayDetails(const FMovementDebugSnapshot& InSnapshot)
{
	FMovementDebugOverlayDetails details;
	if (!IsEnabled() || !InSnapshot.bHasSnapshot) return details;

	details.bHasSnapshot = true;
	details.VelocityWorldText = FormatMovementDebugVector2D(InSnapshot.VelocityWorld);
	details.VelocityLocalText = FormatMovementDebugLocalVector2D(InSnapshot.VelocityLocal);
	details.LastInputWorldText = FormatMovementDebugVector2D(InSnapshot.LastInputWorld);
	details.LastInputLocalText = FormatMovementDebugLocalVector2D(InSnapshot.LastInputLocal);
	details.SpeedText = FString::Printf(TEXT("%.1f"), InSnapshot.Speed);
	details.DirectionText = FString::Printf(TEXT("%.1f"), InSnapshot.Direction);
	return details;
}

void FMovementDebug::DrawWorldDebug(UWorld* InWorld, const APawn* InPawn, const FMovementDebugSnapshot& InSnapshot)
{
#if !UE_BUILD_SHIPPING
	if (!IsEnabled() || !IsValid(InWorld) || !IsValid(InPawn) || !InSnapshot.bHasSnapshot) return;

	const FVector baseLocation = GetMovementDebugBaseLocation(InPawn);
	if (ShouldDrawVelocity())
	{
		DrawMovementDebugArrow(InWorld, baseLocation, InSnapshot.VelocityWorld, InSnapshot.Speed, MovementDebugVelocityReferenceSpeed, FColor::Green);
	}

	if (ShouldDrawInput())
	{
		DrawMovementDebugArrow(InWorld, baseLocation + FVector(0.f, 0.f, MovementDebugArrowVerticalSpacing), InSnapshot.LastInputWorld, InSnapshot.LastInputWorld.Size2D(), 1.f, FColor::Cyan);
	}

	if (ShouldDrawFacing())
	{
		DrawMovementDebugArrow(InWorld, baseLocation + FVector(0.f, 0.f, MovementDebugArrowVerticalSpacing * 2.f), InPawn->GetActorForwardVector(), 1.f, 1.f, FColor::Yellow);
	}

	if (ShouldDrawDebugText())
	{
		const FString debugText = FString::Printf(
			TEXT("Speed: %.1f | Direction: %.1f | Local Velocity: F %.1f R %.1f | Local Input: F %.2f R %.2f"),
			InSnapshot.Speed,
			InSnapshot.Direction,
			InSnapshot.VelocityLocal.X,
			InSnapshot.VelocityLocal.Y,
			InSnapshot.LastInputLocal.X,
			InSnapshot.LastInputLocal.Y);
		DrawDebugString(InWorld, baseLocation + FVector(0.f, 0.f, MovementDebugArrowVerticalSpacing * 3.f), debugText, nullptr, FColor::White, 0.f, false, 1.f);
	}
#endif
}

// ===== Runtime LOD Audit Hooks =====

void FMovementDebug::RecordRuntimeLODMovementModeAppliedForAudit(const AActor* InOwnerActor, const UObject* InComponent, int32 InPreviousMode, int32 InNewMode, bool bInMovementEnabled, bool bInIntentBlocked)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|RuntimeLOD|ModeApplied] Owner=%s | Component=%s | PreviousMode=%d | NewMode=%d | MovementEnabled=%s | RuntimeLODIntentBlocked=%s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InPreviousMode,
		InNewMode,
		bInMovementEnabled ? TEXT("true") : TEXT("false"),
		bInIntentBlocked ? TEXT("true") : TEXT("false")));
}

void FMovementDebug::RecordRuntimeLODMovementIntentAllowedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InEvent)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|RuntimeLOD|IntentAllowed] Event=%s | Owner=%s | Component=%s"),
		InEvent ? InEvent : TEXT("Allow"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent)));
}

void FMovementDebug::RecordRuntimeLODMovementIntentBlockedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InEvent)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|RuntimeLOD|IntentBlocked] Event=%s | Owner=%s | Component=%s"),
		InEvent ? InEvent : TEXT("Block"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent)));
}

// ===== Movement Gate Audit Hooks =====

void FMovementDebug::RecordMovementInputAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FVector2D& InAxis2D, EMovementGait InGait)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|Component|InputAccepted] Owner=%s | Component=%s | Axis=(%.3f, %.3f) | Gait=%s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InAxis2D.X,
		InAxis2D.Y,
		*UEnum::GetValueAsString(InGait)));
}

void FMovementDebug::RecordMovementInputRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FVector2D& InAxis2D, const TCHAR* InReason, EExecutionState InExecutionState, bool bInMovementEnabled, bool bInIntentBlocked)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|Component|InputRejected] Reason=%s | Owner=%s | Component=%s | Axis=(%.3f, %.3f) | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InAxis2D.X,
		InAxis2D.Y,
		*FormatMovementGateState(InExecutionState, bInMovementEnabled, bInIntentBlocked)));
}

// ===== Movement Data Audit Hooks =====

void FMovementDebug::RecordMovementGaitAppliedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EMovementGait InGait, float InSpeed)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|Component|GaitApplied] Owner=%s | Component=%s | Gait=%s | Speed=%.3f"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*UEnum::GetValueAsString(InGait),
		InSpeed));
}

void FMovementDebug::RecordMovementGaitRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, EMovementGait InGait, const TCHAR* InReason)
{
	if (!ShouldAuditMovement()) return;

	FLog::Log(FString::Printf(
		TEXT("[Movement|Component|GaitRejected] Reason=%s | Owner=%s | Component=%s | Gait=%s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*UEnum::GetValueAsString(InGait)));
}
