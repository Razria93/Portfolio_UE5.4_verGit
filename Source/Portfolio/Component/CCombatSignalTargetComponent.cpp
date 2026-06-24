#include "Component/CCombatSignalTargetComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CHealthComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CHitFeedbackComponent.h"
#include "Component/CDefenseComponent.h"
#include "Interface/CombatResultReceiver.h"

#include "Type/CWeaponStructure.h"

namespace
{
	const FName CombatCueBlinkTag(TEXT("Combat.Cue.Blink"));
	const FName CombatCueRepulseTag(TEXT("Combat.Cue.Repulse"));
}

UCCombatSignalTargetComponent::UCCombatSignalTargetComponent()
{
}

void UCCombatSignalTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor_Cached = GetOwner();
	check(OwnerActor_Cached);

	HealthComp_Cached = OwnerActor_Cached->FindComponentByClass<UCHealthComponent>();
	check(HealthComp_Cached);

	ReactionOrchestratorComp_Cached = OwnerActor_Cached->FindComponentByClass<UCReactionOrchestratorComponent>();
	check(ReactionOrchestratorComp_Cached);

	HitFeedbackComp_Cached = OwnerActor_Cached->FindComponentByClass<UCHitFeedbackComponent>();
	check(HitFeedbackComp_Cached);

	DefenseComp_Cached = OwnerActor_Cached->FindComponentByClass<UCDefenseComponent>();
	// check(DefenseComp_Cached);
}

