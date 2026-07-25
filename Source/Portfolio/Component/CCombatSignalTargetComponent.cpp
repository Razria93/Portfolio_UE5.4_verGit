#include "Component/CCombatSignalTargetComponent.h"

#include "ProjectGlobal.h"

#include "Component/CHealthComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CHitFeedbackComponent.h"
#include "Component/CDefenseComponent.h"
#include "Core/Debug/FCombatSignalDebug.h"
#include "Interface/CombatResultReceiver.h"
#include "Type/CReactionTypes.h"
#include "Type/CHealthTypes.h"
#include "Type/CCombatSignalTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CCombatSignalTargetTypes.h"

#include "GameFramework/Character.h"

namespace
{
	const FName CombatCueBlinkTag(TEXT("Combat.Cue.Blink"));
	const FName CombatCueRepulseTag(TEXT("Combat.Cue.Repulse"));
}

UCCombatSignalTargetComponent::UCCombatSignalTargetComponent()
{
}

// Component Reference

void UCCombatSignalTargetComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	HealthComp_Injected = InReferences.HealthComponent;
	DefenseComp_Injected = InReferences.DefenseComponent;
	ReactionOrchestratorComp_Injected = InReferences.ReactionOrchestratorComponent;
	HitFeedbackComp_Injected = InReferences.HitFeedbackComponent;

	ValidateRequiredComponentReferences();
}

