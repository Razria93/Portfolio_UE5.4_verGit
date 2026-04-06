#include "Component/CTakeDamageComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"

#include "Type/CWeaponStructure.h"

UCTakeDamageComponent::UCTakeDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCTakeDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor_Cached = Cast<AActor>(GetOwner());
	check(OwnerActor_Cached);

	HealthComp_Cached = Cast<UCHealthComponent>(OwnerActor_Cached->GetComponentByClass(UCHealthComponent::StaticClass()));
	check(HealthComp_Cached);

	ReactionComp_Cached = Cast<UCReactionComponent>(OwnerActor_Cached->GetComponentByClass(UCReactionComponent::StaticClass()));
	check(ReactionComp_Cached);
}

void UCTakeDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float UCTakeDamageComponent::RequestTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return ProcessTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

float UCTakeDamageComponent::ProcessTakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageEvent.IsOfType(FDefaultDamageEvent::ClassID))
	{
		const FDefaultDamageEvent& damageEvent = static_cast<const FDefaultDamageEvent&>(DamageEvent);
		return HandleDefaultDamageEvent(DamageAmount, damageEvent, EventInstigator, DamageCauser);
	}

	return 0.f;
}

float UCTakeDamageComponent::HandleDefaultDamageEvent(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	// [Function Object]
	// 'FDefaultDamageEvent + @(FTakeDamagePayload) -> FTakeDamageContext' and TakeDamage it-self

	// 1) Validate Request
	if (!FMath::IsFinite(DamageAmount)) return 0.f;
	if (!ValidateRequest(InDefaultDamageEvent, InDamageInstigator, InDamageCauser)) return 0.f;

	// 2) Build Payload / Context
	FTakeDamagePayload takeDamagePayload = BuildPayload(DamageAmount, InDefaultDamageEvent, InDamageInstigator, InDamageCauser);
	FTakeDamageContext takeDamageContext = BuildContext(takeDamagePayload);

	// 3) Validate Context
	if (!ValidateContext(takeDamageContext))
	{
		const FTakeDamageResult rejectedResult = BuildResult(takeDamageContext);
		PrintTakeDamageSummaryInfo(takeDamagePayload, takeDamageContext, rejectedResult);
		DispatchTakeDamageRejected(takeDamagePayload, takeDamageContext, rejectedResult);
		return 0.f;
	}

	// 4) Take Pre-state Snapshot
	takeDamageContext.DeadState_Before = HealthComp_Cached->GetDeadState();
	takeDamageContext.HealthPointBefore = HealthComp_Cached->GetCurrentHP();

	// 5) Validate Policy
	if (!CanTakeDamage(takeDamageContext))
	{
		const FTakeDamageResult rejectedResult = BuildResult(takeDamageContext);
		PrintTakeDamageSummaryInfo(takeDamagePayload, takeDamageContext, rejectedResult);
		DispatchTakeDamageRejected(takeDamagePayload, takeDamageContext, rejectedResult);
		return 0.f;
	}

	// 6) Compute TakeDamage
	ComputeTakeDamage(takeDamageContext);

	if (!takeDamageContext.bAccepted)
	{
		const FTakeDamageResult rejectedResult = BuildResult(takeDamageContext);
		PrintTakeDamageSummaryInfo(takeDamagePayload, takeDamageContext, rejectedResult);
		DispatchTakeDamageRejected(takeDamagePayload, takeDamageContext, rejectedResult);
		return 0.f;
	}

	// 7) Commit
	CommitTakeDamage(takeDamageContext);

	const FTakeDamageResult committedResult = BuildResult(takeDamageContext);
	PrintTakeDamageSummaryInfo(takeDamagePayload, takeDamageContext, committedResult);
	DispatchTakeDamageCommitted(takeDamagePayload, takeDamageContext, committedResult);

	return committedResult.CommittedDamage;
}

