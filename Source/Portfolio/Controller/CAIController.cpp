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
	if (!InitializeBlackboardRuntimeValues()) return false;
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

bool ACAIController::InitializeBlackboardRuntimeValues()
{
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return false;
	if (!IsValid(ControlledPawn_Cached)) return false;

	TSet<FName> pendingCustomKeys;

	ApplyInitialBlackboardValues(blackboardComp, ControlledPawn_Cached, pendingCustomKeys);
	ApplyCustomBlackboardValues(blackboardComp, ControlledPawn_Cached, pendingCustomKeys);

	return ValidateCustomBlackboardKeysApplied(pendingCustomKeys);
}

void ACAIController::ApplyInitialBlackboardValues(UBlackboardComponent* InBlackboardComp, const APawn* InOwnerPawn, TSet<FName>& OutPendingKeys) const
{
	if (!IsValid(InBlackboardComp)) return;

	for (const FAIBlackboardKeySpec& keySpec : CAIKeyRegistry::GetKeySpecs())
	{
		switch (keySpec.InitialValuePolicy)
		{
		case EAIBlackboardInitialValuePolicy::Fixed:
			ApplyFixedInitialBlackboardValue(InBlackboardComp, keySpec);
			break;

		case EAIBlackboardInitialValuePolicy::FromOwnerLocation:
			ApplyOwnerLocationInitialBlackboardValue(InBlackboardComp, InOwnerPawn, keySpec);
			break;

		case EAIBlackboardInitialValuePolicy::Custom:
			OutPendingKeys.Add(keySpec.KeyName);
			break;

		case EAIBlackboardInitialValuePolicy::None:
		default:
			break;
		}
	}
}

void ACAIController::ApplyFixedInitialBlackboardValue(UBlackboardComponent* InBlackboardComp, const FAIBlackboardKeySpec& InKeySpec) const
{
	if (!IsValid(InBlackboardComp)) return;

	switch (InKeySpec.ValueType)
	{
	case EAIBlackboardKeyValueType::Bool:
		InBlackboardComp->SetValueAsBool(InKeySpec.KeyName, InKeySpec.BoolDefault);
		break;

	case EAIBlackboardKeyValueType::Int:
		InBlackboardComp->SetValueAsInt(InKeySpec.KeyName, InKeySpec.IntDefault);
		break;

	case EAIBlackboardKeyValueType::Float:
		InBlackboardComp->SetValueAsFloat(InKeySpec.KeyName, InKeySpec.FloatDefault);
		break;

	case EAIBlackboardKeyValueType::Vector:
		InBlackboardComp->SetValueAsVector(InKeySpec.KeyName, InKeySpec.VectorDefault);
		break;

	case EAIBlackboardKeyValueType::Enum:
		InBlackboardComp->SetValueAsEnum(InKeySpec.KeyName, InKeySpec.EnumDefault);
		break;

	case EAIBlackboardKeyValueType::Object:
		InBlackboardComp->ClearValue(InKeySpec.KeyName);
		break;
	}
}

void ACAIController::ApplyOwnerLocationInitialBlackboardValue(UBlackboardComponent* InBlackboardComp, const APawn* InOwnerPawn, const FAIBlackboardKeySpec& InKeySpec) const
{
	if (!IsValid(InBlackboardComp)) return;
	if (!IsValid(InOwnerPawn)) return;

	InBlackboardComp->SetValueAsVector(InKeySpec.KeyName, InOwnerPawn->GetActorLocation());
}