bool UCCombatSignalTargetComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ HealthComp_Injected, TEXT("UCHealthComponent") },
		{ ReactionOrchestratorComp_Injected, TEXT("UCReactionOrchestratorComponent") },
		{ HitFeedbackComp_Injected, TEXT("UCHitFeedbackComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Entry

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

	FCombatSignalDebug::RecordTargetDamageRequestRejectedForAudit(DamageAmount, DamageEvent, EventInstigator, DamageCauser, TEXT("UnsupportedDamageEvent"));
	return 0.f;
}

bool UCCombatSignalTargetComponent::ProcessCombatSignalTarget(const FCombatSignal& InCombatSignal)
{
	if (InCombatSignal.Header.SignalType == ECombatSignalType::TimingCue)
	{
		return HandleTimingCueSignal(InCombatSignal);
	}

	FCombatSignalDebug::RecordTimingCueRejectedForAudit(InCombatSignal, TEXT("UnsupportedSignalType"));
	return false;
}

float UCCombatSignalTargetComponent::HandleDefaultDamageEvent(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	// Receive: validate engine damage input and normalize it into target-side data.
	if (!FMath::IsFinite(DamageAmount))
	{
		FCombatSignalDebug::RecordTargetDamageRequestRejectedForAudit(DamageAmount, InDefaultDamageEvent, InDamageInstigator, InDamageCauser, TEXT("NonFiniteDamageAmount"));
		return 0.f;
	}
	if (!ValidateRequest(InDefaultDamageEvent, InDamageInstigator, InDamageCauser)) return 0.f;

	FCombatSignalTargetPayload combatSignalTargetPayload = BuildPayload(DamageAmount, InDefaultDamageEvent, InDamageInstigator, InDamageCauser);
	FCombatSignalTargetContext combatSignalTargetContext = BuildContext(combatSignalTargetPayload);

	// Evaluate: validate target-side context and defensive policy before applying state changes.
	if (!ValidateContext(combatSignalTargetContext))
	{
		// Packet / Notify: publish rejected target outcome.
		const FCombatSignalTargetResult rejectedResult = BuildResult(combatSignalTargetContext);
		const FCombatSignalTargetPacket combatSignalTargetPacket = BuildPacket(combatSignalTargetPayload, combatSignalTargetContext, rejectedResult);

		DispatchRejectedCombatResult(combatSignalTargetPacket);
		return 0.f;
	}

	// Evaluate: snapshot target pre-state for outcome/result construction.
	combatSignalTargetContext.DeadState_Before = HealthComp_Injected->GetDeadState();
	combatSignalTargetContext.HealthPointBefore = HealthComp_Injected->GetCurrentHP();

	if (!CanReceiveCombatSignal(combatSignalTargetContext))
	{
		// Packet / Notify: publish rejected target outcome.
		const FCombatSignalTargetResult rejectedResult = BuildResult(combatSignalTargetContext);
		const FCombatSignalTargetPacket combatSignalTargetPacket = BuildPacket(combatSignalTargetPayload, combatSignalTargetContext, rejectedResult);

		DispatchRejectedCombatResult(combatSignalTargetPacket);
		return 0.f;
	}

	ComputeTargetDamage(combatSignalTargetContext);

	if (!combatSignalTargetContext.bAccepted)
	{
		// Packet / Notify: publish rejected target outcome.
		const FCombatSignalTargetResult rejectedResult = BuildResult(combatSignalTargetContext);
		const FCombatSignalTargetPacket combatSignalTargetPacket = BuildPacket(combatSignalTargetPayload, combatSignalTargetContext, rejectedResult);

		DispatchRejectedCombatResult(combatSignalTargetPacket);
		return 0.f;
	}

	// Apply: commit accepted damage to target-side resource state.
	CommitCombatSignalTarget(combatSignalTargetContext);

	// Packet: combine payload, context, and result for notify/debug consumers.
	const FCombatSignalTargetResult committedResult = BuildResult(combatSignalTargetContext);
	const FCombatSignalTargetPacket combatSignalTargetPacket = BuildPacket(combatSignalTargetPayload, combatSignalTargetContext, committedResult);

	// Notify: publish target outcome to reaction, feedback, and source-side result receivers.
	DispatchAcceptedCombatResult(combatSignalTargetPacket);
	DispatchCombatResultToReceiver(combatSignalTargetPacket);
	FCombatSignalDebug::RecordTargetAcceptedForAudit(combatSignalTargetPacket);
	FCombatSignalDebug::PrintTargetPacketDebug(combatSignalTargetPacket);

	return committedResult.CommittedDamage;
}

bool UCCombatSignalTargetComponent::HandleTimingCueSignal(const FCombatSignal& InCombatSignal)
{
	if (!ValidateSignalRequest(InCombatSignal))
	{
		FCombatSignalDebug::RecordTimingCueRejectedForAudit(InCombatSignal, TEXT("InvalidTimingCueSignal"));
		return false;
	}

	if (InCombatSignal.CueTag == CombatCueBlinkTag)
	{
		FCombatSignalDebug::RecordTimingCueAcceptedForAudit(InCombatSignal);
		return true;
	}

	if (InCombatSignal.CueTag == CombatCueRepulseTag)
	{
		FCombatSignalDebug::RecordTimingCueAcceptedForAudit(InCombatSignal);
		return true;
	}

	// V1 hook only. Blink / Repulse evaluation and effects are added in separate branches.
	FCombatSignalDebug::RecordTimingCueRejectedForAudit(InCombatSignal, TEXT("UnknownCueTag"));
	return false;
}

// Receive

bool UCCombatSignalTargetComponent::ValidateRequest(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatSignalDebug::RecordTargetDamageRequestRejectedForAudit(InDefaultDamageEvent.DamageRequestAmount.RequestDamage, InDefaultDamageEvent, InDamageInstigator, InDamageCauser, TEXT("InvalidOwner"));
		return false;
	}
	if (!IsValid(HealthComp_Injected))
	{
		FCombatSignalDebug::RecordTargetDamageRequestRejectedForAudit(InDefaultDamageEvent.DamageRequestAmount.RequestDamage, InDefaultDamageEvent, InDamageInstigator, InDamageCauser, TEXT("InvalidHealthComponent"));
		return false;
	}
	if (!IsValid(InDamageCauser))
	{
		FCombatSignalDebug::RecordTargetDamageRequestRejectedForAudit(InDefaultDamageEvent.DamageRequestAmount.RequestDamage, InDefaultDamageEvent, InDamageInstigator, InDamageCauser, TEXT("InvalidDamageCauser"));
		return false;
	}

	if (IsValid(InDefaultDamageEvent.TargetActor) && InDefaultDamageEvent.TargetActor != OwnerCharacter_Injected)
	{
		FCombatSignalDebug::RecordTargetDamageRequestRejectedForAudit(InDefaultDamageEvent.DamageRequestAmount.RequestDamage, InDefaultDamageEvent, InDamageInstigator, InDamageCauser, TEXT("TargetMismatch"));
		return false;
	}

	if (!FMath::IsFinite(InDefaultDamageEvent.DamageSpec.BaseDamage))
	{
		FCombatSignalDebug::RecordTargetDamageRequestRejectedForAudit(InDefaultDamageEvent.DamageRequestAmount.RequestDamage, InDefaultDamageEvent, InDamageInstigator, InDamageCauser, TEXT("NonFiniteBaseDamage"));
		return false;
	}
	if (!FMath::IsFinite(InDefaultDamageEvent.DamageRequestAmount.RequestDamage))
	{
		FCombatSignalDebug::RecordTargetDamageRequestRejectedForAudit(InDefaultDamageEvent.DamageRequestAmount.RequestDamage, InDefaultDamageEvent, InDamageInstigator, InDamageCauser, TEXT("NonFiniteRequestDamage"));
		return false;
	}

	return true;
}

