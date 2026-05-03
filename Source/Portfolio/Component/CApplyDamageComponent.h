#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CApplyDamageComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCApplyDamageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCApplyDamageComponent();

private:
	UPROPERTY(EditAnywhere)
	TMap<FApplyDamageSpecKey, FApplyDamageSpec> ApplyDamageSpecContainer;	// TODO: Seperate DataAsset (DB)

private:
	TMap<FApplyDamageHitWindowKey, TSet<AActor*>> DamagedTargetContainer;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached;

protected:
	void BeginPlay() override;

public:
	// HitWindow API
	void NotifyHitWindowOpened(AActor* InDamageCauser, int32 InHitWindowId);
	void NotifyHitWindowClosed(AActor* InDamageCauser, int32 InHitWindowId);

public:
	// Entry API
	void RequestApplyDamage(const FHitContext& InHitContext);

private:
	// Pipeline
	void ProcessApplyDamage(const FHitContext& InHitContext);

private:
	bool ValidateRequest(const FHitContext& InHitContext) const;
	bool ValidateContext(FApplyDamageContext & InOutApplyDamageContext) const;
	bool CanApplyDamage(FApplyDamageContext & InOutApplyDamageContext) const;
	void ResolveApplyDamageSpec(FApplyDamageContext & InOutApplyDamageContext) const;
	void ComputeApplyDamage(FApplyDamageContext & InOutApplyDamageContext) const;
	void CommitApplyDamage(FApplyDamageContext & InOutApplyDamageContext);

private:
	float ApplyDamageToTarget(const FApplyDamageContext& InApplyDamageContext) const;

private:
	bool IsDuplicateHit(const FApplyDamageContext & InApplyDamageContext) const;
	bool IsFriendlyTarget(const FApplyDamageContext & InApplyDamageContext) const;

private:
	FApplyDamageHitWindowKey BuildHitWindowKey(const FHitContext& InHitContext) const;
	FApplyDamageSpecKey BuildSpecKey(const FHitContext& InHitContext) const;
	FApplyDamagePayload BuildPayload(const FHitContext & InHitContext) const;
	FApplyDamageContext BuildContext(const FApplyDamagePayload & InApplyDamagePayload) const;
	FApplyDamageResult BuildResult(const FApplyDamageContext& InApplyDamageContext) const;

private:
	AController* ResolveInstigatorController(AActor* InAttacker, AActor* InDamageCauser) const;

private:
	void CacheDamagedTargetInWindow(const FApplyDamageContext& InApplyDamageContext);

private:
	void PrintApplyDamageSummaryInfo(const FHitContext& InHitContext, const FApplyDamageResult& InApplyDamageResult) const;
	void PrintApplyDamageContextInfo(const FHitContext& InHitContext, const FApplyDamageSpec& InApplyDamageSpec, const FApplyDamageResult& InApplyDamageResult) const;
	void PrintApplyDamageRejectedSummaryInfo(const FHitContext& InHitContext, EApplyDamageRejectReason InRejectReason) const;
	void PrintApplyDamageRejectedContextInfo(const FHitContext& InHitContext, EApplyDamageRejectReason InRejectReason) const;

private:
	void PrintOverlapContextInfo(const FOverlapContext& InOverlapContext) const;
	void PrintHitContextInfo(const FWeaponContext& InWeaponContext, const FActionContext& InActionContext) const;
	void PrintDamageSpecInfo(const FApplyDamageSpec& InApplyDamageSpec) const;
	void PrintDamageResultInfo(const FApplyDamageResult& InApplyDamageResult) const;
	void PrintRejectReasonInfo(EApplyDamageRejectReason InRejectReason) const;
};
