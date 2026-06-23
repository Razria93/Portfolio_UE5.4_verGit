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
	FCombatSignalSourcePayload BuildPayload(const FHitContext& InHitContext) const;
	FCombatSignalSourceContext BuildContext(const FCombatSignalSourcePayload& InCombatSignalSourcePayload) const;

private:
	// Resolve
	bool ValidateContext(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const;
	bool CanSendCombatSignal(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const;
	void ResolveSourceDamageSpec(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const;
	void ComputeSourceDamage(FCombatSignalSourceContext& InOutCombatSignalSourceContext) const;
	FCombatSignalSourceResult BuildResult(const FCombatSignalSourceContext& InCombatSignalSourceContext) const;

private:
	// Send
	void CommitCombatSignalSource(FCombatSignalSourceContext& InOutCombatSignalSourceContext);
	float SendDamageToTarget(const FCombatSignalSourceContext& InCombatSignalSourceContext) const;

private:
	// Cache
	void CacheDamagedTargetInWindow(const FCombatSignalSourceContext& InCombatSignalSourceContext);

private:
	// Helper
	FApplyDamageHitWindowKey BuildHitWindowKey(const FHitContext& InHitContext) const;
	FApplyDamageSpecKey BuildSpecKey(const FHitContext& InHitContext) const;
	AController* ResolveInstigatorController(AActor* InAttacker, AActor* InDamageCauser) const;
	bool IsDuplicateHit(const FCombatSignalSourceContext& InCombatSignalSourceContext) const;
	bool IsFriendlyTarget(const FCombatSignalSourceContext& InCombatSignalSourceContext) const;

private:
	// Debug
	void PrintCombatSignalSourceSummaryInfo(const FHitContext& InHitContext, const FCombatSignalSourceResult& InCombatSignalSourceResult) const;
	void PrintCombatSignalSourceContextInfo(const FHitContext& InHitContext, const FApplyDamageSpec& InApplyDamageSpec, const FCombatSignalSourceResult& InCombatSignalSourceResult) const;
	void PrintCombatSignalSourceRejectedSummaryInfo(const FHitContext& InHitContext, ECombatSignalSourceRejectReason InRejectReason) const;
	void PrintCombatSignalSourceRejectedContextInfo(const FHitContext& InHitContext, ECombatSignalSourceRejectReason InRejectReason) const;

	void PrintOverlapContextInfo(const FOverlapContext& InOverlapContext) const;
	void PrintHitContextInfo(const FWeaponContext& InWeaponContext, const FActionContext& InActionContext) const;
	void PrintDamageSpecInfo(const FApplyDamageSpec& InApplyDamageSpec) const;
	void PrintDamageResultInfo(const FCombatSignalSourceResult& InCombatSignalSourceResult) const;
	void PrintRejectReasonInfo(ECombatSignalSourceRejectReason InRejectReason) const;
};
