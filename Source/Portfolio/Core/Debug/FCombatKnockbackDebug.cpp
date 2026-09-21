#include "Core/Debug/FCombatKnockbackDebug.h"
#include "Core/Debug/FCombatMotionDebugDraw.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Component/CMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/RootMotionSource.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING
namespace
{
	TAutoConsoleVariable<int32> Enabled(TEXT("Portfolio.DebugOverlay.Knockback.Enabled"), 0, TEXT("Observe knockback diagnostics."));
	TAutoConsoleVariable<int32> DrawWorld(TEXT("Portfolio.DebugOverlay.Knockback.DrawWorld"), 1, TEXT("Draw knockback intent and displacement."));
	TAutoConsoleVariable<float> DrawDuration(TEXT("Portfolio.DebugOverlay.Knockback.DrawDuration"), 2.f, TEXT("Seconds to retain ended knockback geometry (0-10)."));
	TAutoConsoleVariable<int32> DrawForeground(TEXT("Portfolio.DebugOverlay.Knockback.DrawForeground"), 0, TEXT("Draw over world geometry for visibility."));
	TAutoConsoleVariable<int32> Audit(TEXT("Portfolio.Debug.KnockbackAudit"), 0, TEXT("Write knockback transitions to Output Log."));

	void Publish(const AActor* Actor, const FCombatKnockbackDebugRecord& Record)
	{
		if (Actor->IsActorBeingDestroyed()) return;
		FCombatKnockbackDebugHistory previous;
		if (Record.Generation != 0 && FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(Actor, previous) && Record.Generation < previous.LatestGeneration) return;
		if (FCombatKnockbackDebug::IsEnabled()) FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(Actor, Record);
		if (!FDebugOverlaySnapshotStore::IsCollecting() && Audit.GetValueOnGameThread() == 0) return;
		const FString summary = FString::Printf(TEXT("Owner=%s | Stage=%s | Reason=%s | ReactionGeneration=%llu | Speed=%.1f | Duration=%.3f"),
			*GetNameSafe(Actor), *Record.Stage.ToString(), *Record.Reason.ToString(), Record.Generation, Record.Context.Spec.Speed, Record.Context.Spec.Duration);
		if (FCombatKnockbackDebug::IsEnabled())
			FDebugOverlaySnapshotStore::AddEvent(Actor, TEXT("Knockback"), Record.Result.ToString(), GetNameSafe(Actor), GetNameSafe(Record.OtherActor.Get()), GetNameSafe(Actor), summary, Actor, Record.OtherActor.Get(), Actor);
		if (Audit.GetValueOnGameThread() != 0) UE_LOG(LogTemp, Log, TEXT("[Knockback|%s] %s"), *Record.Result.ToString(), *summary);
	}
}
#endif

// Presentation
bool FCombatKnockbackDebug::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return Enabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

