#include "Component/CCombatSignalSourceComponent.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/ShapeComponent.h"
#include "GameFramework/Character.h"
#include "HAL/IConsoleManager.h"

#include "AI/Blackboard/CAIKey.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatSignalTargetComponent.h"

#include "Core/Debug/FCombatSignalDebug.h"
#include "Core/Profiling/CCombatCollisionProfilingCounters.h"

#include "Type/CWeaponStructure.h"

namespace
{
	TAutoConsoleVariable<int32> CVarDisableEnemyHitProcessing(
		TEXT("Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing"),
		0,
		TEXT("Disable Enemy hit processing for combat collision profiling. 0: process hit, 1: skip Enemy hit processing after overlap."),
		ECVF_Default);

	const FName CombatTimingCueSignalTag(TEXT("Combat.Signal.TimingCue"));
}

UCCombatSignalSourceComponent::UCCombatSignalSourceComponent()
{
}

void UCCombatSignalSourceComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;

	ValidateRequiredComponentReferences();
}

bool UCCombatSignalSourceComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

void UCCombatSignalSourceComponent::NotifyHitWindowOpened(AActor* InDamageCauser, int32 InHitWindowId)
{
	if (!IsValid(InDamageCauser)) return;
	if (InHitWindowId == INDEX_NONE) return;

	FCombatSignalHitWindowKey hitWindowKey;
	hitWindowKey.DamageCauser = InDamageCauser;
	hitWindowKey.HitWindowId = InHitWindowId;

	// Reset stale record for the same hit window key
	DamagedTargetContainer.Remove(hitWindowKey);
	DamagedTargetContainer.FindOrAdd(hitWindowKey);
}

void UCCombatSignalSourceComponent::NotifyHitWindowClosed(AActor* InDamageCauser, int32 InHitWindowId)
{
	if (!IsValid(InDamageCauser)) return;
	if (InHitWindowId == INDEX_NONE) return;

	FCombatSignalHitWindowKey hitWindowKey;
	hitWindowKey.DamageCauser = InDamageCauser;
	hitWindowKey.HitWindowId = InHitWindowId;

	DamagedTargetContainer.Remove(hitWindowKey);
}

void UCCombatSignalSourceComponent::RequestCombatSignalSource(const FHitContext& InHitContext)
{
	if (ShouldSkipEnemyHitProcessingForProfiling()) return;

	FCombatCollisionProfilingCounters::RecordHitProcessing();

	ProcessCombatSignalSource(InHitContext);
}

bool UCCombatSignalSourceComponent::RequestCombatSignalCue(AActor* InTargetActor, FName InCueTag, const FVector& InCueLocation, const FVector& InDirection, AActor* InSignalCauser)
{
	FCombatCollisionProfilingCounters::RecordCombatSignalCueRequest();

	const FCombatSignal combatSignal = BuildCueSignal(InTargetActor, InCueTag, InCueLocation, InDirection, InSignalCauser);
	if (!ValidateCueSignal(combatSignal))
	{
		FCombatSignalDebug::RecordCueRejectedForAudit(combatSignal, TEXT("InvalidCueRequest"));
		return false;
	}

	return SendCueSignal(combatSignal);
}

bool UCCombatSignalSourceComponent::RequestAICombatSignalCue(FName InCueTag)
{
	FCombatCollisionProfilingCounters::RecordAICombatSignalCueRequest();

	AActor* targetActor = ResolveCueTargetActor();
	if (!IsValid(targetActor))
	{
		FCombatSignal emptySignal = BuildCueSignal(nullptr, InCueTag, FVector::ZeroVector, FVector::ZeroVector, OwnerCharacter_Injected);
		FCombatSignalDebug::RecordCueRejectedForAudit(emptySignal, TEXT("MissingAITarget"));
		return false;
	}

	const FVector cueLocation = IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->GetActorLocation() : FVector::ZeroVector;
	const FVector cueDirection = IsValid(OwnerCharacter_Injected) ? (targetActor->GetActorLocation() - OwnerCharacter_Injected->GetActorLocation()).GetSafeNormal() : FVector::ZeroVector;

	return RequestCombatSignalCue(targetActor, InCueTag, cueLocation, cueDirection, OwnerCharacter_Injected);
}

