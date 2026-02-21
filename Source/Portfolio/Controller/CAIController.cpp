#include "Controller/CAIController.h"
#include "ProjectGlobal.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Type/CAIStateStructure.h"
#include "AI/BlackBoard/CAIKey.h"

ACAIController::ACAIController()
{
	// Init AIPerceptionComp
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	check(AIPerceptionComp);

	InitializeSightConfig();

	if (IsValid(SightConfig))
	{
		AIPerceptionComp->SetDominantSense(*SightConfig->GetSenseImplementation());
	}
}

void ACAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ACAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!IsValid(InPawn)) return;

	ControlledPawn_Cached = InPawn;

	if (!InitializePerception()) return;
	if (!InitializeBlackBoard()) return;
	if (!InitializeBehaviorTree()) return;
	if (!InitializeBlackBoardValue()) return;
}

void ACAIController::OnUnPossess()
{
	Super::OnUnPossess();

	ControlledPawn_Cached = nullptr;
}

bool ACAIController::InitializeSightConfig()
{
	if (!IsValid(AIPerceptionComp)) return false;

	// TODO: Move perception config to data-driven asset.
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");
	if (!IsValid(SightConfig)) return false;

	// Set Default (Overridable in Blueprint Editor)
	SightConfig->SightRadius = 500.f;
	SightConfig->LoseSightRadius = 600.f;
	SightConfig->PeripheralVisionAngleDegrees = 45.f;
	SightConfig->SetMaxAge(2.f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	AIPerceptionComp->ConfigureSense(*SightConfig);

	return true;
}

// -----------------------------------------------------------------------------
// [AI Perception & Blackboard Initialization]
// 1. Controller		: Perception and interpretation (like human awareness)
// 2. Blackboard		: Storage of perceived facts
// 3. BehaviorTree		: Decision making based on facts
// 
// 4. Service Node		: Periodically maintains perceived facts
// 5. Decorator Node	: Evaluates logical conditions on perceived facts
// 6. Task Node			: Executes the decided actions
// -----------------------------------------------------------------------------

bool ACAIController::InitializePerception()
{
	if (!IsValid(AIPerceptionComp)) return false;

	AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ACAIController::OnPerceptionUpdated);
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ACAIController::OnTargetPerceptionUpdated);
	AIPerceptionComp->OnTargetPerceptionForgotten.AddDynamic(this, &ACAIController::OnTargetPerceptionForgotten);

	return true;
}

bool ACAIController::InitializeBlackBoard()
{
	if (!BlackboardAsset) return false;
	if (!ValidateBlackboardKeys(BlackboardAsset)) return false;

	// blackboardComp: Out Parameter
	UBlackboardComponent* blackboardComp = nullptr;
	bool bUsed = UseBlackboard(BlackboardAsset, blackboardComp);

	return bUsed && IsValid(blackboardComp);
}

bool ACAIController::InitializeBehaviorTree()
{
	if (!BehaviorTreeAsset) return false;

	return RunBehaviorTree(BehaviorTreeAsset);
}

bool ACAIController::InitializeBlackBoardValue()
{
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return false;

	// Targeting 
	blackboardComp->ClearValue(CAIKey::Targeting::TargetActor);

	// State_StateType
	blackboardComp->SetValueAsEnum(CAIKey::State::AIStateType, static_cast<uint8>(EAIStateType::Wait));

	// State_Perception
	blackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, false);

	// State_Combat
	blackboardComp->SetValueAsBool(CAIKey::Combat::bIsEncounterActive, false);
	blackboardComp->SetValueAsBool(CAIKey::Combat::bIsEngagementActive, false);
	blackboardComp->SetValueAsBool(CAIKey::Combat::bIsInCombatRange, false);
	blackboardComp->SetValueAsBool(CAIKey::Combat::bCanEngageTarget, false);

	// State_Reaction
	blackboardComp->SetValueAsBool(CAIKey::Reaction::bIsHitReacting, false);

	// State_Lifecycle
	blackboardComp->SetValueAsBool(CAIKey::Lifecycle::bIsDead, false);

	if (APawn* ownerPawn = GetPawn())
	{
		blackboardComp->SetValueAsVector(CAIKey::Perception::HomeLocation, ownerPawn->GetActorLocation());
	}

	return true;
}

void ACAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	PrintPerceptionUpdatedSummary(UpdatedActors);
}

void ACAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	PrintTargetPerceptionUpdatedSummary(Actor, Stimulus);

	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!blackboardComp) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		// Initialize Value ([NOTE] Update Value in Service)

		// Targeting
		blackboardComp->SetValueAsObject(CAIKey::Targeting::TargetActor, Actor);

		// Perception
		blackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, true);
		blackboardComp->SetValueAsFloat(CAIKey::Perception::LastSeenTime, world->GetTimeSeconds());
		blackboardComp->SetValueAsVector(CAIKey::Perception::LastKnownLocation, Stimulus.StimulusLocation);

		blackboardComp->SetValueAsBool(CAIKey::Combat::bIsEncounterActive, true);
	}
	else // WasSuccessfullySensed() == false
	{
		blackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, false);
	}
}

