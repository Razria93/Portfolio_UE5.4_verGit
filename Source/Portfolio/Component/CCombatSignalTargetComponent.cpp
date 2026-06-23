#include "Component/CCombatSignalTargetComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CHealthComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CHitFeedbackComponent.h"
#include "Component/CDefenseComponent.h"
#include "Interface/CombatResultReceiver.h"

#include "Type/CWeaponStructure.h"

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

float UCCombatSignalTargetComponent::ProcessCombatSignalTarget(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageEvent.IsOfType(FDefaultDamageEvent::ClassID))
	{
		const FDefaultDamageEvent& damageEvent = static_cast<const FDefaultDamageEvent&>(DamageEvent);
		return HandleDefaultDamageEvent(DamageAmount, damageEvent, EventInstigator, DamageCauser);
	}

	return 0.f;
}

float UCCombatSignalTargetComponent::HandleDefaultDamageEvent(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	// Receive: validate engine damage input and normalize it into target-side data.
	if (!FMath::IsFinite(DamageAmount)) return 0.f;
	if (!ValidateRequest(InDefaultDamageEvent, InDamageInstigator, InDamageCauser)) return 0.f;

	FCombatSignalTargetPayload takeDamagePayload = BuildPayload(DamageAmount, InDefaultDamageEvent, InDamageInstigator, InDamageCauser);
	FCombatSignalTargetContext takeDamageContext = BuildContext(takeDamagePayload);

	// Evaluate: validate target-side context and defensive policy before applying state changes.
	if (!ValidateContext(takeDamageContext))
	{
		// Packet / Notify: publish rejected target outcome.
		const FCombatSignalTargetResult rejectedResult = BuildResult(takeDamageContext);
		const FCombatSignalTargetPacket takeDamagePacket = BuildPacket(takeDamagePayload, takeDamageContext, rejectedResult);

		// PrintCombatSignalTargetSummaryInfo(takeDamagePacket);
		DispatchRejectedCombatResult(takeDamagePacket);
		return 0.f;
	}

	// Evaluate: snapshot target pre-state for outcome/result construction.
	takeDamageContext.DeadState_Before = HealthComp_Cached->GetDeadState();
	takeDamageContext.HealthPointBefore = HealthComp_Cached->GetCurrentHP();

	if (!CanReceiveCombatSignal(takeDamageContext))
	{
		// Packet / Notify: publish rejected target outcome.
		const FCombatSignalTargetResult rejectedResult = BuildResult(takeDamageContext);
		const FCombatSignalTargetPacket takeDamagePacket = BuildPacket(takeDamagePayload, takeDamageContext, rejectedResult);

		// PrintCombatSignalTargetSummaryInfo(takeDamagePacket);
		DispatchRejectedCombatResult(takeDamagePacket);
		return 0.f;
	}

	ComputeTargetDamage(takeDamageContext);

	if (!takeDamageContext.bAccepted)
	{
		// Packet / Notify: publish rejected target outcome.
		const FCombatSignalTargetResult rejectedResult = BuildResult(takeDamageContext);
		const FCombatSignalTargetPacket takeDamagePacket = BuildPacket(takeDamagePayload, takeDamageContext, rejectedResult);

		// PrintCombatSignalTargetSummaryInfo(takeDamagePacket);
		DispatchRejectedCombatResult(takeDamagePacket);
		return 0.f;
	}

	// Apply: commit accepted damage to target-side resource state.
	CommitCombatSignalTarget(takeDamageContext);

	// Packet: combine payload, context, and result for notify/debug consumers.
	const FCombatSignalTargetResult committedResult = BuildResult(takeDamageContext);
	const FCombatSignalTargetPacket takeDamagePacket = BuildPacket(takeDamagePayload, takeDamageContext, committedResult);

	// Notify: publish target outcome to reaction, feedback, and source-side result receivers.
	// PrintCombatSignalTargetSummaryInfo(takeDamagePacket);
	PrintCombatSignalTargetOutcomeInfo(takeDamagePacket);
	DispatchAcceptedCombatResult(takeDamagePacket);
	DispatchCombatResultToReceiver(takeDamagePacket);

	return committedResult.CommittedDamage;
}