float UCCombatSignalTargetComponent::RequestCombatSignalTarget(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return ProcessCombatSignalTarget(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

bool UCCombatSignalTargetComponent::RequestCombatSignalTarget(const FCombatSignal& InCombatSignal)
{
	return ProcessCombatSignalTarget(InCombatSignal);
}

float UCCombatSignalTargetComponent::ProcessCombatSignalTarget(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageEvent.IsOfType(FDefaultDamageEvent::ClassID))
	{
		const FDefaultDamageEvent& damageEvent = static_cast<const FDefaultDamageEvent&>(DamageEvent);
		return HandleDefaultDamageEvent(DamageAmount, damageEvent, EventInstigator, DamageCauser);
	}

	return 0.f;
}

bool UCCombatSignalTargetComponent::ProcessCombatSignalTarget(const FCombatSignal& InCombatSignal)
{
	if (InCombatSignal.Header.SignalType == ECombatSignalType::TimingCue)
	{
		return HandleTimingCueSignal(InCombatSignal);
	}

	return false;
}

float UCCombatSignalTargetComponent::HandleDefaultDamageEvent(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	// Receive: validate engine damage input and normalize it into target-side data.
	if (!FMath::IsFinite(DamageAmount)) return 0.f;
	if (!ValidateRequest(InDefaultDamageEvent, InDamageInstigator, InDamageCauser)) return 0.f;

	FCombatSignalTargetPayload combatSignalTargetPayload = BuildPayload(DamageAmount, InDefaultDamageEvent, InDamageInstigator, InDamageCauser);
	FCombatSignalTargetContext combatSignalTargetContext = BuildContext(combatSignalTargetPayload);

	// Evaluate: validate target-side context and defensive policy before applying state changes.
	if (!ValidateContext(combatSignalTargetContext))
	{
		// Packet / Notify: publish rejected target outcome.
		const FCombatSignalTargetResult rejectedResult = BuildResult(combatSignalTargetContext);
		const FCombatSignalTargetPacket combatSignalTargetPacket = BuildPacket(combatSignalTargetPayload, combatSignalTargetContext, rejectedResult);

		// PrintCombatSignalTargetSummaryInfo(combatSignalTargetPacket);
		DispatchRejectedCombatResult(combatSignalTargetPacket);
		return 0.f;
	}

	// Evaluate: snapshot target pre-state for outcome/result construction.
	combatSignalTargetContext.DeadState_Before = HealthComp_Cached->GetDeadState();
	combatSignalTargetContext.HealthPointBefore = HealthComp_Cached->GetCurrentHP();

	if (!CanReceiveCombatSignal(combatSignalTargetContext))
	{
		// Packet / Notify: publish rejected target outcome.
		const FCombatSignalTargetResult rejectedResult = BuildResult(combatSignalTargetContext);
		const FCombatSignalTargetPacket combatSignalTargetPacket = BuildPacket(combatSignalTargetPayload, combatSignalTargetContext, rejectedResult);

		// PrintCombatSignalTargetSummaryInfo(combatSignalTargetPacket);
		DispatchRejectedCombatResult(combatSignalTargetPacket);
		return 0.f;
	}

	ComputeTargetDamage(combatSignalTargetContext);

	if (!combatSignalTargetContext.bAccepted)
	{
		// Packet / Notify: publish rejected target outcome.
		const FCombatSignalTargetResult rejectedResult = BuildResult(combatSignalTargetContext);
		const FCombatSignalTargetPacket combatSignalTargetPacket = BuildPacket(combatSignalTargetPayload, combatSignalTargetContext, rejectedResult);

		// PrintCombatSignalTargetSummaryInfo(combatSignalTargetPacket);
		DispatchRejectedCombatResult(combatSignalTargetPacket);
		return 0.f;
	}

	// Apply: commit accepted damage to target-side resource state.
	CommitCombatSignalTarget(combatSignalTargetContext);

	// Packet: combine payload, context, and result for notify/debug consumers.
	const FCombatSignalTargetResult committedResult = BuildResult(combatSignalTargetContext);
	const FCombatSignalTargetPacket combatSignalTargetPacket = BuildPacket(combatSignalTargetPayload, combatSignalTargetContext, committedResult);

	// Notify: publish target outcome to reaction, feedback, and source-side result receivers.
	// PrintCombatSignalTargetSummaryInfo(combatSignalTargetPacket);
	PrintCombatSignalTargetOutcomeInfo(combatSignalTargetPacket);
	DispatchAcceptedCombatResult(combatSignalTargetPacket);
	DispatchCombatResultToReceiver(combatSignalTargetPacket);

	return committedResult.CommittedDamage;
}

bool UCCombatSignalTargetComponent::HandleTimingCueSignal(const FCombatSignal& InCombatSignal)
{
	if (!ValidateSignalRequest(InCombatSignal))
		return false;

	if (InCombatSignal.CueTag == CombatCueBlinkTag)
	{
		FLog::Log(TEXT("[CombatSignalTimingCue] Blink cue received"));
		return true;
	}

	if (InCombatSignal.CueTag == CombatCueRepulseTag)
	{
		FLog::Log(TEXT("[CombatSignalTimingCue] Repulse cue received"));
		return true;
	}

	// V1 hook only. Blink / Repulse evaluation and effects are added in separate branches.
	return false;
}

bool UCCombatSignalTargetComponent::ValidateRequest(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	if (!IsValid(OwnerActor_Cached)) return false;
	if (!IsValid(HealthComp_Cached)) return false;
	if (!IsValid(InDamageCauser)) return false;

	if (IsValid(InDefaultDamageEvent.TargetActor) && InDefaultDamageEvent.TargetActor != OwnerActor_Cached) return false;

	if (!FMath::IsFinite(InDefaultDamageEvent.DamageSpec.BaseDamage)) return false;
	if (!FMath::IsFinite(InDefaultDamageEvent.DamageAmount.RequestDamage)) return false;

	return true;
}

bool UCCombatSignalTargetComponent::ValidateSignalRequest(const FCombatSignal& InCombatSignal) const
{
	if (!InCombatSignal.IsValidMinimal()) return false;
	if (InCombatSignal.Header.SignalType != ECombatSignalType::TimingCue) return false;
	if (!IsValid(OwnerActor_Cached)) return false;
	if (!IsValid(InCombatSignal.Header.SourceActor)) return false;
	if (!IsValid(InCombatSignal.Header.TargetActor)) return false;
	if (InCombatSignal.Header.TargetActor != OwnerActor_Cached) return false;
	if (InCombatSignal.CueTag.IsNone()) return false;

	return true;
}

FCombatSignalTargetPayload UCCombatSignalTargetComponent::BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const
{
	FCombatSignalTargetPayload combatSignalTargetPayload = FCombatSignalTargetPayload();

	combatSignalTargetPayload.SourceActor = InDefaultDamageEvent.SourceActor;
	combatSignalTargetPayload.TargetActor = OwnerActor_Cached;
	combatSignalTargetPayload.EventInstigator = InDamageInstigator;
	combatSignalTargetPayload.DamageCauser = InDamageCauser;

	combatSignalTargetPayload.DamageImpactInfo = InDefaultDamageEvent.DamageImpactInfo;
	combatSignalTargetPayload.DamageSpecKey = InDefaultDamageEvent.DamageSpecKey;
	combatSignalTargetPayload.DamageSpec = InDefaultDamageEvent.DamageSpec;
	combatSignalTargetPayload.DamageAmount = InDefaultDamageEvent.DamageAmount;

	combatSignalTargetPayload.RequestedDamage = DamageAmount;

	return combatSignalTargetPayload;
}

FCombatSignalTargetContext UCCombatSignalTargetComponent::BuildContext(const FCombatSignalTargetPayload& InCombatSignalTargetPayload) const
{
	FCombatSignalTargetContext combatSignalTargetContext = FCombatSignalTargetContext();

	combatSignalTargetContext.SourceActor = InCombatSignalTargetPayload.SourceActor;
	combatSignalTargetContext.TargetActor = InCombatSignalTargetPayload.TargetActor;
	combatSignalTargetContext.Instigator = ResolveInstigatorController(InCombatSignalTargetPayload.EventInstigator, InCombatSignalTargetPayload.DamageCauser);
	combatSignalTargetContext.DamageCauser = InCombatSignalTargetPayload.DamageCauser;

	combatSignalTargetContext.DamageImpactInfo = InCombatSignalTargetPayload.DamageImpactInfo;
	combatSignalTargetContext.DamageSpecKey = InCombatSignalTargetPayload.DamageSpecKey;

	combatSignalTargetContext.RequestedDamage = InCombatSignalTargetPayload.RequestedDamage;

	return combatSignalTargetContext;
}

bool UCCombatSignalTargetComponent::ValidateContext(FCombatSignalTargetContext& InOutCombatSignalTargetContext)
{
	if (!IsValid(InOutCombatSignalTargetContext.TargetActor))
	{
		InOutCombatSignalTargetContext.bAccepted = false;
		InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::InvalidTarget;

		return false;
	}

	if (!IsValid(InOutCombatSignalTargetContext.DamageCauser))
	{
		InOutCombatSignalTargetContext.bAccepted = false;
		InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::InvalidCauser;

		return false;
	}

	if (!IsValid(InOutCombatSignalTargetContext.Instigator))
	{
		InOutCombatSignalTargetContext.bAccepted = false;
		InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::InvalidInstigator;

		return false;
	}

	InOutCombatSignalTargetContext.bAccepted = true;
	InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::None;

	return true;
}

bool UCCombatSignalTargetComponent::CanReceiveCombatSignal(FCombatSignalTargetContext& InOutCombatSignalTargetContext)
{
	// Gate 1: already dead
	if (InOutCombatSignalTargetContext.DeadState_Before != EDeadState::Alive)
	{
		InOutCombatSignalTargetContext.bAccepted = false;
		InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::AlreadyDead;

		return false;
	}

	// Gate 2: Parry window intercepts incoming damage before damage commit.
	if (IsValid(DefenseComp_Cached) && DefenseComp_Cached->CanParry())
	{
		InOutCombatSignalTargetContext.bAccepted = true;
		InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::None;
		InOutCombatSignalTargetContext.DefenseOutcome = EDamageDefenseOutcome::Parry;
		InOutCombatSignalTargetContext.bShouldCommitDamage = false;

		return true;
	}

	// TODO:
	// Gate 3: invulnerable / iframe / god-mode state
	// Gate 4: defensive friendly-fire check on receiver side
	// Gate 5: receiver-side damage cooldown / hit immunity window
	// Gate 6: defensive self-damage policy

	InOutCombatSignalTargetContext.bAccepted = true;
	InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::None;
	InOutCombatSignalTargetContext.DefenseOutcome = EDamageDefenseOutcome::None;
	InOutCombatSignalTargetContext.bShouldCommitDamage = true;

	return true;
}

void UCCombatSignalTargetComponent::ComputeTargetDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const
{
	// Process 1: Compute Mitigation Damage
	InOutCombatSignalTargetContext.MitigatedDamage = ComputeMitigatedDamage(InOutCombatSignalTargetContext);	// TODO

	// Gate 1: Zero damage
	if (InOutCombatSignalTargetContext.bShouldCommitDamage && InOutCombatSignalTargetContext.MitigatedDamage <= KINDA_SMALL_NUMBER)
	{
		InOutCombatSignalTargetContext.bAccepted = false;
		InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::ZeroDamage;

		return;
	}

	InOutCombatSignalTargetContext.bAccepted = true;
	InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::None;

	// Process 2: Compute FinalTaken Damage
	InOutCombatSignalTargetContext.FinalTakenDamage = ComputeFinalTakenDamage(InOutCombatSignalTargetContext);	// TODO
}

float UCCombatSignalTargetComponent::ComputeMitigatedDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const
{
	const float requestedDamage = InOutCombatSignalTargetContext.RequestedDamage;

	// Minimal safe policy (Check NaN, +Inf/-Inf)
	if (!FMath::IsFinite(requestedDamage)) return 0.f;

	float mitigatedDamage = requestedDamage;

	if (IsValid(DefenseComp_Cached) && DefenseComp_Cached->CanGuard())
	{
		InOutCombatSignalTargetContext.DefenseOutcome = EDamageDefenseOutcome::Guard;
		mitigatedDamage *= 0.5f;
	}

	// TODO: Defense / Armor / Resistance Policy

	return FMath::Max(0.f, mitigatedDamage);
}

float UCCombatSignalTargetComponent::ComputeFinalTakenDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const
{
	if (!InOutCombatSignalTargetContext.bShouldCommitDamage) return 0.f;

	const float mitigatedDamage = InOutCombatSignalTargetContext.MitigatedDamage;

	// Minimal safe policy (Check NaN, +Inf/-Inf)
	if (!FMath::IsFinite(mitigatedDamage)) return 0.f;

	const float finalTakenDamage = mitigatedDamage;

	// TODO: Critical / Guard / Headshot etc Policy

	return FMath::Max(0.f, finalTakenDamage);
}

FCombatSignalTargetResult UCCombatSignalTargetComponent::BuildResult(const FCombatSignalTargetContext& InCombatSignalTargetContext) const
{
	FCombatSignalTargetResult combatSignalTargetResult = FCombatSignalTargetResult();

	combatSignalTargetResult.bAccepted = InCombatSignalTargetContext.bAccepted;
	combatSignalTargetResult.RejectReason = InCombatSignalTargetContext.RejectReason;
	combatSignalTargetResult.DefenseOutcome = InCombatSignalTargetContext.DefenseOutcome;
	combatSignalTargetResult.bShouldCommitDamage = InCombatSignalTargetContext.bShouldCommitDamage;

	combatSignalTargetResult.DamageSpecKey = InCombatSignalTargetContext.DamageSpecKey;

	combatSignalTargetResult.RequestDamage = InCombatSignalTargetContext.RequestedDamage;
	combatSignalTargetResult.MitigatedDamage = InCombatSignalTargetContext.MitigatedDamage;
	combatSignalTargetResult.FinalTakenDamage = InCombatSignalTargetContext.FinalTakenDamage;
	combatSignalTargetResult.CommittedDamage = InCombatSignalTargetContext.CommittedDamage;

	combatSignalTargetResult.DeadState_Before = InCombatSignalTargetContext.DeadState_Before;
	combatSignalTargetResult.DeadState_After = InCombatSignalTargetContext.DeadState_After;

	return combatSignalTargetResult;
}

void UCCombatSignalTargetComponent::CommitCombatSignalTarget(FCombatSignalTargetContext& InOutCombatSignalTargetContext)
{
	if (!IsValid(HealthComp_Cached)) return;

	// Process 4: Commit Damage To Health
	InOutCombatSignalTargetContext.CommittedDamage = InOutCombatSignalTargetContext.bShouldCommitDamage ? CommitDamageToHealth(InOutCombatSignalTargetContext) : 0.f;

	// TODO: Shield / Mana / Stemina etc + Commit Order

	// Post-state Snapshot: Set BuildResult
	InOutCombatSignalTargetContext.DeadState_After = HealthComp_Cached->GetDeadState();
	InOutCombatSignalTargetContext.HealthPointAfter = HealthComp_Cached->GetCurrentHP();
}

FCombatSignalTargetPacket UCCombatSignalTargetComponent::BuildPacket(const FCombatSignalTargetPayload& InCombatSignalTargetPayload, const FCombatSignalTargetContext& InCombatSignalTargetContext, const FCombatSignalTargetResult& InCombatSignalTargetResult) const
{
	FCombatSignalTargetPacket combatSignalTargetPacket;

	combatSignalTargetPacket.Payload = InCombatSignalTargetPayload;
	combatSignalTargetPacket.Context = InCombatSignalTargetContext;
	combatSignalTargetPacket.Result = InCombatSignalTargetResult;

	return combatSignalTargetPacket;
}

void UCCombatSignalTargetComponent::DispatchAcceptedCombatResult(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (!InCombatSignalTargetPacket.Result.bAccepted) return;

	if (IsValid(ReactionOrchestratorComp_Cached))
	{
		FDamageReactionRequest damageReactionRequest;
		damageReactionRequest.IntentSource = EReactionIntentSource::CombatSignalTarget;
		damageReactionRequest.CombatSignalTargetPacket = InCombatSignalTargetPacket;

		ReactionOrchestratorComp_Cached->RequestDamageReaction(damageReactionRequest);
	}

	if (IsValid(HitFeedbackComp_Cached))
	{
		if (InCombatSignalTargetPacket.Result.bShouldCommitDamage)
		{
			HitFeedbackComp_Cached->PlayHitFeedback(InCombatSignalTargetPacket);
		}
	}

	// TODO:
	// - Debug/UI Feedback
}

void UCCombatSignalTargetComponent::DispatchRejectedCombatResult(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	// - Debug/UI rejected feedback
}

void UCCombatSignalTargetComponent::DispatchCombatResultToReceiver(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (InCombatSignalTargetPacket.Result.DefenseOutcome != EDamageDefenseOutcome::Parry) return;

	const FCombatResultPacket combatResultPacket = BuildCombatResultPacket(InCombatSignalTargetPacket);
	if (!combatResultPacket.IsValidMinimal()) return;

	AActor* receiverActor = ResolveCombatResultReceiverActor(InCombatSignalTargetPacket);
	if (!IsValid(receiverActor))
	{
		FLog::Log(FString::Printf(
			TEXT("[CombatResultDispatch] No receiver | Outcome=%s | Source=%s | DamageCauser=%s | Requester=%s"),
			*UEnum::GetValueAsString(combatResultPacket.DefenseOutcome),
			*GetNameSafe(combatResultPacket.SourceActor),
			*GetNameSafe(combatResultPacket.DamageCauser),
			*GetNameSafe(combatResultPacket.TargetActor)));
		return;
	}

	ICombatResultReceiver* receiver = Cast<ICombatResultReceiver>(receiverActor);
	if (!receiver)
	{
		FLog::Log(FString::Printf(
			TEXT("[CombatResultDispatch] Receiver has no interface | Outcome=%s | Receiver=%s"),
			*UEnum::GetValueAsString(combatResultPacket.DefenseOutcome),
			*GetNameSafe(receiverActor)));
		return;
	}

	FLog::Log(FString::Printf(
		TEXT("[CombatResultDispatch] Delivering | Outcome=%s | Receiver=%s | Requester=%s"),
		*UEnum::GetValueAsString(combatResultPacket.DefenseOutcome),
		*GetNameSafe(receiverActor),
		*GetNameSafe(combatResultPacket.TargetActor)));

	receiver->ReceiveCombatResultPacket(combatResultPacket);

	FLog::Log(FString::Printf(
		TEXT("[CombatResultDispatch] Delivered | Outcome=%s | Receiver=%s | Requester=%s"),
		*UEnum::GetValueAsString(combatResultPacket.DefenseOutcome),
		*GetNameSafe(receiverActor),
		*GetNameSafe(combatResultPacket.TargetActor)));
}

AController* UCCombatSignalTargetComponent::ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser) const
{
	// 1) Best case: engine provided instigator
	if (IsValid(EventInstigator))
		return EventInstigator;

	// 2) Fallback needs a valid causer
	if (!IsValid(DamageCauser))
		return nullptr;

	/* === Fallback Process (DamageCauser-based) === */

	// 2-1) Case 01: Explicit instigator set on the causer (ex. projectile / weaponActor / trap)
	if (AController* causerInstigator = DamageCauser->GetInstigatorController())
		return causerInstigator;

	// 2-2) Case 02: Direct hit (the causer itself is a Pawn/Character)
	if (APawn* causerPawn = Cast<APawn>(DamageCauser))
	{
		if (AController* causerController = causerPawn->GetController())
			return causerController;
	}

	// 2-3) Case 03: Proxy case (projectile / trap / weaponActor owned by another actor)
	if (AActor* causerOwner = DamageCauser->GetOwner())
	{
		// 2-3-1) Case 03-01: Owner is the carrier and holds the correct instigator (ex. projectile / weaponActor / trap)
		if (AController* ownerInstigator = causerOwner->GetInstigatorController())
			return ownerInstigator;

		// 2-3-1) Case 03-02: Fallback (Owner is Pawn/Character)
		if (APawn* ownerPawn = Cast<APawn>(causerOwner))
		{
			if (AController* ownerController = ownerPawn->GetController())
				return ownerController;
		}
	}

	/* ============================================= */

	return nullptr;
}

