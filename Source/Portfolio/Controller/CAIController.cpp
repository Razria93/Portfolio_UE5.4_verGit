#include "Controller/CAIController.h"
#include "ProjectGlobal.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"

#include "Character/Player/CPlayer.h"
#include "Character/Enemy/CEnemy.h"
#include "Core/Debug/FAIPerceptionDebug.h"
#include "Core/Profiling/CAIPerceptionProfiling.h"
#include "AI/Patrol/CPatrolPath.h"

#include "Interface/TargetContextProvider.h"

#include "Type/CStateTypes.h"
#include "Type/CAITypes.h"
#include "AI/Blackboard/CAIKey.h"
#include "AI/Blackboard/CAIKeyRegistry.h"
#include "AI/Blackboard/CAIBlackboardValueHelper.h"
#include "AI/RuntimeLOD/CAIRuntimeLODTierResolver.h"

ACAIController::ACAIController()
{
	CurrentRuntimeLODTier = EAIRuntimeLODTier::Background;

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

ETeamAttitude::Type ACAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (Other.IsA<ACPlayer>())
	{
		return ETeamAttitude::Hostile;
	}

	if (Other.IsA<ACEnemy>())
	{
		return ETeamAttitude::Friendly;
	}

	return ETeamAttitude::Neutral;
}

