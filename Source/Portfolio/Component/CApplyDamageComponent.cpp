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
	if (!ValidateRequest(InHitContext)) return;
	if (!CheckHitRule(InHitContext)) return;

	PrintApplyDamageContextInfo(InHitContext);
}

void UCApplyDamageComponent::RequestStopDamage(const FHitContext& InHitContext)
{
	// TODO Example:
	// 1) Remove target from active overlap / active damage set
	// 2) Cancel timers for repeated hits (DoT)
	// 3) Remove sustained effects (slow, burning, etc.) if they depend on overlap
}

bool UCApplyDamageComponent::ValidateRequest(const FHitContext& InHitContext) const
{
	const FOverlapContext& overlapContext = InHitContext.OverlapContext;

	// V0: Validate Minimal actors (OwnerActor / DamageCauser / OtherActor / OverlapShape)
	if (!overlapContext.IsValidMinimal()) return false;

	// V0: Request owner must match this component owner
	AActor* myOwner = GetOwner();
	if (!IsValid(myOwner) || myOwner != overlapContext.OwnerActor) return false;

	// V2: Prevent self-hit
	if (overlapContext.OtherActor == overlapContext.OwnerActor)
		return false;

	// V3: Validate Components (current policy)
	if (!IsValid(overlapContext.OverlappedComponent) || !IsValid(overlapContext.OtherComponent))
		return false;

	// Attack collision must be ShapeComponent (current policy)
	if (!IsValid(overlapContext.OverlapShape)) return false;


	// V3: Check ownership
	 // 3-1) DamageCauser must be owned by the attacker
	if (overlapContext.DamageCauser->GetOwner() != overlapContext.OwnerActor)
		return false;

	// 3-2) OverlappedComponent must belong to the DamageCauser
	if (overlapContext.OverlappedComponent->GetOwner() != overlapContext.DamageCauser)
		return false;

	// 3-3) OtherComponent must belong to the target actor
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

bool UCApplyDamageComponent::FindDamageSpecData(const FHitContext& InHitContext) const
{
	// TODO:
	return false;
}

void UCApplyDamageComponent::PrintApplyDamageContextInfo(const FHitContext& InHitContext)
{
	FLog::Log(TEXT("========= Apply Damage =========="));
	PrintOverlapContextInfo(InHitContext.OverlapContext);
	Print_HitContextInfo(InHitContext.AttachmentContext, InHitContext.EquipmentContext, InHitContext.ActionContext);
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

	// SweepResult는 bFromSweep일 때만 핵심 정보만 요약 출력
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

void UCApplyDamageComponent::Print_HitContextInfo(const FAttachmentContext& InAttachmentContext, const FEquipmentContext& InEquipmentContext, const FActionContext& InActionContext)
{
	FLog::Log(TEXT("---------- Hit Context ----------"));
	FLog::Log(TEXT("[AttachmentContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CurrentAttachmentType"), *UEnum::GetValueAsString(InAttachmentContext.CurrentAttachmentType)));

	FLog::Log(TEXT("[EquipmentContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CurrentEquipmentType"), *UEnum::GetValueAsString(InEquipmentContext.CurrentEquipmentType)));

	FLog::Log(TEXT("[ActionContext]"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CurrentActionType"), *UEnum::GetValueAsString(InActionContext.CurrentActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Index"), (InActionContext.Index == INDEX_NONE) ? TEXT("NONE") : *FString::FromInt(InActionContext.Index)));
	FLog::Log(TEXT("---------------------------------"));
}