float UCCombatSignalTargetComponent::CommitDamageToHealth(const FCombatSignalTargetContext& InOutCombatSignalTargetContext) const
{
	if (!IsValid(HealthComp_Cached)) return 0.0;

	return HealthComp_Cached->TakeDamage(InOutCombatSignalTargetContext.FinalTakenDamage);
}

AActor* UCCombatSignalTargetComponent::ResolveCombatResultReceiverActor(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (IsValid(InCombatSignalTargetPacket.Context.SourceActor)) return InCombatSignalTargetPacket.Context.SourceActor;

	if (IsValid(InCombatSignalTargetPacket.Context.DamageCauser))
	{
		AActor* damageCauserOwner = InCombatSignalTargetPacket.Context.DamageCauser->GetOwner();
		if (IsValid(damageCauserOwner)) return damageCauserOwner;
	}

	if (IsValid(InCombatSignalTargetPacket.Context.Instigator))
	{
		AActor* instigatorPawn = InCombatSignalTargetPacket.Context.Instigator->GetPawn();
		if (IsValid(instigatorPawn)) return instigatorPawn;
	}

	return nullptr;
}

FCombatResultPacket UCCombatSignalTargetComponent::BuildCombatResultPacket(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	FCombatResultPacket combatResultPacket;

	combatResultPacket.SourceActor = InCombatSignalTargetPacket.Context.SourceActor;
	combatResultPacket.TargetActor = InCombatSignalTargetPacket.Context.TargetActor;
	combatResultPacket.Instigator = InCombatSignalTargetPacket.Context.Instigator;
	combatResultPacket.DamageCauser = InCombatSignalTargetPacket.Context.DamageCauser;
	combatResultPacket.DamageImpactInfo = InCombatSignalTargetPacket.Context.DamageImpactInfo;
	combatResultPacket.DamageSpecKey = InCombatSignalTargetPacket.Result.DamageSpecKey;
	combatResultPacket.DefenseOutcome = InCombatSignalTargetPacket.Result.DefenseOutcome;
	combatResultPacket.bDamageCommitted = InCombatSignalTargetPacket.Result.bShouldCommitDamage;
	combatResultPacket.CommittedDamage = InCombatSignalTargetPacket.Result.CommittedDamage;

	return combatResultPacket;
}