bool UCCombatSignalTargetComponent::ValidateSignalRequest(const FCombatSignal& InCombatSignal) const
{
	if (!InCombatSignal.IsValidMinimal()) return false;
	if (InCombatSignal.Header.SignalType != ECombatSignalType::TimingCue) return false;
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(InCombatSignal.Header.SourceActor)) return false;
	if (!IsValid(InCombatSignal.Header.TargetActor)) return false;
	if (InCombatSignal.Header.TargetActor != OwnerCharacter_Injected) return false;
	if (InCombatSignal.CueTag.IsNone()) return false;

	return true;
}

FCombatSignalTargetPayload UCCombatSignalTargetComponent::BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const
{
	FCombatSignalTargetPayload combatSignalTargetPayload = FCombatSignalTargetPayload();

	combatSignalTargetPayload.SourceActor = InDefaultDamageEvent.SourceActor;
	combatSignalTargetPayload.TargetActor = OwnerCharacter_Injected;
	combatSignalTargetPayload.EventInstigator = InDamageInstigator;
	combatSignalTargetPayload.DamageCauser = InDamageCauser;

	combatSignalTargetPayload.HitImpactContext = InDefaultDamageEvent.HitImpactContext;
	combatSignalTargetPayload.DamageSpecKey = InDefaultDamageEvent.DamageSpecKey;
	combatSignalTargetPayload.DamageSpec = InDefaultDamageEvent.DamageSpec;
	combatSignalTargetPayload.DamageRequestAmount = InDefaultDamageEvent.DamageRequestAmount;

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

	combatSignalTargetContext.HitImpactContext = InCombatSignalTargetPayload.HitImpactContext;
	combatSignalTargetContext.DamageSpecKey = InCombatSignalTargetPayload.DamageSpecKey;

	combatSignalTargetContext.RequestedDamage = InCombatSignalTargetPayload.RequestedDamage;

	return combatSignalTargetContext;
}

// Evaluate

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