bool ACAIController::InitializeSightConfig()
{
	if (!IsValid(AIPerceptionComp)) return false;

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");
	if (!IsValid(SightConfig)) return false;

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

// Runtime Lifecycle

bool ACAIController::InitializeControllerRuntime(APawn* InPawn)
{
	if (!SetPossessionRuntimeState(InPawn)) return false;

	ClearTargetPerceptionStateMap();
	InitializeRuntimeLODTierSnapshot();
	InitializePerceptionStateForProfiling();
	InitializePerceptionCandidateAudit();
	InitializeBlackboardEngageLatencyAudit();

	// Profiling disable path must not bind perception delegates.
	if (ShouldDisableEnemyPerceptionForProfiling())
	{
		DisableEnemyPerceptionForProfiling();
	}
	else
	{
		if (!BindPerceptionEvents()) return false;
	}

	if (!SetupBlackboardComponent()) return false;
	if (!InitializeBlackboardValues()) return false;
	if (!RefreshRuntimeLODTierFromBlackboard()) return false;
	if (!StartBehaviorTreeRuntime()) return false;

	return true;
}

void ACAIController::UninitializeControllerRuntime()
{
	StopBehaviorTreeRuntime();
	ClearBlackboardValues();
	UnbindPerceptionEvents();

	FAIPerceptionDebug::PrintPerceptionCandidateAuditSummary(ControlledPawn_Cached, PerceptionCandidateAuditState);
	FAIPerceptionDebug::PrintBlackboardEngageLatencyAuditSummary(ControlledPawn_Cached, BlackboardEngageLatencyAuditState);

	ClearBlackboardEngageLatencyAudit();
	ClearPerceptionCandidateAudit();
	ClearPerceptionStateForProfiling();
	ClearRuntimeLODTierSnapshot();
	ClearTargetPerceptionStateMap();

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

	UBlackboardComponent* blackboardComp = nullptr;
	bool bUsed = UseBlackboard(BlackboardAsset, blackboardComp);

	return bUsed && IsValid(blackboardComp);
}

// Blackboard Runtime Value

bool ACAIController::InitializeBlackboardValues()
{
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return false;
	if (!IsValid(ControlledPawn_Cached)) return false;

	TSet<FName> pendingCustomKeys;

	CAIBlackboardValueHelper::InitializeValues(blackboardComp, ControlledPawn_Cached, pendingCustomKeys);
	InitializeCustomBlackboardValues(blackboardComp, ControlledPawn_Cached, pendingCustomKeys);

	return CAIBlackboardValueHelper::ValidateCustomKeysApplied(pendingCustomKeys, ControlledPawn_Cached);
}

void ACAIController::InitializeCustomBlackboardValues(UBlackboardComponent* InBlackboardComp, const APawn* InOwnerPawn, TSet<FName>& InOutPendingKeys) const
{
	if (!IsValid(InBlackboardComp)) return;
	if (!IsValid(InOwnerPawn)) return;

	const ACEnemy* enemy = Cast<ACEnemy>(InOwnerPawn);
	if (!IsValid(enemy)) return;

	// Patrol custom values
	CAIBlackboardValueHelper::ApplyCustomBool(InBlackboardComp, InOutPendingKeys, CAIKey::Patrol::bUsePatrol, enemy->ShouldUsePatrol(), ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomObject(InBlackboardComp, InOutPendingKeys, CAIKey::Patrol::PatrolPath, enemy->GetPatrolPath(), ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomEnum(InBlackboardComp, InOutPendingKeys, CAIKey::Patrol::PatrolMode, static_cast<uint8>(enemy->GetPatrolMode()), ControlledPawn_Cached);

	// Investigate custom values
	CAIBlackboardValueHelper::ApplyCustomBool(InBlackboardComp, InOutPendingKeys, CAIKey::Investigate::bUseInvestigate, enemy->ShouldUseInvestigate(), ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Investigate::InvestigateDuration, enemy->GetInvestigateDuration(), ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomInt(InBlackboardComp, InOutPendingKeys, CAIKey::Investigate::InvestigateMaxIndex, enemy->GetInvestigateMaxIndex(), ControlledPawn_Cached);

	// Chase custom values
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Chase::ChaseOffsetRange, enemy->GetChaseOffsetRange(), ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Chase::ChaseEnterBuffer, enemy->GetChaseEnterBuffer(), ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Chase::ChaseExitBuffer, enemy->GetChaseExitBuffer(), ControlledPawn_Cached);

	// Alert custom values
	CAIBlackboardValueHelper::ApplyCustomBool(InBlackboardComp, InOutPendingKeys, CAIKey::Alert::bUseAlertStep, enemy->ShouldUseAlertStep(), ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Alert::StepForwardDistance, enemy->GetStepForwardDistance(), ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Alert::StepSideDistance, enemy->GetStepSideDistance(), ControlledPawn_Cached);
}

void ACAIController::ClearBlackboardValues()
{
	CAIBlackboardValueHelper::ClearValues(GetBlackboardComponent());
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
	// Target-specific perception updates are handled by OnTargetPerceptionUpdated.
}

void ACAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!IsValid(Actor)) return;

	const bool bHasTargetProvider = (Cast<ITargetContextProvider>(Actor) != nullptr);

	if (Stimulus.WasSuccessfullySensed())
	{
		RecordRawPerceptionCandidate(Actor);

		if (bHasTargetProvider)
		{
			RecordValidTargetProvider(Actor);
		}
		else
		{
			RecordInvalidTargetProvider(Actor);
		}
	}

	if (!bHasTargetProvider) return;

	FTargetPerceptionState& data = TargetPerceptionStateMap.FindOrAdd(Actor);

	RecordTargetPerceptionStateMapSizeForAudit();

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
	// Target forgotten state is derived from LOS and memory state.
}

// Query

EPerceptionBuildResult ACAIController::BuildPerceptionContext(FTargetPerceptionState& OutTargetPerceptionState)
{
	if (bPerceptionDisabledForProfiling) return EPerceptionBuildResult::NoData;

	UpdateTargetPerceptionStateMap();
	return SelectTopPriority(OutTargetPerceptionState);
}

// Target Data

void ACAIController::UpdateTargetPerceptionStateMap()
{
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	float nowTime = world->GetTimeSeconds();

	TArray<AActor*> removeKeys;
	for (TPair<AActor*, FTargetPerceptionState>& pair : TargetPerceptionStateMap)
	{
		AActor* actorKey = pair.Key;
		FTargetPerceptionState& data = pair.Value;

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
		TargetPerceptionStateMap.Remove(removeKey);
	}

	RecordTargetPerceptionStateMapSizeForAudit();
}

void ACAIController::ClearTargetPerceptionStateMap()
{
	TargetPerceptionStateMap.Reset();
}

EPerceptionBuildResult ACAIController::SelectTopPriority(FTargetPerceptionState& OutTargetPerceptionState)
{
	OutTargetPerceptionState = FTargetPerceptionState();

	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EPerceptionBuildResult::Error;

	if (TargetPerceptionStateMap.IsEmpty()) return EPerceptionBuildResult::NoData;

	int bestPriority = INT_MAX;
	FTargetPerceptionState topState;

	for (TPair<AActor*, FTargetPerceptionState>& pair : TargetPerceptionStateMap)
	{
		AActor* actorKey = pair.Key;
		FTargetPerceptionState& data = pair.Value;

		if (!IsValid(actorKey) || !data.IsValidData()) continue;

		if (data.TargetPriority < bestPriority)
		{
			bestPriority = data.TargetPriority;
			topState = data;
		}
	}

	if (bestPriority == INT_MAX || !topState.IsValidData()) return EPerceptionBuildResult::NoData;

	OutTargetPerceptionState = topState;
	return EPerceptionBuildResult::Success;
}

// Runtime LOD Snapshot

EAIRuntimeLODTier ACAIController::GetCurrentRuntimeLODTier() const
{
	return CurrentRuntimeLODTier;
}

bool ACAIController::RefreshRuntimeLODTierFromBlackboard()
{
	const UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		SetCurrentRuntimeLODTier(EAIRuntimeLODTier::Background);
		return false;
	}

	SetCurrentRuntimeLODTier(FAIRuntimeLODTierResolver::ResolveTier(*blackboardComp));
	return true;
}

