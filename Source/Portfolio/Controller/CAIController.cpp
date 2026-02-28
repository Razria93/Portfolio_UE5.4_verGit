#include "Controller/CAIController.h"
#include "ProjectGlobal.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Interface/TargetContextProducer.h"

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
	blackboardComp->SetValueAsInt(CAIKey::Targeting::TargetPriority, INT_MAX);

	// State
	blackboardComp->SetValueAsEnum(CAIKey::State::AIStateType, static_cast<uint8>(EAIStateType::Idle));

	// Perception
	blackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, false);

	// Navigation
	blackboardComp->SetValueAsBool(CAIKey::Navigation::bUsePatrol, false);
	blackboardComp->SetValueAsBool(CAIKey::Navigation::bReturnHome, false);

	// Combat
	blackboardComp->SetValueAsBool(CAIKey::Combat::bInRange, false);
	blackboardComp->SetValueAsBool(CAIKey::Combat::bCanAttack, false);

	// Reaction
	blackboardComp->SetValueAsBool(CAIKey::Reaction::bIsHitReacting, false);

	// Lifecycle
	blackboardComp->SetValueAsBool(CAIKey::Lifecycle::bIsDead, false);

	if (APawn* ownerPawn = GetPawn())
	{
		blackboardComp->SetValueAsVector(CAIKey::Navigation::HomeLocation, ownerPawn->GetActorLocation());
	}

	return true;
}

void ACAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	// [Disable OnPerceptionUpdated]
}

void ACAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	PrintTargetPerceptionUpdatedSummary(Actor, Stimulus);

	if (!IsValid(Actor)) return;

	FTargetData& data = TargetDataMap.FindOrAdd(Actor);

	if (Stimulus.WasSuccessfullySensed())
	{
		data.bHasLOS = true;
		data.TargetActor = Actor;
		data.LastKnownLocation = Stimulus.StimulusLocation;
	}
	else // WasSuccessfullySensed() == false
	{
		data.bHasLOS = false;
	}
}

void ACAIController::OnTargetPerceptionForgotten(AActor* Actor)
{
	// [Disable OnTargetPerceptionForgotten]
	// - TargetForgotten is Controlled by bHasLOS and bHasMemory
}


void ACAIController::UpdateTargetDataMap()
{
	// -----------------------------------------------------------------------------
	// [Target Perception Level]
	// 1. Active Target 
	//	- TargetActor	: Valid
	//	- bHasLOS		: true
	//	- bHasMemory	: true
	// 
	// 2. Lost Target but Remembered 
	//	- TargetActor	: Invalid
	//	- bHasLOS		: false
	//	- bHasMemory	: true
	// 
	// 3. Timeout and Expired
	//	- TargetActor	: Invalid
	//	- bHasLOS		: false
	//	- bHasMemory	: false
	// -----------------------------------------------------------------------------

	// Used BT_Service API
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	float nowTime = world->GetTimeSeconds();

	TArray<AActor*> removeKeys;
	for (TPair<AActor*, FTargetData>& pair : TargetDataMap)
	{
		AActor* actorKey = pair.Key;
		FTargetData& data = pair.Value;

		if (!IsValid(actorKey) || !data.IsValidData())
		{
			removeKeys.Add(actorKey);
			continue;
		}

		if (data.bHasLOS)
		{
			ITargetContextProducer* producer = Cast<ITargetContextProducer>(data.TargetActor);
			if (!producer) continue;

			data.TargetPriority = producer->GetTargetPriority();
			data.LastSeenTime = nowTime;
		}
		else // bHasLOS == false
		{
			bool bMemoryValid = ((nowTime - data.LastSeenTime) > TargetMemoryTimeout);

			if (bMemoryValid)
			{
				removeKeys.Add(actorKey);
				continue;
			}
		}
	}

	for (AActor* removeKey : removeKeys)
	{
		FLog::Log(FString::Printf(TEXT("RemoveActor = %s"), *GetNameSafe(removeKey)));

		FLog::Log(TEXT("[Remove Actors Before]"));
		PrintAllTargetData();
		
		TargetDataMap.Remove(removeKey);

		FLog::Log(TEXT("[Remove Actors After]"));
		PrintAllTargetData();
	}
}

// Used BT_Service API
void ACAIController::UpdateBlackboardContext()
{
	// -----------------------------------------------------------------------------
	// [Target Perception Level]
	// 1. Active Target 
	//	- TargetActor	: Valid
	//	- bHasLOS		: true
	//	- bHasMemory	: true
	// 
	// 2. Lost Target but Remembered 
	//	- TargetActor	: Invalid
	//	- bHasLOS		: false
	//	- bHasMemory	: true
	// 
	// 3. Timeout and Expired
	//	- TargetActor	: Invalid
	//	- bHasLOS		: false
	//	- bHasMemory	: false
	// -----------------------------------------------------------------------------

	// Update Blackboard
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;


	FTargetData newTargetData; // Out: Data for the highest priority target
	bool bHasTopPriority = SelectTopPriority(newTargetData);

	AActor* currentTarget = Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor));

	if (bHasTopPriority && newTargetData.IsValidData())
	{
		// Change TargetActor (Current != New)
		if (currentTarget != newTargetData.TargetActor)
		{
			blackboardComp->SetValueAsObject(CAIKey::Targeting::TargetActor, newTargetData.TargetActor);
			blackboardComp->SetValueAsInt(CAIKey::Targeting::TargetPriority, newTargetData.TargetPriority);
		}

		if (newTargetData.bHasLOS)
		{
			blackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, newTargetData.bHasLOS);
			blackboardComp->SetValueAsFloat(CAIKey::Perception::LastSeenTime, newTargetData.LastSeenTime);
			blackboardComp->SetValueAsVector(CAIKey::Perception::LastKnownLocation, newTargetData.LastKnownLocation);
		}
		else // bHasLOS == false
		{
			blackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS, false);
		}

		return;
	}
	else
	{
		blackboardComp->ClearValue(CAIKey::Targeting::TargetActor);
		blackboardComp->ClearValue(CAIKey::Targeting::TargetPriority);
		blackboardComp->ClearValue(CAIKey::Perception::bHasLOS);
		blackboardComp->ClearValue(CAIKey::Perception::LastSeenTime);
		blackboardComp->ClearValue(CAIKey::Perception::LastKnownLocation);

		return;
	}
}

