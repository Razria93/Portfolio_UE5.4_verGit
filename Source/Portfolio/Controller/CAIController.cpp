#include "Controller/CAIController.h"
#include "ProjectGlobal.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"

#include "Character/Enemy/CEnemy.h"
#include "AI/Patrol/CPatrolPath.h"

#include "Interface/TargetContextProvider.h"

#include "Type/CStateStructure.h"
#include "Type/CAIStructure.h"
#include "AI/BlackBoard/CAIKey.h"
#include "AI/BlackBoard/CAIKeyRegistry.h"

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

	if (!InitializeControllerRuntime(InPawn))
	{
		UninitializeControllerRuntime();
		return;
	}
}

void ACAIController::OnUnPossess()
{
	UninitializeControllerRuntime();

	Super::OnUnPossess();
}

void ACAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeControllerRuntime();

	Super::EndPlay(EndPlayReason);
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

// Runtime Lifecycle

bool ACAIController::InitializeControllerRuntime(APawn* InPawn)
{
	if (!SetPossessionRuntimeState(InPawn)) return false;

	ClearTargetDataMap();

	if (!BindPerceptionEvents()) return false;
	if (!SetupBlackboardComponent()) return false;
	if (!SetInitialBlackboardRuntimeValues()) return false;
	if (!StartBehaviorTreeRuntime()) return false;

	return true;
}

void ACAIController::UninitializeControllerRuntime()
{
	StopBehaviorTreeRuntime();
	ClearBlackboardRuntimeValues();
	UnbindPerceptionEvents();

	ClearTargetDataMap();

	ResetPossessionRuntimeState();
}

// Possession Runtime

bool ACAIController::SetPossessionRuntimeState(APawn* InPawn)
{
	if (!IsValid(InPawn)) return false;

	ControlledPawn_Cached = InPawn;
	return true;
}

void ACAIController::ResetPossessionRuntimeState()
{
	ControlledPawn_Cached = nullptr;
}

// Perception Binding

bool ACAIController::BindPerceptionEvents()
{
	if (!IsValid(AIPerceptionComp)) return false;

	UnbindPerceptionEvents();

	AIPerceptionComp->OnPerceptionUpdated.AddUniqueDynamic(this, &ACAIController::OnPerceptionUpdated);
	AIPerceptionComp->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ACAIController::OnTargetPerceptionUpdated);
	AIPerceptionComp->OnTargetPerceptionForgotten.AddUniqueDynamic(this, &ACAIController::OnTargetPerceptionForgotten);

	return true;
}

void ACAIController::UnbindPerceptionEvents()
{
	if (!IsValid(AIPerceptionComp)) return;

	AIPerceptionComp->OnPerceptionUpdated.RemoveDynamic(this, &ACAIController::OnPerceptionUpdated);
	AIPerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &ACAIController::OnTargetPerceptionUpdated);
	AIPerceptionComp->OnTargetPerceptionForgotten.RemoveDynamic(this, &ACAIController::OnTargetPerceptionForgotten);
}

// Blackboard Setup

bool ACAIController::SetupBlackboardComponent()
{
	if (!BlackboardAsset) return false;
	if (!CAIKeyRegistry::ValidateRequiredKeys(BlackboardAsset)) return false;

	// blackboardComp: Out Parameter
	UBlackboardComponent* blackboardComp = nullptr;
	bool bUsed = UseBlackboard(BlackboardAsset, blackboardComp);

	return bUsed && IsValid(blackboardComp);
}

// Blackboard Runtime Value

