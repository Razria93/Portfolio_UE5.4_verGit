#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/CPlayer.h"
#include "Action/CAction_Guard.h"
#include "Action/CAction_Dodge.h"
#include "Action/CAction_Execution.h"
#include "Reaction/CReaction_Execution.h"
#include "Component/CActionComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CActionFeedbackComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CHitFeedbackComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CExecutionCollaborationComponent.h"
#include "Component/CCombatHUDPresenterComponent.h"
#include "Component/CObservableOverlayComponent.h"

namespace HUDActionTest
{
	// Test-only runtime fixtures avoid playing montages or loading a gameplay map.
	template<typename T> T& Field(UObject* Object, const TCHAR* Name)
	{
		FProperty* property = FindFProperty<FProperty>(Object->GetClass(), Name);
		check(property);
		return *property->ContainerPtrToValuePtr<T>(Object);
	}
	template<typename T> T* Comp(AActor* Actor) { return Actor->FindComponentByClass<T>(); }
	FCharacterComponentReferences References(ACPlayer* Player)
	{
		FCharacterComponentReferences r;
		r.OwnerCharacter = Player;
		r.MovementComponent = Comp<UCMovementComponent>(Player);
		r.DefenseComponent = Comp<UCDefenseComponent>(Player);
		r.HitFeedbackComponent = Comp<UCHitFeedbackComponent>(Player);
		r.ExecutionCollaborationComponent = Comp<UCExecutionCollaborationComponent>(Player);
		r.HealthComponent = Comp<UCHealthComponent>(Player);
		r.StateComponent = Comp<UCStateComponent>(Player);
		r.WeaponComponent = Comp<UCWeaponComponent>(Player);
		r.ActionComponent = Comp<UCActionComponent>(Player);
		r.ActionOrchestratorComponent = Comp<UCActionOrchestratorComponent>(Player);
		r.ActionFeedbackComponent = Comp<UCActionFeedbackComponent>(Player);
		r.ReactionComponent = Comp<UCReactionComponent>(Player);
		r.ReactionOrchestratorComponent = Comp<UCReactionOrchestratorComponent>(Player);
		r.ReactionFeedbackComponent = Comp<UCReactionFeedbackComponent>(Player);
		r.CombatTargetComponent = Comp<UCCombatTargetComponent>(Player);
		r.CombatSignalTargetComponent = Comp<UCCombatSignalTargetComponent>(Player);
		r.BalanceComponent = Comp<UCBalanceComponent>(Player);
		r.ObservableOverlayComponent = Comp<UCObservableOverlayComponent>(Player);
		return r;
	}
	FActionExecutionContext Prepare(ACPlayer* Player, EActionType Type, int32 Index, UClass* Class)
	{
		UCActionComponent* component = Comp<UCActionComponent>(Player);
		UCAction* executor = NewObject<UCAction>(component, Class);
		executor->InitializeReferences(References(Player));
		FActionData data;
		data.ActionDataKey.ActionType = Type;
		data.ActionDataKey.ActionIndex = Index;
		data.ActionExecutorKey = Class;
		data.Montage = NewObject<UAnimMontage>();
		data.StandardExecutionDamage = 20.f;
		Field<TMap<FActionDataKey, FActionData>>(component, TEXT("ActionDataMap")).Add(data.ActionDataKey, data);
		Field<TMap<UClass*, UCAction*>>(component, TEXT("ActionExecutorMap")).Add(Class, executor);
		FActionExecutionContext context;
		component->FindPreparedActionContext(data.ActionDataKey, context);
		return context;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDActionAvailabilityTest, "Portfolio.UI.CombatHUD.ActionAvailability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHUDActionAvailabilityTest::RunTest(const FString& Parameters)
{
	using namespace HUDActionTest;
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	ACPlayer* player = world->SpawnActor<ACPlayer>();
	player->GetHealthComp()->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
	UCActionComponent* action = Comp<UCActionComponent>(player);
	UCActionOrchestratorComponent* query = Comp<UCActionOrchestratorComponent>(player);
	UCDefenseComponent* defense = Comp<UCDefenseComponent>(player);
	const FCharacterComponentReferences references = References(player);
	query->InitializeReferences(references);
	Comp<UCStateComponent>(player)->InitializeReferences(references);
	Comp<UCObservableOverlayComponent>(player)->InitializeReferences(references);
	EActionRequestRejectReason reason = EActionRequestRejectReason::None;
	TestFalse(TEXT("Missing prepared data is unavailable"), query->QueryCombatActionAvailability(ECombatActionIntent::Dodge, reason));
	TestTrue(TEXT("Missing data reason returned"), reason == EActionRequestRejectReason::ActionDataNotFound);
	TestEqual(TEXT("Query does not allocate executors"), Field<TMap<UClass*, UCAction*>>(action, TEXT("ActionExecutorMap")).Num(), 0);
	const FActionExecutionContext guard = Prepare(player, EActionType::Guard, GetGuardActionPhaseIndex(EGuardActionPhase::In), UCAction_Guard::StaticClass());
	Prepare(player, EActionType::Dodge, 0, UCAction_Dodge::StaticClass());
	TestFalse(TEXT("Unarmed guard rejected"), query->QueryCombatActionAvailability(ECombatActionIntent::Guard, reason));
	Field<EWeaponType>(Comp<UCWeaponComponent>(player), TEXT("CurrentWeaponType")) = EWeaponType::Sword;
	for (int32 i = 0; i < 50; ++i)
	{
		TestTrue(TEXT("Armed guard ready"), query->QueryCombatActionAvailability(ECombatActionIntent::Guard, reason));
		TestTrue(TEXT("Idle dodge ready"), query->QueryCombatActionAvailability(ECombatActionIntent::Dodge, reason));
	}
	TestFalse(TEXT("Queries do not set guard intent"), defense->WantsGuarding());
	TestFalse(TEXT("Queries do not open parry window"), defense->CanParry());
	TestFalse(TEXT("Queries do not start actions"), action->IsActive());
	defense->HandleGuardInputPressed();
	defense->HandleGuardInStarted();
	TestTrue(TEXT("Dodge can leave guard overlay"), query->QueryCombatActionAvailability(ECombatActionIntent::Dodge, reason));
	TestTrue(TEXT("Query does not clear guard overlay"), defense->IsGuardingPose());
	defense->ClearGuardState();

	// A real active action can allow dodge only inside its intervention window.
	FActionData activeData = guard.ActionData;
	FExecutionInterventionAllowRule rule;
	rule.Timing = EExecutionInterventionTiming::Window;
	rule.WindowKey = TEXT("HUDTestCancel");
	FExecutionInterventionParticipantFilter filter;
	filter.Domain = EExecutionDomain::Action;
	filter.ActionType = EActionType::Dodge;
	rule.ParticipantFilters.Add(filter);
	activeData.AllowInterventionRules.Add(rule);
	Field<FActionData>(guard.ActionExecutor, TEXT("ActiveData_Cached")) = activeData;
	Field<EActionType>(action, TEXT("ActiveActionType")) = EActionType::Guard;
	Field<int32>(action, TEXT("ActiveActionIndex")) = activeData.ActionDataKey.ActionIndex;
	Field<FActionData>(action, TEXT("ActiveActionData")) = activeData;
	Field<UCAction*>(action, TEXT("ActiveActionExecutor")) = guard.ActionExecutor;
	Comp<UCStateComponent>(player)->SetActionState();
	TestFalse(TEXT("Closed cancel window blocks dodge"), query->QueryCombatActionAvailability(ECombatActionIntent::Dodge, reason));
	guard.ActionExecutor->OpenAllowInterventionWindow(rule.WindowKey);
	TestTrue(TEXT("Open cancel window allows dodge"), query->QueryCombatActionAvailability(ECombatActionIntent::Dodge, reason));
	TestTrue(TEXT("Query does not interrupt active action"), action->IsActiveActionType(EActionType::Guard));
	guard.ActionExecutor->CloseAllowInterventionWindow(rule.WindowKey);
	TestFalse(TEXT("Closing window blocks dodge again"), query->QueryCombatActionAvailability(ECombatActionIntent::Dodge, reason));
	Field<EActionType>(action, TEXT("ActiveActionType")) = EActionType::Idle;
	Comp<UCStateComponent>(player)->SetIdleState();
	player->GetHealthComp()->TryKill();
	TestFalse(TEXT("Dead player cannot dodge"), query->QueryCombatActionAvailability(ECombatActionIntent::Dodge, reason));
	TestTrue(TEXT("Dead player reason returned"), reason == EActionRequestRejectReason::Dead);
	world->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDExecutionAvailabilityTest, "Portfolio.UI.CombatHUD.ExecutionAvailability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHUDExecutionAvailabilityTest::RunTest(const FString& Parameters)
{
	using namespace HUDActionTest;
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	ACPlayer* source = world->SpawnActor<ACPlayer>();
	ACPlayer* target = world->SpawnActor<ACPlayer>();
	source->GetHealthComp()->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
	target->GetHealthComp()->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
	source->SetActorLocation(FVector::ZeroVector);
	source->SetActorRotation(FRotator::ZeroRotator);
	target->SetActorLocation(FVector(200, 0, 0));
	UCBalanceComponent* balance = NewObject<UCBalanceComponent>(target);
	target->AddInstanceComponent(balance);
	UCExecutionCollaborationComponent* execution = Comp<UCExecutionCollaborationComponent>(source);
	execution->InitializeReferences(References(source));
	Comp<UCExecutionCollaborationComponent>(target)->InitializeReferences(References(target));
	Comp<UCStateComponent>(source)->InitializeReferences(References(source));
	Comp<UCStateComponent>(target)->InitializeReferences(References(target));
	Comp<UCActionOrchestratorComponent>(source)->InitializeReferences(References(source));
	Comp<UCReactionOrchestratorComponent>(target)->InitializeReferences(References(target));
	Comp<UCCombatSignalTargetComponent>(target)->InitializeReferences(References(target));
	EExecutionAvailabilityBlock block;
	TestFalse(TEXT("No selected target"), execution->QueryCombatExecutionAvailability(block));
	TestTrue(TEXT("No target block reason"), block == EExecutionAvailabilityBlock::NoTarget);
	source->GetCombatTargetComp()->RequestSetCombatTarget(target, ECombatTargetChangeReason::PlayerSelection);
	TestFalse(TEXT("No collapse opportunity"), execution->QueryCombatExecutionAvailability(block));
	TestTrue(TEXT("No opportunity block reason"), block == EExecutionAvailabilityBlock::NoOpportunity);
	Field<EBalanceLifecycleState>(balance, TEXT("BalanceLifecycleState")) = EBalanceLifecycleState::CollapseLoopActive;
	Field<uint32>(balance, TEXT("BalanceLifecycleSerial")) = 1;
	TestFalse(TEXT("Unprepared execution unavailable"), execution->QueryCombatExecutionAvailability(block));
	TestTrue(TEXT("Missing data block reason"), block == EExecutionAvailabilityBlock::MissingData);
	const FActionExecutionContext prepared = Prepare(source, EActionType::Execution, 0, UCAction_Execution::StaticClass());
	UCReactionComponent* reactions = Comp<UCReactionComponent>(target);
	UCReaction_Execution* executor = NewObject<UCReaction_Execution>(reactions);
	executor->InitializeReferences(References(target));
	FReactionData data;
	data.ReactionDataKey.MatchMode = EReactionDataMatchMode::Global;
	data.ReactionDataKey.ReactionType = EReactionType::ExecutionStandard;
	data.ReactionDataKey.ReactionIndex = INDEX_NONE;
	data.ReactionExecutorKey = UCReaction_Execution::StaticClass();
	data.Montage = NewObject<UAnimMontage>();
	Field<TMap<FReactionDataKey, FReactionData>>(reactions, TEXT("ReactionDataMap")).Add(data.ReactionDataKey, data);
	Field<TMap<UClass*, UCReaction*>>(reactions, TEXT("ReactionExecutorMap")).Add(UCReaction_Execution::StaticClass(), executor);

	UCReactionOrchestratorComponent* reactionQuery = Comp<UCReactionOrchestratorComponent>(target);
	EReactionRequestRejectReason reactionReason = EReactionRequestRejectReason::InvalidRequest;
	TestTrue(TEXT("Prepared reaction common query accepts start"), reactionQuery->QueryPreparedReactionAvailability(data.ReactionDataKey, false, reactionReason));
	TestTrue(TEXT("Successful query clears previous reason"), reactionReason == EReactionRequestRejectReason::None);
	TestTrue(TEXT("Execution wrapper accepts start"), reactionQuery->QueryPreparedExecutionReactionAvailability(EReactionType::ExecutionStandard, reactionReason));
	TestFalse(TEXT("Execution wrapper rejects non-execution type"), reactionQuery->QueryPreparedExecutionReactionAvailability(EReactionType::Hit, reactionReason));
	TestTrue(TEXT("Invalid execution type reports reason"), reactionReason == EReactionRequestRejectReason::InvalidRequest);
	FReactionDataKey invalidKey = data.ReactionDataKey;
	invalidKey.MatchMode = EReactionDataMatchMode::DamageSpec;
	TestFalse(TEXT("Common query rejects unsupported lookup mode"), reactionQuery->QueryPreparedReactionAvailability(invalidKey, false, reactionReason));
	TestTrue(TEXT("Unsupported mode reports invalid request"), reactionReason == EReactionRequestRejectReason::InvalidRequest);
	FReactionDataKey missingKey = data.ReactionDataKey;
	missingKey.ReactionType = EReactionType::ExecutionLethal;
	TestFalse(TEXT("Common query rejects missing prepared data"), reactionQuery->QueryPreparedReactionAvailability(missingKey, false, reactionReason));
	TestTrue(TEXT("Missing data reason returned"), reactionReason == EReactionRequestRejectReason::ReactionDataNotFound);
	TestFalse(TEXT("Availability queries never start a reaction"), reactions->IsActive());

	const FRotator rotation = target->GetActorRotation();
	const float remaining = balance->GetCollapseLoopRemainingSeconds();
	// Data and executor exist, but the executor's decision rejects an invalid owner.
	Field<ACharacter*>(prepared.ActionExecutor, TEXT("OwnerCharacter_Injected")) = nullptr;
	TestFalse(TEXT("Prepared execution decision rejected"), execution->QueryCombatExecutionAvailability(block));
	TestTrue(TEXT("Decision rejection is not missing data"), block == EExecutionAvailabilityBlock::ExecutionBlocked);
	TestFalse(TEXT("Request also rejects executor decision before reservation"), execution->RequestCombatExecution());
	TestTrue(TEXT("Rejected decision preserves opportunity"), balance->IsExecutionOpportunityAvailable());
	Field<ACharacter*>(prepared.ActionExecutor, TEXT("OwnerCharacter_Injected")) = source;
	for (int32 i = 0; i < 50; ++i) TestTrue(TEXT("Valid opportunity is ready"), execution->QueryCombatExecutionAvailability(block));
	TestTrue(TEXT("Successful query clears previous block reason"), block == EExecutionAvailabilityBlock::None);
	TestFalse(TEXT("Query never starts source session"), execution->HasActiveExecutionSession());
	TestFalse(TEXT("Query never starts target session"), Comp<UCExecutionCollaborationComponent>(target)->HasActiveExecutionSession());
	TestTrue(TEXT("Query never reserves opportunity"), balance->IsExecutionOpportunityAvailable());
	TestEqual(TEXT("Query preserves timer"), balance->GetCollapseLoopRemainingSeconds(), remaining);
	TestEqual(TEXT("Query preserves target facing"), target->GetActorRotation(), rotation);
	target->SetActorLocation(FVector(301, 0, 0));
	TestFalse(TEXT("Request re-evaluates geometry after successful HUD query"), execution->RequestCombatExecution());
	TestTrue(TEXT("Geometry rejection does not reserve opportunity"), balance->IsExecutionOpportunityAvailable());
	TestFalse(TEXT("Geometry rejection does not start source session"), execution->HasActiveExecutionSession());
	TestFalse(TEXT("Geometry rejection does not start target session"), Comp<UCExecutionCollaborationComponent>(target)->HasActiveExecutionSession());
	TestFalse(TEXT("Outside range"), execution->QueryCombatExecutionAvailability(block));
	TestTrue(TEXT("Geometry block reason"), block == EExecutionAvailabilityBlock::Geometry);
	target->SetActorLocation(FVector(300, 0, 0));
	TestTrue(TEXT("Range boundary accepted"), execution->QueryCombatExecutionAvailability(block));
	source->SetActorRotation(FRotator(0, 20, 0));
	TestFalse(TEXT("Outside facing angle"), execution->QueryCombatExecutionAvailability(block));
	source->SetActorRotation(FRotator::ZeroRotator);
	Comp<UCStateComponent>(source)->SetActionState();
	TestFalse(TEXT("Request rejects changed source state"), execution->RequestCombatExecution());
	TestFalse(TEXT("Busy source"), execution->QueryCombatExecutionAvailability(block));
	Comp<UCStateComponent>(source)->SetIdleState();
	Comp<UCStateComponent>(target)->SetReactionState();
	TestFalse(TEXT("Request rejects changed target state"), execution->RequestCombatExecution());
	TestFalse(TEXT("Busy target"), execution->QueryCombatExecutionAvailability(block));
	Comp<UCStateComponent>(target)->SetIdleState();
	FExecutionSessionId session;
	session.SourceActor = source;
	session.Serial = 1;
	FExecutionOpportunityReservation reservation;
	TestTrue(TEXT("Fixture reservation"), balance->TryReserveExecutionOpportunity(session, reservation));
	TestFalse(TEXT("Reserved opportunity unavailable"), execution->QueryCombatExecutionAvailability(block));
	balance->ReleaseExecutionOpportunityReservation(reservation);
	Field<EBalanceLifecycleState>(balance, TEXT("BalanceLifecycleState")) = EBalanceLifecycleState::CollapseOutPending;
	TestFalse(TEXT("Expired opportunity unavailable"), execution->QueryCombatExecutionAvailability(block));
	source->GetCombatTargetComp()->RequestClearCombatTarget(ECombatTargetChangeReason::ManualClear);
	TestFalse(TEXT("Cleared target unavailable"), execution->QueryCombatExecutionAvailability(block));
	TestFalse(TEXT("Request rejects cleared target"), execution->RequestCombatExecution());
	world->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDActionPresentationTest, "Portfolio.UI.CombatHUD.ActionPresentationLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHUDActionPresentationTest::RunTest(const FString& Parameters)
{
	using namespace HUDActionTest;
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(world);
	ACPlayer* player = world->SpawnActor<ACPlayer>();
	player->GetHealthComp()->InitializeHealth(100.f, 100.f, EMaxHPUpdatePolicy::ClampCurrent);
	UCCombatHUDPresenterComponent* presenter = NewObject<UCCombatHUDPresenterComponent>(player);
	player->AddInstanceComponent(presenter);
	presenter->RegisterComponentWithWorld(world);
	presenter->SetControlledPlayer(player);
	UActorComponent* tick = presenter;
	UCDefenseComponent* defense = Comp<UCDefenseComponent>(player);
	defense->HandleGuardInStarted();
	tick->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Guard pose active"), presenter->GetViewData().Actions.Guard == EHUDActionState::Active);
	TestFalse(TEXT("Parry window is not success"), presenter->GetViewData().Actions.bParrySuccess);
	UCCombatSignalTargetComponent* signals = Comp<UCCombatSignalTargetComponent>(player);
	FCombatSignalTargetPacket packet;
	packet.Context.TargetActor = player;
	packet.ResultSerial = 1;
	packet.Result.DefenseOutcome = EDamageDefenseOutcome::Parry;
	packet.Result.bAccepted = false;
	signals->OnCombatSignalTargetAccepted.Broadcast(packet);
	tick->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Unaccepted parry does not flash"), presenter->GetViewData().Actions.bParrySuccess);
	packet.Result.bAccepted = true;
	signals->OnCombatSignalTargetAccepted.Broadcast(packet);
	tick->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Accepted parry flashes"), presenter->GetViewData().Actions.bParrySuccess);
	const double parryStartTime = world->GetTimeSeconds();
	world->Tick(LEVELTICK_TimeOnly, 0.3f);
	TestTrue(TEXT("World time advances beyond parry highlight"), world->GetTimeSeconds() - parryStartTime > 0.25);
	signals->OnCombatSignalTargetAccepted.Broadcast(packet);
	tick->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Flash expires; duplicate does not restart it"), presenter->GetViewData().Actions.bParrySuccess);
	packet.ResultSerial = 2;
	signals->OnCombatSignalTargetAccepted.Broadcast(packet);
	tick->TickComponent(0.f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("New parry result restarts highlight"), presenter->GetViewData().Actions.bParrySuccess);
	player->GetHealthComp()->TryKill();
	TestFalse(TEXT("Death clears success"), presenter->GetViewData().Actions.bParrySuccess);
	TestTrue(TEXT("Death clears active guard"), presenter->GetViewData().Actions.Guard == EHUDActionState::Unavailable);
	presenter->SetControlledPlayer(nullptr);
	TestFalse(TEXT("Unpossess removes combat subscription"), signals->OnCombatSignalTargetAccepted.IsBoundToObject(presenter));
	TestFalse(TEXT("Unpossess disables polling"), presenter->IsComponentTickEnabled());
	presenter->DestroyComponent();
	GEngine->DestroyWorldContext(world);
	world->DestroyWorld(false);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHUDExecutionReferenceBindingTest, "Portfolio.UI.CombatHUD.ExecutionReferenceBinding",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHUDExecutionReferenceBindingTest::RunTest(const FString& Parameters)
{
	using namespace HUDActionTest;
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	ACPlayer* original = world->SpawnActor<ACPlayer>();
	ACPlayer* replacement = world->SpawnActor<ACPlayer>();
	UCExecutionCollaborationComponent* execution = NewObject<UCExecutionCollaborationComponent>(original);
	Comp<UCExecutionCollaborationComponent>(original)->InitializeReferences(References(original));
	const FName actionHandler(TEXT("HandleActionEvent"));

	auto CheckBindings = [this, execution, actionHandler](ACPlayer* participant, bool bExpected)
	{
		TestEqual(TEXT("Health subscription"), participant->GetHealthComp()->OnDeadStateChanged.IsBoundToObject(execution), bExpected);
		TestEqual(TEXT("Target subscription"), participant->GetCombatTargetComp()->OnCombatTargetChanged.IsBoundToObject(execution), bExpected);
		TestEqual(TEXT("Action subscription"), Comp<UCActionComponent>(participant)->OnActionEvent.Contains(execution, actionHandler), bExpected);
		TestEqual(TEXT("Reaction subscription"), Comp<UCReactionComponent>(participant)->OnReactionExecutionLifecycleEvent.IsBoundToObject(execution), bExpected);
	};

	execution->InitializeReferences(References(original));
	CheckBindings(original, true);
	execution->InitializeReferences(References(original));
	CheckBindings(original, true);

	// Replacing references must detach from old objects, not just the new ones.
	execution->InitializeReferences(References(replacement));
	CheckBindings(original, false);
	CheckBindings(replacement, true);
	TestTrue(TEXT("Other subscriber remains bound"), original->GetHealthComp()->OnDeadStateChanged.IsBoundToObject(Comp<UCExecutionCollaborationComponent>(original)));

	execution->InitializeReferences(FCharacterComponentReferences());
	CheckBindings(replacement, false);
	world->DestroyWorld(false);
	return true;
}
#endif
