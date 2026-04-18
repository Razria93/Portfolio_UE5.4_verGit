#include "Controller/CAIController.h"
#include "ProjectGlobal.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Character/Enemy/CEnemy.h"
#include "AI/Patrol/CPatrolPath.h"

#include "Interface/TargetContextProvider.h"

#include "Type/CStateStructure.h"
#include "Type/CAIStructure.h"
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
	if (!InitializeBlackBoardValue()) return;
	if (!InitializeBehaviorTree()) return;
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
	blackboardComp->SetValueAsBool(CAIKey::Navigation::bReturnHome, false);

	if (APawn* ownerPawn = GetPawn())
	{
		blackboardComp->SetValueAsVector(CAIKey::Navigation::HomeLocation, ownerPawn->GetActorLocation());

		if (ACEnemy* enemy = Cast<ACEnemy>(ownerPawn))
		{
			// --- Patrol ---
			bool bUsePatrol = enemy->GetbUsePatrol();
			ACPatrolPath* patrolPath = enemy->GetPatrolPath();
			EPatrolMode patrolMode = enemy->GetPatrolMode();

			// Set
			blackboardComp->SetValueAsBool(CAIKey::Patrol::bUsePatrol, bUsePatrol);
			blackboardComp->SetValueAsObject(CAIKey::Patrol::PatrolPath, patrolPath ? patrolPath : nullptr);
			blackboardComp->SetValueAsEnum(CAIKey::Patrol::PatrolMode, static_cast<uint8>(patrolMode));

			// Init
			blackboardComp->SetValueAsBool(CAIKey::Patrol::bPatrolReverse, false);
			blackboardComp->SetValueAsVector(CAIKey::Patrol::PatrolLocation, ownerPawn->GetActorLocation());
			blackboardComp->SetValueAsInt(CAIKey::Patrol::PatrolIndex, -1);

			// --- Investigate ---
			bool bUseInvestigate = enemy->GetbUseInvestigate();
			float investigateDuration = enemy->GetInvestigateDuration();
			int  investigateMaxIndex = enemy->GetInvestigateMaxIndex();

			// Set
			blackboardComp->SetValueAsBool(CAIKey::Investigate::bUseInvestigate, bUseInvestigate);
			blackboardComp->SetValueAsFloat(CAIKey::Investigate::InvestigateDuration, investigateDuration);
			blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateMaxIndex, investigateMaxIndex);

			// Init
			blackboardComp->SetValueAsBool(CAIKey::Investigate::bCanInvestigate, false);
			blackboardComp->SetValueAsBool(CAIKey::Investigate::bIsInvestigating, false);
			blackboardComp->SetValueAsVector(CAIKey::Investigate::InvestigateLocation, ownerPawn->GetActorLocation());
			blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateIndex, INDEX_NONE);

			// --- Chase ---
			float chaseoffsetRange = enemy->GetChaseOffsetRange();
			float chaseEnterBuffer = enemy->GetChaseEnterBuffer();
			float chaseExitBuffer = enemy->GetChaseExitBuffer();

			// Set
			blackboardComp->SetValueAsFloat(CAIKey::Chase::ChaseOffsetRange, chaseoffsetRange);
			blackboardComp->SetValueAsFloat(CAIKey::Chase::ChaseEnterBuffer, chaseEnterBuffer);
			blackboardComp->SetValueAsFloat(CAIKey::Chase::ChaseExitBuffer, chaseExitBuffer);

			// --- Alert ---
			bool bUseAlertStep = enemy->GetbUseAlertStep();
			float stepForwardDistance = enemy->GetStepForwardDistance();
			float stepSideDistance = enemy->GetStepSideDistance();

			// Set
			blackboardComp->SetValueAsBool(CAIKey::Alert::bUseAlertStep, bUseAlertStep);
			blackboardComp->SetValueAsFloat(CAIKey::Alert::StepForwardDistance, stepForwardDistance);
			blackboardComp->SetValueAsFloat(CAIKey::Alert::StepSideDistance, stepSideDistance);

			// Init
			blackboardComp->SetValueAsBool(CAIKey::Alert::bInAlertRange, false);
			blackboardComp->SetValueAsVector(CAIKey::Alert::AlertStepLocation, ownerPawn->GetActorLocation());

			// --- Engage ---
			// Init
			blackboardComp->SetValueAsBool(CAIKey::Engage::bShouldEngage, false);
			blackboardComp->SetValueAsBool(CAIKey::Engage::bInEngageRange, false);
			blackboardComp->SetValueAsBool(CAIKey::Engage::bCanAttack, false);
			blackboardComp->SetValueAsBool(CAIKey::Engage::bIsAttacking, false);
			blackboardComp->SetValueAsFloat(CAIKey::Engage::AttackableTime, -1.f);
			blackboardComp->SetValueAsInt(CAIKey::Engage::LastAttackIndex, INDEX_NONE);

			blackboardComp->SetValueAsInt(CAIKey::Engage::AttackIndex, INDEX_NONE);
			blackboardComp->SetValueAsEnum(CAIKey::Engage::AttackActionType, static_cast<uint8>(EActionType::Max));

			// --- Reaction ---
			// Init
			blackboardComp->SetValueAsBool(CAIKey::Reaction::bHasPendingReaction, false);
			blackboardComp->SetValueAsBool(CAIKey::Reaction::bHasActiveReaction, false);
			blackboardComp->SetValueAsInt(CAIKey::Reaction::PendingReactionVersion, INDEX_NONE);

			// --- Dead ---
			// Init
			blackboardComp->SetValueAsEnum(CAIKey::Dead::DeadState, static_cast<uint8>(EDeadState::Alive));
		}
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