bool ACAIController::SetInitialBlackboardRuntimeValues()
{
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return false;

	// Targeting 
	blackboardComp->ClearValue(CAIKey::Targeting::TargetActor.KeyName);
	blackboardComp->SetValueAsInt(CAIKey::Targeting::TargetPriority.KeyName, INT_MAX);

	// State
	blackboardComp->SetValueAsEnum(CAIKey::State::AIIntentState.KeyName, static_cast<uint8>(EAIIntentState::Idle));

	// Perception
	blackboardComp->SetValueAsBool(CAIKey::Perception::bHasLOS.KeyName, false);

	// Navigation
	blackboardComp->SetValueAsBool(CAIKey::Navigation::bReturnHome.KeyName, false);

	if (APawn* ownerPawn = GetPawn())
	{
		blackboardComp->SetValueAsVector(CAIKey::Navigation::HomeLocation.KeyName, ownerPawn->GetActorLocation());

		if (ACEnemy* enemy = Cast<ACEnemy>(ownerPawn))
		{
			// --- Patrol ---
			bool bUsePatrol = enemy->GetbUsePatrol();
			ACPatrolPath* patrolPath = enemy->GetPatrolPath();
			EPatrolMode patrolMode = enemy->GetPatrolMode();

			// Set
			blackboardComp->SetValueAsBool(CAIKey::Patrol::bUsePatrol.KeyName, bUsePatrol);
			blackboardComp->SetValueAsObject(CAIKey::Patrol::PatrolPath.KeyName, patrolPath ? patrolPath : nullptr);
			blackboardComp->SetValueAsEnum(CAIKey::Patrol::PatrolMode.KeyName, static_cast<uint8>(patrolMode));

			// Init
			blackboardComp->SetValueAsBool(CAIKey::Patrol::bPatrolReverse.KeyName, false);
			blackboardComp->SetValueAsVector(CAIKey::Patrol::PatrolLocation.KeyName, ownerPawn->GetActorLocation());
			blackboardComp->SetValueAsInt(CAIKey::Patrol::PatrolIndex.KeyName, -1);

			// --- Investigate ---
			bool bUseInvestigate = enemy->GetbUseInvestigate();
			float investigateDuration = enemy->GetInvestigateDuration();
			int  investigateMaxIndex = enemy->GetInvestigateMaxIndex();

			// Set
			blackboardComp->SetValueAsBool(CAIKey::Investigate::bUseInvestigate.KeyName, bUseInvestigate);
			blackboardComp->SetValueAsFloat(CAIKey::Investigate::InvestigateDuration.KeyName, investigateDuration);
			blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateMaxIndex.KeyName, investigateMaxIndex);

			// Init
			blackboardComp->SetValueAsBool(CAIKey::Investigate::bCanInvestigate.KeyName, false);
			blackboardComp->SetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName, false);
			blackboardComp->SetValueAsVector(CAIKey::Investigate::InvestigateLocation.KeyName, ownerPawn->GetActorLocation());
			blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateIndex.KeyName, INDEX_NONE);

			// --- Chase ---
			float chaseoffsetRange = enemy->GetChaseOffsetRange();
			float chaseEnterBuffer = enemy->GetChaseEnterBuffer();
			float chaseExitBuffer = enemy->GetChaseExitBuffer();

			// Set
			blackboardComp->SetValueAsFloat(CAIKey::Chase::ChaseOffsetRange.KeyName, chaseoffsetRange);
			blackboardComp->SetValueAsFloat(CAIKey::Chase::ChaseEnterBuffer.KeyName, chaseEnterBuffer);
			blackboardComp->SetValueAsFloat(CAIKey::Chase::ChaseExitBuffer.KeyName, chaseExitBuffer);

			// --- Alert ---
			bool bUseAlertStep = enemy->GetbUseAlertStep();
			float stepForwardDistance = enemy->GetStepForwardDistance();
			float stepSideDistance = enemy->GetStepSideDistance();

			// Set
			blackboardComp->SetValueAsBool(CAIKey::Alert::bUseAlertStep.KeyName, bUseAlertStep);
			blackboardComp->SetValueAsFloat(CAIKey::Alert::StepForwardDistance.KeyName, stepForwardDistance);
			blackboardComp->SetValueAsFloat(CAIKey::Alert::StepSideDistance.KeyName, stepSideDistance);

			// Init
			blackboardComp->SetValueAsBool(CAIKey::Alert::bInAlertRange.KeyName, false);
			blackboardComp->SetValueAsVector(CAIKey::Alert::AlertStepLocation.KeyName, ownerPawn->GetActorLocation());

			// --- Engage ---
			// Init
			blackboardComp->SetValueAsBool(CAIKey::Engage::bShouldEngage.KeyName, false);
			blackboardComp->SetValueAsBool(CAIKey::Engage::bCanCombatAction.KeyName, false);

			blackboardComp->SetValueAsBool(CAIKey::Engage::bIsCombatAction.KeyName, false);
			blackboardComp->SetValueAsBool(CAIKey::Engage::bInEngageRange.KeyName, false);
			blackboardComp->SetValueAsFloat(CAIKey::Engage::NextCombatActionTime.KeyName, -1.f);

			// --- Reaction ---
			// Init
			blackboardComp->SetValueAsBool(CAIKey::Reaction::bIsActiveReaction.KeyName, false);

			// --- Dead ---
			// Init
			blackboardComp->SetValueAsEnum(CAIKey::Dead::DeadState.KeyName, static_cast<uint8>(EDeadState::Alive));
		}
	}

	return true;
}

