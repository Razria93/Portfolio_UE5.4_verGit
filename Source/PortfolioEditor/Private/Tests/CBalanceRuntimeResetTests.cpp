#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Component/CBalanceComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CReactionOrchestrationTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBalanceRuntimeResetTest,
	"Portfolio.Balance.RuntimeResetObservation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBalanceRuntimeResetTest::RunTest(const FString& Parameters)
{
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("World"), world)) return false;
	AActor* owner = world->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Owner"), owner))
	{
		world->DestroyWorld(false);
		return false;
	}
	UCBalanceComponent* balance = NewObject<UCBalanceComponent>(owner);
	FCombatResultPacket packet;
	packet.TargetActor = owner;
	packet.DefenseOutcome = EDamageDefenseOutcome::Parry;
	uint64 packetSerial = 0;
	auto enterCollapseLoop = [&]()
	{
		for (int32 index = 0; index < balance->GetBalanceThreshold(); ++index)
		{
			packet.CombatSignalResultSerial = ++packetSerial;
			balance->AdvanceBalanceFromParry(packet);
		}
		FReactionExecutionContext context;
		context.ReactionDataKey.ReactionType = EReactionType::CollapseIn;
		context.BalanceLifecycleSerial = balance->GetBalanceLifecycleSerial();
		TestTrue(TEXT("Collapse starts"), balance->HandleBalanceLifecycleReactionExecutionStarted(context));
		TestTrue(TEXT("Collapse presentation starts"), balance->TrySetIncapacitatedPresentation(context, EIncapacitatedPresentation::Collapse));
		FReactionExecutionLifecycleEvent event;
		event.Context = context;
		event.EventType = EReactionExecutionLifecycleEventType::Completed;
		balance->HandleBalanceLifecycleReactionExecutionTerminal(event);
		TestTrue(TEXT("Collapse timer armed"), balance->GetCollapseLoopRemainingSeconds() > 0.f);
	};

	enterCollapseLoop();
	const uint32 firstSerial = balance->GetBalanceLifecycleSerial();
	FString notifications;
	bool bObserveCleanup = true;
	auto checkCommittedState = [&]()
	{
		TestEqual(TEXT("Observer sees reset count"), balance->GetCurrentBalanceCount(), 0);
		TestTrue(TEXT("Observer sees reset lifecycle"), balance->GetBalanceLifecycleState() == EBalanceLifecycleState::Accumulating);
		TestTrue(TEXT("Observer sees reset presentation"), balance->GetIncapacitatedPresentation() == EIncapacitatedPresentation::None);
		TestFalse(TEXT("Observer sees no execution opportunity"), balance->IsExecutionOpportunityAvailable());
		TestEqual(TEXT("Observer sees no collapse time"), balance->GetCollapseLoopRemainingSeconds(), 0.f);
	};
	int32 reactionRequests = 0;
	balance->OnBalanceLifecycleReactionRequested.AddLambda([&](const FBalanceLifecyclePacket&) { ++reactionRequests; });
	balance->OnIncapacitatedPresentationChanged.AddLambda([&](EIncapacitatedPresentation)
	{
		if (!bObserveCleanup) return;
		checkCommittedState();
		notifications += TEXT("P");
	});
	balance->OnExecutionDownPresentationChanged.AddLambda([&](bool bActive)
	{
		if (!bObserveCleanup) return;
		checkCommittedState();
		TestFalse(TEXT("Legacy presentation cleared"), bActive);
		notifications += TEXT("D");
	});
	balance->OnBalanceLifecycleStateChanged.AddLambda([&](EBalanceLifecycleState previous, EBalanceLifecycleState current)
	{
		if (!bObserveCleanup) return;
		checkCommittedState();
		TestTrue(TEXT("Reset transition retains previous state"), previous == EBalanceLifecycleState::CollapseLoopActive);
		TestTrue(TEXT("Reset transition ends accumulating"), current == EBalanceLifecycleState::Accumulating);
		notifications += TEXT("S");
	});
	balance->OnBalanceValuesChanged.AddLambda([&]()
	{
		if (!bObserveCleanup) return;
		checkCommittedState();
		notifications += TEXT("V");
	});

	balance->AbortBalanceLifecycle(EBalanceAbortReason::CollapseInInterrupted);
	TestEqual(TEXT("Reset notification order"), notifications, FString(TEXT("PDSV")));
	TestEqual(TEXT("Lifecycle serial survives reset"), balance->GetBalanceLifecycleSerial(), firstSerial);
	TestTrue(TEXT("Abort diagnostics survive reset"), balance->GetLastAbortReason() == EBalanceAbortReason::CollapseInInterrupted);
	balance->AdvanceBalanceFromParry(packet);
	TestEqual(TEXT("Old packet still rejected after reset"), balance->GetCurrentBalanceCount(), 0);
	TestEqual(TEXT("Duplicate adds no notification"), notifications, FString(TEXT("PDSV")));
	world->GetTimerManager().Tick(10.f);
	TestEqual(TEXT("Reset cancels collapse request timer"), reactionRequests, 0);

	bObserveCleanup = false;
	enterCollapseLoop();
	TestTrue(TEXT("Next cycle has a new serial"), balance->GetBalanceLifecycleSerial() != firstSerial);
	const uint32 secondSerial = balance->GetBalanceLifecycleSerial();
	bObserveCleanup = true;
	notifications.Reset();
	balance->ShutdownBalanceRuntime();
	TestEqual(TEXT("Shutdown omits gameplay state event"), notifications, FString(TEXT("PDV")));
	TestEqual(TEXT("Shutdown retains lifecycle serial"), balance->GetBalanceLifecycleSerial(), secondSerial);
	balance->ShutdownBalanceRuntime();
	TestEqual(TEXT("Repeated shutdown is notification-idempotent"), notifications, FString(TEXT("PDV")));
	world->GetTimerManager().Tick(10.f);
	TestEqual(TEXT("Shutdown cancels collapse request timer"), reactionRequests, 0);
	bObserveCleanup = false;
	balance->AdvanceBalanceFromParry(packet);
	TestEqual(TEXT("Shutdown cleared packet history"), balance->GetCurrentBalanceCount(), 1);
	balance->ShutdownBalanceRuntime();
	balance->OnBalanceValuesChanged.Clear();
	balance->OnBalanceLifecycleStateChanged.Clear();
	balance->OnIncapacitatedPresentationChanged.Clear();
	balance->OnExecutionDownPresentationChanged.Clear();
	balance->OnBalanceLifecycleReactionRequested.Clear();
	world->DestroyWorld(false);
	return true;
}
#endif