void ACAIController::OnTargetPerceptionForgotten(AActor* Actor)
{
	PrintTargetPerceptionForgotten(Actor);

	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!blackboardComp) return;

	blackboardComp->ClearValue(CAIKey::Targeting::TargetActor);
	blackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, false);
}

bool ACAIController::ValidateBlackboardKeys(const UBlackboardData* InBlackboardAsset) const
{
	if (!IsValid(InBlackboardAsset)) return false;

	// Targeting
	const bool bHasTargetActorKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Targeting::TargetActor);

	// StateType
	const bool bHasAIStateTypeKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::State::AIStateType);

	// Perception
	const bool bHasHasLOSKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Perception::bHasLOS);
	const bool bHasLastSeenTimeKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Perception::LastSeenTime);
	const bool bHasLastKnownLocationKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Perception::LastKnownLocation);
	const bool bHasHomeLocationKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Perception::HomeLocation);

	// Metric
	const bool bHasDistanceToTargetKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Metric::DistanceToTarget);

	// Combat | State
	const bool bHasIsEncounterActiveKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Combat::bIsEncounterActive);
	const bool bHasIsEngagementActiveKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Combat::bIsEngagementActive);
	const bool bHasIsInCombatRangeKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Combat::bIsInCombatRange);

	// Combat | Able
	const bool bHasCanEngageTargetKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Combat::bCanEngageTarget);

	// Reaction | State
	const bool bHasIsHitReactingKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Reaction::bIsHitReacting);

	// Lifecycle | State
	const bool bHasIsDeadKey = ValidateBlackboardBKey(InBlackboardAsset, CAIKey::Lifecycle::bIsDead);

	bool bAllValid = true;

	bAllValid &= bHasTargetActorKey;

	bAllValid &= bHasAIStateTypeKey;

	bAllValid &= bHasHasLOSKey;
	bAllValid &= bHasLastSeenTimeKey;
	bAllValid &= bHasLastKnownLocationKey;
	bAllValid &= bHasHomeLocationKey;

	bAllValid &= bHasDistanceToTargetKey;

	bAllValid &= bHasIsEncounterActiveKey;
	bAllValid &= bHasIsEngagementActiveKey;
	bAllValid &= bHasIsInCombatRangeKey;

	bAllValid &= bHasCanEngageTargetKey;

	bAllValid &= bHasIsHitReactingKey;

	bAllValid &= bHasIsDeadKey;

	if (!bAllValid)
	{
		FLog::Log(FString::Printf(TEXT("%-20s"), TEXT("[Error|ACAIController] Missing Blackboard keys.")));
		return false;
	}

	return true;
}

bool ACAIController::ValidateBlackboardBKey(const UBlackboardData* InBlackboardAsset, const FName& InKeyName) const
{
	// -----------------------------------------------------------------------------
	// [Blackboard Key Validate]
	// [EngineAPI] GetKeyID (UBlackboardData / UBlackboardComponent)
	// - true  : returns 'a valid FKey'
	// - false : returns 'FBlackboard::InvalidKey'
	// -----------------------------------------------------------------------------

	return IsValid(InBlackboardAsset) && (InBlackboardAsset->GetKeyID(InKeyName) != FBlackboard::InvalidKey);
}

void ACAIController::PrintPerceptionUpdatedSummary(const TArray<AActor*>& UpdatedActors) const
{
	FLog::Log(TEXT("====== Perception Updated ======="));
	FLog::Log(TEXT("-------- Updated Actors ---------"));

	if (UpdatedActors.Num() == 0)
	{
		FLog::Log(TEXT("None"));
	}
	else
	{
		FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("UpdateActors"), UpdatedActors.Num()));

		// NOTE: List of actors whose perception state changed during this frame
		// - ex. added / updated / removed
		for (const AActor* updatedActor : UpdatedActors)
		{
			FLog::Log(FString::Printf(TEXT("- %s"), *GetNameSafe(updatedActor)));
		}
	}

	FLog::Log(TEXT("================================="));
}

void ACAIController::PrintTargetPerceptionUpdatedSummary(AActor* Actor, const FAIStimulus& Stimulus) const
{
	FLog::Log(TEXT("=== Target Perception Updated ==="));

	FLog::Log(FString::Printf(TEXT("TargetActor = %s"), *GetNameSafe(Actor)));

	FLog::Log(FString::Printf(TEXT("Sense = %s | Perceived = %s | Age = %.2f"),
		*GetNameSafe(UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus)),
		Stimulus.WasSuccessfullySensed() ? TEXT("Gained") : TEXT("Lost"),
		Stimulus.GetAge()));

	FLog::Log(TEXT("================================="));
}

void ACAIController::PrintTargetPerceptionForgotten(AActor* Actor) const
{
	FLog::Log(TEXT("== Target Perception Forgotten =="));

	FLog::Log(FString::Printf(TEXT("TargetActor = %s"), *GetNameSafe(Actor)));

	FLog::Log(TEXT("================================="));
}