bool UCCombatSignalTargetComponent::ValidateRequest(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	if (!IsValid(OwnerActor_Cached)) return false;
	if (!IsValid(HealthComp_Cached)) return false;
	if (!IsValid(InDamageCauser)) return false;

	if (IsValid(InDefaultDamageEvent.TargetActor) && InDefaultDamageEvent.TargetActor != OwnerActor_Cached) return false;

	if (!FMath::IsFinite(InDefaultDamageEvent.ApplyDamageSpec.BaseDamage)) return false;
	if (!FMath::IsFinite(InDefaultDamageEvent.ApplyDamageAmount.RequestDamage)) return false;

	return true;
}

FCombatSignalTargetPayload UCCombatSignalTargetComponent::BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const
{
	FCombatSignalTargetPayload takeDamagePayload = FCombatSignalTargetPayload();

	takeDamagePayload.SourceActor = InDefaultDamageEvent.SourceActor;
	takeDamagePayload.TargetActor = OwnerActor_Cached;
	takeDamagePayload.EventInstigator = InDamageInstigator;
	takeDamagePayload.DamageCauser = InDamageCauser;

	takeDamagePayload.DamageImpactInfo = InDefaultDamageEvent.DamageImpactInfo;
	takeDamagePayload.ApplyDamageSpecKey = InDefaultDamageEvent.ApplyDamageSpecKey;
	takeDamagePayload.ApplyDamageSpec = InDefaultDamageEvent.ApplyDamageSpec;
	takeDamagePayload.ApplyDamageAmount = InDefaultDamageEvent.ApplyDamageAmount;

	takeDamagePayload.RequestedDamage = DamageAmount;

	return takeDamagePayload;
}

FCombatSignalTargetContext UCCombatSignalTargetComponent::BuildContext(const FCombatSignalTargetPayload& InTakeDamagePayload) const
{
	FCombatSignalTargetContext takeDamageContext = FCombatSignalTargetContext();

	takeDamageContext.SourceActor = InTakeDamagePayload.SourceActor;
	takeDamageContext.TargetActor = InTakeDamagePayload.TargetActor;
	takeDamageContext.Instigator = ResolveInstigatorController(InTakeDamagePayload.EventInstigator, InTakeDamagePayload.DamageCauser);
	takeDamageContext.DamageCauser = InTakeDamagePayload.DamageCauser;

	takeDamageContext.DamageImpactInfo = InTakeDamagePayload.DamageImpactInfo;
	takeDamageContext.ApplyDamageSpecKey = InTakeDamagePayload.ApplyDamageSpecKey;

	takeDamageContext.RequestedDamage = InTakeDamagePayload.RequestedDamage;

	return takeDamageContext;
}

bool UCCombatSignalTargetComponent::ValidateContext(FCombatSignalTargetContext& InOutTakeDamageContext)
{
	if (!IsValid(InOutTakeDamageContext.TargetActor))
	{
		InOutTakeDamageContext.bAccepted = false;
		InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::InvalidTarget;

		return false;
	}

	if (!IsValid(InOutTakeDamageContext.DamageCauser))
	{
		InOutTakeDamageContext.bAccepted = false;
		InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::InvalidCauser;

		return false;
	}

	if (!IsValid(InOutTakeDamageContext.Instigator))
	{
		InOutTakeDamageContext.bAccepted = false;
		InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::InvalidInstigator;

		return false;
	}

	InOutTakeDamageContext.bAccepted = true;
	InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::None;

	return true;
}

bool UCCombatSignalTargetComponent::CanReceiveCombatSignal(FCombatSignalTargetContext& InOutTakeDamageContext)
{
	// Gate 1: already dead
	if (InOutTakeDamageContext.DeadState_Before != EDeadState::Alive)
	{
		InOutTakeDamageContext.bAccepted = false;
		InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::AlreadyDead;

		return false;
	}

	// Gate 2: Parry window intercepts incoming damage before damage commit.
	if (IsValid(DefenseComp_Cached) && DefenseComp_Cached->CanParry())
	{
		InOutTakeDamageContext.bAccepted = true;
		InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::None;
		InOutTakeDamageContext.DefenseOutcome = EDamageDefenseOutcome::Parry;
		InOutTakeDamageContext.bShouldCommitDamage = false;

		return true;
	}

	// TODO:
	// Gate 3: invulnerable / iframe / god-mode state
	// Gate 4: defensive friendly-fire check on receiver side
	// Gate 5: receiver-side damage cooldown / hit immunity window
	// Gate 6: defensive self-damage policy

	InOutTakeDamageContext.bAccepted = true;
	InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::None;
	InOutTakeDamageContext.DefenseOutcome = EDamageDefenseOutcome::None;
	InOutTakeDamageContext.bShouldCommitDamage = true;

	return true;
}

