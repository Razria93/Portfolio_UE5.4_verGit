#include "Controller/CAIController.h"

#include "ProjectGlobal.h"

#include "Character/Player/CPlayer.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CEnemyCombatParticipationComponent.h"
#include "Component/CEnemyCombatTargetFacingComponent.h"
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

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "NavigationSystem.h"
#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<float> CVarHitReactiveInvestigatePredictionLeadSeconds(
		TEXT("Portfolio.AI.CombatParticipation.HitReactiveInvestigatePredictionLeadSeconds"),
		1.0f,
		TEXT("Seconds of last observed Target velocity used when HitReactive is the final Evidence source for Investigate."),
		ECVF_Default);

	TAutoConsoleVariable<float> CVarHitReactiveInvestigateMaxPredictionDistance(
		TEXT("Portfolio.AI.CombatParticipation.HitReactiveInvestigateMaxPredictionDistance"),
		600.0f,
		TEXT("Maximum distance from LastKnownLocation for HitReactive terminal Investigate prediction. 0: use LastKnownLocation."),
		ECVF_Default);

	FVector ResolveHitReactiveInvestigateLocation(UWorld* InWorld, const FCombatParticipationLastKnownTargetContext& InContext)
	{
		FVector location = InContext.LastKnownLocation;
		const FVector velocity2D = InContext.LastObservedVelocity.GetSafeNormal2D();
		const float speed2D = InContext.LastObservedVelocity.Size2D();
		const float leadSeconds = FMath::Max(0.f, CVarHitReactiveInvestigatePredictionLeadSeconds.GetValueOnGameThread());
		const float maxDistance = FMath::Max(0.f, CVarHitReactiveInvestigateMaxPredictionDistance.GetValueOnGameThread());
		if (!velocity2D.IsNearlyZero() && speed2D > KINDA_SMALL_NUMBER && leadSeconds > 0.f && maxDistance > 0.f)
		{
			location += velocity2D * FMath::Min(speed2D * leadSeconds, maxDistance);
		}

		if (const UNavigationSystemV1* navigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(InWorld))
		{
			FNavLocation projectedLocation;
			if (navigationSystem->ProjectPointToNavigation(location, projectedLocation)) return projectedLocation.Location;
		}

		return InContext.LastKnownLocation;
	}
}

ACAIController::ACAIController()
{
	CurrentRuntimeLODTier = EAIRuntimeLODTier::Background;

	// Initialize AI perception component.
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	check(AIPerceptionComp);

	InitializeSightConfig();
}

void ACAIController::BeginPlay()
{
	Super::BeginPlay();

	ConfigureSightConfig();
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

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (!IsValid(SightConfig)) return false;

	return ConfigureSightConfig();
}

bool ACAIController::ConfigureSightConfig()
{
	if (!IsValid(AIPerceptionComp)) return false;
	if (!IsValid(SightConfig)) return false;

	SightConfig->SightRadius = PerceptionSetup.SightRadius;
	SightConfig->LoseSightRadius = PerceptionSetup.LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PerceptionSetup.PeripheralVisionAngleDegrees;
	SightConfig->SetMaxAge(PerceptionSetup.MaxAge);

	SightConfig->DetectionByAffiliation.bDetectEnemies = PerceptionSetup.bDetectEnemies;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = PerceptionSetup.bDetectFriendlies;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = PerceptionSetup.bDetectNeutrals;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(*SightConfig->GetSenseImplementation());

	return true;
}

// Runtime Lifecycle

bool ACAIController::InitializeControllerRuntime(APawn* InPawn)
{
	if (!SetPossessionRuntimeState(InPawn)) return false;

	ClearPerceptionTargetContextMap();
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
	ClearPerceptionTargetContextMap();

	ResetPossessionRuntimeState();
}

// Possession Runtime

bool ACAIController::SetPossessionRuntimeState(APawn* InPawn)
{
	if (!IsValid(InPawn)) return false;

	ControlledPawn_Cached = InPawn;

	ACEnemy* enemy = Cast<ACEnemy>(InPawn);
	UCEnemyCombatTargetFacingComponent* combatTargetFacingComp = IsValid(enemy) ? enemy->GetEnemyCombatTargetFacingComp() : nullptr;
	if (IsValid(combatTargetFacingComp))
	{
		combatTargetFacingComp->SetAIController(this);
	}

	UCEnemyCombatParticipationComponent* combatParticipationComp = IsValid(enemy) ? enemy->GetEnemyCombatParticipationComp() : nullptr;
	if (IsValid(combatParticipationComp))
	{
		combatParticipationComp->SetAIController(this);
	}

	return true;
}