void UCCombatSignalTargetComponent::PrintCombatSignalTargetSummaryInfo(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	FLog::Log(TEXT("====== Combat Signal Target Summary ======"));
	FLog::Log(TEXT("[@ COMBAT SIGNAL TARGET]"));

	FLog::Log(FString::Printf(TEXT("SourceActor = %s | TargetActor = %s | Instigator = %s | DamageCauser = %s"),
		*GetNameSafe(InCombatSignalTargetPacket.Context.SourceActor),
		*GetNameSafe(InCombatSignalTargetPacket.Context.TargetActor),
		*GetNameSafe(InCombatSignalTargetPacket.Context.Instigator),
		*GetNameSafe(InCombatSignalTargetPacket.Context.DamageCauser)
	));

	FLog::Log(FString::Printf(TEXT("RequestDamage = %.3f | MitigatedDamage = %.3f | FinalTakenDamage = %.3f | CommittedDamage = %.3f"),
		InCombatSignalTargetPacket.Result.RequestDamage,
		InCombatSignalTargetPacket.Result.MitigatedDamage,
		InCombatSignalTargetPacket.Result.FinalTakenDamage,
		InCombatSignalTargetPacket.Result.CommittedDamage
	));
	FLog::Log(TEXT("================================="));
}

void UCCombatSignalTargetComponent::PrintCombatSignalTargetContextInfo(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	FLog::Log(TEXT("/////- Combat Signal Target Context -/////"));
	PrintObjectInfo(InCombatSignalTargetPacket);
	PrintSpecKeyInfo(InCombatSignalTargetPacket);
	PrintDamageAmountInfo(InCombatSignalTargetPacket);
	FLog::Log(TEXT("/////////////////////////////////"));
}

