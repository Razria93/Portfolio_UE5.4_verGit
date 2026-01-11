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
	UPROPERTY(EditAnywhere)
	TMap<FApplyDamageSpecKey, FApplyDamageSpec> DamageSpecMap;	// TODO: Seperate DataAsset (DB)

public:
	UCApplyDamageComponent();

private:
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void RequestApplyDamage(const FHitContext& InHitContext);
	void RequestStopDamage(const FHitContext& InHitContext);

private:
	void ProcessApplyDamage(const FHitContext& InHitContext);

private:
	bool ValidateRequest(const FHitContext& InHitContext) const;
	bool CheckApplyDamageRule(const FHitContext& InHitContext) const;
	bool ResolveApplyDamageSpec(const FHitContext& InHitContext, FApplyDamageSpec & OutApplyDamageSpec) const;
	bool ComputeApplyDamageResult(const FHitContext& InHitContext, const FApplyDamageSpec & InApplyDamageSpec, FApplyDamageResult& OutApplyDamageResult) const;
	bool ApplyDamageToTarget(const FHitContext& InHitContext, const FApplyDamageSpec & InApplyDamageSpec, const FApplyDamageResult& InApplyDamageResult) const;

private:
	FApplyDamageSpecKey BuildSpecKey(const FHitContext& InHitContext) const;

private:
	void PrintApplyDamageSummaryInfo(const FHitContext& InHitContext, const FApplyDamageSpec & InApplyDamageSpec, const FApplyDamageResult& InApplyDamageResult) const;
	void PrintApplyDamageContextInfo(const FHitContext& InHitContext, const FApplyDamageSpec & InApplyDamageSpec, const FApplyDamageResult& InApplyDamageResult) const;

private:
	void PrintOverlapContextInfo(const FOverlapContext& InOverlapContext) const;
	void PrintHitContextInfo(const FAttachmentContext& InAttachmentContext, const FEquipmentContext& InEquipmentContext, const FActionContext& InActionContext) const;
	void PrintDamageSpecInfo(const FApplyDamageSpec & InApplyDamageSpec) const;
	void PrintDamageResultInfo(const FApplyDamageResult & InApplyDamageResult) const;
};