EPerceptionBuildResult ACAIController::BuildPerceptionContext(FTargetData& OutTargetData)
{
	UpdateTargetDataMap();
	return SelectTopPriority(OutTargetData);
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
	const bool bReturnHomeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Navigation::bReturnHome);

	// Patrol
	const bool bUsePatrolKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Patrol::bUsePatrol);
	const bool bPatrolPathKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Patrol::PatrolPath);
	const bool bPatrolModeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Patrol::PatrolMode);

	const bool bPatrolReverseKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Patrol::bPatrolReverse);
	const bool bPatrolLocationKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Patrol::PatrolLocation);
	const bool bPatrolIndexKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Patrol::PatrolIndex);

	// Investigate
	const bool bUseInvestigateKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Investigate::bUseInvestigate);
	const bool bInvestigateDurationKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Investigate::InvestigateDuration);
	const bool bInvestigateMaxIndexKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Investigate::InvestigateMaxIndex);

	const bool bCanInvestigateKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Investigate::bCanInvestigate);
	const bool bIsInvestigatingKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Investigate::bIsInvestigating);
	const bool bInvestigateLocationKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Investigate::InvestigateLocation);
	const bool bInvestigateIndexKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Investigate::InvestigateIndex);

	// Chase
	const bool bChaseOffsetDintanceKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Chase::ChaseOffsetRange);
	const bool bChaseEnterBufferKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Chase::ChaseEnterBuffer);
	const bool bChaseExitBufferKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Chase::ChaseExitBuffer);

	// Alert
	const bool bUseAlertStepKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Alert::bUseAlertStep);
	const bool bStepForwardDistanceKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Alert::StepForwardDistance);
	const bool bStepSideDistanceKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Alert::StepSideDistance);

	const bool bInAlertRangeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Alert::bInAlertRange);
	const bool bAlertStepLocationKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Alert::AlertStepLocation);

	// Engage
	const bool bShouldEngageKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Engage::bShouldEngage);
	const bool bInEngageRangeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Engage::bInEngageRange);
	const bool bCanAttackKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Engage::bCanAttack);
	const bool bIsAttackingKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Engage::bIsAttacking);
	const bool bAttackableTimeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Engage::AttackableTime);
	const bool bLastAttackIndexKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Engage::LastAttackIndex);

	const bool bAttackIndexKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Engage::AttackIndex);
	const bool bAttackActionTypeKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Engage::AttackActionType);
	

	// Reaction
	const bool bHasPendingReactionKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Reaction::bHasPendingReaction);
	const bool bHasActiveReactionKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Reaction::bHasActiveReaction);
	const bool bPendingReactionVersionKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Reaction::PendingReactionVersion);

	// Dead
	const bool bDeadStateKey = ValidateBlackboardKey(InBlackboardAsset, CAIKey::Dead::DeadState);

	bool bAllValid = true;

	// Targeting
	bAllValid &= bTargetActorKey;
	bAllValid &= bTargetPriorityKey;

	// StateType
	bAllValid &= bAIStateTypeKey;

	// Perception
	bAllValid &= bHasLOSKey;
	bAllValid &= bLastSeenTimeKey;
	bAllValid &= bLastKnownLocationKey;

	// Metric
	bAllValid &= bDistanceToTargetKey;
	bAllValid &= bDistanceToHomeKey;

	// Navigation
	bAllValid &= bHomeLocationKey;
	bAllValid &= bReturnHomeKey;

	// Patrol
	bAllValid &= bUsePatrolKey;
	bAllValid &= bPatrolPathKey;
	bAllValid &= bPatrolModeKey;

	bAllValid &= bPatrolReverseKey;
	bAllValid &= bPatrolLocationKey;
	bAllValid &= bPatrolIndexKey;

	// Investigate
	bAllValid &= bUseInvestigateKey;
	bAllValid &= bInvestigateDurationKey;
	bAllValid &= bInvestigateMaxIndexKey;

	bAllValid &= bCanInvestigateKey;
	bAllValid &= bIsInvestigatingKey;
	bAllValid &= bInvestigateLocationKey;
	bAllValid &= bInvestigateIndexKey;

	// Chase
	bAllValid &= bChaseOffsetDintanceKey;
	bAllValid &= bChaseEnterBufferKey;
	bAllValid &= bChaseExitBufferKey;

	bAllValid &= bInAlertRangeKey;

	// Alert
	bAllValid &= bUseAlertStepKey;
	bAllValid &= bStepForwardDistanceKey;
	bAllValid &= bStepSideDistanceKey;

	bAllValid &= bAlertStepLocationKey;

	// Engage
	bAllValid &= bShouldEngageKey;
	bAllValid &= bInEngageRangeKey;
	bAllValid &= bCanAttackKey;
	bAllValid &= bIsAttackingKey;
	bAllValid &= bAttackableTimeKey;
	bAllValid &= bAttackActionTypeKey;
	bAllValid &= bLastAttackIndexKey;
	
	bAllValid &= bAttackIndexKey;
	bAllValid &= bAttackActionTypeKey;
	

	// Reaction
	bAllValid &= bHasPendingReactionKey;
	bAllValid &= bHasActiveReactionKey;
	bAllValid &= bPendingReactionVersionKey;

	// Dead
	bAllValid &= bDeadStateKey;

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