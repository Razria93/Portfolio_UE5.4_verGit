#include "Core/Debug/FCombatCounterRushDebug.h"
#include "Core/Debug/FCombatMotionDebugDraw.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"
#include "DrawDebugHelpers.h"

#if !UE_BUILD_SHIPPING
namespace CounterRushDiagnostics
{
	TAutoConsoleVariable<int32> Enabled(TEXT("Portfolio.DebugOverlay.CounterRush.Enabled"), 0, TEXT("Capture and draw fixed rush plan and actual movement. Enable before starting a rush."));
	TAutoConsoleVariable<float> Retain(TEXT("Portfolio.DebugOverlay.CounterRush.DrawDuration"), 2.f, TEXT("Retain completed rush geometry (0-10 seconds)."));
	TAutoConsoleVariable<int32> Foreground(TEXT("Portfolio.DebugOverlay.CounterRush.DrawForeground"), 0, TEXT("Draw rush diagnostics through geometry."));
	TAutoConsoleVariable<int32> Audit(TEXT("Portfolio.Debug.CounterRushAudit"), 0, TEXT("Log rush start and movement completion."));
}
#endif

void FCombatCounterRushDebug::Begin(const AActor* InActor, const FCounterRushPlan& InPlan, uint64 InSerial, float InDuration)
{
	*this = FCombatCounterRushDebug();
#if !UE_BUILD_SHIPPING
	if (!IsValid(InActor) || !InPlan.bValid) return;
	Plan = InPlan;
	Serial = InSerial;
	Duration = InDuration;
	bCapturing = CounterRushDiagnostics::Enabled.GetValueOnGameThread() != 0 || CounterRushDiagnostics::Audit.GetValueOnGameThread() != 0;
	if (!bCapturing) return;
	Samples.Add(InPlan.Origin);
	if (CounterRushDiagnostics::Audit.GetValueOnGameThread())
		UE_LOG(LogTemp, Log, TEXT("[CounterRush|Start] Owner=%s Serial=%llu Spacing=%.1f Radii=%.1f+%.1f Planned=%.1f Duration=%.3f Target=%s"),
			*GetNameSafe(InActor), Serial, Plan.CenterSpacing, Plan.PlayerRadius, Plan.TargetRadius, Plan.TravelDistance, Duration, *Plan.Target.ToCompactString());
	Draw(InActor, TEXT("Running"), 0.f);
#endif
}

void FCombatCounterRushDebug::Sample(const AActor* InActor)
{
#if !UE_BUILD_SHIPPING
	if (!bCapturing || !IsValid(InActor)) return;
	const FVector location = InActor->GetActorLocation();
	if (location.ContainsNaN()) return;
	if (Samples.IsEmpty()) Samples.Add(location);
	else if (!Samples.Last().Equals(location, .1f))
	{
		PathDistance += FVector::Dist2D(Samples.Last(), location);
		if (Samples.Num() < 128) Samples.Add(location);
		else Samples.Last() = location;
	}
	Draw(InActor, TEXT("Running"), 0.f);
#endif
}

void FCombatCounterRushDebug::End(const AActor* InActor, const TCHAR* InReason)
{
#if !UE_BUILD_SHIPPING
	if (!bCapturing || !IsValid(InActor)) { bCapturing = false; return; }
	Sample(InActor);
	if (CounterRushDiagnostics::Audit.GetValueOnGameThread())
		UE_LOG(LogTemp, Log, TEXT("[CounterRush|End] Owner=%s Serial=%llu Reason=%s Planned=%.1f ActualXY=%.1f PathXY=%.1f ErrorXY=%.1f"),
			*GetNameSafe(InActor), Serial, InReason, Plan.TravelDistance, FVector::Dist2D(Plan.Origin, InActor->GetActorLocation()),
			PathDistance, FVector::Dist2D(Plan.LimitedEnd, InActor->GetActorLocation()));
	Draw(InActor, InReason, FMath::Clamp(CounterRushDiagnostics::Retain.GetValueOnGameThread(), 0.f, 10.f));
#endif
	bCapturing = false;
}

void FCombatCounterRushDebug::Draw(const AActor* InActor, const TCHAR* InReason, float InLifetime) const
{
#if !UE_BUILD_SHIPPING
	if (!CounterRushDiagnostics::Enabled.GetValueOnGameThread() || !IsValid(InActor) || !InActor->GetWorld()) return;
	UWorld* world = InActor->GetWorld();
	const uint8 depth = CounterRushDiagnostics::Foreground.GetValueOnGameThread() ? SDPG_Foreground : SDPG_World;
	const FVector footOffset = CombatMotionDebugDraw::FootAnchor(InActor, Plan.Origin) - Plan.Origin;
	const FVector origin = Plan.Origin + footOffset;
	const FVector desired = Plan.DesiredEnd + footOffset;
	const FVector limited = Plan.LimitedEnd + footOffset;
	const FVector target = FVector(Plan.Target.X, Plan.Target.Y, Plan.Origin.Z) + footOffset;
	DrawDebugLine(world, origin, desired, FColor::Silver, false, InLifetime, depth, 1.f);
	DrawDebugDirectionalArrow(world, origin, limited, 15.f, FColor::Cyan, false, InLifetime, depth, 3.f);
	DrawDebugCircle(world, target, Plan.CenterSpacing, 32, FColor::Yellow, false, InLifetime, depth, 1.f, FVector::ForwardVector, FVector::RightVector, false);
	DrawDebugCircle(world, target, Plan.TargetRadius, 24, FColor::Orange, false, InLifetime, depth, 2.f, FVector::ForwardVector, FVector::RightVector, false);
	DrawDebugCircle(world, limited, Plan.PlayerRadius, 24, FColor::Cyan, false, InLifetime, depth, 2.f, FVector::ForwardVector, FVector::RightVector, false);
	for (int32 i = 1; i < Samples.Num(); ++i)
		DrawDebugLine(world, Samples[i - 1] + footOffset + FVector(0,0,3), Samples[i] + footOffset + FVector(0,0,3), FColor::Magenta, false, InLifetime, depth, 3.f);
	DrawDebugPoint(world, InActor->GetActorLocation() + footOffset, 12.f, FColor::Magenta, false, InLifetime, depth);
	const FString label = FString::Printf(TEXT("Rush %llu %s\nPlan %.1f / Actual XY %.1f / Path %.1f\nSpacing %.1f / End error %.1f"),
		Serial, InReason, Plan.TravelDistance, FVector::Dist2D(Plan.Origin, InActor->GetActorLocation()), PathDistance,
		Plan.CenterSpacing, FVector::Dist2D(Plan.LimitedEnd, InActor->GetActorLocation()));
	DrawDebugString(world, limited + FVector(0,0,30), label, nullptr, FColor::White, FMath::Max(.05f, InLifetime), true);
#endif
}
