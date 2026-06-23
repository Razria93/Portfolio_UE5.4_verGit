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
	// HitWindow
	void NotifyHitWindowOpened(AActor* InDamageCauser, int32 InHitWindowId);
	void NotifyHitWindowClosed(AActor* InDamageCauser, int32 InHitWindowId);

public:
	// Entry
	void RequestApplyDamage(const FHitContext& InHitContext);

private:
	void ProcessApplyDamage(const FHitContext& InHitContext);

private:
	// Receive
	bool ValidateRequest(const FHitContext& InHitContext) const;
	FApplyDamagePayload BuildPayload(const FHitContext& InHitContext) const;
	FApplyDamageContext BuildContext(const FApplyDamagePayload& InApplyDamagePayload) const;

private:
	// Resolve
	bool ValidateContext(FApplyDamageContext& InOutApplyDamageContext) const;
	bool CanApplyDamage(FApplyDamageContext& InOutApplyDamageContext) const;
	void ResolveApplyDamageSpec(FApplyDamageContext& InOutApplyDamageContext) const;
	void ComputeApplyDamage(FApplyDamageContext& InOutApplyDamageContext) const;
	FApplyDamageResult BuildResult(const FApplyDamageContext& InApplyDamageContext) const;

private:
	// Send
	void CommitApplyDamage(FApplyDamageContext& InOutApplyDamageContext);
	float ApplyDamageToTarget(const FApplyDamageContext& InApplyDamageContext) const;

private:
	// Cache
	void CacheDamagedTargetInWindow(const FApplyDamageContext& InApplyDamageContext);

private:
	// Helper
	FApplyDamageHitWindowKey BuildHitWindowKey(const FHitContext& InHitContext) const;
	FApplyDamageSpecKey BuildSpecKey(const FHitContext& InHitContext) const;
	AController* ResolveInstigatorController(AActor* InAttacker, AActor* InDamageCauser) const;
	bool IsDuplicateHit(const FApplyDamageContext& InApplyDamageContext) const;
	bool IsFriendlyTarget(const FApplyDamageContext& InApplyDamageContext) const;

private:
	// Debug
	void PrintApplyDamageSummaryInfo(const FHitContext& InHitContext, const FApplyDamageResult& InApplyDamageResult) const;
	void PrintApplyDamageContextInfo(const FHitContext& InHitContext, const FApplyDamageSpec& InApplyDamageSpec, const FApplyDamageResult& InApplyDamageResult) const;
	void PrintApplyDamageRejectedSummaryInfo(const FHitContext& InHitContext, EApplyDamageRejectReason InRejectReason) const;
	void PrintApplyDamageRejectedContextInfo(const FHitContext& InHitContext, EApplyDamageRejectReason InRejectReason) const;

	void PrintOverlapContextInfo(const FOverlapContext& InOverlapContext) const;
	void PrintHitContextInfo(const FWeaponContext& InWeaponContext, const FActionContext& InActionContext) const;
	void PrintDamageSpecInfo(const FApplyDamageSpec& InApplyDamageSpec) const;
	void PrintDamageResultInfo(const FApplyDamageResult& InApplyDamageResult) const;
	void PrintRejectReasonInfo(EApplyDamageRejectReason InRejectReason) const;
};