void UCCombatSignalSourceComponent::ProcessCombatSignalSource(const FHitContext& InHitContext)
{
	// Receive: validate overlap hit input and normalize it into source-side data.
	if (!ValidateRequest(InHitContext))
	{
		return;
	}

	FCombatSignalSourcePayload combatSignalSourcePayload = BuildPayload(InHitContext);
	FCombatSignalSourceContext combatSignalSourceContext = BuildContext(combatSignalSourcePayload);

	// Resolve: validate source-side context and sender policy before target delivery.
	if (!ValidateContext(combatSignalSourceContext))
	{
		FCombatSignalDebug::RecordSourceRejectedForAudit(combatSignalSourceContext);
		FCombatSignalDebug::PrintSourceContextDebug(combatSignalSourceContext);
		return;
	}

	if (!CanSendCombatSignal(combatSignalSourceContext))
	{
		FCombatSignalDebug::RecordSourceRejectedForAudit(combatSignalSourceContext);
		FCombatSignalDebug::PrintSourceContextDebug(combatSignalSourceContext);
		return;
	}

	// Resolve: resolve damage spec and compute request damage.
	ResolveSourceDamageSpec(combatSignalSourceContext);
	if (!combatSignalSourceContext.bAccepted)
	{
		FCombatSignalDebug::RecordSourceRejectedForAudit(combatSignalSourceContext);
		FCombatSignalDebug::PrintSourceContextDebug(combatSignalSourceContext);
		return;
	}

	ComputeSourceDamage(combatSignalSourceContext);
	if (!combatSignalSourceContext.bAccepted)
	{
		FCombatSignalDebug::RecordSourceRejectedForAudit(combatSignalSourceContext);
		FCombatSignalDebug::PrintSourceContextDebug(combatSignalSourceContext);
		return;
	}

	// Send: deliver the source-side damage event to the target damage entry.
	CommitCombatSignalSource(combatSignalSourceContext);
	if (!combatSignalSourceContext.bAccepted)
	{
		FCombatSignalDebug::RecordSourceRejectedForAudit(combatSignalSourceContext);
		FCombatSignalDebug::PrintSourceContextDebug(combatSignalSourceContext);
		return;
	}

	FCombatSignalDebug::RecordSourceAcceptedForAudit(combatSignalSourceContext);
	FCombatSignalDebug::PrintSourceContextDebug(combatSignalSourceContext);
}

// Profiling

bool UCCombatSignalSourceComponent::ShouldSkipEnemyHitProcessingForProfiling() const
{
	if (CVarDisableEnemyHitProcessing.GetValueOnGameThread() == 0) return false;

	return IsEnemyHitProcessingProfilingTarget();
}

bool UCCombatSignalSourceComponent::IsEnemyHitProcessingProfilingTarget() const
{
	return IsValid(OwnerCharacter_Injected) && OwnerCharacter_Injected->IsA<ACEnemy>();
}

