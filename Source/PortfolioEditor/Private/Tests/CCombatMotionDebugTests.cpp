#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Core/Debug/FCombatKnockbackDebug.h"
#include "Core/Debug/FCombatMotionDebugDraw.h"
#include "Components/LineBatchComponent.h"
#include "Core/Debug/FActionFacingDebug.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FDebugOverlaySettingsRegistry.h"
#include "Character/Player/CPlayer.h"
#include "Component/CMovementComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CBalanceComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"

namespace CombatMotionDebugTest
{
	struct FCVarScope
	{
		IConsoleVariable* Variable;
		FString Previous;
		EConsoleVariableFlags Priority;
		FCVarScope(const TCHAR* Name, const TCHAR* Value)
			: Variable(IConsoleManager::Get().FindConsoleVariable(Name))
		{
			check(Variable);
			Previous = Variable->GetString();
			Priority = static_cast<EConsoleVariableFlags>(Variable->GetFlags() & ECVF_SetByMask);
			Variable->Set(Value, ECVF_SetByCode);
		}
		~FCVarScope()
		{
			Variable->Set(Previous.GetCharArray().GetData(), ECVF_SetByCode);
			Variable->ClearFlags(ECVF_SetByMask);
			Variable->SetFlags(Priority);
		}
	};

