#include "Core/Debug/FActionFacingDebug.h"
#include "Core/Debug/FCombatMotionDebugDraw.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Component/CActionComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING
namespace
{
	TAutoConsoleVariable<int32> Enabled(TEXT("Portfolio.DebugOverlay.AttackFacing.Enabled"), 0, TEXT("Observe attack start facing diagnostics."));
	TAutoConsoleVariable<int32> DrawWorld(TEXT("Portfolio.DebugOverlay.AttackFacing.DrawWorld"), 1, TEXT("Draw recorded attack facing directions."));
	TAutoConsoleVariable<int32> DrawLimits(TEXT("Portfolio.DebugOverlay.AttackFacing.DrawLimits"), 0, TEXT("Draw limits relative to the recorded pre-correction heading."));
	TAutoConsoleVariable<float> DrawDuration(TEXT("Portfolio.DebugOverlay.AttackFacing.DrawDuration"), 2.f, TEXT("Seconds to retain facing geometry (0-10)."));
	TAutoConsoleVariable<int32> DrawForeground(TEXT("Portfolio.DebugOverlay.AttackFacing.DrawForeground"), 0, TEXT("Draw over world geometry for visibility."));
	TAutoConsoleVariable<int32> Audit(TEXT("Portfolio.Debug.AttackFacingAudit"), 0, TEXT("Write attack facing transitions to Output Log."));
}
#endif

// Presentation
bool FActionFacingDebug::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return Enabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

TArray<FString> FActionFacingDebug::BuildOverlayLines(const AActor* InActor)
{
	TArray<FString> lines;
#if !UE_BUILD_SHIPPING
	if (!IsEnabled()) return lines;
	if (!IsValid(InActor)) { lines.Add(TEXT("Current: NoActor")); return lines; }
	const auto* action = InActor->FindComponentByClass<UCActionComponent>();
	if (!action) { lines.Add(TEXT("Current: Unavailable")); return lines; }
	lines.Add(FString::Printf(TEXT("Current: %s | Option: %s | Pending: %s"), action->IsActive() ? TEXT("Active") : TEXT("Inactive"), action->ActiveActionData.bFaceCombatTargetOnStart ? TEXT("On") : TEXT("Off"), action->PendingStartFacingGeneration ? TEXT("Yes") : TEXT("No")));
	lines.Add(FString::Printf(TEXT("Current action: %s[%d] | Gen: %llu"), *UEnum::GetValueAsString(action->ActiveActionType), action->ActiveActionIndex, action->ActiveActionGeneration));
	FActionFacingDebugRecord record;
	if (!FDebugOverlaySnapshotStore::TryGetAttackFacingDiagnostic(InActor, record)) { lines.Add(TEXT("Last: NotCaptured")); return lines; }
	lines.Add(FString::Printf(TEXT("Last: %s / %s | %s[%d] | Gen: %llu"), *record.Result.ToString(), *record.Reason.ToString(), *UEnum::GetValueAsString(record.ActionKey.ActionType), record.ActionKey.ActionIndex, record.Generation));
	lines.Add(FString::Printf(TEXT("Target (recorded): %s | Age (world): %.2f s"), *GetNameSafe(record.Target.Get()), InActor->GetWorld()->GetTimeSeconds() - record.WorldTime));
	if (record.bHasGeometry)
	{
		lines.Add(FString::Printf(TEXT("Distance: %.1f / %.1f cm | Angle: %.1f / %.1f deg"), record.Distance, record.MaxDistance, record.Angle, record.MaxAngle));
		lines.Add(FString::Printf(TEXT("Yaw (recorded): %.1f -> %.1f | Delta: %.1f deg"), record.BeforeYaw, record.AfterYaw,
			FMath::FindDeltaAngleDegrees(record.BeforeYaw, record.AfterYaw)));
	}
#endif
	return lines;
}