bool UCCombatSignalSourceComponent::ValidateRequest(const FHitContext& InHitContext) const
{
	const FOverlapContext& overlapContext = InHitContext.OverlapContext;

	// V1: Validate core actors (OwnerActor / DamageCauser / OtherActor)
	if (!overlapContext.IsValidMinimal())
	{
		FCombatSignalDebug::RecordSourceHitRequestRejectedForAudit(InHitContext, TEXT("InvalidMinimalOverlapContext"));
		return false;
	}

	// V2: Check Valid Hit Window
	if (overlapContext.HitWindowId == INDEX_NONE)
	{
		FCombatSignalDebug::RecordSourceHitRequestRejectedForAudit(InHitContext, TEXT("InvalidHitWindow"));
		return false;
	}

	// V3: Check Valid Object
	// 3-1): Validate Components (current policy)
	if (!IsValid(overlapContext.OverlappedComponent) || !IsValid(overlapContext.OtherComponent))
	{
		FCombatSignalDebug::RecordSourceHitRequestRejectedForAudit(InHitContext, TEXT("InvalidOverlapComponent"));
		return false;
	}

	// 3-2): Attack collision must be ShapeComponent (current policy)
	if (!IsValid(overlapContext.OverlapShape))
	{
		FCombatSignalDebug::RecordSourceHitRequestRejectedForAudit(InHitContext, TEXT("InvalidOverlapShape"));
		return false;
	}

	// V4: Check ownership
	 // 4-1) DamageCauser must be owned by the attacker
	if (overlapContext.DamageCauser->GetOwner() != overlapContext.OwnerActor)
	{
		FCombatSignalDebug::RecordSourceHitRequestRejectedForAudit(InHitContext, TEXT("DamageCauserOwnerMismatch"));
		return false;
	}

	// 4-2) OverlappedComponent must belong to the DamageCauser
	if (overlapContext.OverlappedComponent->GetOwner() != overlapContext.DamageCauser)
	{
		FCombatSignalDebug::RecordSourceHitRequestRejectedForAudit(InHitContext, TEXT("OverlappedComponentOwnerMismatch"));
		return false;
	}

	// 4-3) OtherComponent must belong to the target actor
	if (overlapContext.OtherComponent->GetOwner() != overlapContext.OtherActor)
	{
		FCombatSignalDebug::RecordSourceHitRequestRejectedForAudit(InHitContext, TEXT("OtherComponentOwnerMismatch"));
		return false;
	}

	return true;
}

FCombatSignalSourcePayload UCCombatSignalSourceComponent::BuildPayload(const FHitContext& InHitContext) const
{
	FCombatSignalSourcePayload combatSignalSourcePayload;

	combatSignalSourcePayload.HitContext = InHitContext;
	combatSignalSourcePayload.SourceActor = InHitContext.OverlapContext.OwnerActor;
	combatSignalSourcePayload.DamageCauser = InHitContext.OverlapContext.DamageCauser;
	combatSignalSourcePayload.TargetActor = InHitContext.OverlapContext.OtherActor;
	combatSignalSourcePayload.DamageImpactInfo = InHitContext.DamageImpactInfo;
	combatSignalSourcePayload.HitWindowKey = BuildHitWindowKey(InHitContext);
	combatSignalSourcePayload.DamageSpecKey = BuildSpecKey(InHitContext);

	return combatSignalSourcePayload;
}

FCombatSignalSourceContext UCCombatSignalSourceComponent::BuildContext(const FCombatSignalSourcePayload& InCombatSignalSourcePayload) const
{
	FCombatSignalSourceContext combatSignalSourceContext;

	combatSignalSourceContext.HitContext = InCombatSignalSourcePayload.HitContext;
	combatSignalSourceContext.SourceActor = InCombatSignalSourcePayload.SourceActor;
	combatSignalSourceContext.DamageCauser = InCombatSignalSourcePayload.DamageCauser;
	combatSignalSourceContext.TargetActor = InCombatSignalSourcePayload.TargetActor;
	combatSignalSourceContext.HitWindowKey = InCombatSignalSourcePayload.HitWindowKey;
	combatSignalSourceContext.DamageImpactInfo = InCombatSignalSourcePayload.DamageImpactInfo;
	combatSignalSourceContext.DamageSpecKey = InCombatSignalSourcePayload.DamageSpecKey;
	combatSignalSourceContext.Instigator = ResolveInstigatorController(InCombatSignalSourcePayload.SourceActor, InCombatSignalSourcePayload.DamageCauser);

	return combatSignalSourceContext;
}

bool UCCombatSignalSourceComponent::ValidateContext(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const
{
	if (!IsValid(InOutCombatSignalSourceContext.SourceActor))
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::InvalidAttacker;
		return false;
	}

	if (!IsValid(InOutCombatSignalSourceContext.DamageCauser))
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::InvalidDamageCauser;
		return false;
	}

	if (!IsValid(InOutCombatSignalSourceContext.TargetActor))
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::InvalidTarget;
		return false;
	}

	if (!IsValid(InOutCombatSignalSourceContext.Instigator))
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::InvalidInstigator;
		return false;
	}

	// Valid Context
	InOutCombatSignalSourceContext.bAccepted = true;
	InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::None;
	return true;
}

