#include "Component/CCombatSignalSourceComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

#include "Type/CWeaponStructure.h"

UCCombatSignalSourceComponent::UCCombatSignalSourceComponent()
{
}

void UCCombatSignalSourceComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);
}

void UCCombatSignalSourceComponent::NotifyHitWindowOpened(AActor* InDamageCauser, int32 InHitWindowId)
{
	// FLog::Log(FString::Printf(TEXT("%-20s: %s = %d"), TEXT("[UCCombatSignalSourceComponent|NotifyHitWindowOpened]"), TEXT("InHitWindowId"), InHitWindowId));

	if (!IsValid(InDamageCauser)) return;
	if (InHitWindowId == INDEX_NONE) return;

	FApplyDamageHitWindowKey hitWindowKey;
	hitWindowKey.DamageCauser = InDamageCauser;
	hitWindowKey.HitWindowId = InHitWindowId;

	// Reset stale record for the same hit window key
	DamagedTargetContainer.Remove(hitWindowKey);
	DamagedTargetContainer.FindOrAdd(hitWindowKey);
}

void UCCombatSignalSourceComponent::NotifyHitWindowClosed(AActor* InDamageCauser, int32 InHitWindowId)
{
	// FLog::Log(FString::Printf(TEXT("%-20s: %s = %d"), TEXT("[UCCombatSignalSourceComponent|NotifyHitWindowClosed]"), TEXT("InHitWindowId"), InHitWindowId));

	if (!IsValid(InDamageCauser)) return;
	if (InHitWindowId == INDEX_NONE) return;

	FApplyDamageHitWindowKey hitWindowKey;
	hitWindowKey.DamageCauser = InDamageCauser;
	hitWindowKey.HitWindowId = InHitWindowId;

	DamagedTargetContainer.Remove(hitWindowKey);
}

void UCCombatSignalSourceComponent::RequestCombatSignalSource(const FHitContext& InHitContext)
{
	ProcessCombatSignalSource(InHitContext);
}

void UCCombatSignalSourceComponent::ProcessCombatSignalSource(const FHitContext& InHitContext)
{
	// Receive: validate overlap hit input and normalize it into source-side data.
	if (!ValidateRequest(InHitContext))
	{
		// PrintCombatSignalSourceRejectedSummaryInfo(InHitContext, ECombatSignalSourceRejectReason::InvalidRequest);
		return;
	}

	FCombatSignalSourcePayload combatSignalSourcePayload = BuildPayload(InHitContext);
	FCombatSignalSourceContext combatSignalSourceContext = BuildContext(combatSignalSourcePayload);

	// Resolve: validate source-side context and sender policy before target delivery.
	if (!ValidateContext(combatSignalSourceContext))
	{
		const FCombatSignalSourceResult combatSignalSourceResult = BuildResult(combatSignalSourceContext);
		// PrintCombatSignalSourceRejectedSummaryInfo(combatSignalSourceContext.HitContext, combatSignalSourceResult.RejectReason);
		return;
	}

	if (!CanSendCombatSignal(combatSignalSourceContext))
	{
		const FCombatSignalSourceResult combatSignalSourceResult = BuildResult(combatSignalSourceContext);
		// PrintCombatSignalSourceRejectedSummaryInfo(combatSignalSourceContext.HitContext, combatSignalSourceResult.RejectReason);
		return;
	}

	// Resolve: resolve damage spec and compute request damage.
	ResolveSourceDamageSpec(combatSignalSourceContext);
	if (!combatSignalSourceContext.bAccepted)
	{
		const FCombatSignalSourceResult combatSignalSourceResult = BuildResult(combatSignalSourceContext);
		// PrintCombatSignalSourceRejectedSummaryInfo(combatSignalSourceContext.HitContext, combatSignalSourceResult.RejectReason);
		return;
	}

	ComputeSourceDamage(combatSignalSourceContext);
	if (!combatSignalSourceContext.bAccepted)
	{
		const FCombatSignalSourceResult combatSignalSourceResult = BuildResult(combatSignalSourceContext);
		// PrintCombatSignalSourceRejectedSummaryInfo(combatSignalSourceContext.HitContext, combatSignalSourceResult.RejectReason);
		return;
	}

	// Send: deliver the source-side damage event to the target damage entry.
	CommitCombatSignalSource(combatSignalSourceContext);
	if (!combatSignalSourceContext.bAccepted)
	{
		const FCombatSignalSourceResult combatSignalSourceResult = BuildResult(combatSignalSourceContext);
		// PrintCombatSignalSourceRejectedSummaryInfo(combatSignalSourceContext.HitContext, combatSignalSourceResult.RejectReason);
		return;
	}

	// Debug: build the final source-side result for optional reporting.
	const FCombatSignalSourceResult combatSignalSourceResult = BuildResult(combatSignalSourceContext);
	// PrintCombatSignalSourceSummaryInfo(combatSignalSourceContext.HitContext, combatSignalSourceResult);
}