void ACAIController::InitializeRuntimeLODTierSnapshot()
{
	SetCurrentRuntimeLODTier(EAIRuntimeLODTier::Background);
}

void ACAIController::ClearRuntimeLODTierSnapshot()
{
	SetCurrentRuntimeLODTier(EAIRuntimeLODTier::Background);
}

void ACAIController::SetCurrentRuntimeLODTier(EAIRuntimeLODTier InTier)
{
	CurrentRuntimeLODTier = InTier;
}

// Perception Profiling Gate

void ACAIController::InitializePerceptionStateForProfiling()
{
	EnableEnemyPerceptionForProfiling();
}

void ACAIController::ClearPerceptionStateForProfiling()
{
	EnableEnemyPerceptionForProfiling();
}

bool ACAIController::ShouldDisableEnemyPerceptionForProfiling() const
{
	return FAIPerceptionProfiling::ShouldDisableEnemyPerception(ControlledPawn_Cached);
}

void ACAIController::DisableEnemyPerceptionForProfiling()
{
	bPerceptionDisabledForProfiling = true;
	ClearTargetPerceptionStateMap();
	SetPerceptionSenseEnabledForProfiling(false);
}

void ACAIController::EnableEnemyPerceptionForProfiling()
{
	bPerceptionDisabledForProfiling = false;
	SetPerceptionSenseEnabledForProfiling(true);
}

void ACAIController::SetPerceptionSenseEnabledForProfiling(bool bEnabled)
{
	if (!IsValid(AIPerceptionComp)) return;
	if (!IsValid(SightConfig)) return;

	AIPerceptionComp->SetSenseEnabled(SightConfig->GetSenseImplementation(), bEnabled);
}

// Perception Candidate Audit Lifecycle

void ACAIController::InitializePerceptionCandidateAudit()
{
	ClearPerceptionCandidateAudit();

	PerceptionCandidateAuditState.bEnabled = ShouldAuditPerceptionCandidates();
	if (!PerceptionCandidateAuditState.bEnabled) return;

	UWorld* world = GetWorld();
	PerceptionCandidateAuditState.RuntimeStartTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	PerceptionCandidateAuditState.RuntimeStartFrame = GFrameCounter;
}

void ACAIController::ClearPerceptionCandidateAudit()
{
	PerceptionCandidateAuditState.Reset();
}

// Perception Candidate Audit Condition

bool ACAIController::ShouldAuditPerceptionCandidates() const
{
	if (!FAIPerceptionDebug::ShouldAuditPerceptionCandidates()) return false;

	return IsValid(ControlledPawn_Cached) && ControlledPawn_Cached->IsA<ACEnemy>();
}

// Perception Candidate Audit Record

void ACAIController::RecordRawPerceptionCandidate(AActor* InActor)
{
	if (!PerceptionCandidateAuditState.bEnabled) return;
	if (!IsValid(InActor)) return;

	++PerceptionCandidateAuditState.RawPerceptionEventCount;
	PerceptionCandidateAuditState.RawPerceptionActors.Add(InActor);

	if (PerceptionCandidateAuditState.FirstRawPerceptionTime >= 0.f) return;

	UWorld* world = GetWorld();
	PerceptionCandidateAuditState.FirstRawPerceptionTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	PerceptionCandidateAuditState.FirstRawPerceptionFrame = GFrameCounter;
}

void ACAIController::RecordValidTargetProvider(AActor* InActor)
{
	if (!PerceptionCandidateAuditState.bEnabled) return;
	if (!IsValid(InActor)) return;

	PerceptionCandidateAuditState.ValidTargetProviderActors.Add(InActor);

	if (PerceptionCandidateAuditState.FirstValidTargetTime >= 0.f) return;

	UWorld* world = GetWorld();
	PerceptionCandidateAuditState.FirstValidTargetTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	PerceptionCandidateAuditState.FirstValidTargetFrame = GFrameCounter;
}