bool UCCombatSignalSourceComponent::CanSendCombatSignal(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const
{
	const FOverlapContext& overlapContext = InOutCombatSignalSourceContext.HitContext.OverlapContext;

	AActor* myOwner = OwnerCharacter_Injected;
	if (!IsValid(myOwner) || myOwner != overlapContext.OwnerActor)
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::InvalidOwner;
		return false;
	}

	if (overlapContext.OtherActor == overlapContext.OwnerActor)
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::SelfTarget;
		return false;
	}

	if (IsFriendlyTarget(InOutCombatSignalSourceContext))
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::FriendlyTarget;
		return false;
	}

	if (IsDuplicateHit(InOutCombatSignalSourceContext))
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::DuplicateHitInWindow;
		return false;
	}

	InOutCombatSignalSourceContext.bAccepted = true;
	InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::None;
	return true;
}

void UCCombatSignalSourceComponent::ResolveSourceDamageSpec(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const
{
	const FDamageSpec* foundDamageSpec = DamageSpecContainer.Find(InOutCombatSignalSourceContext.DamageSpecKey);

	if (!foundDamageSpec)
	{
		InOutCombatSignalSourceContext.DamageSpec = FDamageSpec();
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::SpecNotFound;
		return;
	}

	InOutCombatSignalSourceContext.DamageSpec = *foundDamageSpec;
	InOutCombatSignalSourceContext.bAccepted = true;
	InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::None;
}

void UCCombatSignalSourceComponent::ComputeSourceDamage(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const
{
	if (!IsValid(InOutCombatSignalSourceContext.SourceActor) || !IsValid(InOutCombatSignalSourceContext.DamageCauser) || !IsValid(InOutCombatSignalSourceContext.TargetActor))
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::ComputeFailed;
		return;
	}

	// [NOTE] Minimal sender-side request damage.
	InOutCombatSignalSourceContext.DamageAmount = FDamageAmount();
	InOutCombatSignalSourceContext.DamageAmount.RequestDamage = InOutCombatSignalSourceContext.DamageSpec.BaseDamage;

	InOutCombatSignalSourceContext.bAccepted = true;
	InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::None;
}

FCombatSignalSourceResult UCCombatSignalSourceComponent::BuildResult(const FCombatSignalSourceContext& InCombatSignalSourceContext) const
{
	FCombatSignalSourceResult combatSignalSourceResult;

	combatSignalSourceResult.bAccepted = InCombatSignalSourceContext.bAccepted;
	combatSignalSourceResult.RejectReason = InCombatSignalSourceContext.RejectReason;
	combatSignalSourceResult.HitWindowKey = InCombatSignalSourceContext.HitWindowKey;
	combatSignalSourceResult.DamageSpecKey = InCombatSignalSourceContext.DamageSpecKey;
	combatSignalSourceResult.BaseDamage = InCombatSignalSourceContext.DamageSpec.BaseDamage;
	combatSignalSourceResult.RequestDamage = InCombatSignalSourceContext.DamageAmount.RequestDamage;
	combatSignalSourceResult.CommittedDamage = InCombatSignalSourceContext.CommittedDamage;

	return combatSignalSourceResult;
}

void UCCombatSignalSourceComponent::CommitCombatSignalSource(FCombatSignalSourceContext& InOutCombatSignalSourceContext)
{
	FCombatCollisionProfilingCounters::RecordCombatSignal();

	InOutCombatSignalSourceContext.CommittedDamage = SendDamageToTarget(InOutCombatSignalSourceContext);

	if (InOutCombatSignalSourceContext.CommittedDamage <= 0.f)
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::CommitFailed;
		return;
	}

	InOutCombatSignalSourceContext.bAccepted = true;
	InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::None;

	CacheDamagedTargetInWindow(InOutCombatSignalSourceContext);
}