bool UCCombatSignalSourceComponent::ValidateRequest(const FHitContext& InHitContext) const
{
	const FOverlapContext& overlapContext = InHitContext.OverlapContext;

	// V1: Validate core actors (OwnerActor / DamageCauser / OtherActor)
	if (!overlapContext.IsValidMinimal()) return false;

	// V2: Check Valid Hit Window
	if (overlapContext.HitWindowId == INDEX_NONE) return false;

	// V3: Check Valid Object
	// 3-1): Validate Components (current policy)
	if (!IsValid(overlapContext.OverlappedComponent) || !IsValid(overlapContext.OtherComponent))
		return false;

	// 3-2): Attack collision must be ShapeComponent (current policy)
	if (!IsValid(overlapContext.OverlapShape))
		return false;

	// V4: Check ownership
	 // 4-1) DamageCauser must be owned by the attacker
	if (overlapContext.DamageCauser->GetOwner() != overlapContext.OwnerActor)
		return false;

	// 4-2) OverlappedComponent must belong to the DamageCauser
	if (overlapContext.OverlappedComponent->GetOwner() != overlapContext.DamageCauser)
		return false;

	// 4-3) OtherComponent must belong to the target actor
	if (overlapContext.OtherComponent->GetOwner() != overlapContext.OtherActor)
		return false;

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
	combatSignalSourcePayload.ApplyDamageSpecKey = BuildSpecKey(InHitContext);

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
	combatSignalSourceContext.ApplyDamageSpecKey = InCombatSignalSourcePayload.ApplyDamageSpecKey;
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

	AActor* myOwner = GetOwner();
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

	if (IsDuplicateHit(InOutCombatSignalSourceContext))
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::DuplicateHitInWindow;
		return false;
	}

	if (IsFriendlyTarget(InOutCombatSignalSourceContext))
	{
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::FriendlyTarget;
		return false;
	}

	InOutCombatSignalSourceContext.bAccepted = true;
	InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::None;
	return true;
}