void FActionFacingDebug::DrawWorldDebug(UWorld* InWorld, const AActor* InActor)
{
#if !UE_BUILD_SHIPPING
	if (!IsEnabled() || DrawWorld.GetValueOnGameThread() == 0 || !IsValid(InWorld) || !IsValid(InActor)) return;
	FActionFacingDebugRecord record;
	if (!FDebugOverlaySnapshotStore::TryGetAttackFacingDiagnostic(InActor, record) || !record.bHasGeometry
		|| InWorld->GetTimeSeconds() - record.WorldTime > FMath::Clamp(DrawDuration.GetValueOnGameThread(), 0.f, 10.f)) return;
	const uint8 depth = DrawForeground.GetValueOnGameThread() != 0 ? SDPG_Foreground : SDPG_World;
	const FVector origin = record.DrawOrigin;
	const FVector before = FRotator(0, record.BeforeYaw, 0).Vector();
	const FVector after = FRotator(0, record.AfterYaw, 0).Vector();
	const FVector target = (record.TargetLocation - record.Origin).GetSafeNormal2D();
	const float deltaYaw = FMath::FindDeltaAngleDegrees(record.BeforeYaw, record.AfterYaw);
	if (FMath::Abs(deltaYaw) > CombatMotionDebugDraw::FacingAngleTolerance)
	{
		DrawDebugLine(InWorld, origin, origin + before * 90.f, FColor::Silver, false, -1.f, depth, 2.f);
		CombatMotionDebugDraw::FacingArc(InWorld, origin, record.BeforeYaw, deltaYaw, depth);
	}
	const FColor resultColor = record.Result == TEXT("Applied") ? FColor::Green : FColor::Orange;
	DrawDebugDirectionalArrow(InWorld, origin, origin + after * CombatMotionDebugDraw::FacingRadius, 12.f, resultColor, false, -1.f, depth, 2.f);
	if (!target.IsNearlyZero())
		CombatMotionDebugDraw::Ring(InWorld, origin + target * CombatMotionDebugDraw::FacingRadius, 5.f, FColor::Yellow, depth);
	if (DrawLimits.GetValueOnGameThread() != 0 && FMath::IsFinite(record.MaxDistance) && FMath::IsFinite(record.MaxAngle) && record.MaxDistance > 0.f && record.MaxAngle > 0.f && record.MaxAngle <= 180.f)
	{
		FVector previous = origin + FRotator(0, record.BeforeYaw - record.MaxAngle, 0).Vector() * record.MaxDistance;
		DrawDebugLine(InWorld, origin, previous, FColor::Orange, false, -1.f, depth);
		for (int32 i = 1; i <= 32; ++i)
		{
			const FVector next = origin + FRotator(0, record.BeforeYaw - record.MaxAngle + 2.f * record.MaxAngle * i / 32.f, 0).Vector() * record.MaxDistance;
			DrawDebugLine(InWorld, previous, next, FColor::Orange, false, -1.f, depth);
			previous = next;
		}
		DrawDebugLine(InWorld, origin, previous, FColor::Orange, false, -1.f, depth);
	}
#endif
}

// Diagnostic Hooks
void FActionFacingDebug::Record(const UCActionComponent* InAction, uint64 InGeneration, const AActor* InTarget,
	const TCHAR* InResult, const TCHAR* InReason, const FVector* InOrigin, const float* InBeforeYaw)
{
#if !UE_BUILD_SHIPPING
	if ((!IsEnabled() && Audit.GetValueOnGameThread() == 0) || !InAction || InGeneration == 0) return;
	if (InAction->ActiveActionGeneration != 0 && InGeneration < InAction->ActiveActionGeneration) return;
	const AActor* actor = InAction->OwnerCharacter_Injected;
	if (!IsValid(actor) || actor->IsActorBeingDestroyed()) return;
	FActionFacingDebugRecord record;
	record.Result = InResult; record.Reason = InReason; record.Generation = InGeneration;
	record.ActionKey.ActionType = InAction->ActiveActionType; record.ActionKey.ActionIndex = InAction->ActiveActionIndex;
	record.Target = const_cast<AActor*>(InTarget);
	record.WorldTime = actor->GetWorld()->GetTimeSeconds();
	record.MaxDistance = InAction->ActiveActionData.StartFacingMaxDistance;
	record.MaxAngle = InAction->ActiveActionData.StartFacingMaxAngle;
	record.Origin = InOrigin ? *InOrigin : actor->GetActorLocation();
	record.DrawOrigin = CombatMotionDebugDraw::FootAnchor(actor, record.Origin) + FVector(0, 0, 4);
	record.BeforeYaw = InBeforeYaw ? *InBeforeYaw : actor->GetActorRotation().Yaw;
	record.AfterYaw = actor->GetActorRotation().Yaw;
	if (IsValid(InTarget))
	{
		record.TargetLocation = InTarget->GetActorLocation();
		const FVector delta = record.TargetLocation - record.Origin;
		record.Distance = delta.Size2D();
		record.Angle = FMath::Abs(FMath::FindDeltaAngleDegrees(record.BeforeYaw, delta.Rotation().Yaw));
		record.bHasGeometry = !delta.ContainsNaN() && FMath::IsFinite(record.BeforeYaw) && FMath::IsFinite(record.AfterYaw);
	}
	if (IsEnabled()) FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(actor, record);
	if (!FDebugOverlaySnapshotStore::IsCollecting() && Audit.GetValueOnGameThread() == 0) return;
	const FString summary = FString::Printf(TEXT("Owner=%s | Target=%s | Action=%s[%d] | Generation=%llu | Reason=%s | Distance=%.1f/%.1f | Angle=%.1f/%.1f | Yaw=%.1f->%.1f"),
		*GetNameSafe(actor), *GetNameSafe(InTarget), *UEnum::GetValueAsString(record.ActionKey.ActionType), record.ActionKey.ActionIndex, InGeneration, InReason,
		record.Distance, record.MaxDistance, record.Angle, record.MaxAngle, record.BeforeYaw, record.AfterYaw);
	if (IsEnabled()) FDebugOverlaySnapshotStore::AddEvent(actor, TEXT("AttackFacing"), InResult, GetNameSafe(actor), GetNameSafe(actor), GetNameSafe(InTarget), summary, actor, actor, InTarget);
	if (Audit.GetValueOnGameThread() != 0) UE_LOG(LogTemp, Log, TEXT("[AttackFacing|%s] %s"), InResult, *summary);
#endif
}
