#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"
#include "Misc/OutputDeviceNull.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDebugOverlayClearEventLogTest,
	"Portfolio.DebugOverlay.ClearEventLog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDebugOverlayClearEventLogTest::RunTest(const FString& Parameters)
{
	IConsoleVariable* capture = IConsoleManager::Get().FindConsoleVariable(TEXT("Portfolio.DebugOverlay.CaptureEnabled"));
	if (!TestNotNull(TEXT("Capture CVar"), capture)) return false;
	const int32 previousCapture = capture->GetInt();
	const EConsoleVariableFlags priority = static_cast<EConsoleVariableFlags>(capture->GetFlags() & ECVF_SetByMask);
	capture->Set(1, priority);
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	UWorld* otherWorld = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* actor = world->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Actor"), actor))
	{
		world->DestroyWorld(false);
		otherWorld->DestroyWorld(false);
		capture->Set(previousCapture, priority);
		return false;
	}

	// Wrap the world ring and actor history before clearing.
	for (int32 index = 0; index < 300; ++index)
	{
		FDebugOverlaySnapshotStore::AddEvent(world, TEXT("Combat"), TEXT("BeforeClear"), TEXT("Actor"), TEXT(""), TEXT(""), TEXT("Old event"), actor);
	}
	FDebugOverlaySnapshotStore::RecordExecutionDecision(world, actor, TEXT("Action"), TEXT("Attack"), TEXT("Accept"), TEXT("Start"), TEXT("None"), TEXT("DecisionBeforeClear"));
	FDebugOverlaySnapshotStore::AddEvent(otherWorld, TEXT("Combat"), TEXT("OtherWorld"), TEXT(""), TEXT(""), TEXT(""), TEXT("Keep"));
	TestTrue(TEXT("World history populated"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(world, 32).Num() > 0);
	TestTrue(TEXT("Actor history populated"), FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(world, 32, TEXT("All"), actor).Num() > 0);
	FDebugOverlaySnapshot before;
	FDebugOverlaySnapshotStore::TryGetSnapshotCopy(world, before);

	// Exercise the actual runtime console command, not only the store API.
	FOutputDeviceNull output;
	TestTrue(TEXT("Clear command registered"), IConsoleManager::Get().ProcessUserConsoleInput(TEXT("Portfolio.DebugOverlay.ClearEventLog"), output, world));
	TestEqual(TEXT("World history cleared"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(world, 32).Num(), 0);
	TestEqual(TEXT("Actor history cleared"), FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(world, 32, TEXT("All"), actor).Num(), 0);
	FDebugOverlaySnapshot after;
	TestTrue(TEXT("Store preserved"), FDebugOverlaySnapshotStore::TryGetSnapshotCopy(world, after));
	TestEqual(TEXT("Snapshot events cleared"), after.RecentEvents.Num(), 0);
	TestEqual(TEXT("Diagnostic snapshot preserved"), after.LastActionReaction.Summary, before.LastActionReaction.Summary);
	TestEqual(TEXT("Other world preserved"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(otherWorld, 32).Num(), 1);
	TestEqual(TEXT("Capture remains enabled"), capture->GetInt(), 1);

	FDebugOverlaySnapshotStore::ClearEventLog(world);
	FDebugOverlaySnapshotStore::ClearEventLog(nullptr);
	FDebugOverlaySnapshotStore::AddEvent(world, TEXT("Combat"), TEXT("AfterClear"), TEXT("Actor"), TEXT(""), TEXT(""), TEXT("New event"), actor);
	const auto events = FDebugOverlaySnapshotStore::GetRecentEventsCopy(world, 32);
	TestEqual(TEXT("New event captured"), events.Num(), 1);
	if (events.Num() == 1) TestEqual(TEXT("Only new event remains"), events[0].EventName, FString(TEXT("AfterClear")));
	TestEqual(TEXT("Actor history resumes"), FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(world, 32, TEXT("All"), actor).Num(), 1);
	capture->Set(0, priority);
	FDebugOverlaySnapshotStore::ClearEventLog(world);
	TestEqual(TEXT("Clear works while capture is disabled"), FDebugOverlaySnapshotStore::GetRecentEventsCopy(world, 32).Num(), 0);
	FDebugOverlaySnapshotStore::Reset(world);
	FDebugOverlaySnapshotStore::Reset(otherWorld);
	world->DestroyWorld(false);
	otherWorld->DestroyWorld(false);
	capture->Set(previousCapture, priority);
	return true;
}
#endif