float UCCombatSignalSourceComponent::SendDamageToTarget(const FCombatSignalSourceContext& InCombatSignalSourceContext) const
{
	if (!IsValid(InCombatSignalSourceContext.TargetActor) || !IsValid(InCombatSignalSourceContext.DamageCauser) || !IsValid(InCombatSignalSourceContext.Instigator))
		return 0.f;

	FDefaultDamageEvent damageEvent;

	damageEvent.SourceActor = InCombatSignalSourceContext.SourceActor;
	damageEvent.TargetActor = InCombatSignalSourceContext.TargetActor;
	damageEvent.DamageSpecKey = InCombatSignalSourceContext.DamageSpecKey;
	damageEvent.DamageImpactInfo = InCombatSignalSourceContext.DamageImpactInfo;
	damageEvent.DamageSpec = InCombatSignalSourceContext.DamageSpec;
	damageEvent.DamageAmount = InCombatSignalSourceContext.DamageAmount;

	return InCombatSignalSourceContext.TargetActor->TakeDamage(InCombatSignalSourceContext.DamageAmount.RequestDamage, damageEvent, InCombatSignalSourceContext.Instigator, InCombatSignalSourceContext.DamageCauser);
}

bool UCCombatSignalSourceComponent::SendCueSignal(const FCombatSignal& InCombatSignal) const
{
	if (!ValidateCueSignal(InCombatSignal))
	{
		FCombatSignalDebug::RecordCueRejectedForAudit(InCombatSignal, TEXT("InvalidCueSignal"));
		return false;
	}

	AActor* targetActor = InCombatSignal.Header.TargetActor;
	if (!IsValid(targetActor))
	{
		FCombatSignalDebug::RecordCueRejectedForAudit(InCombatSignal, TEXT("InvalidTarget"));
		return false;
	}

	UCCombatSignalTargetComponent* targetComponent = targetActor->FindComponentByClass<UCCombatSignalTargetComponent>();
	if (!IsValid(targetComponent))
	{
		FCombatSignalDebug::RecordCueRejectedForAudit(InCombatSignal, TEXT("MissingTargetComponent"));
		return false;
	}

	FCombatCollisionProfilingCounters::RecordCombatSignalCueSend();

	const bool bAccepted = targetComponent->RequestCombatSignalTarget(InCombatSignal);
	if (!bAccepted)
	{
		FCombatSignalDebug::RecordCueRejectedForAudit(InCombatSignal, TEXT("TargetRejectedCue"));
		return false;
	}

	FCombatSignalDebug::RecordCueAcceptedForAudit(InCombatSignal);
	return true;
}

void UCCombatSignalSourceComponent::CacheDamagedTargetInWindow(const FCombatSignalSourceContext& InCombatSignalSourceContext)
{
	AActor* targetActor = InCombatSignalSourceContext.TargetActor;
	if (!IsValid(targetActor)) return;

	// Cached
	auto& damagedTargets = DamagedTargetContainer.FindOrAdd(InCombatSignalSourceContext.HitWindowKey);
	damagedTargets.Add(targetActor);
}

// Hit Helper

FCombatSignalHitWindowKey UCCombatSignalSourceComponent::BuildHitWindowKey(const FHitContext& InHitContext) const
{
	FCombatSignalHitWindowKey hitWindowKey;

	hitWindowKey.DamageCauser = InHitContext.OverlapContext.DamageCauser;
	hitWindowKey.HitWindowId = InHitContext.OverlapContext.HitWindowId;

	return hitWindowKey;
}

FDamageSpecKey UCCombatSignalSourceComponent::BuildSpecKey(const FHitContext& InHitContext) const
{
	FDamageSpecKey damageSpecKey;

	damageSpecKey.WeaponType = InHitContext.WeaponContext.WeaponType;
	damageSpecKey.ActionType = InHitContext.ActionContext.ActionType;
	damageSpecKey.ActionIndex = InHitContext.ActionContext.ActionIndex;

	return damageSpecKey;
}

