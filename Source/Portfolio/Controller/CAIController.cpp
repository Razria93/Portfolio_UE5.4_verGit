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

	/* --- Blackboard Key Validate --- */
	// [EngineAPI] GetKeyID (UBlackboardData / UBlackboardComponent)
	// - true  : returns 'a valid FKey'
	// - false : returns 'FBlackboard::InvalidKey'
	const bool bHasAIStateTypeKey = BlackboardAsset->GetKeyID(CAIKey::AIStateType) != FBlackboard::InvalidKey;
	const bool bHasTargetKey = BlackboardAsset->GetKeyID(CAIKey::TargetActor) != FBlackboard::InvalidKey;

	// [Error] Missing Blackboard keys
	if (!bHasAIStateTypeKey || !bHasTargetKey)
	{
		FLog::Log(FString::Printf(TEXT("[Error] Missing Blackboard Keys : AIStateTypeKey = %s | TargetActorKey = %s"),
			*CAIKey::AIStateType.ToString(),
			*CAIKey::TargetActor.ToString()));

		return false;
	}

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
	UBlackboardComponent* blackboardComponent = GetBlackboardComponent();
	if (!IsValid(blackboardComponent)) return false;

	blackboardComponent->ClearValue(CAIKey::TargetActor);
	blackboardComponent->SetValueAsEnum(CAIKey::AIStateType, static_cast<uint8>(EAIStateType::Wait));

	return true;
}

void ACAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	PrintPerceptionUpdatedSummary(UpdatedActors);
}

void ACAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	PrintTargetPerceptionUpdatedSummary(Actor, Stimulus);

	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!blackboardComp) return;

	if (Stimulus.WasSuccessfullySensed())
	{
		blackboardComp->SetValueAsObject(CAIKey::TargetActor, Actor);
	}
	else
	{
		blackboardComp->ClearValue(CAIKey::TargetActor);
	}
}

void ACAIController::OnTargetPerceptionForgotten(AActor* Actor)
{
	PrintTargetPerceptionForgotten(Actor);

	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!blackboardComp) return;

	blackboardComp->ClearValue(CAIKey::TargetActor);
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

void ACAIController::PrintTargetPerceptionUpdatedSummary(AActor* Actor, const FAIStimulus& Stimulus
) const
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
