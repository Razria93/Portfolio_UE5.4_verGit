#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Component/CBalanceComponent.h"
#include "Component/CExecutionCollaborationComponent.h"
#include "Component/CHealthComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CReactionOrchestrationTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBalanceLifecycleBoundaryTest,
	"Portfolio.Balance.LifecycleBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBalanceLifecycleBoundaryTest::RunTest(const FString& Parameters)
{
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("World"), world)) return false;
	ACharacter* owner = world->SpawnActor<ACharacter>();
	ACharacter* source = world->SpawnActor<ACharacter>();
	if (!TestNotNull(TEXT("Owner"), owner) || !TestNotNull(TEXT("Source"), source))
	{
		world->DestroyWorld(false);
		return false;
	}
	UCBalanceComponent* balance = NewObject<UCBalanceComponent>(owner);
	FTimerManager& timers = world->GetTimerManager();
	uint64 packetSerial = 0;
	auto enterLoop = [&]()
	{
		balance->ShutdownBalanceRuntime();
		FCombatResultPacket packet;
		packet.TargetActor = source;
		packet.DefenseOutcome = EDamageDefenseOutcome::Parry;
		for (int32 index = 0; index < balance->GetBalanceThreshold(); ++index)
		{
			packet.CombatSignalResultSerial = ++packetSerial;
			balance->AdvanceBalanceFromParry(packet);
		}
		FReactionExecutionContext context;
		context.ReactionDataKey.ReactionType = EReactionType::CollapseIn;
		context.BalanceLifecycleSerial = balance->GetBalanceLifecycleSerial();
		TestTrue(TEXT("Collapse started"), balance->HandleBalanceLifecycleReactionExecutionStarted(context));
		TestTrue(TEXT("Collapse pose enabled"), balance->TrySetIncapacitatedPresentation(context, EIncapacitatedPresentation::Collapse));
		FReactionExecutionLifecycleEvent event;
		event.Context = context;
		event.EventType = EReactionExecutionLifecycleEventType::Completed;
		balance->HandleBalanceLifecycleReactionExecutionTerminal(event);
		return event;
	};
	FExecutionSessionId session;
	session.SourceActor = source;
	session.Serial = 1;
	auto reserve = [&]()
	{
		FExecutionOpportunityReservation reservation;
		TestTrue(TEXT("Reservation acquired"), balance->TryReserveExecutionOpportunity(session, reservation));
		return reservation;
	};
	auto enterDown = [&]()
	{
		enterLoop();
		const FExecutionOpportunityReservation reservation = reserve();
		TestTrue(TEXT("Reservation activated"), balance->ActivateExecutionOpportunityReservation(reservation));
		TestTrue(TEXT("Reservation committed"), balance->CommitExecutionOpportunityReservation(reservation));
		FReactionExecutionContext context;
		context.ReactionDataKey.ReactionType = EReactionType::ExecutionStandard;
		context.BalanceLifecycleSerial = balance->GetBalanceLifecycleSerial();
		TestTrue(TEXT("Execution down pose enabled"), balance->TrySetIncapacitatedPresentation(context, EIncapacitatedPresentation::ExecutionDown));
		TestTrue(TEXT("Down entered"), balance->EnterExecutionDownLifecycle(context.BalanceLifecycleSerial));
	};

	// A completed old stage must not abort the current stage of the same lifecycle.
	const FReactionExecutionLifecycleEvent completed = enterLoop();
	const FTimerHandle loopTimer = balance->CollapseLoopTimerHandle;
	for (EReactionExecutionLifecycleEventType type : {
		EReactionExecutionLifecycleEventType::Completed,
		EReactionExecutionLifecycleEventType::Interrupted,
		EReactionExecutionLifecycleEventType::Ignored,
		EReactionExecutionLifecycleEventType::Started })
	{
		FReactionExecutionLifecycleEvent stale = completed;
		stale.EventType = type;
		balance->HandleBalanceLifecycleReactionExecutionTerminal(stale);
		TestTrue(TEXT("Old stage event ignored"), balance->IsCollapseLoopActive());
		TestTrue(TEXT("Old stage event preserves timer"), timers.TimerExists(loopTimer));
	}

	const EIncapacitatedPresentation previousPose = balance->GetIncapacitatedPresentation();
	FExecutionOpportunityReservation reservation = reserve();
	TestFalse(TEXT("Reservation actually cancels loop timer"), timers.TimerExists(loopTimer));
	TestTrue(TEXT("Reserved opportunity released"), balance->ReleaseExecutionOpportunityReservation(reservation));
	TestTrue(TEXT("Reservation-only release preserves presentation"), balance->GetIncapacitatedPresentation() == previousPose);
	TestEqual(TEXT("Release resumes saved time"), balance->GetCollapseLoopRemainingSeconds(), reservation.SuspendedLoopRemainingSeconds);
	reservation = reserve();
	balance->ActivateExecutionOpportunityReservation(reservation);
	balance->SetIncapacitatedPresentation(EIncapacitatedPresentation::None);
	TestTrue(TEXT("Active execution released"), balance->ReleaseExecutionOpportunityReservation(reservation));
	TestTrue(TEXT("Active release restores collapse presentation"), balance->IsCollapsePresentationActive());
	TestTrue(TEXT("Active release resumes loop"), balance->IsCollapseLoopActive());

	// Assert actual TimerManager removal, not merely the callback's state guard.
	const FTimerHandle resumedTimer = balance->CollapseLoopTimerHandle;
	balance->AbortBalanceLifecycle(EBalanceAbortReason::ExecutionCancelled);
	TestFalse(TEXT("Reset removes loop timer"), timers.TimerExists(resumedTimer));
	enterDown();
	const FTimerHandle downTimer = balance->ExecutionDownTimerHandle;
	TestTrue(TEXT("Down timer exists"), timers.TimerExists(downTimer));
	balance->ShutdownBalanceRuntime();
	TestFalse(TEXT("Shutdown removes down timer"), timers.TimerExists(downTimer));

	// Reject all recovery attempts: zero delay must be queued, bounded and cancellable.
	balance->ExecutionRecoveryRetryDelay = 0.f;
	int32 attempts = 0;
	balance->OnBalanceLifecycleReactionRequested.AddLambda([&](const FBalanceLifecyclePacket& packet)
	{
		if (packet.ReactionType != EReactionType::ExecutionRecovery) return;
		++attempts;
		FReactionRequestResult rejected;
		rejected.ResultType = EReactionRequestResultType::Rejected;
		rejected.RejectReason = EReactionRequestRejectReason::ReactionDataNotFound;
		balance->HandleBalanceLifecycleReactionRequestResolved(packet, rejected);
	});
	enterDown();
	balance->RequestExecutionRecovery();
	TestEqual(TEXT("No synchronous recursive retry"), attempts, 1);
	TestTrue(TEXT("Zero delay schedules a timer"), timers.TimerExists(balance->ExecutionRecoveryRetryTimerHandle));
	// TimerManager ticks once per engine frame. This synchronous test advances its
	// frame counter explicitly; it does not tick an editor/game world.
	for (int32 frame = 0; frame < 5 && balance->IsBalanceLifecycleBlocking(); ++frame)
	{
		++GFrameCounter;
		timers.Tick(0.01f);
	}
	TestEqual(TEXT("Initial request plus bounded retries"), attempts, 1 + balance->MaxExecutionRecoveryRetryCount);
	TestFalse(TEXT("Retry exhaustion releases lifecycle"), balance->IsBalanceLifecycleBlocking());
	TestTrue(TEXT("Retry exhaustion records failure"), balance->GetLastAbortReason() == EBalanceAbortReason::ExecutionRecoveryRejected);

	for (bool bShutdown : { false, true })
	{
		enterDown();
		balance->RequestExecutionRecovery();
		const FTimerHandle retryTimer = balance->ExecutionRecoveryRetryTimerHandle;
		TestTrue(TEXT("Retry queued before cleanup"), timers.TimerExists(retryTimer));
		if (bShutdown) balance->ShutdownBalanceRuntime();
		else balance->AbortBalanceLifecycle(EBalanceAbortReason::ExecutionCancelled);
		TestFalse(TEXT("Cleanup removes next-tick retry"), timers.TimerExists(retryTimer));
		const int32 beforeTick = attempts;
		++GFrameCounter;
		timers.Tick(0.01f);
		TestEqual(TEXT("Cleanup prevents retry callback"), attempts, beforeTick);
	}
	balance->OnBalanceLifecycleReactionRequested.Clear();

	// Exercise the actual session cancellation path without requiring montages.
	UCExecutionCollaborationComponent* collaboration = NewObject<UCExecutionCollaborationComponent>(owner);
	UCHealthComponent* health = NewObject<UCHealthComponent>(owner);
	health->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
	FCharacterComponentReferences references;
	references.OwnerCharacter = owner;
	references.BalanceComponent = balance;
	references.HealthComponent = health;
	collaboration->InitializeReferences(references);
	for (EExecutionCollaborationCancelReason reason : {
		EExecutionCollaborationCancelReason::ParticipantDeath,
		EExecutionCollaborationCancelReason::ParticipantEndPlay })
	{
		enterLoop();
		reservation = reserve();
		balance->ActivateExecutionOpportunityReservation(reservation);
		balance->CommitExecutionOpportunityReservation(reservation);
		collaboration->ActiveContext.SessionId = session;
		collaboration->ActiveContext.TargetSnapshot.TargetActor = owner;
		collaboration->ActiveContext.TargetSnapshot.Revision = 1;
		collaboration->ActiveContext.OpportunityReservation = reservation;
		collaboration->ActiveContext.OutcomePolicy = EExecutionOutcomePolicy::Standard;
		collaboration->CollaborationState = EExecutionCollaborationState::Committed;
		collaboration->bIsSourceRole = false;
		health->TakeDamage(10.f);
		const float committedHP = health->GetCurrentHP();
		collaboration->ReceivePartnerCancellation(session, reason);
		TestFalse(TEXT("Partner cancellation ends session"), collaboration->HasActiveExecutionSession());
		TestFalse(TEXT("Committed cancellation releases Balance"), balance->IsBalanceLifecycleBlocking());
		TestTrue(TEXT("Committed cancellation clears presentation"), balance->GetIncapacitatedPresentation() == EIncapacitatedPresentation::None);
		TestTrue(TEXT("Cancellation reason recorded"), balance->GetLastAbortReason() == EBalanceAbortReason::ExecutionCancelled);
		TestEqual(TEXT("Committed damage is not rolled back"), health->GetCurrentHP(), committedHP);
	}
	collaboration->InitializeReferences(FCharacterComponentReferences());
	balance->ShutdownBalanceRuntime();
	world->DestroyWorld(false);
	return true;
}
#endif