void ACAIController::ApplyCustomBlackboardValues(UBlackboardComponent* InBlackboardComp, const APawn* InOwnerPawn, TSet<FName>& InOutPendingKeys) const
{
	if (!IsValid(InBlackboardComp)) return;
	if (!IsValid(InOwnerPawn)) return;

	const ACEnemy* enemy = Cast<ACEnemy>(InOwnerPawn);
	if (!IsValid(enemy)) return;

	// Patrol custom values
	SetCustomBlackboardBoolValue(InBlackboardComp, InOutPendingKeys, CAIKey::Patrol::bUsePatrol, enemy->GetbUsePatrol());
	SetCustomBlackboardObjectValue(InBlackboardComp, InOutPendingKeys, CAIKey::Patrol::PatrolPath, enemy->GetPatrolPath());
	SetCustomBlackboardEnumValue(InBlackboardComp, InOutPendingKeys, CAIKey::Patrol::PatrolMode, static_cast<uint8>(enemy->GetPatrolMode()));

	// Investigate custom values
	SetCustomBlackboardBoolValue(InBlackboardComp, InOutPendingKeys, CAIKey::Investigate::bUseInvestigate, enemy->GetbUseInvestigate());
	SetCustomBlackboardFloatValue(InBlackboardComp, InOutPendingKeys, CAIKey::Investigate::InvestigateDuration, enemy->GetInvestigateDuration());
	SetCustomBlackboardIntValue(InBlackboardComp, InOutPendingKeys, CAIKey::Investigate::InvestigateMaxIndex, enemy->GetInvestigateMaxIndex());

	// Chase custom values
	SetCustomBlackboardFloatValue(InBlackboardComp, InOutPendingKeys, CAIKey::Chase::ChaseOffsetRange, enemy->GetChaseOffsetRange());
	SetCustomBlackboardFloatValue(InBlackboardComp, InOutPendingKeys, CAIKey::Chase::ChaseEnterBuffer, enemy->GetChaseEnterBuffer());
	SetCustomBlackboardFloatValue(InBlackboardComp, InOutPendingKeys, CAIKey::Chase::ChaseExitBuffer, enemy->GetChaseExitBuffer());

	// Alert custom values
	SetCustomBlackboardBoolValue(InBlackboardComp, InOutPendingKeys, CAIKey::Alert::bUseAlertStep, enemy->GetbUseAlertStep());
	SetCustomBlackboardFloatValue(InBlackboardComp, InOutPendingKeys, CAIKey::Alert::StepForwardDistance, enemy->GetStepForwardDistance());
	SetCustomBlackboardFloatValue(InBlackboardComp, InOutPendingKeys, CAIKey::Alert::StepSideDistance, enemy->GetStepSideDistance());
}

void ACAIController::SetCustomBlackboardBoolValue(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, bool InValue) const
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsBool(InKeySpec.KeyName, InValue);
	MarkCustomBlackboardKeyApplied(InOutPendingKeys, InKeySpec);
}

void ACAIController::SetCustomBlackboardIntValue(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, int32 InValue) const
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsInt(InKeySpec.KeyName, InValue);
	MarkCustomBlackboardKeyApplied(InOutPendingKeys, InKeySpec);
}

void ACAIController::SetCustomBlackboardFloatValue(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, float InValue) const
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsFloat(InKeySpec.KeyName, InValue);
	MarkCustomBlackboardKeyApplied(InOutPendingKeys, InKeySpec);
}

void ACAIController::SetCustomBlackboardVectorValue(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, const FVector& InValue) const
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsVector(InKeySpec.KeyName, InValue);
	MarkCustomBlackboardKeyApplied(InOutPendingKeys, InKeySpec);
}

void ACAIController::SetCustomBlackboardEnumValue(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, uint8 InValue) const
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsEnum(InKeySpec.KeyName, InValue);
	MarkCustomBlackboardKeyApplied(InOutPendingKeys, InKeySpec);
}

void ACAIController::SetCustomBlackboardObjectValue(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, UObject* InValue) const
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsObject(InKeySpec.KeyName, InValue);
	MarkCustomBlackboardKeyApplied(InOutPendingKeys, InKeySpec);
}

void ACAIController::MarkCustomBlackboardKeyApplied(TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec) const
{
	ensureMsgf(
		InOutPendingKeys.Remove(InKeySpec.KeyName) > 0,
		TEXT("[AIController] Custom Blackboard key was not registered | Owner=%s | Key=%s"),
		*GetNameSafe(ControlledPawn_Cached),
		*InKeySpec.KeyName.ToString());
}

bool ACAIController::ValidateCustomBlackboardKeysApplied(const TSet<FName>& InPendingKeys) const
{
	if (InPendingKeys.IsEmpty()) return true;

	TArray<FString> pendingKeyNames;
	for (const FName& keyName : InPendingKeys)
	{
		pendingKeyNames.Add(keyName.ToString());
	}

	const FString pendingCustomKeys = FString::Join(pendingKeyNames, TEXT(", "));

	ensureMsgf(false,
		TEXT("[AIController] Missing custom Blackboard initial values | Owner=%s | Pending=%s"),
		*GetNameSafe(ControlledPawn_Cached),
		*pendingCustomKeys);

	return false;
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
