#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CCombatSignalSourceComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCCombatSignalSourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCombatSignalSourceComponent();

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
	void RequestCombatSignalSource(const FHitContext& InHitContext);

private:
	void ProcessCombatSignalSource(const FHitContext& InHitContext);

private:
	// Receive
	bool ValidateRequest(const FHitContext& InHitContext) const;
	FApplyDamagePayload BuildPayload(const FHitContext& InHitContext) const;
	FApplyDamageContext BuildContext(const FApplyDamagePayload& InApplyDamagePayload) const;

private:
	// Resolve
	bool ValidateContext(FApplyDamageContext& InOutApplyDamageContext) const;
	bool CanSendCombatSignal(FApplyDamageContext& InOutApplyDamageContext) const;
	void ResolveSourceDamageSpec(FApplyDamageContext& InOutApplyDamageContext) const;
	void ComputeSourceDamage(FApplyDamageContext& InOutApplyDamageContext) const;
	FApplyDamageResult BuildResult(const FApplyDamageContext& InApplyDamageContext) const;

private:
	// Send
	void CommitCombatSignalSource(FApplyDamageContext& InOutApplyDamageContext);
	float SendDamageToTarget(const FApplyDamageContext& InApplyDamageContext) const;

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
	void PrintCombatSignalSourceSummaryInfo(const FHitContext& InHitContext, const FApplyDamageResult& InApplyDamageResult) const;
	void PrintCombatSignalSourceContextInfo(const FHitContext& InHitContext, const FApplyDamageSpec& InApplyDamageSpec, const FApplyDamageResult& InApplyDamageResult) const;
	void PrintCombatSignalSourceRejectedSummaryInfo(const FHitContext& InHitContext, EApplyDamageRejectReason InRejectReason) const;
	void PrintCombatSignalSourceRejectedContextInfo(const FHitContext& InHitContext, EApplyDamageRejectReason InRejectReason) const;

	void PrintOverlapContextInfo(const FOverlapContext& InOverlapContext) const;
	void PrintHitContextInfo(const FWeaponContext& InWeaponContext, const FActionContext& InActionContext) const;
	void PrintDamageSpecInfo(const FApplyDamageSpec& InApplyDamageSpec) const;
	void PrintDamageResultInfo(const FApplyDamageResult& InApplyDamageResult) const;
	void PrintRejectReasonInfo(EApplyDamageRejectReason InRejectReason) const;
};