TArray<FString> FCombatKnockbackDebug::BuildOverlayLines(const AActor* InActor)
{
	TArray<FString> lines;
#if !UE_BUILD_SHIPPING
	if (!IsEnabled()) return lines;
	const auto* movement = IsValid(InActor) ? InActor->FindComponentByClass<UCMovementComponent>() : nullptr;
	const bool active = movement && movement->IsKnockbackActive();
	lines.Add(active
		? FString::Printf(TEXT("Current: Active | Owner Gen: %llu"), movement->KnockbackOwnerSerial)
		: FString::Printf(TEXT("Current: %s | Owner Gen: N/A"), !IsValid(InActor) ? TEXT("NoActor") : movement ? TEXT("Inactive") : TEXT("Unavailable")));
	FString sourceLine = TEXT("Current source: N/A");
	FString velocityLine = TEXT("Current velocity: N/A");
	if (active)
	{
		sourceLine = FString::Printf(TEXT("Current source: %d | Time: %.3f / %.3f s"), movement->KnockbackSource->LocalID, movement->KnockbackSource->GetTime(), movement->KnockbackSource->Duration);
		if (movement->KnockbackSource->GetScriptStruct() == FRootMotionSource_ConstantForce::StaticStruct())
		{
			const FVector force = static_cast<const FRootMotionSource_ConstantForce*>(movement->KnockbackSource.Get())->Force;
			velocityLine = FString::Printf(TEXT("Current velocity: X=%.2f Y=%.2f cm/s"), force.X, force.Y);
		}
	}
	lines.Add(sourceLine);
	lines.Add(velocityLine);
	FCombatKnockbackDebugHistory history;
	const bool hasHistory = FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(InActor, history);
	lines.Add(hasHistory ? FString::Printf(TEXT("Last attempt: %s / %s | %s | Gen: %llu"), *history.Last.Stage.ToString(), *history.Last.Result.ToString(), *history.Last.Reason.ToString(), history.Last.Generation) : TEXT("Last attempt: NotCaptured"));
	lines.Add(hasHistory ? FString::Printf(TEXT("Last spec: %.1f cm/s / %.3f s"), history.Last.Context.Spec.Speed, history.Last.Context.Spec.Duration) : TEXT("Last spec: N/A"));
	lines.Add(hasHistory ? FString::Printf(TEXT("Last direction: X=%.2f Y=%.2f"), history.Last.Context.Direction.X, history.Last.Context.Direction.Y) : TEXT("Last direction: N/A"));
	lines.Add(hasHistory ? FString::Printf(TEXT("Record age (world): %.2f s"), InActor->GetWorld()->GetTimeSeconds() - history.Last.WorldTime) : TEXT("Record age (world): N/A"));
	if (history.Motion.bHasGeometry)
	{
		const auto& motion = history.Motion;
		const bool motionActive = active && movement->KnockbackOwnerSerial == motion.Generation;
		const FVector end = motionActive ? InActor->GetActorLocation() : motion.End;
		lines.Add(FString::Printf(TEXT("Motion: Gen %llu | Source %d | %s"), motion.Generation, motion.SourceId, motionActive ? TEXT("Running") : motion.Result == TEXT("Stopped") ? TEXT("Stopped") : TEXT("NotActive")));
		lines.Add(motion.Result == TEXT("Stopped") ? FString::Printf(TEXT("Motion stopped time: %.3f / %.3f s"), motion.SourceTime, motion.Context.Spec.Duration) : TEXT("Motion stopped time: N/A"));
		lines.Add(FString::Printf(TEXT("Motion distance: Nominal %.1f cm / Actual XY %.1f cm"),
			motion.Context.Spec.Speed * motion.Context.Spec.Duration, FVector::Dist2D(motion.Origin, end)));
	}
	else
	{
		lines.Add(TEXT("Motion: NotCaptured"));
		lines.Add(TEXT("Motion stopped time: N/A"));
		lines.Add(TEXT("Motion distance: N/A"));
	}
#endif
	return lines;
}

void FCombatKnockbackDebug::DrawWorldDebug(UWorld* InWorld, const AActor* InActor)
{
#if !UE_BUILD_SHIPPING
	if (!IsEnabled() || DrawWorld.GetValueOnGameThread() == 0 || !IsValid(InWorld) || !IsValid(InActor)) return;
	FCombatKnockbackDebugHistory history;
	if (!FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(InActor, history) || !history.Motion.bHasGeometry) return;
	const auto& record = history.Motion;
	const auto* movement = InActor->FindComponentByClass<UCMovementComponent>();
	const bool active = movement && movement->IsKnockbackActive() && movement->KnockbackOwnerSerial == record.Generation;
	if (!active && InWorld->GetTimeSeconds() - record.WorldTime > FMath::Clamp(DrawDuration.GetValueOnGameThread(), 0.f, 10.f)) return;
	const uint8 depth = DrawForeground.GetValueOnGameThread() != 0 ? SDPG_Foreground : SDPG_World;
	const FVector origin = record.DrawOrigin;
	const FVector intended = origin + record.Context.Direction * record.Context.Spec.Speed * record.Context.Spec.Duration;
	const FVector actual = active ? CombatMotionDebugDraw::FootAnchor(InActor, InActor->GetActorLocation()) : record.DrawEnd;
	if (origin.ContainsNaN() || intended.ContainsNaN() || actual.ContainsNaN()) return;
	CombatMotionDebugDraw::DashedLine(InWorld, origin + FVector(0, 0, 2), intended + FVector(0, 0, 2), depth);
	CombatMotionDebugDraw::Ring(InWorld, intended, 10.f, FColor::Yellow, depth);
	DrawDebugLine(InWorld, origin, actual, FColor::Cyan, false, -1.f, depth, 2.f);
	CombatMotionDebugDraw::Ring(InWorld, actual, 6.f, FColor::Cyan, depth);
	CombatMotionDebugDraw::Ring(InWorld, origin, 4.f, FColor::White, depth);
#endif
}