bool UCCombatSignalTargetComponent::CanReceiveCombatSignal(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const
{
	// Gate 1: already dead
	if (InOutCombatSignalTargetContext.DeadState_Before != EDeadState::Alive)
	{
		InOutCombatSignalTargetContext.bAccepted = false;
		InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::AlreadyDead;

		return false;
	}

	// Gate 2: Parry window intercepts incoming damage before damage commit.
	if (IsValid(DefenseComp_Injected) && DefenseComp_Injected->CanParry())
	{
		InOutCombatSignalTargetContext.bAccepted = true;
		InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::None;
		InOutCombatSignalTargetContext.DefenseOutcome = EDamageDefenseOutcome::Parry;
		InOutCombatSignalTargetContext.bShouldCommitDamage = false;

		return true;
	}

	// TODO(CombatPolicy): Add target-side defensive gates.
	// - Invulnerable / iframe / god-mode state
	// - Defensive friendly-fire check on receiver side
	// - Receiver-side damage cooldown / hit immunity window
	// - Defensive self-damage policy

	InOutCombatSignalTargetContext.bAccepted = true;
	InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::None;
	InOutCombatSignalTargetContext.DefenseOutcome = EDamageDefenseOutcome::None;
	InOutCombatSignalTargetContext.bShouldCommitDamage = true;

	return true;
}

void UCCombatSignalTargetComponent::ComputeTargetDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const
{
	InOutCombatSignalTargetContext.MitigatedDamage = ComputeMitigatedDamage(InOutCombatSignalTargetContext);

	if (InOutCombatSignalTargetContext.bShouldCommitDamage && InOutCombatSignalTargetContext.MitigatedDamage <= KINDA_SMALL_NUMBER)
	{
		InOutCombatSignalTargetContext.bAccepted = false;
		InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::ZeroDamage;

		return;
	}

	InOutCombatSignalTargetContext.bAccepted = true;
	InOutCombatSignalTargetContext.RejectReason = ECombatSignalTargetRejectReason::None;

	InOutCombatSignalTargetContext.FinalTakenDamage = ComputeFinalTakenDamage(InOutCombatSignalTargetContext);
}

float UCCombatSignalTargetComponent::ComputeMitigatedDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const
{
	const float requestedDamage = InOutCombatSignalTargetContext.RequestedDamage;

	// Non-finite damage is clamped out before target-side policy evaluation.
	if (!FMath::IsFinite(requestedDamage)) return 0.f;

	float mitigatedDamage = requestedDamage;

	if (IsValid(DefenseComp_Injected) && DefenseComp_Injected->CanGuard())
	{
		InOutCombatSignalTargetContext.DefenseOutcome = EDamageDefenseOutcome::Guard;
		mitigatedDamage *= 0.5f;
	}

	// TODO(CombatPolicy): Add defense / armor / resistance mitigation policy.

	return FMath::Max(0.f, mitigatedDamage);
}

float UCCombatSignalTargetComponent::ComputeFinalTakenDamage(FCombatSignalTargetContext& InOutCombatSignalTargetContext) const
{
	if (!InOutCombatSignalTargetContext.bShouldCommitDamage) return 0.f;

	const float mitigatedDamage = InOutCombatSignalTargetContext.MitigatedDamage;

	// Non-finite damage is clamped out before final damage policy evaluation.
	if (!FMath::IsFinite(mitigatedDamage)) return 0.f;

	const float finalTakenDamage = mitigatedDamage;

	// TODO(CombatPolicy): Add critical / guard / headshot final damage policy.

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

// Apply

void UCCombatSignalTargetComponent::CommitCombatSignalTarget(FCombatSignalTargetContext& InOutCombatSignalTargetContext)
{
	if (!IsValid(HealthComp_Injected)) return;

	InOutCombatSignalTargetContext.CommittedDamage = InOutCombatSignalTargetContext.bShouldCommitDamage ? CommitDamageToHealth(InOutCombatSignalTargetContext) : 0.f;

	// TODO(CombatPolicy): Add shield / mana / stamina resource commit order.

	InOutCombatSignalTargetContext.DeadState_After = HealthComp_Injected->GetDeadState();
	InOutCombatSignalTargetContext.HealthPointAfter = HealthComp_Injected->GetCurrentHP();
}

// Packet

FCombatSignalTargetPacket UCCombatSignalTargetComponent::BuildPacket(const FCombatSignalTargetPayload& InCombatSignalTargetPayload, const FCombatSignalTargetContext& InCombatSignalTargetContext, const FCombatSignalTargetResult& InCombatSignalTargetResult) const
{
	FCombatSignalTargetPacket combatSignalTargetPacket;

	combatSignalTargetPacket.Payload = InCombatSignalTargetPayload;
	combatSignalTargetPacket.Context = InCombatSignalTargetContext;
	combatSignalTargetPacket.Result = InCombatSignalTargetResult;

	return combatSignalTargetPacket;
}

// Notify

void UCCombatSignalTargetComponent::DispatchAcceptedCombatResult(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (!InCombatSignalTargetPacket.Result.bAccepted) return;

	if (IsValid(ReactionOrchestratorComp_Injected))
	{
		FDamageReactionRequest damageReactionRequest;
		damageReactionRequest.IntentSource = EReactionIntentSource::CombatSignalTarget;
		damageReactionRequest.CombatSignalTargetPacket = InCombatSignalTargetPacket;

		ReactionOrchestratorComp_Injected->RequestDamageReaction(damageReactionRequest);
	}

	if (IsValid(HitFeedbackComp_Injected))
	{
		if (InCombatSignalTargetPacket.Result.bShouldCommitDamage)
		{
			HitFeedbackComp_Injected->PlayHitFeedback(InCombatSignalTargetPacket);
		}
	}
}

void UCCombatSignalTargetComponent::DispatchRejectedCombatResult(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	FCombatSignalDebug::RecordTargetRejectedForAudit(InCombatSignalTargetPacket);
	FCombatSignalDebug::PrintTargetPacketDebug(InCombatSignalTargetPacket);
}

void UCCombatSignalTargetComponent::DispatchCombatResultToReceiver(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (InCombatSignalTargetPacket.Result.DefenseOutcome != EDamageDefenseOutcome::Parry) return;

	const FCombatResultPacket combatResultPacket = BuildCombatResultPacket(InCombatSignalTargetPacket);
	if (!combatResultPacket.IsValidMinimal()) return;

	AActor* receiverActor = ResolveCombatResultReceiverActor(InCombatSignalTargetPacket);
	if (!IsValid(receiverActor))
	{
		FCombatSignalDebug::RecordCombatResultDispatchForAudit(InCombatSignalTargetPacket, combatResultPacket, nullptr, TEXT("NoReceiver"));
		return;
	}

	ICombatResultReceiver* receiver = Cast<ICombatResultReceiver>(receiverActor);
	if (!receiver)
	{
		FCombatSignalDebug::RecordCombatResultDispatchForAudit(InCombatSignalTargetPacket, combatResultPacket, receiverActor, TEXT("MissingReceiverInterface"));
		return;
	}

	FCombatSignalDebug::RecordCombatResultDispatchForAudit(InCombatSignalTargetPacket, combatResultPacket, receiverActor, TEXT("Delivering"));

	receiver->ReceiveCombatResultPacket(combatResultPacket);

	FCombatSignalDebug::RecordCombatResultDispatchForAudit(InCombatSignalTargetPacket, combatResultPacket, receiverActor, TEXT("Delivered"));
}

// Helper

AController* UCCombatSignalTargetComponent::ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser) const
{
	// 1) Best case: engine provided instigator
	if (IsValid(EventInstigator))
		return EventInstigator;

	// 2) Fallback needs a valid causer
	if (!IsValid(DamageCauser))
		return nullptr;

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

	return nullptr;
}

float UCCombatSignalTargetComponent::CommitDamageToHealth(const FCombatSignalTargetContext& InOutCombatSignalTargetContext) const
{
	if (!IsValid(HealthComp_Injected)) return 0.0;

	return HealthComp_Injected->TakeDamage(InOutCombatSignalTargetContext.FinalTakenDamage);
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
	combatResultPacket.HitImpactContext = InCombatSignalTargetPacket.Context.HitImpactContext;
	combatResultPacket.DamageSpecKey = InCombatSignalTargetPacket.Result.DamageSpecKey;
	combatResultPacket.DefenseOutcome = InCombatSignalTargetPacket.Result.DefenseOutcome;
	combatResultPacket.bDamageCommitted = InCombatSignalTargetPacket.Result.bShouldCommitDamage;
	combatResultPacket.CommittedDamage = InCombatSignalTargetPacket.Result.CommittedDamage;

	return combatResultPacket;
}
