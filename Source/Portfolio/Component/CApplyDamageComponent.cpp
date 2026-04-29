#include "Component/CApplyDamageComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

#include "Type/CWeaponStructure.h"

UCApplyDamageComponent::UCApplyDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCApplyDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);
}

void UCApplyDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCApplyDamageComponent::NotifyHitWindowOpened(AActor* InDamageCauser, int32 InHitWindowId)
{
	FLog::Log(FString::Printf(TEXT("%-20s: %s = %d"), TEXT("[UCApplyDamageComponent|NotifyHitWindowOpened]"), TEXT("InHitWindowId"), InHitWindowId));

	if (!IsValid(InDamageCauser)) return;
	if (InHitWindowId == INDEX_NONE) return;

	FApplyDamageHitWindowKey applyDamageHitWindowKey;
	applyDamageHitWindowKey.DamageCauser = InDamageCauser;
	applyDamageHitWindowKey.HitWindowId = InHitWindowId;

	// Reset stale record for the same hit window key
	DamagedTargetContainer.Remove(applyDamageHitWindowKey);
	DamagedTargetContainer.FindOrAdd(applyDamageHitWindowKey);
}

void UCApplyDamageComponent::NotifyHitWindowClosed(AActor* InDamageCauser, int32 InHitWindowId)
{
	FLog::Log(FString::Printf(TEXT("%-20s: %s = %d"), TEXT("[UCApplyDamageComponent|NotifyHitWindowClosed]"), TEXT("InHitWindowId"), InHitWindowId));

	if (!IsValid(InDamageCauser)) return;
	if (InHitWindowId == INDEX_NONE) return;

	FApplyDamageHitWindowKey applyDamageHitWindowKey;
	applyDamageHitWindowKey.DamageCauser = InDamageCauser;
	applyDamageHitWindowKey.HitWindowId = InHitWindowId;

	DamagedTargetContainer.Remove(applyDamageHitWindowKey);
}

void UCApplyDamageComponent::RequestApplyDamage(const FHitContext& InHitContext)
{
	ProcessApplyDamage(InHitContext);
}

void UCApplyDamageComponent::ProcessApplyDamage(const FHitContext& InHitContext)
{
	if (!ValidateRequest(InHitContext))
	{
		PrintApplyDamageRejectedSummaryInfo(InHitContext, EApplyDamageRejectReason::InvalidRequest);
		return;
	}

	FApplyDamagePayload applyDamagePayload = BuildPayload(InHitContext);
	FApplyDamageContext applyDamageContext = BuildContext(applyDamagePayload);

	if (!ValidateContext(applyDamageContext))
	{
		const FApplyDamageResult applyDamageResult = BuildResult(applyDamageContext);
		PrintApplyDamageRejectedSummaryInfo(applyDamageContext.HitContext, applyDamageResult.RejectReason);
		return;
	}

	if (!CanApplyDamage(applyDamageContext))
	{
		const FApplyDamageResult applyDamageResult = BuildResult(applyDamageContext);
		PrintApplyDamageRejectedSummaryInfo(applyDamageContext.HitContext, applyDamageResult.RejectReason);
		return;
	}

	ResolveApplyDamageSpec(applyDamageContext);
	if (!applyDamageContext.bAccepted)
	{
		const FApplyDamageResult applyDamageResult = BuildResult(applyDamageContext);
		PrintApplyDamageRejectedSummaryInfo(applyDamageContext.HitContext, applyDamageResult.RejectReason);
		return;
	}

	ComputeApplyDamage(applyDamageContext);
	if (!applyDamageContext.bAccepted)
	{
		const FApplyDamageResult applyDamageResult = BuildResult(applyDamageContext);
		PrintApplyDamageRejectedSummaryInfo(applyDamageContext.HitContext, applyDamageResult.RejectReason);
		return;
	}

	CommitApplyDamage(applyDamageContext);
	if (!applyDamageContext.bAccepted)
	{
		const FApplyDamageResult applyDamageResult = BuildResult(applyDamageContext);
		PrintApplyDamageRejectedSummaryInfo(applyDamageContext.HitContext, applyDamageResult.RejectReason);
		return;
	}

	const FApplyDamageResult applyDamageResult = BuildResult(applyDamageContext);
	PrintApplyDamageSummaryInfo(applyDamageContext.HitContext, applyDamageResult);
}


bool UCApplyDamageComponent::ValidateRequest(const FHitContext& InHitContext) const
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