AController* UCCombatSignalSourceComponent::ResolveInstigatorController(AActor* InAttacker, AActor* InDamageCauser) const
{
	if (IsValid(InAttacker))
	{
		// 1) Preferred: attacker-provided instigator
		if (AController* attackerInstigator = InAttacker->GetInstigatorController())
			return attackerInstigator;

		// 2) Fallback: attacker is a pawn/character
		if (APawn* attackerPawn = Cast<APawn>(InAttacker))
		{
			if (AController* attackerController = attackerPawn->GetController())
				return attackerController;
		}
	}

	if (IsValid(InDamageCauser))
	{
		// 3) Fallback: causer-provided instigator
		if (AController* causerInstigator = InDamageCauser->GetInstigatorController())
			return causerInstigator;

		if (AActor* causerOwner = InDamageCauser->GetOwner())
		{
			// 4) Fallback: owner of the causer provides the instigator
			if (AController* ownerInstigator = causerOwner->GetInstigatorController())
				return ownerInstigator;

			// 5) Final fallback: owner of the causer is a pawn/character
			if (APawn* ownerPawn = Cast<APawn>(causerOwner))
			{
				if (AController* ownerController = ownerPawn->GetController())
					return ownerController;
			}
		}
	}

	return nullptr;
}

bool UCCombatSignalSourceComponent::IsDuplicateHit(const FCombatSignalSourceContext& InCombatSignalSourceContext) const
{
	const TSet<TWeakObjectPtr<AActor>>* foundTargets = DamagedTargetContainer.Find(InCombatSignalSourceContext.HitWindowKey);

	if (!foundTargets) return false;

	return foundTargets->Contains(TWeakObjectPtr<AActor>(InCombatSignalSourceContext.TargetActor));
}

bool UCCombatSignalSourceComponent::IsFriendlyTarget(const FCombatSignalSourceContext& InCombatSignalSourceContext) const
{
	AActor* ownerActor = InCombatSignalSourceContext.SourceActor;
	AActor* targetActor = InCombatSignalSourceContext.TargetActor;

	if (!IsValid(ownerActor) || !IsValid(targetActor)) return false;

	const bool bOwnerEnemy = ownerActor->IsA<ACEnemy>();
	const bool bTargetEnemy = targetActor->IsA<ACEnemy>();

	return bOwnerEnemy && bTargetEnemy;
}

// Cue Helper

AActor* UCCombatSignalSourceComponent::ResolveCueTargetActor() const
{
	if (!IsValid(OwnerCharacter_Injected)) return nullptr;

	const AAIController* aiController = Cast<AAIController>(OwnerCharacter_Injected->GetController());
	if (!IsValid(aiController)) return nullptr;

	const UBlackboardComponent* blackboardComp = aiController->GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return nullptr;

	return Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName));
}

FCombatSignal UCCombatSignalSourceComponent::BuildCueSignal(AActor* InTargetActor, FName InCueTag, const FVector& InCueLocation, const FVector& InDirection, AActor* InSignalCauser) const
{
	FCombatSignal combatSignal;

	AActor* ownerActor = OwnerCharacter_Injected;
	AActor* signalCauser = IsValid(InSignalCauser) ? InSignalCauser : ownerActor;

	combatSignal.Header.SignalType = ECombatSignalType::TimingCue;
	combatSignal.Header.SourceActor = ownerActor;
	combatSignal.Header.TargetActor = InTargetActor;
	combatSignal.Header.InstigatorActor = ownerActor;
	combatSignal.Header.SignalCauser = signalCauser;
	combatSignal.Header.DebugTag = CombatTimingCueSignalTag;

	combatSignal.SignalTag = CombatTimingCueSignalTag;
	combatSignal.CueTag = InCueTag;
	combatSignal.RequestedDamage = 0.f;
	combatSignal.ImpactLocation = InCueLocation;

	FVector direction = InDirection;
	if (direction.IsNearlyZero() && IsValid(ownerActor) && IsValid(InTargetActor))
	{
		direction = InTargetActor->GetActorLocation() - ownerActor->GetActorLocation();
	}
	combatSignal.Direction = direction.GetSafeNormal();

	return combatSignal;
}

bool UCCombatSignalSourceComponent::ValidateCueSignal(const FCombatSignal& InCombatSignal) const
{
	if (!InCombatSignal.IsValidMinimal()) return false;
	if (InCombatSignal.Header.SignalType != ECombatSignalType::TimingCue) return false;
	if (!IsValid(InCombatSignal.Header.SourceActor)) return false;
	if (!IsValid(InCombatSignal.Header.TargetActor)) return false;
	if (InCombatSignal.CueTag.IsNone()) return false;

	return true;
}