void UCCombatSignalTargetComponent::ComputeTargetDamage(FCombatSignalTargetContext& InOutTakeDamageContext) const
{
	// Process 1: Compute Mitigation Damage
	InOutTakeDamageContext.MitigatedDamage = ComputeMitigatedDamage(InOutTakeDamageContext);	// TODO

	// Gate 1: Zero damage
	if (InOutTakeDamageContext.bShouldCommitDamage && InOutTakeDamageContext.MitigatedDamage <= KINDA_SMALL_NUMBER)
	{
		InOutTakeDamageContext.bAccepted = false;
		InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::ZeroDamage;

		return;
	}

	InOutTakeDamageContext.bAccepted = true;
	InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::None;

	// Process 2: Compute FinalTaken Damage
	InOutTakeDamageContext.FinalTakenDamage = ComputeFinalTakenDamage(InOutTakeDamageContext);	// TODO
}

float UCCombatSignalTargetComponent::ComputeMitigatedDamage(FCombatSignalTargetContext& InOutTakeDamageContext) const
{
	const float requestedDamage = InOutTakeDamageContext.RequestedDamage;

	// Minimal safe policy (Check NaN, +Inf/-Inf)
	if (!FMath::IsFinite(requestedDamage)) return 0.f;

	float mitigatedDamage = requestedDamage;

	if (IsValid(DefenseComp_Cached) && DefenseComp_Cached->CanGuard())
	{
		InOutTakeDamageContext.DefenseOutcome = EDamageDefenseOutcome::Guard;
		mitigatedDamage *= 0.5f;
	}

	// TODO: Defense / Armor / Resistance Policy

	return FMath::Max(0.f, mitigatedDamage);
}

float UCCombatSignalTargetComponent::ComputeFinalTakenDamage(FCombatSignalTargetContext& InOutTakeDamageContext) const
{
	if (!InOutTakeDamageContext.bShouldCommitDamage) return 0.f;

	const float mitigatedDamage = InOutTakeDamageContext.MitigatedDamage;

	// Minimal safe policy (Check NaN, +Inf/-Inf)
	if (!FMath::IsFinite(mitigatedDamage)) return 0.f;

	const float finalTakenDamage = mitigatedDamage;

	// TODO: Critical / Guard / Headshot etc Policy

	return FMath::Max(0.f, finalTakenDamage);
}

FCombatSignalTargetResult UCCombatSignalTargetComponent::BuildResult(const FCombatSignalTargetContext& InTakeDamageContext) const
{
	FCombatSignalTargetResult takeDamageResult = FCombatSignalTargetResult();

	takeDamageResult.bAccepted = InTakeDamageContext.bAccepted;
	takeDamageResult.RejectReason = InTakeDamageContext.RejectReason;
	takeDamageResult.DefenseOutcome = InTakeDamageContext.DefenseOutcome;
	takeDamageResult.bShouldCommitDamage = InTakeDamageContext.bShouldCommitDamage;

	takeDamageResult.ApplyDamageSpecKey = InTakeDamageContext.ApplyDamageSpecKey;

	takeDamageResult.RequestDamage = InTakeDamageContext.RequestedDamage;
	takeDamageResult.MitigatedDamage = InTakeDamageContext.MitigatedDamage;
	takeDamageResult.FinalTakenDamage = InTakeDamageContext.FinalTakenDamage;
	takeDamageResult.CommittedDamage = InTakeDamageContext.CommittedDamage;

	takeDamageResult.DeadState_Before = InTakeDamageContext.DeadState_Before;
	takeDamageResult.DeadState_After = InTakeDamageContext.DeadState_After;

	return takeDamageResult;
}

void UCCombatSignalTargetComponent::CommitCombatSignalTarget(FCombatSignalTargetContext& InOutTakeDamageContext)
{
	if (!IsValid(HealthComp_Cached)) return;

	// Process 4: Apply Damage To Health
	InOutTakeDamageContext.CommittedDamage = InOutTakeDamageContext.bShouldCommitDamage ? CommitDamageToHealth(InOutTakeDamageContext) : 0.f;

	// TODO: Shield / Mana / Stemina etc + Commit Order

	// Post-state Snapshot: Set BuildResult
	InOutTakeDamageContext.DeadState_After = HealthComp_Cached->GetDeadState();
	InOutTakeDamageContext.HealthPointAfter = HealthComp_Cached->GetCurrentHP();
}