bool UCApplyDamageComponent::ValidateContext(FApplyDamageContext& InOutApplyDamageContext) const
{
	if (!IsValid(InOutApplyDamageContext.SourceActor))
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::InvalidAttacker;
		return false;
	}

	if (!IsValid(InOutApplyDamageContext.DamageCauser))
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::InvalidDamageCauser;
		return false;
	}

	if (!IsValid(InOutApplyDamageContext.TargetActor))
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::InvalidTarget;
		return false;
	}

	if (!IsValid(InOutApplyDamageContext.Instigator))
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::InvalidInstigator;
		return false;
	}

	// Valid Context
	InOutApplyDamageContext.bAccepted = true;
	InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::None;
	return true;
}

bool UCApplyDamageComponent::CanApplyDamage(FApplyDamageContext& InOutApplyDamageContext) const
{
	const FOverlapContext& overlapContext = InOutApplyDamageContext.HitContext.OverlapContext;

	AActor* myOwner = GetOwner();
	if (!IsValid(myOwner) || myOwner != overlapContext.OwnerActor)
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::InvalidOwner;
		return false;
	}

	if (overlapContext.OtherActor == overlapContext.OwnerActor)
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::SelfTarget;
		return false;
	}

	if (IsDuplicateHit(InOutApplyDamageContext))
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::DuplicateHitInWindow;
		return false;
	}

	if (IsFriendlyTarget(InOutApplyDamageContext))
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::FriendlyTarget;
		return false;
	}

	InOutApplyDamageContext.bAccepted = true;
	InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::None;
	return true;
}

void UCApplyDamageComponent::ResolveApplyDamageSpec(FApplyDamageContext& InOutApplyDamageContext) const
{
	const FApplyDamageSpec* foundApplyDamageSpec = ApplyDamageSpecContainer.Find(InOutApplyDamageContext.ApplyDamageSpecKey);

	if (!foundApplyDamageSpec)
	{
		InOutApplyDamageContext.ApplyDamageSpec = FApplyDamageSpec();
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::SpecNotFound;
		return;
	}

	InOutApplyDamageContext.ApplyDamageSpec = *foundApplyDamageSpec;
	InOutApplyDamageContext.bAccepted = true;
	InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::None;
}

void UCApplyDamageComponent::ComputeApplyDamage(FApplyDamageContext& InOutApplyDamageContext) const
{
	if (!IsValid(InOutApplyDamageContext.SourceActor) || !IsValid(InOutApplyDamageContext.DamageCauser) || !IsValid(InOutApplyDamageContext.TargetActor))
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::ComputeFailed;
		return;
	}

	// [NOTE] Minimal sender-side request damage.
	InOutApplyDamageContext.ApplyDamageAmount = FApplyDamageAmount();
	InOutApplyDamageContext.ApplyDamageAmount.RequestDamage = InOutApplyDamageContext.ApplyDamageSpec.BaseDamage;

	InOutApplyDamageContext.bAccepted = true;
	InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::None;
}

void UCApplyDamageComponent::CommitApplyDamage(FApplyDamageContext& InOutApplyDamageContext)
{
	InOutApplyDamageContext.CommittedDamage = ApplyDamageToTarget(InOutApplyDamageContext);

	if (InOutApplyDamageContext.CommittedDamage <= 0.f)
	{
		InOutApplyDamageContext.bAccepted = false;
		InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::CommitFailed;
		return;
	}

	InOutApplyDamageContext.bAccepted = true;
	InOutApplyDamageContext.RejectReason = EApplyDamageRejectReason::None;

	CacheDamagedTargetInWindow(InOutApplyDamageContext);
}


float UCApplyDamageComponent::ApplyDamageToTarget(const FApplyDamageContext& InApplyDamageContext) const
{
	if (!IsValid(InApplyDamageContext.TargetActor) || !IsValid(InApplyDamageContext.DamageCauser) || !IsValid(InApplyDamageContext.Instigator))
		return 0.f;

	FDefaultDamageEvent damageEvent;
	damageEvent.SourceActor = InApplyDamageContext.SourceActor;
	damageEvent.TargetActor = InApplyDamageContext.TargetActor;
	damageEvent.ApplyDamageSpecKey = InApplyDamageContext.ApplyDamageSpecKey;
	damageEvent.ApplyDamageSpec = InApplyDamageContext.ApplyDamageSpec;
	damageEvent.ApplyDamageAmount = InApplyDamageContext.ApplyDamageAmount;

	return InApplyDamageContext.TargetActor->TakeDamage(InApplyDamageContext.ApplyDamageAmount.RequestDamage, damageEvent, InApplyDamageContext.Instigator, InApplyDamageContext.DamageCauser);
}