void UCCombatSignalSourceComponent::ResolveSourceDamageSpec(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const
{
	const FDamageSpec* foundApplyDamageSpec = ApplyDamageSpecContainer.Find(InOutCombatSignalSourceContext.ApplyDamageSpecKey);

	if (!foundApplyDamageSpec)
	{
		InOutCombatSignalSourceContext.ApplyDamageSpec = FDamageSpec();
		InOutCombatSignalSourceContext.bAccepted = false;
		InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::SpecNotFound;
		return;
	}

	InOutCombatSignalSourceContext.ApplyDamageSpec = *foundApplyDamageSpec;
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
	InOutCombatSignalSourceContext.ApplyDamageAmount = FDamageAmount();
	InOutCombatSignalSourceContext.ApplyDamageAmount.RequestDamage = InOutCombatSignalSourceContext.ApplyDamageSpec.BaseDamage;

	InOutCombatSignalSourceContext.bAccepted = true;
	InOutCombatSignalSourceContext.RejectReason = ECombatSignalSourceRejectReason::None;
}

FCombatSignalSourceResult UCCombatSignalSourceComponent::BuildResult(const FCombatSignalSourceContext& InCombatSignalSourceContext) const
{
	FCombatSignalSourceResult combatSignalSourceResult;

	combatSignalSourceResult.bAccepted = InCombatSignalSourceContext.bAccepted;
	combatSignalSourceResult.RejectReason = InCombatSignalSourceContext.RejectReason;
	combatSignalSourceResult.HitWindowKey = InCombatSignalSourceContext.HitWindowKey;
	combatSignalSourceResult.ApplyDamageSpecKey = InCombatSignalSourceContext.ApplyDamageSpecKey;
	combatSignalSourceResult.BaseDamage = InCombatSignalSourceContext.ApplyDamageSpec.BaseDamage;
	combatSignalSourceResult.RequestDamage = InCombatSignalSourceContext.ApplyDamageAmount.RequestDamage;
	combatSignalSourceResult.CommittedDamage = InCombatSignalSourceContext.CommittedDamage;

	return combatSignalSourceResult;
}

void UCCombatSignalSourceComponent::CommitCombatSignalSource(FCombatSignalSourceContext& InOutCombatSignalSourceContext)
{
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
	damageEvent.ApplyDamageSpecKey = InCombatSignalSourceContext.ApplyDamageSpecKey;
	damageEvent.DamageImpactInfo = InCombatSignalSourceContext.DamageImpactInfo;
	damageEvent.ApplyDamageSpec = InCombatSignalSourceContext.ApplyDamageSpec;
	damageEvent.ApplyDamageAmount = InCombatSignalSourceContext.ApplyDamageAmount;

	return InCombatSignalSourceContext.TargetActor->TakeDamage(InCombatSignalSourceContext.ApplyDamageAmount.RequestDamage, damageEvent, InCombatSignalSourceContext.Instigator, InCombatSignalSourceContext.DamageCauser);
}

void UCCombatSignalSourceComponent::CacheDamagedTargetInWindow(const FCombatSignalSourceContext& InCombatSignalSourceContext)
{
	AActor* targetActor = InCombatSignalSourceContext.TargetActor;
	if (!IsValid(targetActor)) return;

	// Cached
	auto& damagedTargets = DamagedTargetContainer.FindOrAdd(InCombatSignalSourceContext.HitWindowKey);
	damagedTargets.Add(targetActor);
}

FApplyDamageHitWindowKey UCCombatSignalSourceComponent::BuildHitWindowKey(const FHitContext& InHitContext) const
{
	FApplyDamageHitWindowKey hitWindowKey;

	hitWindowKey.DamageCauser = InHitContext.OverlapContext.DamageCauser;
	hitWindowKey.HitWindowId = InHitContext.OverlapContext.HitWindowId;

	return hitWindowKey;
}

FDamageSpecKey UCCombatSignalSourceComponent::BuildSpecKey(const FHitContext& InHitContext) const
{
	FDamageSpecKey applyDamageSpecKey;

	applyDamageSpecKey.WeaponType = InHitContext.WeaponContext.WeaponType;
	applyDamageSpecKey.ActionType = InHitContext.ActionContext.ActionType;
	applyDamageSpecKey.ActionIndex = InHitContext.ActionContext.ActionIndex;

	return applyDamageSpecKey;
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
	const TSet<AActor*>* foundTargets = DamagedTargetContainer.Find(InCombatSignalSourceContext.HitWindowKey);

	if (!foundTargets) return false;

	return foundTargets->Contains(InCombatSignalSourceContext.TargetActor);
}

bool UCCombatSignalSourceComponent::IsFriendlyTarget(const FCombatSignalSourceContext& InCombatSignalSourceContext) const
{
	AActor* ownerActor = InCombatSignalSourceContext.SourceActor;
	AActor* targetActor = InCombatSignalSourceContext.TargetActor;

	if (!IsValid(ownerActor) || !IsValid(targetActor)) return false;

	// TODO:
	// Team / Friendly Fire policy
	//
	// Suggested direction:
	// 1. Resolve team source from ownerActor
	// 2. Resolve team source from targetActor
	// 3. Compare team ids or attitudes
	// 4. Return true when friendly-fire should be blocked
	//
	// Example candidates:
	// - Team component on character
	// - Team interface on actor
	// - Gameplay tag based faction policy

	return false;
}

void UCCombatSignalSourceComponent::PrintCombatSignalSourceSummaryInfo(const FHitContext& InHitContext, const FCombatSignalSourceResult& InCombatSignalSourceResult) const
{
	FLog::Log(TEXT("===== Combat Signal Source Summary ======"));
	FLog::Log(TEXT("[@ COMBAT SIGNAL SOURCE]"));

	FLog::Log(FString::Printf(
		TEXT("DamageCauser = %s | Target = %s | HitWindowId = %d | Base = %.3f | Request = %.3f | Committed = %.3f"),
		*GetNameSafe(InHitContext.OverlapContext.DamageCauser),
		*GetNameSafe(InHitContext.OverlapContext.OtherActor),
		InHitContext.OverlapContext.HitWindowId,
		InCombatSignalSourceResult.BaseDamage,
		InCombatSignalSourceResult.RequestDamage,
		InCombatSignalSourceResult.CommittedDamage
	));
	FLog::Log(TEXT("================================="));
}

void UCCombatSignalSourceComponent::PrintCombatSignalSourceContextInfo(const FHitContext& InHitContext, const FDamageSpec& InApplyDamageSpec, const FCombatSignalSourceResult& InCombatSignalSourceResult) const
{
	FLog::Log(TEXT("////- Combat Signal Source Context -/////"));
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	PrintHitContextInfo(InHitContext.WeaponContext, InHitContext.ActionContext);
	PrintDamageSpecInfo(InApplyDamageSpec);
	PrintDamageResultInfo(InCombatSignalSourceResult);
	FLog::Log(TEXT("/////////////////////////////////"));
}

void UCCombatSignalSourceComponent::PrintCombatSignalSourceRejectedSummaryInfo(const FHitContext& InHitContext, ECombatSignalSourceRejectReason InRejectReason) const
{
	FLog::Log(TEXT("= Combat Signal Source Rejected Summary ="));
	FLog::Log(TEXT("[@ COMBAT SIGNAL SOURCE REJECTED]"));

	FLog::Log(FString::Printf(
		TEXT("RejectReason = %s | DamageCauser = %s | Target = %s | HitWindowId = %d"),
		*UEnum::GetValueAsString(InRejectReason),
		*GetNameSafe(InHitContext.OverlapContext.DamageCauser),
		*GetNameSafe(InHitContext.OverlapContext.OtherActor),
		InHitContext.OverlapContext.HitWindowId
	));
	FLog::Log(TEXT("================================="));
}

void UCCombatSignalSourceComponent::PrintCombatSignalSourceRejectedContextInfo(const FHitContext& InHitContext, ECombatSignalSourceRejectReason InRejectReason) const
{
	FLog::Log(TEXT("////- Combat Signal Source Rejected Context -////"));
	PrintRejectReasonInfo(InRejectReason);
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	PrintHitContextInfo(InHitContext.WeaponContext, InHitContext.ActionContext);
	FLog::Log(TEXT("/////////////////////////////////"));
}

void UCCombatSignalSourceComponent::PrintOverlapContextInfo(const FOverlapContext& InOverlapContext) const
{
	FLog::Log(TEXT("-------- Overlap Context --------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(InOverlapContext.OwnerActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("DamageCauser"), *GetNameSafe(InOverlapContext.DamageCauser)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OverlappedComponent"), *GetNameSafe(InOverlapContext.OverlappedComponent)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OverlapShape"), *GetNameSafe(InOverlapContext.OverlapShape))); // cast result (can be NULL)
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OtherActor"), *GetNameSafe(InOverlapContext.OtherActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OtherComponent"), *GetNameSafe(InOverlapContext.OtherComponent)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("OtherBodyIndex"), InOverlapContext.OtherBodyIndex));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bFromSweep"), InOverlapContext.bFromSweep ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("HitWindowId"), InOverlapContext.HitWindowId));

	if (InOverlapContext.bFromSweep)
	{
		const FHitResult& hitResult = InOverlapContext.SweepResult;

		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.BlockingHit"), hitResult.bBlockingHit ? TEXT("true") : TEXT("false")));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.Actor"), *GetNameSafe(hitResult.GetActor())));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.Component"), *GetNameSafe(hitResult.GetComponent())));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.BoneName"), *hitResult.BoneName.ToString()));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.ImpactPoint"), *hitResult.ImpactPoint.ToString()));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Sweep.ImpactNormal"), *hitResult.ImpactNormal.ToString()));
	}
	FLog::Log(TEXT("---------------------------------"));
}