FCombatSignalTargetPacket UCCombatSignalTargetComponent::BuildPacket(const FCombatSignalTargetPayload& InTakeDamagePayload, const FCombatSignalTargetContext& InTakeDamageContext, const FCombatSignalTargetResult& InTakeDamageResult) const
{
	FCombatSignalTargetPacket takeDamagePacket;

	takeDamagePacket.Payload = InTakeDamagePayload;
	takeDamagePacket.Context = InTakeDamageContext;
	takeDamagePacket.Result = InTakeDamageResult;

	return takeDamagePacket;
}

void UCCombatSignalTargetComponent::DispatchAcceptedCombatResult(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	if (!InTakeDamagePacket.Result.bAccepted) return;

	if (IsValid(ReactionOrchestratorComp_Cached))
	{
		FDamageReactionRequest damageReactionRequest;
		damageReactionRequest.IntentSource = EReactionIntentSource::TakeDamage;
		damageReactionRequest.TakeDamagePacket = InTakeDamagePacket;

		ReactionOrchestratorComp_Cached->RequestDamageReaction(damageReactionRequest);
	}

	if (IsValid(HitFeedbackComp_Cached))
	{
		if (InTakeDamagePacket.Result.bShouldCommitDamage)
		{
			HitFeedbackComp_Cached->PlayHitFeedback(InTakeDamagePacket);
		}
	}

	// TODO:
	// - Debug/UI Feedback
}

void UCCombatSignalTargetComponent::DispatchRejectedCombatResult(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	// - Debug/UI rejected feedback
}

void UCCombatSignalTargetComponent::DispatchCombatResultToReceiver(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	if (InTakeDamagePacket.Result.DefenseOutcome != EDamageDefenseOutcome::Parry) return;

	const FCombatResultPacket combatResultPacket = BuildCombatResultPacket(InTakeDamagePacket);
	if (!combatResultPacket.IsValidMinimal()) return;

	AActor* receiverActor = ResolveCombatResultReceiverActor(InTakeDamagePacket);
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

float UCCombatSignalTargetComponent::CommitDamageToHealth(const FCombatSignalTargetContext& InOutTakeDamageContext) const
{
	if (!IsValid(HealthComp_Cached)) return 0.0;

	return HealthComp_Cached->TakeDamage(InOutTakeDamageContext.FinalTakenDamage);
}

AActor* UCCombatSignalTargetComponent::ResolveCombatResultReceiverActor(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	if (IsValid(InTakeDamagePacket.Context.SourceActor)) return InTakeDamagePacket.Context.SourceActor;

	if (IsValid(InTakeDamagePacket.Context.DamageCauser))
	{
		AActor* damageCauserOwner = InTakeDamagePacket.Context.DamageCauser->GetOwner();
		if (IsValid(damageCauserOwner)) return damageCauserOwner;
	}

	if (IsValid(InTakeDamagePacket.Context.Instigator))
	{
		AActor* instigatorPawn = InTakeDamagePacket.Context.Instigator->GetPawn();
		if (IsValid(instigatorPawn)) return instigatorPawn;
	}

	return nullptr;
}

FCombatResultPacket UCCombatSignalTargetComponent::BuildCombatResultPacket(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	FCombatResultPacket combatResultPacket;

	combatResultPacket.SourceActor = InTakeDamagePacket.Context.SourceActor;
	combatResultPacket.TargetActor = InTakeDamagePacket.Context.TargetActor;
	combatResultPacket.Instigator = InTakeDamagePacket.Context.Instigator;
	combatResultPacket.DamageCauser = InTakeDamagePacket.Context.DamageCauser;
	combatResultPacket.DamageImpactInfo = InTakeDamagePacket.Context.DamageImpactInfo;
	combatResultPacket.ApplyDamageSpecKey = InTakeDamagePacket.Result.ApplyDamageSpecKey;
	combatResultPacket.DefenseOutcome = InTakeDamagePacket.Result.DefenseOutcome;
	combatResultPacket.bDamageCommitted = InTakeDamagePacket.Result.bShouldCommitDamage;
	combatResultPacket.CommittedDamage = InTakeDamagePacket.Result.CommittedDamage;

	return combatResultPacket;
}

void UCCombatSignalTargetComponent::PrintCombatSignalTargetSummaryInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	FLog::Log(TEXT("====== Take Damage Summary ======"));
	FLog::Log(TEXT("[@ TAKE DAMAGE]"));

	FLog::Log(FString::Printf(TEXT("SourceActor = %s | TargetActor = %s | Instigator = %s | DamageCauser = %s"),
		*GetNameSafe(InTakeDamagePacket.Context.SourceActor),
		*GetNameSafe(InTakeDamagePacket.Context.TargetActor),
		*GetNameSafe(InTakeDamagePacket.Context.Instigator),
		*GetNameSafe(InTakeDamagePacket.Context.DamageCauser)
	));

	FLog::Log(FString::Printf(TEXT("RequestDamage = %.3f | MitigatedDamage = %.3f | FinalTakenDamage = %.3f | CommittedDamage = %.3f"),
		InTakeDamagePacket.Result.RequestDamage,
		InTakeDamagePacket.Result.MitigatedDamage,
		InTakeDamagePacket.Result.FinalTakenDamage,
		InTakeDamagePacket.Result.CommittedDamage
	));
	FLog::Log(TEXT("================================="));
}

