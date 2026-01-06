#include "Component/CApplyDamageComponent.h"
#include "ProjectGlobal.h"

#include "Components/ShapeComponent.h"

#include "Type/CWeaponStructure.h"

UCApplyDamageComponent::UCApplyDamageComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCApplyDamageComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCApplyDamageComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCApplyDamageComponent::RequestApplyDamage(const FHitContext& InHitContext)
{
	ProcessApplyDamage(InHitContext);
}

void UCApplyDamageComponent::RequestStopDamage(const FHitContext& InHitContext)
{
	// TODO Example:
	// 1) Remove target from active overlap / active damage set
	// 2) Cancel timers for repeated hits (DoT)
	// 3) Remove sustained effects (slow, burning, etc.) if they depend on overlap
}

void UCApplyDamageComponent::ProcessApplyDamage(const FHitContext& InHitContext)
{
	// 1) Check Valid of Request
	if (!ValidateRequest(InHitContext)) return;

	// 2) Check Hit Rules
	if (!CheckHitRule(InHitContext)) return;

	// 3) Resolve DamageSpec
	FDamageSpec damageSpec = FDamageSpec();
	if (!ResolveDamageSpec(InHitContext, damageSpec)) return;

	// 4) Compute DamageResult
	FDamageResult damageResult = FDamageResult();
	if (!ComputeDamageResult(InHitContext, damageSpec, damageResult)) return;

	// 5) Apply Damage (Call Target->TakeDamage)
	if (!ApplyDamageToTarget(InHitContext, damageSpec, damageResult)) return;

	// Debugging
	PrintApplyDamageContextInfo(InHitContext, damageSpec, damageResult);
}