void UCCombatSignalTargetComponent::PrintCombatSignalTargetOutcomeInfo(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	const FCombatSignalTargetResult& result = InCombatSignalTargetPacket.Result;

	if (result.DefenseOutcome == EDamageDefenseOutcome::None && result.CommittedDamage <= KINDA_SMALL_NUMBER) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatSignalTargetOutcome] Outcome=%s | Commit=%s | Damage=%.3f | HP=%.3f->%.3f"),
		*UEnum::GetValueAsString(result.DefenseOutcome),
		result.bShouldCommitDamage ? TEXT("true") : TEXT("false"),
		result.CommittedDamage,
		InCombatSignalTargetPacket.Context.HealthPointBefore,
		InCombatSignalTargetPacket.Context.HealthPointAfter));
}

void UCCombatSignalTargetComponent::PrintObjectInfo(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	FLog::Log(TEXT("========== Object Info =========="));
	FLog::Log(TEXT("--------- Payload Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Payload] EventInstigator"), *GetNameSafe(InCombatSignalTargetPacket.Payload.EventInstigator)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Payload] DamageCauser"), *GetNameSafe(InCombatSignalTargetPacket.Payload.DamageCauser)));
	FLog::Log(TEXT("--------- Context Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] SourceActor"), *GetNameSafe(InCombatSignalTargetPacket.Context.SourceActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] TargetActor"), *GetNameSafe(InCombatSignalTargetPacket.Context.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] Instigator"), *GetNameSafe(InCombatSignalTargetPacket.Context.Instigator)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] DamageCauser"), *GetNameSafe(InCombatSignalTargetPacket.Context.DamageCauser)));
	FLog::Log(TEXT("================================="));
}

void UCCombatSignalTargetComponent::PrintSpecKeyInfo(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	FLog::Log(TEXT("========= SpecKey Info =========="));
	FLog::Log(TEXT("--------- Payload Info ----------"));
	const FDamageSpecKey& damageSpecKey = InCombatSignalTargetPacket.Payload.DamageSpecKey;
	const FString actionIndexText = (damageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(damageSpecKey.ActionIndex);

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(damageSpecKey.WeaponType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(damageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(TEXT("================================="));
}

void UCCombatSignalTargetComponent::PrintDamageAmountInfo(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	FLog::Log(TEXT("======= DamageAmount Info ======="));
	FLog::Log(TEXT("--------- Payload Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("BaseDamage"), InCombatSignalTargetPacket.Payload.DamageSpec.BaseDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("RequestDamage"), InCombatSignalTargetPacket.Payload.DamageAmount.RequestDamage));
	FLog::Log(TEXT("---------- Amount Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("FinalTakenDamage"), InCombatSignalTargetPacket.Result.FinalTakenDamage));
	FLog::Log(TEXT("================================="));
}