bool UCApplyDamageComponent::IsDuplicateHit(const FApplyDamageContext& InApplyDamageContext) const
{
	const TSet<AActor*>* foundTargets = DamagedTargetContainer.Find(InApplyDamageContext.HitWindowKey);

	if (!foundTargets) return false;

	return foundTargets->Contains(InApplyDamageContext.TargetActor);
}

bool UCApplyDamageComponent::IsFriendlyTarget(const FApplyDamageContext& InApplyDamageContext) const
{
	AActor* ownerActor = InApplyDamageContext.SourceActor;
	AActor* targetActor = InApplyDamageContext.TargetActor;

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

FApplyDamageHitWindowKey UCApplyDamageComponent::BuildHitWindowKey(const FHitContext& InHitContext) const
{
	FApplyDamageHitWindowKey applyDamageWindowKey;

	applyDamageWindowKey.DamageCauser = InHitContext.OverlapContext.DamageCauser;
	applyDamageWindowKey.HitWindowId = InHitContext.OverlapContext.HitWindowId;

	return applyDamageWindowKey;
}

FApplyDamageSpecKey UCApplyDamageComponent::BuildSpecKey(const FHitContext& InHitContext) const
{
	FApplyDamageSpecKey applyDamageSpecKey;

	applyDamageSpecKey.WeaponType = InHitContext.WeaponContext.WeaponType;
	applyDamageSpecKey.ActionType = InHitContext.ActionContext.ActionType;
	applyDamageSpecKey.ActionIndex = InHitContext.ActionContext.ActionIndex;

	return applyDamageSpecKey;
}

FApplyDamagePayload UCApplyDamageComponent::BuildPayload(const FHitContext& InHitContext) const
{
	FApplyDamagePayload applyDamagePayload;

	applyDamagePayload.HitContext = InHitContext;
	applyDamagePayload.SourceActor = InHitContext.OverlapContext.OwnerActor;
	applyDamagePayload.DamageCauser = InHitContext.OverlapContext.DamageCauser;
	applyDamagePayload.TargetActor = InHitContext.OverlapContext.OtherActor;
	applyDamagePayload.HitWindowKey = BuildHitWindowKey(InHitContext);
	applyDamagePayload.ApplyDamageSpecKey = BuildSpecKey(InHitContext);

	return applyDamagePayload;
}

FApplyDamageContext UCApplyDamageComponent::BuildContext(const FApplyDamagePayload& InApplyDamagePayload) const
{
	FApplyDamageContext applyDamageContext;

	applyDamageContext.HitContext = InApplyDamagePayload.HitContext;
	applyDamageContext.SourceActor = InApplyDamagePayload.SourceActor;
	applyDamageContext.DamageCauser = InApplyDamagePayload.DamageCauser;
	applyDamageContext.TargetActor = InApplyDamagePayload.TargetActor;
	applyDamageContext.HitWindowKey = InApplyDamagePayload.HitWindowKey;
	applyDamageContext.ApplyDamageSpecKey = InApplyDamagePayload.ApplyDamageSpecKey;
	applyDamageContext.Instigator = ResolveInstigatorController(InApplyDamagePayload.SourceActor, InApplyDamagePayload.DamageCauser);

	return applyDamageContext;
}

FApplyDamageResult UCApplyDamageComponent::BuildResult(const FApplyDamageContext& InApplyDamageContext) const
{
	FApplyDamageResult applyDamageResult;

	applyDamageResult.bAccepted = InApplyDamageContext.bAccepted;
	applyDamageResult.RejectReason = InApplyDamageContext.RejectReason;
	applyDamageResult.HitWindowKey = InApplyDamageContext.HitWindowKey;
	applyDamageResult.ApplyDamageSpecKey = InApplyDamageContext.ApplyDamageSpecKey;
	applyDamageResult.BaseDamage = InApplyDamageContext.ApplyDamageSpec.BaseDamage;
	applyDamageResult.RequestDamage = InApplyDamageContext.ApplyDamageAmount.RequestDamage;
	applyDamageResult.CommittedDamage = InApplyDamageContext.CommittedDamage;

	return applyDamageResult;
}

AController* UCApplyDamageComponent::ResolveInstigatorController(AActor* InAttacker, AActor* InDamageCauser) const
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

void UCApplyDamageComponent::CacheDamagedTargetInWindow(const FApplyDamageContext& InApplyDamageContext)
{
	AActor* targetActor = InApplyDamageContext.TargetActor;
	if (!IsValid(targetActor)) return;

	// Cached
	auto& damagedTargets = DamagedTargetContainer.FindOrAdd(InApplyDamageContext.HitWindowKey);
	damagedTargets.Add(targetActor);
}

void UCApplyDamageComponent::PrintApplyDamageSummaryInfo(const FHitContext& InHitContext, const FApplyDamageResult& InApplyDamageResult) const
{
	FLog::Log(TEXT("===== Apply Damage Summary ======"));
	FLog::Log(TEXT("[@ APPLY DAMAGE]"));

	FLog::Log(FString::Printf(
		TEXT("DamageCauser = %s | Target = %s | HitWindowId = %d | Base = %.3f | Request = %.3f | Committed = %.3f"),
		*GetNameSafe(InHitContext.OverlapContext.DamageCauser),
		*GetNameSafe(InHitContext.OverlapContext.OtherActor),
		InHitContext.OverlapContext.HitWindowId,
		InApplyDamageResult.BaseDamage,
		InApplyDamageResult.RequestDamage,
		InApplyDamageResult.CommittedDamage
	));
	FLog::Log(TEXT("================================="));
}

void UCApplyDamageComponent::PrintApplyDamageContextInfo(const FHitContext& InHitContext, const FApplyDamageSpec& InApplyDamageSpec, const FApplyDamageResult& InApplyDamageResult) const
{
	FLog::Log(TEXT("////- Apply Damage Context -/////"));
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	PrintHitContextInfo(InHitContext.WeaponContext, InHitContext.ActionContext);
	PrintDamageSpecInfo(InApplyDamageSpec);
	PrintDamageResultInfo(InApplyDamageResult);
	FLog::Log(TEXT("/////////////////////////////////"));
}

void UCApplyDamageComponent::PrintApplyDamageRejectedSummaryInfo(const FHitContext& InHitContext, EApplyDamageRejectReason InRejectReason) const
{
	FLog::Log(TEXT("= Apply Damage Rejected Summary ="));
	FLog::Log(TEXT("[@ REJECT DAMAGE]"));

	FLog::Log(FString::Printf(
		TEXT("RejectReason = %s | DamageCauser = %s | Target = %s | HitWindowId = %d"),
		*UEnum::GetValueAsString(InRejectReason),
		*GetNameSafe(InHitContext.OverlapContext.DamageCauser),
		*GetNameSafe(InHitContext.OverlapContext.OtherActor),
		InHitContext.OverlapContext.HitWindowId
	));
	FLog::Log(TEXT("================================="));
}

void UCApplyDamageComponent::PrintApplyDamageRejectedContextInfo(const FHitContext& InHitContext, EApplyDamageRejectReason InRejectReason) const
{
	FLog::Log(TEXT("////- Reject Damage Context -////"));
	PrintRejectReasonInfo(InRejectReason);
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	PrintHitContextInfo(InHitContext.WeaponContext, InHitContext.ActionContext);
	FLog::Log(TEXT("/////////////////////////////////"));
}

void UCApplyDamageComponent::PrintOverlapContextInfo(const FOverlapContext& InOverlapContext) const
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

void UCApplyDamageComponent::PrintHitContextInfo(const FWeaponContext& InWeaponContext, const FActionContext& InActionContext) const
{
	FLog::Log(TEXT("---------- Hit Context ----------"));
	FLog::Log(TEXT("[WeaponContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("WeaponType"), *UEnum::GetValueAsString(InWeaponContext.WeaponType)));

	FLog::Log(TEXT("[ActionContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InActionContext.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Index"), (InActionContext.ActionIndex == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(InActionContext.ActionIndex)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCApplyDamageComponent::PrintDamageSpecInfo(const FApplyDamageSpec& InApplyDamageSpec) const
{
	FLog::Log(TEXT("---------- Damage Spec ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("BaseDamage"), InApplyDamageSpec.BaseDamage));
	FLog::Log(TEXT("---------------------------------"));
}

void UCApplyDamageComponent::PrintDamageResultInfo(const FApplyDamageResult& InApplyDamageResult) const
{
	FLog::Log(TEXT("--------- Damage Result ---------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("BaseDamage"), InApplyDamageResult.BaseDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("RequestDamage"), InApplyDamageResult.RequestDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("CommittedDamage"), InApplyDamageResult.CommittedDamage));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bAccepted"), InApplyDamageResult.bAccepted ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("RejectReason"), *UEnum::GetValueAsString(InApplyDamageResult.RejectReason)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCApplyDamageComponent::PrintRejectReasonInfo(EApplyDamageRejectReason InRejectReason) const
{
	FLog::Log(TEXT("--------- Reject Reason ---------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("RejectReason"), *UEnum::GetValueAsString(InRejectReason)));
	FLog::Log(TEXT("---------------------------------"));
}