void ACAIController::ClearBlackboardRuntimeValues()
{
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	// Targeting
	blackboardComp->ClearValue(CAIKey::Targeting::TargetActor.KeyName);
	blackboardComp->ClearValue(CAIKey::Targeting::TargetPriority.KeyName);

	// State
	blackboardComp->ClearValue(CAIKey::State::AIIntentState.KeyName);

	// Perception
	blackboardComp->ClearValue(CAIKey::Perception::bHasLOS.KeyName);
	blackboardComp->ClearValue(CAIKey::Perception::LastSeenTime.KeyName);
	blackboardComp->ClearValue(CAIKey::Perception::LastKnownLocation.KeyName);

	// Metric
	blackboardComp->ClearValue(CAIKey::Metric::DistanceToTarget.KeyName);
	blackboardComp->ClearValue(CAIKey::Metric::DistanceToHome.KeyName);

	// Navigation
	blackboardComp->ClearValue(CAIKey::Navigation::HomeLocation.KeyName);
	blackboardComp->ClearValue(CAIKey::Navigation::bReturnHome.KeyName);

	// Patrol
	blackboardComp->ClearValue(CAIKey::Patrol::bUsePatrol.KeyName);
	blackboardComp->ClearValue(CAIKey::Patrol::PatrolPath.KeyName);
	blackboardComp->ClearValue(CAIKey::Patrol::PatrolMode.KeyName);
	blackboardComp->ClearValue(CAIKey::Patrol::bPatrolReverse.KeyName);
	blackboardComp->ClearValue(CAIKey::Patrol::PatrolLocation.KeyName);
	blackboardComp->ClearValue(CAIKey::Patrol::PatrolIndex.KeyName);

	// Investigate
	blackboardComp->ClearValue(CAIKey::Investigate::bUseInvestigate.KeyName);
	blackboardComp->ClearValue(CAIKey::Investigate::InvestigateDuration.KeyName);
	blackboardComp->ClearValue(CAIKey::Investigate::InvestigateMaxIndex.KeyName);
	blackboardComp->ClearValue(CAIKey::Investigate::bCanInvestigate.KeyName);
	blackboardComp->ClearValue(CAIKey::Investigate::bIsInvestigating.KeyName);
	blackboardComp->ClearValue(CAIKey::Investigate::InvestigateLocation.KeyName);
	blackboardComp->ClearValue(CAIKey::Investigate::InvestigateIndex.KeyName);

	// Chase
	blackboardComp->ClearValue(CAIKey::Chase::ChaseOffsetRange.KeyName);
	blackboardComp->ClearValue(CAIKey::Chase::ChaseEnterBuffer.KeyName);
	blackboardComp->ClearValue(CAIKey::Chase::ChaseExitBuffer.KeyName);

	// Alert
	blackboardComp->ClearValue(CAIKey::Alert::bUseAlertStep.KeyName);
	blackboardComp->ClearValue(CAIKey::Alert::StepForwardDistance.KeyName);
	blackboardComp->ClearValue(CAIKey::Alert::StepSideDistance.KeyName);
	blackboardComp->ClearValue(CAIKey::Alert::bInAlertRange.KeyName);
	blackboardComp->ClearValue(CAIKey::Alert::AlertStepLocation.KeyName);

	// Engage
	blackboardComp->ClearValue(CAIKey::Engage::bShouldEngage.KeyName);
	blackboardComp->ClearValue(CAIKey::Engage::bCanCombatAction.KeyName);
	blackboardComp->ClearValue(CAIKey::Engage::bIsCombatAction.KeyName);
	blackboardComp->ClearValue(CAIKey::Engage::bInEngageRange.KeyName);
	blackboardComp->ClearValue(CAIKey::Engage::NextCombatActionTime.KeyName);

	// Reaction
	blackboardComp->ClearValue(CAIKey::Reaction::bIsActiveReaction.KeyName);

	// Dead
	blackboardComp->ClearValue(CAIKey::Dead::DeadState.KeyName);
}

// Behavior Tree Runtime

bool ACAIController::StartBehaviorTreeRuntime()
{
	if (!BehaviorTreeAsset) return false;

	return RunBehaviorTree(BehaviorTreeAsset);
}

void ACAIController::StopBehaviorTreeRuntime()
{
	UBrainComponent* brainComp = GetBrainComponent();
	if (!IsValid(brainComp)) return;

	brainComp->StopLogic(TEXT("StopBehaviorTreeRuntime"));
}

// Perception Event Callback

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

// Query

EPerceptionBuildResult ACAIController::BuildPerceptionContext(FTargetData& OutTargetData)
{
	UpdateTargetDataMap();
	return SelectTopPriority(OutTargetData);
}

// Target Data

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
			ITargetContextProvider* provider = Cast<ITargetContextProvider>(data.TargetActor);
			if (!provider) continue;

			data.TargetPriority = provider->GetTargetPriority();
			data.LastSeenTime = nowTime;
			data.LastKnownLocation = data.TargetActor->GetActorLocation();
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

void ACAIController::ClearTargetDataMap()
{
	TargetDataMap.Reset();
}

EPerceptionBuildResult ACAIController::SelectTopPriority(FTargetData& OutTargetData)
{
	OutTargetData = FTargetData();

	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EPerceptionBuildResult::Error;

	if (TargetDataMap.IsEmpty()) return EPerceptionBuildResult::NoData;

	int bestPriority = INT_MAX;
	FTargetData topData;

	for (TPair<AActor*, FTargetData>& pair : TargetDataMap)
	{
		AActor* actorKey = pair.Key;
		FTargetData& data = pair.Value;

		if (!IsValid(actorKey) || !data.IsValidData()) continue;

		if (data.TargetPriority < bestPriority)
		{
			bestPriority = data.TargetPriority;
			topData = data;
		}
	}

	if (bestPriority == INT_MAX || !topData.IsValidData()) return EPerceptionBuildResult::NoData;

	OutTargetData = topData;
	return EPerceptionBuildResult::Success;
}

// Debug

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