bool UCTakeDamageComponent::ValidateRequest(const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser)
{
	if (!IsValid(OwnerActor_Cached)) return false;
	if (!IsValid(HealthComp_Cached)) return false;
	if (!IsValid(InDamageCauser)) return false;

	if (!FMath::IsFinite(InDefaultDamageEvent.ApplyDamageSpec.BaseDamage)) return false;
	if (!FMath::IsFinite(InDefaultDamageEvent.ApplyDamageAmount.RequestDamage)) return false;

	return true;
}

bool UCTakeDamageComponent::ValidateContext(FTakeDamageContext& InOutTakeDamageContext)
{
	if (!IsValid(InOutTakeDamageContext.DamagedActor))
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

bool UCTakeDamageComponent::CanTakeDamage(FTakeDamageContext& InOutTakeDamageContext)
{
	// Gate 1: already dead
	if (InOutTakeDamageContext.DeadState_Before != EDeadState::Alive)
	{
		InOutTakeDamageContext.bAccepted = false;
		InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::AlreadyDead;

		return false;
	}

	// TODO:
	// Gate 2: invulnerable / iframe / god-mode state
	// Gate 3: defensive friendly-fire check on receiver side
	// Gate 4: receiver-side damage cooldown / hit immunity window
	// Gate 5: defensive self-damage policy

	InOutTakeDamageContext.bAccepted = true;
	InOutTakeDamageContext.RejectReason = ETakeDamageRejectReason::None;

	return true;
}

void UCTakeDamageComponent::ComputeTakeDamage(FTakeDamageContext& InOutTakeDamageContext) const
{
	// Process 1: Compute Mitigation Damage
	InOutTakeDamageContext.MitigatedDamage = ComputeMitigatedDamage(InOutTakeDamageContext);	// TODO

	// Gate 1: Zero damage
	if (InOutTakeDamageContext.MitigatedDamage <= KINDA_SMALL_NUMBER)
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

void UCTakeDamageComponent::CommitTakeDamage(FTakeDamageContext& InOutTakeDamageContext)
{
	if (!IsValid(HealthComp_Cached)) return;

	// Process 4: Apply Damage To Health
	InOutTakeDamageContext.CommittedDamage = CommitDamageToHealth(InOutTakeDamageContext);

	// TODO: Shield / Mana / Stemina etc + Commit Order

	// Post-state Snapshot: Set BuildResult
	InOutTakeDamageContext.DeadState_After = HealthComp_Cached->GetDeadState();
	InOutTakeDamageContext.HealthPointAfter = HealthComp_Cached->GetCurrentHP();
}

AController* UCTakeDamageComponent::ResolveInstigatorController(AController* EventInstigator, AActor* DamageCauser) const
{
	// 1) Best case: engine provided instigator
	if (IsValid(EventInstigator))
		return EventInstigator;

	// 2) Fallback needs a valid causer
	if (!IsValid(DamageCauser))
		return nullptr;

	/* === Fallback Process (DamageCauser-based) === */

	// 2-1) Case 01: Explicit instigator set on the causer (ex. projectile / attachment / trap)
	if (AController* causerInstigator = DamageCauser->GetInstigatorController())
		return causerInstigator;

	// 2-2) Case 02: Direct hit (the causer itself is a Pawn/Character)
	if (APawn* causerPawn = Cast<APawn>(DamageCauser))
	{
		if (AController* causerController = causerPawn->GetController())
			return causerController;
	}

	// 2-3) Case 03: Proxy case (projectile / trap / attachment owned by another actor)
	if (AActor* causerOwner = DamageCauser->GetOwner())
	{
		// 2-3-1) Case 03-01: Owner is the carrier and holds the correct instigator (ex. projectile / attachment / trap)
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

float UCTakeDamageComponent::ComputeMitigatedDamage(FTakeDamageContext& InOutTakeDamageContext) const
{
	const float requestedDamage = InOutTakeDamageContext.RequestedDamage;

	// Minimal safe policy (Check NaN, +Inf/-Inf)
	if (!FMath::IsFinite(requestedDamage)) return 0.f;

	const float mitigatedDamage = requestedDamage;

	// TODO: Defense / Armor / Resistance Policy

	return FMath::Max(0.f, mitigatedDamage);
}

float UCTakeDamageComponent::ComputeFinalTakenDamage(FTakeDamageContext& InOutTakeDamageContext) const
{

	const float mitigatedDamage = InOutTakeDamageContext.MitigatedDamage;

	// Minimal safe policy (Check NaN, +Inf/-Inf)
	if (!FMath::IsFinite(mitigatedDamage)) return 0.f;

	const float finalTakenDamage = mitigatedDamage;

	// TODO: Critical / Guard / Headshot etc Policy

	return FMath::Max(0.f, finalTakenDamage);
}

float UCTakeDamageComponent::CommitDamageToHealth(const FTakeDamageContext& InOutTakeDamageContext) const
{
	if (!IsValid(HealthComp_Cached)) return 0.0;

	return HealthComp_Cached->TakeDamage(InOutTakeDamageContext.FinalTakenDamage);
}

FTakeDamagePayload UCTakeDamageComponent::BuildPayload(float DamageAmount, const FDefaultDamageEvent& InDefaultDamageEvent, AController* InDamageInstigator, AActor* InDamageCauser) const
{
	FTakeDamagePayload takeDamagePayload = FTakeDamagePayload();

	takeDamagePayload.DamagedActor = OwnerActor_Cached;
	takeDamagePayload.EventInstigator = InDamageInstigator;
	takeDamagePayload.DamageCauser = InDamageCauser;

	takeDamagePayload.ApplyDamageSpecKey = InDefaultDamageEvent.ApplyDamageSpecKey;
	takeDamagePayload.ApplyDamageSpec = InDefaultDamageEvent.ApplyDamageSpec;
	takeDamagePayload.ApplyDamageAmount = InDefaultDamageEvent.ApplyDamageAmount;

	takeDamagePayload.RequestedDamage = DamageAmount;

	return takeDamagePayload;
}

FTakeDamageContext UCTakeDamageComponent::BuildContext(const FTakeDamagePayload& InTakeDamagePayload) const
{
	FTakeDamageContext takeDamageContext = FTakeDamageContext();

	takeDamageContext.DamagedActor = InTakeDamagePayload.DamagedActor;
	takeDamageContext.Instigator = ResolveInstigatorController(InTakeDamagePayload.EventInstigator, InTakeDamagePayload.DamageCauser);
	takeDamageContext.DamageCauser = InTakeDamagePayload.DamageCauser;
	takeDamageContext.ApplyDamageSpecKey = InTakeDamagePayload.ApplyDamageSpecKey;
	takeDamageContext.RequestedDamage = InTakeDamagePayload.RequestedDamage;

	return takeDamageContext;
}

FTakeDamageResult UCTakeDamageComponent::BuildResult(const FTakeDamageContext& InTakeDamageContext) const
{
	FTakeDamageResult takeDamageResult = FTakeDamageResult();

	takeDamageResult.bAccepted = InTakeDamageContext.bAccepted;
	takeDamageResult.RejectReason = InTakeDamageContext.RejectReason;

	takeDamageResult.ApplyDamageSpecKey = InTakeDamageContext.ApplyDamageSpecKey;

	takeDamageResult.RequestDamage = InTakeDamageContext.RequestedDamage;
	takeDamageResult.MitigatedDamage = InTakeDamageContext.MitigatedDamage;
	takeDamageResult.FinalTakenDamage = InTakeDamageContext.FinalTakenDamage;
	takeDamageResult.CommittedDamage = InTakeDamageContext.CommittedDamage;

	takeDamageResult.DeadState_Before = InTakeDamageContext.DeadState_Before;
	takeDamageResult.DeadState_After = InTakeDamageContext.DeadState_After;

	return takeDamageResult;
}

void UCTakeDamageComponent::DispatchTakeDamageCommitted(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const
{
	// TODO: OnTakeDamageCommitted broadcast
	if (InTakeDamageResult.bAccepted)
	{
		if (IsValid(ReactionComp_Cached))
		{
			ReactionComp_Cached->TryRequestPendingDamageReaction(InTakeDamageResult);
		}
	}

	// TODO:
	// - VFX/SFX
	// - Knockback
	// - HitStop
	// - UI Damage Number
}

void UCTakeDamageComponent::DispatchTakeDamageRejected(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const
{
	// TODO: OnTakeDamageRejected broadcast
}

void UCTakeDamageComponent::PrintTakeDamageSummaryInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const
{
	FLog::Log(TEXT("====== Take Damage Summary ======"));
	FLog::Log(TEXT("[@ TAKE DAMAGE]"));

	FLog::Log(FString::Printf(TEXT("DamagedActor = %s | Instigator = %s | DamageCauser = %s"),
		*GetNameSafe(InTakeDamageContext.DamagedActor),
		*GetNameSafe(InTakeDamageContext.Instigator),
		*GetNameSafe(InTakeDamageContext.DamageCauser)
	));

	FLog::Log(FString::Printf(TEXT("RequestDamage = %.3f | MitigatedDamage = %.3f | FinalTakenDamage = %.3f | CommittedDamage = %.3f"),
		InTakeDamageResult.RequestDamage,
		InTakeDamageResult.MitigatedDamage,
		InTakeDamageResult.FinalTakenDamage,
		InTakeDamageResult.CommittedDamage
	));
	FLog::Log(TEXT("================================="));
}

void UCTakeDamageComponent::PrintTakeDamageContextInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const
{
	FLog::Log(TEXT("/////- Take Damage Context -/////"));
	PrintObjectInfo(InTakeDamagePayload, InTakeDamageContext, InTakeDamageResult);
	PrintSpecKeyInfo(InTakeDamagePayload, InTakeDamageContext, InTakeDamageResult);
	PrintDamageAmountInfo(InTakeDamagePayload, InTakeDamageContext, InTakeDamageResult);
	FLog::Log(TEXT("/////////////////////////////////"));
}

void UCTakeDamageComponent::PrintObjectInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const
{
	FLog::Log(TEXT("========== Object Info =========="));
	FLog::Log(TEXT("--------- Payload Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Payload] EventInstigator"), *GetNameSafe(InTakeDamagePayload.EventInstigator)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Payload] DamageCauser"), *GetNameSafe(InTakeDamagePayload.DamageCauser)));
	FLog::Log(TEXT("--------- Context Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] DamagedActor"), *GetNameSafe(InTakeDamageContext.DamagedActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] Instigator"), *GetNameSafe(InTakeDamageContext.Instigator)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("[Context] DamageCauser"), *GetNameSafe(InTakeDamageContext.DamageCauser)));
	FLog::Log(TEXT("================================="));
}

void UCTakeDamageComponent::PrintSpecKeyInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const
{
	FLog::Log(TEXT("========= SpecKey Info =========="));
	FLog::Log(TEXT("--------- Payload Info ----------"));
	const FApplyDamageSpecKey& applyDamageSpecKey = InTakeDamagePayload.ApplyDamageSpecKey;
	const FString actionIndexText = (applyDamageSpecKey.ActionIndex == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(applyDamageSpecKey.ActionIndex);

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AttachmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.AttachmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("EquipmentType"), *UEnum::GetValueAsString(applyDamageSpecKey.EquipmentType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(applyDamageSpecKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionIndex"), *actionIndexText));
	FLog::Log(TEXT("================================="));
}

void UCTakeDamageComponent::PrintDamageAmountInfo(const FTakeDamagePayload& InTakeDamagePayload, const FTakeDamageContext& InTakeDamageContext, const FTakeDamageResult& InTakeDamageResult) const
{
	FLog::Log(TEXT("======= DamageAmount Info ======="));
	FLog::Log(TEXT("--------- Payload Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("BaseDamage"), InTakeDamagePayload.ApplyDamageSpec.BaseDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("RequestDamage"), InTakeDamagePayload.ApplyDamageAmount.RequestDamage));
	FLog::Log(TEXT("---------- Amount Info ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("FinalTakenDamage"), InTakeDamageResult.FinalTakenDamage));
	FLog::Log(TEXT("================================="));
}