bool UCApplyDamageComponent::ValidateRequest(const FHitContext& InHitContext) const
{
	const FOverlapContext& overlapContext = InHitContext.OverlapContext;

	// V0: Validate Minimal actors (OwnerActor / DamageCauser / OtherActor)
	if (!overlapContext.IsValidMinimal()) return false;

	// V1: Request owner must match this component owner
	AActor* myOwner = GetOwner();
	if (!IsValid(myOwner) || myOwner != overlapContext.OwnerActor) return false;

	// V2: Prevent self-hit
	if (overlapContext.OtherActor == overlapContext.OwnerActor)
		return false;

	// V3: Check Valid
	// 3-1): Validate Components (current policy)
	if (!IsValid(overlapContext.OverlappedComponent) || !IsValid(overlapContext.OtherComponent))
		return false;

	// 3-2): Attack collision must be ShapeComponent (current policy)
	if (!IsValid(overlapContext.OverlapShape)) return false;

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

bool UCApplyDamageComponent::CheckHitRule(const FHitContext& InDamageRequestData) const
{
	// TODO:
	// P1: CheckAlreadyHit
	// P2: CheckTeam

	return true;
}

bool UCApplyDamageComponent::ResolveDamageSpec(const FHitContext& InHitContext, FDamageSpec& OutDamageSpec) const
{
	const FDamageSpecKey damageSpectKey = BuildSpecKey(InHitContext);

	if (const FDamageSpec* foundDamageSpec = DamageSpecMap.Find(damageSpectKey))
	{
		OutDamageSpec = FDamageSpec();
		OutDamageSpec = *foundDamageSpec;
		return true;
	}

	return false;
}

bool UCApplyDamageComponent::ComputeDamageResult(const FHitContext& InHitContext, const FDamageSpec& InDamageSpec, FDamageResult& OutDamageResult) const
{
	const FOverlapContext& overlapContext = InHitContext.OverlapContext;

	AActor* attacker = overlapContext.OwnerActor;
	AActor* damageCauser = overlapContext.DamageCauser;
	AActor* target = overlapContext.OtherActor;

	if (!IsValid(attacker) || !IsValid(damageCauser) || !IsValid(target))
		return false;

	// ComputeDamage (Minimal, [TODO] Implement `ComputeDamage()`)
	OutDamageResult = FDamageResult();
	OutDamageResult.FinalDamage = InDamageSpec.BaseDamage;

	return true;
}

bool UCApplyDamageComponent::ApplyDamageToTarget(const FHitContext& InHitContext, const FDamageSpec& InDamageSpec, const FDamageResult& InDamageResult) const
{
	AActor* attacker = InHitContext.OverlapContext.OwnerActor;
	AActor* damageCauser = InHitContext.OverlapContext.DamageCauser;
	AActor* target = InHitContext.OverlapContext.OtherActor;

	if (!IsValid(attacker) || !IsValid(damageCauser) || !IsValid(target)) return false;

	AController* instigatorController = attacker->GetInstigatorController();
	if (!IsValid(instigatorController))
	{
		// Fallback: if DamageCauser has no valid InstigatorController, derive it from Attacker
		if (APawn* Pawn = Cast<APawn>(attacker))
			instigatorController = Pawn->GetController();
	}
	if (!IsValid(instigatorController)) return false;

	FDamageSpecKey damageSpectKey = BuildSpecKey(InHitContext);

	FDefaultDamageEvent damageEvent;
	damageEvent.DamageSpecKey = damageSpectKey;
	damageEvent.DamageSpec = InDamageSpec;
	damageEvent.DamageResult = InDamageResult;

	const float appliedDamage = target->TakeDamage(InDamageResult.FinalDamage, damageEvent, instigatorController, damageCauser);

	FLog::Log("[@ APPLY DAMAGE]");
	FLog::Log(FString::Printf(TEXT("Target = %s | Request = %.3f | Apply = %.3f"), *GetNameSafe(target), InDamageResult.FinalDamage, appliedDamage));

	return true;
}

FDamageSpecKey UCApplyDamageComponent::BuildSpecKey(const FHitContext& InHitContext) const
{
	FDamageSpecKey damageSpecKey;

	damageSpecKey.AttachmentType = InHitContext.AttachmentContext.CurrentAttachmentType;
	damageSpecKey.EquipmentType = InHitContext.EquipmentContext.CurrentEquipmentType;
	damageSpecKey.ActionType = InHitContext.ActionContext.CurrentActionType;
	damageSpecKey.ActionIndex = InHitContext.ActionContext.ActionIndex;

	return damageSpecKey;
}

void UCApplyDamageComponent::PrintApplyDamageContextInfo(const FHitContext& InHitContext, const FDamageSpec& InDamageSpec, const FDamageResult& InDamageResult)
{
	FLog::Log(TEXT("========= Apply Damage =========="));
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	PrintHitContextInfo(InHitContext.AttachmentContext, InHitContext.EquipmentContext, InHitContext.ActionContext);
	PrintDamageSpecInfo(InDamageSpec);
	PrintDamageResultInfo(InDamageResult);
	FLog::Log(TEXT("================================="));
}

void UCApplyDamageComponent::PrintOverlapContextInfo(const FOverlapContext& InOverlapContext)
{
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(InOverlapContext.OwnerActor)));

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("DamageCauser"), *GetNameSafe(InOverlapContext.DamageCauser)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OverlappedComponent"), *GetNameSafe(InOverlapContext.OverlappedComponent)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OverlapShape"), *GetNameSafe(InOverlapContext.OverlapShape))); // cast result (can be NULL)
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OtherActor"), *GetNameSafe(InOverlapContext.OtherActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OtherComponent"), *GetNameSafe(InOverlapContext.OtherComponent)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("OtherBodyIndex"), InOverlapContext.OtherBodyIndex));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bFromSweep"), InOverlapContext.bFromSweep ? TEXT("true") : TEXT("false")));

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
}

void UCApplyDamageComponent::PrintHitContextInfo(const FAttachmentContext& InAttachmentContext, const FEquipmentContext& InEquipmentContext, const FActionContext& InActionContext)
{
	FLog::Log(TEXT("---------- Hit Context ----------"));
	FLog::Log(TEXT("[AttachmentContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CurrentAttachmentType"), *UEnum::GetValueAsString(InAttachmentContext.CurrentAttachmentType)));

	FLog::Log(TEXT("[EquipmentContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CurrentEquipmentType"), *UEnum::GetValueAsString(InEquipmentContext.CurrentEquipmentType)));

	FLog::Log(TEXT("[ActionContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CurrentActionType"), *UEnum::GetValueAsString(InActionContext.CurrentActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Index"), (InActionContext.ActionIndex == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(InActionContext.ActionIndex)));
	FLog::Log(TEXT("---------------------------------"));
}

void UCApplyDamageComponent::PrintDamageSpecInfo(const FDamageSpec& InDamageSpec)
{
	FLog::Log(TEXT("---------- Damage Spec ----------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("BaseDamage"), InDamageSpec.BaseDamage));
	FLog::Log(TEXT("---------------------------------"));
}

void UCApplyDamageComponent::PrintDamageResultInfo(const FDamageResult& InDamageResult)
{
	FLog::Log(TEXT("--------- Damage Result ---------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("FinalDamage"), InDamageResult.FinalDamage));
	FLog::Log(TEXT("---------------------------------"));
}