void ACAIController::ResetPossessionRuntimeState()
{
	ACEnemy* enemy = Cast<ACEnemy>(ControlledPawn_Cached);
	UCEnemyCombatTargetFacingComponent* combatTargetFacingComp = IsValid(enemy) ? enemy->GetEnemyCombatTargetFacingComp() : nullptr;
	if (IsValid(combatTargetFacingComp))
	{
		combatTargetFacingComp->ClearAIController();
	}

	UCEnemyCombatParticipationComponent* combatParticipationComp = IsValid(enemy) ? enemy->GetEnemyCombatParticipationComp() : nullptr;
	if (IsValid(combatParticipationComp))
	{
		combatParticipationComp->ClearAIController();
	}

	ControlledPawn_Cached = nullptr;
}

// Perception Binding

bool ACAIController::BindPerceptionEvents()
{
	if (!IsValid(AIPerceptionComp)) return false;

	UnbindPerceptionEvents();

	AIPerceptionComp->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ACAIController::OnTargetPerceptionUpdated);
	AIPerceptionComp->OnTargetPerceptionForgotten.AddUniqueDynamic(this, &ACAIController::OnTargetPerceptionForgotten);

	return true;
}

void ACAIController::UnbindPerceptionEvents()
{
	if (!IsValid(AIPerceptionComp)) return;

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

	const FEnemyPatrolConfig& patrolConfig = enemy->GetPatrolConfig();
	const FEnemyInvestigateConfig& investigateConfig = enemy->GetInvestigateConfig();
	const FEnemyChaseConfig& chaseConfig = enemy->GetChaseConfig();
	const FEnemyAlertConfig& alertConfig = enemy->GetAlertConfig();

	// Apply patrol blackboard values.
	CAIBlackboardValueHelper::ApplyCustomBool(InBlackboardComp, InOutPendingKeys, CAIKey::Patrol::bUsePatrol, patrolConfig.bUsePatrol, ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomObject(InBlackboardComp, InOutPendingKeys, CAIKey::Patrol::PatrolPath, patrolConfig.PatrolPath, ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomEnum(InBlackboardComp, InOutPendingKeys, CAIKey::Patrol::PatrolMode, static_cast<uint8>(patrolConfig.PatrolMode), ControlledPawn_Cached);

	// Apply investigate blackboard values.
	CAIBlackboardValueHelper::ApplyCustomBool(InBlackboardComp, InOutPendingKeys, CAIKey::Investigate::bUseInvestigate, investigateConfig.bUseInvestigate, ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Investigate::InvestigateDuration, investigateConfig.Duration, ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomInt(InBlackboardComp, InOutPendingKeys, CAIKey::Investigate::InvestigateMaxIndex, investigateConfig.MaxIndex, ControlledPawn_Cached);

	// Apply chase blackboard values.
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Chase::ChaseOffsetRange, chaseConfig.DistanceBand.OffsetRange, ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Chase::ChaseEnterBuffer, chaseConfig.DistanceBand.EnterBuffer, ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Chase::ChaseExitBuffer, chaseConfig.DistanceBand.ExitBuffer, ControlledPawn_Cached);

	// Apply alert blackboard values.
	CAIBlackboardValueHelper::ApplyCustomBool(InBlackboardComp, InOutPendingKeys, CAIKey::Alert::bUseAlertStep, alertConfig.bUseAlertStep, ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Alert::StepForwardDistance, alertConfig.StepForwardDistance, ControlledPawn_Cached);
	CAIBlackboardValueHelper::ApplyCustomFloat(InBlackboardComp, InOutPendingKeys, CAIKey::Alert::StepSideDistance, alertConfig.StepSideDistance, ControlledPawn_Cached);
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

	ACEnemy* enemy = Cast<ACEnemy>(GetPawn());
	UCEnemyCombatParticipationComponent* combatParticipationComp = IsValid(enemy) ? enemy->GetEnemyCombatParticipationComp() : nullptr;
	ITargetContextProvider* targetProvider = Cast<ITargetContextProvider>(Actor);
	if (IsValid(combatParticipationComp) && targetProvider)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			FCombatParticipationEvidenceContext evidenceContext;
			evidenceContext.TargetPriority = targetProvider->GetTargetPriority();
			evidenceContext.DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
			evidenceContext.ObservedTargetLocation = Actor->GetActorLocation();
			evidenceContext.ObservedTargetVelocity = Actor->GetVelocity();
			evidenceContext.bHasTargetObservation = true;
			combatParticipationComp->ReportEvidence(ECombatParticipationSource::Perception, Actor, evidenceContext);
		}
	}

	FPerceptionTargetContext& data = PerceptionTargetContextMap.FindOrAdd(Actor);

	RecordPerceptionTargetContextMapSizeForAudit();

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

EPerceptionBuildResult ACAIController::BuildPerceptionContext(FPerceptionTargetContext& OutPerceptionTargetContext)
{
	if (bPerceptionDisabledForProfiling) return EPerceptionBuildResult::NoData;

	UpdatePerceptionTargetContextMap();
	return SelectTopPriority(OutPerceptionTargetContext);
}

void ACAIController::RefreshParticipationEvidenceFromPerception()
{
	ACEnemy* enemy = Cast<ACEnemy>(GetPawn());
	UCEnemyCombatParticipationComponent* participationComp = IsValid(enemy) ? enemy->GetEnemyCombatParticipationComp() : nullptr;
	if (!IsValid(participationComp)) return;

	for (const TPair<AActor*, FPerceptionTargetContext>& pair : PerceptionTargetContextMap)
	{
		const FPerceptionTargetContext& context = pair.Value;
		if (!context.bHasLOS || !IsValid(context.TargetActor)) continue;

		ITargetContextProvider* targetProvider = Cast<ITargetContextProvider>(context.TargetActor);
		if (!targetProvider) continue;

		FCombatParticipationEvidenceContext evidenceContext;
		evidenceContext.TargetPriority = targetProvider->GetTargetPriority();
		evidenceContext.DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), context.TargetActor->GetActorLocation());
		evidenceContext.ObservedTargetLocation = context.TargetActor->GetActorLocation();
		evidenceContext.ObservedTargetVelocity = context.TargetActor->GetVelocity();
		evidenceContext.bHasTargetObservation = true;

		participationComp->ReportEvidence(ECombatParticipationSource::Perception, context.TargetActor, evidenceContext);
	}
}

