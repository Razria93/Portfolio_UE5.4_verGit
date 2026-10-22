#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Controller/CAIController.h"
#include "System/Combat/CWorldSubsystem_CombatParticipation.h"
#include "AI/Blackboard/CAIKey.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "Character/Player/CPlayer.h"
#include "Engine/World.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionComponent.h"
#include "AI/BehaviorTree/Service/CBTService_UpdateAIIntentState.h"
#include "Component/CMovementComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CStateComponent.h"
#include "GameFramework/Character.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAIObservedTargetTest, "Portfolio.AI.ObservedTarget.SightLossAndReacquisition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAIObservedTargetTest::RunTest(const FString& Parameters)
{
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	world->CreateAISystem();
	ACAIController* controller = world->SpawnActor<ACAIController>();
	// This test needs only identity and a transform, not character gameplay startup.
	ACPlayer* target = world->SpawnActorDeferred<ACPlayer>(ACPlayer::StaticClass(), FTransform::Identity);
	UCWorldSubsystem_CombatParticipation* participation = world->GetSubsystem<UCWorldSubsystem_CombatParticipation>();
	if (!TestNotNull(TEXT("Participation subsystem"), participation)) { world->DestroyWorld(false); return false; }
	UBlackboardData* data = NewObject<UBlackboardData>(controller);
	auto addKey = [data](FName name, UBlackboardKeyType* type)
	{
		FBlackboardEntry entry; entry.EntryName = name; entry.KeyType = type; data->Keys.Add(entry);
	};
	for (FName name : { CAIKey::Investigate::bUseInvestigate.KeyName, CAIKey::Investigate::bShouldInvestigate.KeyName,
		CAIKey::Investigate::bIsInvestigating.KeyName, CAIKey::Investigate::bShouldEndInvestigate.KeyName, CAIKey::Navigation::bReturnHome.KeyName })
		addKey(name, NewObject<UBlackboardKeyType_Bool>(data));
	addKey(CAIKey::Perception::LastKnownLocation.KeyName, NewObject<UBlackboardKeyType_Vector>(data));
	addKey(CAIKey::Perception::LastSeenTime.KeyName, NewObject<UBlackboardKeyType_Float>(data));
	addKey(CAIKey::Investigate::InvestigateLocation.KeyName, NewObject<UBlackboardKeyType_Vector>(data));
	addKey(CAIKey::Investigate::InvestigateIndex.KeyName, NewObject<UBlackboardKeyType_Int>(data));
	UBlackboardComponent* bb = nullptr;
	if (!TestTrue(TEXT("Initialize blackboard"), controller->UseBlackboard(data, bb))) { world->DestroyWorld(false); return false; }
	if (!TestTrue(TEXT("Blackboard allocated its value storage"), bb->GetBlackboardAsset() == data)) { world->DestroyWorld(false); return false; }
	bb->SetValueAsBool(CAIKey::Investigate::bUseInvestigate.KeyName, true);

	const FVector observed(200.f, 300.f, 90.f);
	// Delayed stimulus delivery must use the sensed sample even when the actor moved.
	target->SetActorLocation(FVector(9000.f, 8000.f, 90.f));
	const FAIStimulus sample(*GetDefault<UAISense_Sight>(), 1.f, observed, FVector::ZeroVector);
	controller->OnTargetPerceptionUpdated(target, sample);
	TestEqual(TEXT("Queued sight callback preserves sensed location"), controller->PerceptionTargetContextMap.FindChecked(target).LastKnownLocation, observed);
	controller->AIPerceptionComp->RegisterStimulus(target, sample);
	controller->AIPerceptionComp->ProcessStimuli();
	FVector sampledLocation;
	TestTrue(TEXT("Perception stores a current sight sample"), controller->TryGetCurrentSightSample(target, sampledLocation));
	TestEqual(TEXT("Sight sample does not read actor live transform"), sampledLocation, observed);
	controller->UpdatePerceptionTargetContextMap();
	TestEqual(TEXT("BT refresh retains sensed location before sight-loss event"), controller->PerceptionTargetContextMap.FindChecked(target).LastKnownLocation, observed);

	FCombatParticipationEvidenceContext evidence;
	evidence.bHasTargetObservation = true;
	evidence.ObservedTargetLocation = observed;
	participation->ReportEvidence(controller, ECombatParticipationSource::Perception, target, evidence);
	FPerceptionTargetContext& sight = controller->PerceptionTargetContextMap.FindOrAdd(target);
	sight.TargetActor = target; sight.bHasLOS = true; sight.LastKnownLocation = observed;
	TestTrue(TEXT("Visible target permits combat projection"), controller->RefreshCombatTargetObservation(target));
	sight.bHasLOS = false;
	target->SetActorLocation(FVector(4000.f, -3000.f, 90.f));
	TestFalse(TEXT("Hidden target denies actor tracking immediately, before memory expires"), controller->RefreshCombatTargetObservation(target));
	TestTrue(TEXT("Sight loss starts investigation"), bb->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName));
	TestEqual(TEXT("Investigation uses the observation, not the hidden transform"), bb->GetValueAsVector(CAIKey::Perception::LastKnownLocation.KeyName), observed);
	FVector resolved;
	TestTrue(TEXT("Memory still retains an observation"), controller->TryGetTargetObservationLocation(target, resolved));
	TestEqual(TEXT("Memory does not follow hidden movement"), resolved, observed);
	bb->SetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName, false);
	bb->SetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName, true);
	FCombatParticipationEvidenceExhaustedEvent expiry;
	expiry.TargetActor = target;
	participation->TryGetLastKnownTargetContext(controller, target, expiry.LastKnownTargetContext);
	controller->HandleCombatParticipationEvidenceExhausted(expiry);
	TestTrue(TEXT("Memory expiry preserves the active search"), bb->GetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName));
	TestFalse(TEXT("Memory expiry does not restart search"), bb->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName));
	bb->SetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName, false);
	controller->RefreshCombatTargetObservation(target);
	TestFalse(TEXT("Completed search is not rearmed by unchanged memory"), bb->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName));

	sight.bHasLOS = true;
	controller->CancelInvestigateForNewCombatEvidence();
	evidence.ObservedTargetLocation = target->GetActorLocation();
	participation->ReportEvidence(controller, ECombatParticipationSource::Perception, target, evidence);
	TestTrue(TEXT("Reacquisition restores combat projection"), controller->RefreshCombatTargetObservation(target));
	controller->TryGetTargetObservationLocation(target, resolved);
	TestEqual(TEXT("Reacquisition updates the observation"), resolved, target->GetActorLocation());
	sight.bHasLOS = false;
	controller->RefreshCombatTargetObservation(target);
	TestTrue(TEXT("A second loss starts a new search"), bb->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName));
	controller->CancelInvestigateForNewCombatEvidence();
	bb->SetValueAsBool(CAIKey::Navigation::bReturnHome.KeyName, true);
	evidence.ObservedTargetLocation += FVector(100.f, 0.f, 0.f);
	participation->ReportEvidence(controller, ECombatParticipationSource::HitReactive, target, evidence);
	controller->RefreshCombatTargetObservation(target);
	TestFalse(TEXT("Return home prevents new investigation"), bb->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName));
	bb->SetValueAsBool(CAIKey::Navigation::bReturnHome.KeyName, false);
	controller->RefreshCombatTargetObservation(target);
	TestTrue(TEXT("Unseen hit evidence starts snapshot investigation"), bb->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName));
	TestFalse(TEXT("Unseen hit does not authorize actor tracking"), controller->HasCurrentSightOf(target));

	AActor* unknown = world->SpawnActor<AActor>();
	TestFalse(TEXT("Missing observation never falls back to live actor location"), controller->TryGetTargetObservationLocation(unknown, resolved));
	controller->CancelInvestigateForNewCombatEvidence();
	bb->SetValueAsBool(CAIKey::Investigate::bUseInvestigate.KeyName, false);
	controller->InvestigatedObservationTarget.Reset();
	controller->RefreshCombatTargetObservation(target);
	TestFalse(TEXT("Disabled search does not become omniscient chase"), bb->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName));
	world->DestroyWorld(false);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAIInvestigateReadinessTest, "Portfolio.AI.ObservedTarget.InvestigateWaitsForMovement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAIInvestigateReadinessTest::RunTest(const FString&)
{
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	world->CreateAISystem();
	AAIController* controller = world->SpawnActor<AAIController>();
	ACharacter* pawn = world->SpawnActor<ACharacter>();
	controller->Possess(pawn);
	UCMovementComponent* movement = NewObject<UCMovementComponent>(pawn);
	pawn->AddInstanceComponent(movement);
	FCharacterComponentReferences refs;
	refs.OwnerCharacter = pawn; refs.CharacterMovementComponent = pawn->GetCharacterMovement();
	refs.HealthComponent = NewObject<UCHealthComponent>(pawn); refs.StateComponent = NewObject<UCStateComponent>(pawn);
	refs.HealthComponent->InitializeHealth(100,100,EMaxHPUpdatePolicy::ClampCurrent);
	movement->InitializeReferences(refs);
	UBlackboardData* data = LoadObject<UBlackboardData>(nullptr,TEXT("/Game/02_Controller/02_Enemy/AI/Blackboard/BB_Default.BB_Default"));
	UBlackboardComponent* bb = nullptr;
	if (!TestTrue(TEXT("Native blackboard initialized"), data && controller->UseBlackboard(data,bb))) { world->DestroyWorld(false); return false; }
	bb->SetValueAsEnum(CAIKey::Dead::DeadState.KeyName,uint8(EDeadState::Alive));
	bb->SetValueAsBool(CAIKey::Investigate::bUseInvestigate.KeyName,true);
	bb->SetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName,true);
	bb->SetValueAsEnum(CAIKey::State::AIIntentState.KeyName,uint8(EAIIntentState::Engage));
	const FVector anchor(100,200,90);
	bb->SetValueAsVector(CAIKey::Perception::LastKnownLocation.KeyName,anchor);
	UCBTService_UpdateAIIntentState* service = NewObject<UCBTService_UpdateAIIntentState>();
	movement->SetMovementEnabled(false);
	auto next = service->DecideNextAIIntentState(bb,controller,0);
	TestEqual(TEXT("Blocked pending search waits outside BT_Investigate"),next,EAIIntentState::Idle);
	service->ChangeAIIntentState(bb,next);
	TestEqual(TEXT("Leaving Engage actually enters the waiting state"),bb->GetValueAsEnum(CAIKey::State::AIIntentState.KeyName),uint8(EAIIntentState::Idle));
	TestTrue(TEXT("Waiting preserves the search request"),bb->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName));
	TestEqual(TEXT("Waiting preserves its observed anchor"),bb->GetValueAsVector(CAIKey::Perception::LastKnownLocation.KeyName),anchor);
	movement->SetMovementEnabled(true);
	TestEqual(TEXT("Movement recovery starts the same request"),service->DecideNextAIIntentState(bb,controller,0),EAIIntentState::Investigate);
	bb->SetValueAsBool(CAIKey::Reaction::bIsActiveReaction.KeyName,true);
	TestEqual(TEXT("Reaction remains higher priority"),service->DecideNextAIIntentState(bb,controller,0),EAIIntentState::HitReact);
	bb->SetValueAsEnum(CAIKey::Dead::DeadState.KeyName,uint8(EDeadState::Dead));
	TestEqual(TEXT("Death remains highest priority"),service->DecideNextAIIntentState(bb,controller,0),EAIIntentState::Dead);
	world->DestroyWorld(false);
	return true;
}
#endif