void UCCombatSignalTargetComponent::PrintCombatSignalTargetContextInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	FLog::Log(TEXT("/////- Take Damage Context -/////"));
	PrintObjectInfo(InTakeDamagePacket);
	PrintSpecKeyInfo(InTakeDamagePacket);
	PrintDamageAmountInfo(InTakeDamagePacket);
	FLog::Log(TEXT("/////////////////////////////////"));
}

void UCCombatSignalTargetComponent::PrintCombatSignalTargetOutcomeInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	const FCombatSignalTargetResult& result = InTakeDamagePacket.Result;

	if (result.DefenseOutcome == EDamageDefenseOutcome::None && result.CommittedDamage <= KINDA_SMALL_NUMBER) return;

	FLog::Log(FString::Printf(
		TEXT("[TakeDamageOutcome] Outcome=%s | Commit=%s | Damage=%.3f | HP=%.3f->%.3f"),
		*UEnum::GetValueAsString(result.DefenseOutcome),
		result.bShouldCommitDamage ? TEXT("true") : TEXT("false"),
		result.CommittedDamage,
		InTakeDamagePacket.Context.HealthPointBefore,
		InTakeDamagePacket.Context.HealthPointAfter));
}

void UCCombatSignalTargetComponent::PrintObjectInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	FLog::Log(TEXT("========== Object Info =========="));
	FLog::Log(TEXT("--------- Payload Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Payload] EventInstigator"), *GetNameSafe(InTakeDamagePacket.Payload.EventInstigator)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Payload] DamageCauser"), *GetNameSafe(InTakeDamagePacket.Payload.DamageCauser)));
	FLog::Log(TEXT("--------- Context Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] SourceActor"), *GetNameSafe(InTakeDamagePacket.Context.SourceActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] TargetActor"), *GetNameSafe(InTakeDamagePacket.Context.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] Instigator"), *GetNameSafe(InTakeDamagePacket.Context.Instigator)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] DamageCauser"), *GetNameSafe(InTakeDamagePacket.Context.DamageCauser)));
	FLog::Log(TEXT("================================="));
}

void UCCombatSignalTargetComponent::PrintSpecKeyInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	FLog::Log(TEXT("========= SpecKey Info =========="));
	FLog::Log(TEXT("--------- Payload Info ----------"));
	const FApplyDamageSpecKey& applyDamageSpecKey = InTakeDamagePacket.Payload.ApplyDamageSpecKey;
	const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(applyDamageSpecKey.ActionIndex);

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(applyDamageSpecKey.WeaponType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(TEXT("================================="));
}

void UCCombatSignalTargetComponent::PrintDamageAmountInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	FLog::Log(TEXT("======= DamageAmount Info ======="));
	FLog::Log(TEXT("--------- Payload Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("BaseDamage"), InTakeDamagePacket.Payload.ApplyDamageSpec.BaseDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("RequestDamage"), InTakeDamagePacket.Payload.ApplyDamageAmount.RequestDamage));
	FLog::Log(TEXT("---------- Amount Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("FinalTakenDamage"), InTakeDamagePacket.Result.FinalTakenDamage));
	FLog::Log(TEXT("================================="));
}