	struct FWorldScope
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		~FWorldScope()
		{
			FDebugOverlaySnapshotStore::Reset(World);
			World->DestroyWorld(false);
		}
	};

	FCombatKnockbackContext Context()
	{
		FCombatKnockbackContext result;
		result.Spec.Speed = 300.f;
		result.Spec.Duration = 0.2f;
		result.Direction = FVector::ForwardVector;
		return result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatMotionDebugGatesTest, "Portfolio.DebugOverlay.CombatMotion.Gates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatMotionDebugGatesTest::RunTest(const FString& Parameters)
{
	using namespace CombatMotionDebugTest;
	FCVarScope domain(TEXT("Portfolio.DebugOverlay.Knockback.Enabled"), TEXT("1"));
	FCVarScope audit(TEXT("Portfolio.Debug.KnockbackAudit"), TEXT("0"));
	FCVarScope capture(TEXT("Portfolio.DebugOverlay.CaptureEnabled"), TEXT("0"));
	FCVarScope hud(TEXT("Portfolio.DebugOverlay.HUDVisible"), TEXT("0"));
	FCVarScope noise(TEXT("Portfolio.DebugOverlay.HideNoiseEvents"), TEXT("0"));
	FWorldScope f;
	AActor* owner = f.World->SpawnActor<AActor>();
	AActor* source = f.World->SpawnActor<AActor>();
	FCombatKnockbackDebugHistory history;
	FCombatKnockbackDebug::Record(owner, TEXT("Damage"), TEXT("Resolved"), TEXT("None"), Context(), 0, source);
	TestTrue(TEXT("Domain snapshot is independent of HUD and event capture"), FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(owner, history));
	TestEqual(TEXT("Capture off produces no events"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(f.World, 32).Num(), 0);
	capture.Variable->Set(1, ECVF_SetByCode);
	FCombatKnockbackDebug::Record(owner, TEXT("Damage"), TEXT("Resolved"), TEXT("None"), Context(), 0, source);
	TestEqual(TEXT("HUD hidden still captures"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(f.World, 32, TEXT("Knockback")).Num(), 1);
	TestEqual(TEXT("Source is included in focused actor history"), FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(f.World, 32, TEXT("Knockback"), source).Num(), 1);
	TestEqual(TEXT("Owner is included in focused actor history"), FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(f.World, 32, TEXT("Knockback"), owner).Num(), 1);
	FDebugOverlaySnapshotStore::Reset(f.World);
	domain.Variable->Set(0, ECVF_SetByCode);
	hud.Variable->Set(1, ECVF_SetByCode);
	FCombatKnockbackDebug::Record(owner, TEXT("Damage"), TEXT("Resolved"), TEXT("None"), Context());
	TestFalse(TEXT("Disabled domain does not collect snapshot"), FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(owner, history));
	TestEqual(TEXT("Disabled domain does not emit events"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(f.World, 32).Num(), 0);
	domain.Variable->Set(1, ECVF_SetByCode);
	FCombatKnockbackDebug::Record(owner, TEXT("Damage"), TEXT("Skipped"), TEXT("Disabled"), {});
	TestEqual(TEXT("Default zero settings are silent"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(f.World, 32).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatMotionDebugHistoryTest, "Portfolio.DebugOverlay.CombatMotion.History",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatMotionDebugHistoryTest::RunTest(const FString& Parameters)
{
	using namespace CombatMotionDebugTest;
	FWorldScope f;
	AActor* actor = f.World->SpawnActor<AActor>();
	FCombatKnockbackDebugRecord knockback;
	knockback.Generation = 20;
	knockback.Result = TEXT("Started");
	knockback.bHasGeometry = true;
	knockback.Origin = FVector(12, 34, 0);
	FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(actor, knockback);
	knockback.Generation = 19;
	knockback.Result = TEXT("Stopped");
	FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(actor, knockback);
	FCombatKnockbackDebugHistory history;
	TestTrue(TEXT("Knockback record exists at world time zero"), FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(actor, history));
	TestEqual(TEXT("Stale knockback does not overwrite replacement"), history.Last.Generation, uint64(20));
	knockback.Generation = 30;
	knockback.bHasGeometry = false;
	knockback.Result = TEXT("Rejected");
	FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(actor, knockback);
	knockback.Generation = 29;
	FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(actor, knockback);
	FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(actor, history);
	TestEqual(TEXT("Stale failure cannot overwrite newer failure without geometry"), history.Last.Generation, uint64(30));
	TestEqual(TEXT("Failed attempt preserves last motion geometry"), history.Motion.Generation, uint64(20));
	FActionFacingDebugRecord facing;
	facing.Generation = 30;
	facing.Result = TEXT("Applied");
	facing.BeforeYaw = 10.f;
	facing.AfterYaw = 45.f;
	FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(actor, facing);
	facing.Generation = 29;
	facing.Result = TEXT("Cancelled");
	FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(actor, facing);
	TestTrue(TEXT("Facing record exists"), FDebugOverlaySnapshotStore::TryGetAttackFacingDiagnostic(actor, facing));
	TestEqual(TEXT("Stale facing does not overwrite replacement"), facing.Generation, uint64(30));
	TestEqual(TEXT("Historical pre-correction yaw preserved"), facing.BeforeYaw, 10.f);
	FDebugOverlaySnapshotStore::RemoveActorDebugData(f.World, actor);
	TestFalse(TEXT("Actor removal clears knockback"), FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(actor, history));
	TestFalse(TEXT("Actor removal clears facing"), FDebugOverlaySnapshotStore::TryGetAttackFacingDiagnostic(actor, facing));
	TArray<AActor*> actors;
	for (int32 i = 0; i < 70; ++i)
	{
		AActor* next = f.World->SpawnActor<AActor>();
		actors.Add(next);
		FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(next, knockback);
		FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(next, facing);
	}
	int32 knockbackCount = 0, facingCount = 0;
	for (AActor* next : actors)
	{
		knockbackCount += FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(next, history) ? 1 : 0;
		facingCount += FDebugOverlaySnapshotStore::TryGetAttackFacingDiagnostic(next, facing) ? 1 : 0;
	}
	TestEqual(TEXT("Knockback actor history is bounded"), knockbackCount, 64);
	TestEqual(TEXT("Facing actor history is bounded"), facingCount, 64);
	FDebugOverlaySnapshotStore::Reset(f.World);
	TestFalse(TEXT("Reset removes knockback history"), FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(actors.Last(), history));
	TestFalse(TEXT("Reset removes facing history"), FDebugOverlaySnapshotStore::TryGetAttackFacingDiagnostic(actors.Last(), facing));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatMotionDebugRegistryTest, "Portfolio.DebugOverlay.CombatMotion.Registry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatMotionDebugRegistryTest::RunTest(const FString& Parameters)
{
	using namespace CombatMotionDebugTest;
	FCVarScope filter(TEXT("Portfolio.DebugOverlay.EventLogFilter"), TEXT("knockback"));
	TestEqual(TEXT("Knockback filter normalizes"), FDebugOverlaySnapshotStore::GetEventLogFilter(), FString(TEXT("Knockback")));
	filter.Variable->Set(TEXT("attackfacing"), ECVF_SetByCode);
	TestEqual(TEXT("Attack facing filter normalizes"), FDebugOverlaySnapshotStore::GetEventLogFilter(), FString(TEXT("AttackFacing")));
	const auto& settings = FDebugOverlaySettingsRegistry::GetSettings();
	for (const TCHAR* name : { TEXT("Portfolio.DebugOverlay.Knockback.Enabled"), TEXT("Portfolio.DebugOverlay.AttackFacing.Enabled"), TEXT("Portfolio.DebugOverlay.Player.Knockback.Enabled"), TEXT("Portfolio.DebugOverlay.Enemy.AttackFacing.Enabled") })
	{
		TestTrue(FString::Printf(TEXT("Registry contains %s"), name), settings.ContainsByPredicate([name](const FDebugOverlaySettingDefinition& entry) { return entry.CVarName == name; }));
		TestNotNull(FString::Printf(TEXT("CVar exists %s"), name), IConsoleManager::Get().FindConsoleVariable(name));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatMotionDebugOwnershipTest, "Portfolio.DebugOverlay.CombatMotion.Ownership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatMotionDebugOwnershipTest::RunTest(const FString& Parameters)
{
	using namespace CombatMotionDebugTest;
	FCVarScope domain(TEXT("Portfolio.DebugOverlay.Knockback.Enabled"), TEXT("1"));
	FCVarScope capture(TEXT("Portfolio.DebugOverlay.CaptureEnabled"), TEXT("1"));
	FCVarScope noise(TEXT("Portfolio.DebugOverlay.HideNoiseEvents"), TEXT("0"));
	FWorldScope f;
	ACPlayer* player = f.World->SpawnActor<ACPlayer>();
	const TArray<FString> emptyLines = FCombatKnockbackDebug::BuildOverlayLines(player);
	const auto checkLayout = [this, &emptyLines](const TArray<FString>& Lines)
	{
		TestEqual(TEXT("Knockback uses ten fixed rows"), Lines.Num(), 10);
		if (Lines.Num() != emptyLines.Num()) return;
		for (int32 i = 0; i < Lines.Num(); ++i)
		{
			FString prefix, unused;
			emptyLines[i].Split(TEXT(":"), &prefix, &unused);
			TestTrue(TEXT("Row identity stays fixed"), Lines[i].StartsWith(prefix + TEXT(":")));
		}
	};
	checkLayout(emptyLines);
	checkLayout(FCombatKnockbackDebug::BuildOverlayLines(nullptr));
	auto* movement = player->FindComponentByClass<UCMovementComponent>();
	FCharacterComponentReferences refs;
	refs.OwnerCharacter = player;
	refs.CharacterMovementComponent = player->GetCharacterMovement();
	refs.HealthComponent = player->GetHealthComp();
	refs.StateComponent = player->FindComponentByClass<UCStateComponent>();
	refs.BalanceComponent = player->FindComponentByClass<UCBalanceComponent>();
	refs.HealthComponent->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
	movement->InitializeReferences(refs);
	refs.CharacterMovementComponent->Activate(true);
	refs.CharacterMovementComponent->SetMovementMode(MOVE_Walking);
	AActor* firstSource = f.World->SpawnActor<AActor>();
	AActor* secondSource = f.World->SpawnActor<AActor>();
	FCombatKnockbackContext firstContext = Context();
	firstContext.DebugSourceActor = firstSource;
	FCombatKnockbackContext secondContext = Context();
	secondContext.DebugSourceActor = secondSource;
	TestTrue(TEXT("First motion starts"), movement->StartKnockback(firstContext, 10));
	const auto activeLines = FCombatKnockbackDebug::BuildOverlayLines(player);
	checkLayout(activeLines);
	TestTrue(TEXT("Active source is current"), activeLines[1].Contains(TEXT("Time:")));
	TestTrue(TEXT("Direction always contains both axes"), activeLines[5].Contains(TEXT("X=1.00 Y=0.00")));
	TestTrue(TEXT("Replacement starts"), movement->StartKnockback(secondContext, 11));
	const int32 eventCount = FDebugOverlaySnapshotStore::GetRecentEventsCopy(f.World, 32, TEXT("Knockback")).Num();
	movement->StopKnockback(10);
	FCombatKnockbackDebugHistory history;
	TestTrue(TEXT("Runtime recorded motion"), FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(player, history));
	TestEqual(TEXT("Stale stop preserves diagnostic owner"), history.Motion.Generation, uint64(11));
	TestTrue(TEXT("Replacement associates its own source, not previous motion source"), history.Motion.OtherActor.Get() == secondSource);
	const auto replacementEvents = FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(f.World, 32, TEXT("Knockback"), secondSource);
	TestEqual(TEXT("Replacement start belongs to second source history"), replacementEvents.Num(), 1);
	FCombatKnockbackDebug::DrawWorldDebug(f.World, player);
	TestEqual(TEXT("Stale stop is silent"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(f.World, 32, TEXT("Knockback")).Num(), eventCount);
	movement->StopKnockback(11);
	const auto stoppedLines = FCombatKnockbackDebug::BuildOverlayLines(player);
	checkLayout(stoppedLines);
	TestEqual(TEXT("Stopped current source is unavailable"), stoppedLines[1], FString(TEXT("Current source: N/A")));
	TestFalse(TEXT("Stopped motion time remains visible"), stoppedLines[8].Contains(TEXT("N/A")));
	TestTrue(TEXT("Stopped record remains queryable"), FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(player, history));
	TestEqual(TEXT("Actual owner produces stopped result"), history.Last.Result, FName(TEXT("Stopped")));
	TestTrue(TEXT("Stop keeps source association"), history.Last.OtherActor.Get() == secondSource);
	FCombatKnockbackDebug::Record(player, TEXT("Damage"), TEXT("Excluded"), TEXT("NotGrounded"), secondContext);
	const auto excludedLines = FCombatKnockbackDebug::BuildOverlayLines(player);
	checkLayout(excludedLines);
	TestTrue(TEXT("Latest attempt changes independently"), excludedLines[3].Contains(TEXT("Excluded")));
	TestEqual(TEXT("Later exclusion preserves stopped motion time"), excludedLines[8], stoppedLines[8]);
	FCombatKnockbackDebug::DrawWorldDebug(f.World, player);
	const int32 stoppedCount = FDebugOverlaySnapshotStore::GetRecentEventsCopy(f.World, 32, TEXT("Knockback")).Num();
	for (int32 i = 0; i < 4; ++i)
	{
		movement->StopKnockback(11);
		static_cast<UActorComponent*>(movement)->TickComponent(1.f / 60.f, LEVELTICK_All, &movement->PrimaryComponentTick);
	}
	TestEqual(TEXT("Inactive cleanup and ticks are silent"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(f.World, 32, TEXT("Knockback")).Num(), stoppedCount);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatMotionDrawGeometryTest, "Portfolio.DebugOverlay.CombatMotion.DrawGeometry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatMotionDrawGeometryTest::RunTest(const FString& Parameters)
{
	using namespace CombatMotionDebugTest;
	FCVarScope knockback(TEXT("Portfolio.DebugOverlay.Knockback.Enabled"), TEXT("1"));
	FCVarScope facing(TEXT("Portfolio.DebugOverlay.AttackFacing.Enabled"), TEXT("1"));
	FCVarScope knockbackDraw(TEXT("Portfolio.DebugOverlay.Knockback.DrawWorld"), TEXT("1"));
	FCVarScope facingDraw(TEXT("Portfolio.DebugOverlay.AttackFacing.DrawWorld"), TEXT("1"));
	FCVarScope limits(TEXT("Portfolio.DebugOverlay.AttackFacing.DrawLimits"), TEXT("0"));
	FCVarScope knockbackDepth(TEXT("Portfolio.DebugOverlay.Knockback.DrawForeground"), TEXT("0"));
	FCVarScope facingDepth(TEXT("Portfolio.DebugOverlay.AttackFacing.DrawForeground"), TEXT("0"));
	FCVarScope duration(TEXT("Portfolio.DebugOverlay.AttackFacing.DrawDuration"), TEXT("2"));
	FWorldScope f;
	ACharacter* actor = f.World->SpawnActor<ACharacter>();
	actor->SetActorScale3D(FVector(1.5));
	const FVector center(0, 0, 200);
	const FVector foot = CombatMotionDebugDraw::FootAnchor(actor, center);
	TestTrue(TEXT("Scaled capsule bottom plus clearance"), FMath::IsNearlyEqual(foot.Z,
		center.Z - actor->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 8.f));
	if (!f.World->LineBatcher) f.World->LineBatcher = NewObject<ULineBatchComponent>(f.World);
	if (!f.World->ForegroundLineBatcher) f.World->ForegroundLineBatcher = NewObject<ULineBatchComponent>(f.World);
	f.World->LineBatcher->Flush();
	FActionFacingDebugRecord record;
	record.Generation = 1;
	record.Result = TEXT("Applied");
	record.bHasGeometry = true;
	record.Origin = center;
	record.DrawOrigin = foot;
	record.TargetLocation = center + FVector(100, 0, 0);
	FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(actor, record);
	FActionFacingDebug::DrawWorldDebug(f.World, actor);
	const int32 alignedLines = f.World->LineBatcher->BatchedLines.Num();
	TestTrue(TEXT("Aligned facing enqueues geometry"), alignedLines > 0);
	TestFalse(TEXT("Aligned heading omits previous direction and arc"), f.World->LineBatcher->BatchedLines.ContainsByPredicate([](const FBatchedLine& Line)
	{
		return Line.Color == FLinearColor(FColor::Silver) || Line.Color == FLinearColor(FColor::Cyan);
	}));
	f.World->LineBatcher->Flush();
	record.AfterYaw = 45.f;
	FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(actor, record);
	FActionFacingDebug::DrawWorldDebug(f.World, actor);
	TestTrue(TEXT("Nonzero correction adds comparison geometry"), f.World->LineBatcher->BatchedLines.Num() > alignedLines);
	f.World->LineBatcher->Flush();
	record.WorldTime = -3.0;
	FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(actor, record);
	FActionFacingDebug::DrawWorldDebug(f.World, actor);
	TestEqual(TEXT("Expired geometry is omitted"), f.World->LineBatcher->BatchedLines.Num(), 0);
	duration.Variable->Set(TEXT("4"), ECVF_SetByCode);
	facingDepth.Variable->Set(TEXT("1"), ECVF_SetByCode);
	FActionFacingDebug::DrawWorldDebug(f.World, actor);
	TestTrue(TEXT("Foreground and configurable retention"), f.World->ForegroundLineBatcher->BatchedLines.Num() > 0);
	FCombatKnockbackDebugRecord motion;
	motion.Generation = 2;
	motion.bHasGeometry = true;
	motion.Context = Context();
	motion.Origin = center;
	motion.End = center + FVector(30, 0, 0);
	motion.DrawOrigin = foot;
	motion.DrawEnd = foot + FVector(30, 0, 0);
	FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(actor, motion);
	actor->SetActorLocation(FVector(1000, 1000, 1000));
	FCombatKnockbackDebug::DrawWorldDebug(f.World, actor);
	TestTrue(TEXT("Knockback enqueues foot-level geometry"), f.World->LineBatcher->BatchedLines.Num() > 0);
	for (const FBatchedLine& line : f.World->LineBatcher->BatchedLines)
	{
		TestTrue(TEXT("Stopped geometry stays at captured feet"), FMath::Abs(line.Start.Z - foot.Z) <= 2.01);
		TestTrue(TEXT("Stopped endpoint ignores later actor movement"), line.End.X < 100.0);
	}
	return true;
}
#endif