bool ACAIController::ValidateBlackboardKeys(const UBlackboardData* InBlackboardAsset) const
{
	if (!IsValid(InBlackboardAsset)) return false;

	// Targeting
	const bool bTargetActorKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Targeting::TargetActor);
	const bool bTargetPriorityKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Targeting::TargetPriority);

	// StateType
	const bool bAIStateTypeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::State::AIStateType);

	// Perception
	const bool bHasLOSKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Perception::bHasLOS);
	const bool bLastSeenTimeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Perception::LastSeenTime);
	const bool bLastKnownLocationKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Perception::LastKnownLocation);

	// Metric
	const bool bDistanceToTargetKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Metric::DistanceToTarget);
	const bool bDistanceToHomeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Metric::DistanceToHome);

	// Navigation
	const bool bHomeLocationKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Navigation::HomeLocation);
	const bool bUsePatrolKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Navigation::bUsePatrol);
	const bool bReturnHomeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Navigation::bReturnHome);

	// Combat
	const bool bInRangeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Combat::bInRange);
	const bool bCanAttackKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Combat::bCanAttack);

	// Reaction
	const bool bHasIsHitReactingKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Reaction::bIsHitReacting);

	// Lifecycle
	const bool bHasIsDeadKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Lifecycle::bIsDead);

	bool bAllValid = true;

	bAllValid &= bTargetActorKey;
	bAllValid &= bTargetPriorityKey;

	bAllValid &= bAIStateTypeKey;

	bAllValid &= bHasLOSKey;
	bAllValid &= bLastSeenTimeKey;
	bAllValid &= bLastKnownLocationKey;

	bAllValid &= bDistanceToTargetKey;
	bAllValid &= bDistanceToHomeKey;

	bAllValid &= bHomeLocationKey;
	bAllValid &= bUsePatrolKey;
	bAllValid &= bReturnHomeKey;

	bAllValid &= bInRangeKey;
	bAllValid &= bCanAttackKey;

	bAllValid &= bHasIsHitReactingKey;

	bAllValid &= bHasIsDeadKey;

	if (!bAllValid)
	{
		FLog::Log(FString::Printf(TEXT("%-20s"), TEXT("[Error|ACAIController] Missing Blackboard keys.")));
		return false;
	}

	return true;
}

bool ACAIController::ValidateBlackboardKey(const UBlackboardData* InBlackboardAsset, const FName& InKeyName) const
{
	// -----------------------------------------------------------------------------
	// [Blackboard Key Validate]
	// [EngineAPI] GetKeyID (UBlackboardData / UBlackboardComponent)
	// - true  : returns 'a valid FKey'
	// - false : returns 'FBlackboard::InvalidKey'
	// -----------------------------------------------------------------------------

	return IsValid(InBlackboardAsset) && (InBlackboardAsset->GetKeyID(InKeyName) != FBlackboard::InvalidKey);
}

bool ACAIController::SelectTopPriority(FTargetData& OutTargetData)
{
	if (TargetDataMap.IsEmpty()) return false;

	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return false;

	int bestPriority = INT_MAX;
	
	FTargetData bestData;
	for (TPair<AActor*, FTargetData>& pair : TargetDataMap)
	{
		AActor* actorKey = pair.Key;
		FTargetData& data = pair.Value;

		if (!IsValid(actorKey) || !data.IsValidData()) continue;

		if (data.TargetPriority < bestPriority)
		{
			bestPriority = data.TargetPriority;
			bestData = data;
			continue;
		}
	}

	if (bestPriority == INT_MAX || !bestData.IsValidData()) return false;

	OutTargetData = bestData;
	return true;
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

void ACAIController::PrintAllTargetData() const
{
	FLog::Log(TEXT("========= TargetDataMap ========="));

	if (TargetDataMap.IsEmpty())
	{
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetDataMap"), TEXT("IsEmpty")));
	}

	for (const TPair<AActor*, FTargetData>& pair : TargetDataMap)
	{
		PrintTargetData(pair.Value);
	}

	FLog::Log(TEXT("================================="));
}

void ACAIController::PrintTargetData(const FTargetData& InData) const
{
	FLog::Log(TEXT("---------- TargetData -----------"));

	if (!InData.IsValidData())
	{
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetData"), TEXT("InValid")));
		FLog::Log(TEXT("---------------------------------"));
		return;
	}

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InData.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("TargetPriority"), InData.TargetPriority));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bHasLOS"), InData.bHasLOS ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %.2f"), TEXT("LastSeenTime"), InData.LastSeenTime));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("LastKnownLocation"), *InData.LastKnownLocation.ToCompactString()));
	FLog::Log(TEXT("---------------------------------"));
}