bool ACAIController::TryGetPerceptionEvidenceLifetimeDebug(const AActor* InTarget, bool& OutHasLOS, float& OutMemoryRemainingSeconds) const
{
	OutHasLOS = false;
	OutMemoryRemainingSeconds = 0.f;
	if (!IsValid(InTarget)) return false;

	const FPerceptionTargetContext* context = PerceptionTargetContextMap.Find(const_cast<AActor*>(InTarget));
	if (!context || !context->HasTarget()) return false;

	OutHasLOS = context->bHasLOS;
	if (OutHasLOS) return true;

	const UWorld* world = GetWorld();
	if (!IsValid(world)) return false;

	OutMemoryRemainingSeconds = FMath::Max(0.f, PerceptionSetup.TargetMemoryTimeout - (world->GetTimeSeconds() - context->LastSeenTime));
	return true;
}

void ACAIController::HandleCombatParticipationEvidenceExhausted(const FCombatParticipationEvidenceExhaustedEvent& InEvent)
{
	if (!InEvent.LastKnownTargetContext.bHasObservation) return;

	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	UWorld* world = GetWorld();
	if (!IsValid(blackboardComp) || !IsValid(world)) return;
	if (!blackboardComp->GetValueAsBool(CAIKey::Investigate::bUseInvestigate.KeyName)) return;
	if (blackboardComp->GetValueAsBool(CAIKey::Navigation::bReturnHome.KeyName)) return;
	if (blackboardComp->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName)) return;
	if (blackboardComp->GetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName)) return;

	const FVector investigateLocation = InEvent.FinalEvidenceSource == ECombatParticipationSource::HitReactive
		? ResolveHitReactiveInvestigateLocation(world, InEvent.LastKnownTargetContext)
		: InEvent.LastKnownTargetContext.LastKnownLocation;

	CAIBlackboardValueHelper::SetVectorIfChanged(blackboardComp, CAIKey::Perception::LastKnownLocation.KeyName, investigateLocation);
	CAIBlackboardValueHelper::SetFloatIfChanged(blackboardComp, CAIKey::Perception::LastSeenTime.KeyName, world->GetTimeSeconds());
	CAIBlackboardValueHelper::SetBoolIfChanged(blackboardComp, CAIKey::Investigate::bShouldInvestigate.KeyName, true);
}

void ACAIController::CancelInvestigateForNewCombatEvidence()
{
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	const bool bHasInvestigateContext = blackboardComp->GetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName)
		|| blackboardComp->GetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName)
		|| blackboardComp->GetValueAsBool(CAIKey::Investigate::bShouldEndInvestigate.KeyName);
	if (!bHasInvestigateContext) return;

	CAIBlackboardValueHelper::SetBoolIfChanged(blackboardComp, CAIKey::Investigate::bShouldInvestigate.KeyName, false);
	CAIBlackboardValueHelper::SetBoolIfChanged(blackboardComp, CAIKey::Investigate::bIsInvestigating.KeyName, false);
	CAIBlackboardValueHelper::SetBoolIfChanged(blackboardComp, CAIKey::Investigate::bShouldEndInvestigate.KeyName, false);
	blackboardComp->ClearValue(CAIKey::Investigate::InvestigateLocation.KeyName);
	CAIBlackboardValueHelper::SetIntIfChanged(blackboardComp, CAIKey::Investigate::InvestigateIndex.KeyName, INDEX_NONE);
	blackboardComp->ClearValue(CAIKey::Perception::LastSeenTime.KeyName);
	blackboardComp->ClearValue(CAIKey::Perception::LastKnownLocation.KeyName);
}