void UCCombatSignalSourceComponent::PrintHitContextInfo(const FWeaponContext& InWeaponContext, const FActionContext& InActionContext) const
{
	FLog::Log(TEXT("---------- Hit Context ----------"));
	FLog::Log(TEXT("[WeaponContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(InWeaponContext.WeaponType)));

	FLog::Log(TEXT("[ActionContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InActionContext.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Index"), (InActionContext.ActionIndex == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(InActionContext.ActionIndex)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCCombatSignalSourceComponent::PrintDamageSpecInfo(const FDamageSpec& InApplyDamageSpec) const
{
	FLog::Log(TEXT("---------- Damage Spec ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("BaseDamage"), InApplyDamageSpec.BaseDamage));
	FLog::Log(TEXT("---------------------------------"));
}

void UCCombatSignalSourceComponent::PrintDamageResultInfo(const FCombatSignalSourceResult& InCombatSignalSourceResult) const
{
	FLog::Log(TEXT("--------- Damage Result ---------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("BaseDamage"), InCombatSignalSourceResult.BaseDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("RequestDamage"), InCombatSignalSourceResult.RequestDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CommittedDamage"), InCombatSignalSourceResult.CommittedDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bAccepted"), InCombatSignalSourceResult.bAccepted ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("RejectReason"), *UEnum::GetValueAsString(InCombatSignalSourceResult.RejectReason)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCCombatSignalSourceComponent::PrintRejectReasonInfo(ECombatSignalSourceRejectReason InRejectReason) const
{
	FLog::Log(TEXT("--------- Reject Reason ---------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("RejectReason"), *UEnum::GetValueAsString(InRejectReason)));
	FLog::Log(TEXT("---------------------------------"));
}