// Diagnostic Hooks
void FCombatKnockbackDebug::Record(const AActor* InActor, const TCHAR* InStage, const TCHAR* InResult, const TCHAR* InReason,
	const FCombatKnockbackContext& InContext, uint64 InGeneration, const AActor* InOtherActor)
{
#if !UE_BUILD_SHIPPING
	if ((!IsEnabled() && Audit.GetValueOnGameThread() == 0) || !IsValid(InActor)) return;
	if (InContext.Spec.Speed == 0.f && InContext.Spec.Duration == 0.f) return;
	FCombatKnockbackDebugRecord record;
	record.Stage = InStage; record.Result = InResult; record.Reason = InReason;
	record.Context = InContext; record.Generation = InGeneration;
	record.OtherActor = InOtherActor ? const_cast<AActor*>(InOtherActor) : InContext.DebugSourceActor.Get();
	record.WorldTime = InActor->GetWorld()->GetTimeSeconds();
	Publish(InActor, record);
#endif
}

void FCombatKnockbackDebug::RecordStarted(const UCMovementComponent* InMovement, const FCombatKnockbackContext& InContext)
{
#if !UE_BUILD_SHIPPING
	if ((!IsEnabled() && Audit.GetValueOnGameThread() == 0) || !InMovement || !InMovement->KnockbackSource.IsValid()) return;
	const auto* actor = InMovement->GetOwner();
	if (!IsValid(actor)) return;
	FCombatKnockbackDebugRecord record;
	record.Stage = TEXT("Movement"); record.Result = TEXT("Started"); record.Reason = TEXT("None");
	record.Context = InContext; record.Generation = InMovement->KnockbackOwnerSerial;
	record.OtherActor = InContext.DebugSourceActor;
	record.Origin = record.End = actor->GetActorLocation(); record.bHasGeometry = true;
	record.DrawOrigin = record.DrawEnd = CombatMotionDebugDraw::FootAnchor(actor, record.Origin);
	record.SourceId = InMovement->KnockbackSource->LocalID;
	record.WorldTime = actor->GetWorld()->GetTimeSeconds();
	Publish(actor, record);
#endif
}

void FCombatKnockbackDebug::RecordStopped(const UCMovementComponent* InMovement, const TCHAR* InReason)
{
#if !UE_BUILD_SHIPPING
	if ((!IsEnabled() && Audit.GetValueOnGameThread() == 0) || !InMovement || !InMovement->KnockbackSource.IsValid()) return;
	const auto* actor = InMovement->GetOwner();
	if (!IsValid(actor)) return;
	FCombatKnockbackDebugHistory history;
	FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(actor, history);
	auto record = history.Motion;
	if (record.Generation != InMovement->KnockbackOwnerSerial) record = {};
	record.Stage = TEXT("Movement"); record.Result = TEXT("Stopped"); record.Reason = InReason;
	record.Generation = InMovement->KnockbackOwnerSerial;
	record.SourceId = InMovement->KnockbackSource->LocalID;
	record.SourceTime = InMovement->KnockbackSource->GetTime();
	record.End = actor->GetActorLocation();
	record.DrawEnd = CombatMotionDebugDraw::FootAnchor(actor, record.End);
	record.WorldTime = actor->GetWorld()->GetTimeSeconds();
	Publish(actor, record);
#endif
}