// Target Data

void ACAIController::UpdatePerceptionTargetContextMap()
{
	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	float nowTime = world->GetTimeSeconds();
	ACEnemy* enemy = Cast<ACEnemy>(GetPawn());
	UCEnemyCombatParticipationComponent* combatParticipationComp = IsValid(enemy) ? enemy->GetEnemyCombatParticipationComp() : nullptr;

	TArray<AActor*> removeKeys;
	for (TPair<AActor*, FPerceptionTargetContext>& pair : PerceptionTargetContextMap)
	{
		AActor* actorKey = pair.Key;
		FPerceptionTargetContext& data = pair.Value;

		if (!IsValid(actorKey) || !data.HasTarget())
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

			if (IsValid(combatParticipationComp))
			{
				FCombatParticipationEvidenceContext evidenceContext;
				evidenceContext.TargetPriority = data.TargetPriority;
				evidenceContext.DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), data.LastKnownLocation);
				evidenceContext.ObservedTargetLocation = data.LastKnownLocation;
				evidenceContext.ObservedTargetVelocity = data.TargetActor->GetVelocity();
				evidenceContext.bHasTargetObservation = true;
				combatParticipationComp->ReportEvidence(ECombatParticipationSource::Perception, data.TargetActor, evidenceContext);
			}
		}
		else // bHasLOS == false
		{
			bool bMemoryExpired = ((nowTime - data.LastSeenTime) > PerceptionSetup.TargetMemoryTimeout);

			if (bMemoryExpired)
			{
				removeKeys.Add(actorKey);
				continue;
			}
		}
	}

	for (AActor* removeKey : removeKeys)
	{
		if (IsValid(combatParticipationComp))
		{
			combatParticipationComp->WithdrawEvidence(ECombatParticipationSource::Perception, removeKey);
		}

		PerceptionTargetContextMap.Remove(removeKey);
	}

	RecordPerceptionTargetContextMapSizeForAudit();
}

void ACAIController::ClearPerceptionTargetContextMap()
{
	ACEnemy* enemy = Cast<ACEnemy>(GetPawn());
	UCEnemyCombatParticipationComponent* combatParticipationComp = IsValid(enemy) ? enemy->GetEnemyCombatParticipationComp() : nullptr;
	if (IsValid(combatParticipationComp))
	{
		for (const TPair<AActor*, FPerceptionTargetContext>& pair : PerceptionTargetContextMap)
		{
			combatParticipationComp->WithdrawEvidence(ECombatParticipationSource::Perception, pair.Key, false);
		}
	}

	PerceptionTargetContextMap.Reset();
}

EPerceptionBuildResult ACAIController::SelectTopPriority(FPerceptionTargetContext& OutPerceptionTargetContext) const
{
	OutPerceptionTargetContext = FPerceptionTargetContext();

	const UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EPerceptionBuildResult::Error;

	if (PerceptionTargetContextMap.IsEmpty()) return EPerceptionBuildResult::NoData;

	int bestPriority = INT_MAX;
	FPerceptionTargetContext topContext;

	for (const TPair<AActor*, FPerceptionTargetContext>& pair : PerceptionTargetContextMap)
	{
		AActor* actorKey = pair.Key;
		const FPerceptionTargetContext& data = pair.Value;

		if (!IsValid(actorKey) || !data.HasTarget()) continue;

		if (data.TargetPriority < bestPriority)
		{
			bestPriority = data.TargetPriority;
			topContext = data;
		}
	}

	if (bestPriority == INT_MAX || !topContext.HasTarget()) return EPerceptionBuildResult::NoData;

	OutPerceptionTargetContext = topContext;
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
	ClearPerceptionTargetContextMap();
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

void ACAIController::RecordPerceptionTargetContextMapSizeForAudit()
{
	if (!PerceptionCandidateAuditState.bEnabled) return;

	PerceptionCandidateAuditState.MaxPerceptionTargetContextMapSize = FMath::Max(
		PerceptionCandidateAuditState.MaxPerceptionTargetContextMapSize,
		PerceptionTargetContextMap.Num());
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