void ACAIController::RecordInvalidTargetProvider(AActor* InActor)
{
	if (!PerceptionCandidateAuditState.bEnabled) return;
	if (!IsValid(InActor)) return;

	PerceptionCandidateAuditState.InvalidTargetProviderActors.Add(InActor);
}

void ACAIController::RecordTargetPerceptionStateMapSizeForAudit()
{
	if (!PerceptionCandidateAuditState.bEnabled) return;

	PerceptionCandidateAuditState.MaxTargetPerceptionStateMapSize = FMath::Max(
		PerceptionCandidateAuditState.MaxTargetPerceptionStateMapSize,
		TargetPerceptionStateMap.Num());
}

// Blackboard / Engage Latency Audit Lifecycle

void ACAIController::InitializeBlackboardEngageLatencyAudit()
{
	ClearBlackboardEngageLatencyAudit();

	BlackboardEngageLatencyAuditState.bEnabled = ShouldAuditBlackboardEngageLatency();
	if (!BlackboardEngageLatencyAuditState.bEnabled) return;

	UWorld* world = GetWorld();
	BlackboardEngageLatencyAuditState.RuntimeStartTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	BlackboardEngageLatencyAuditState.RuntimeStartFrame = GFrameCounter;
}

void ACAIController::ClearBlackboardEngageLatencyAudit()
{
	BlackboardEngageLatencyAuditState.Reset();
}

// Blackboard / Engage Latency Audit Condition

bool ACAIController::ShouldAuditBlackboardEngageLatency() const
{
	if (!FAIPerceptionDebug::ShouldAuditBlackboardEngageLatency()) return false;

	return IsValid(ControlledPawn_Cached) && ControlledPawn_Cached->IsA<ACEnemy>();
}

// Blackboard / Engage Latency Audit Record

void ACAIController::RecordPerceptionContextBuiltForAudit(AActor* InTargetActor)
{
	if (!BlackboardEngageLatencyAuditState.bEnabled) return;
	if (!IsValid(InTargetActor)) return;
	if (BlackboardEngageLatencyAuditState.FirstPerceptionContextTime >= 0.f) return;

	UWorld* world = GetWorld();
	BlackboardEngageLatencyAuditState.FirstPerceptionContextTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	BlackboardEngageLatencyAuditState.FirstPerceptionContextFrame = GFrameCounter;
	BlackboardEngageLatencyAuditState.FirstPerceptionTargetActor = InTargetActor;
}

void ACAIController::RecordBlackboardTargetSetForAudit(AActor* InTargetActor)
{
	if (!BlackboardEngageLatencyAuditState.bEnabled) return;
	if (!IsValid(InTargetActor)) return;
	if (BlackboardEngageLatencyAuditState.FirstBlackboardTargetTime >= 0.f) return;

	UWorld* world = GetWorld();
	BlackboardEngageLatencyAuditState.FirstBlackboardTargetTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	BlackboardEngageLatencyAuditState.FirstBlackboardTargetFrame = GFrameCounter;
	BlackboardEngageLatencyAuditState.FirstBlackboardTargetActor = InTargetActor;
}

void ACAIController::RecordEngageRequestSubmittedForAudit(AActor* InTargetActor)
{
	if (!BlackboardEngageLatencyAuditState.bEnabled) return;
	if (!IsValid(InTargetActor)) return;
	if (BlackboardEngageLatencyAuditState.FirstEngageRequestTime >= 0.f) return;

	UWorld* world = GetWorld();
	BlackboardEngageLatencyAuditState.FirstEngageRequestTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	BlackboardEngageLatencyAuditState.FirstEngageRequestFrame = GFrameCounter;
	BlackboardEngageLatencyAuditState.FirstEngageRequestTargetActor = InTargetActor;
}

void ACAIController::RecordEngageAssignmentResolvedForAudit(AActor* InTargetActor)
{
	if (!BlackboardEngageLatencyAuditState.bEnabled) return;
	if (!IsValid(InTargetActor)) return;
	if (BlackboardEngageLatencyAuditState.FirstEngageAssignmentTime >= 0.f) return;

	UWorld* world = GetWorld();
	BlackboardEngageLatencyAuditState.FirstEngageAssignmentTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	BlackboardEngageLatencyAuditState.FirstEngageAssignmentFrame = GFrameCounter;
	BlackboardEngageLatencyAuditState.FirstEngageAssignmentTargetActor = InTargetActor;
}

