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
	TMap<FDamageSpecKey, FDamageSpec> DamageSpecMap;	// TODO: Seperate DataAsset (DB)

public:
	UCApplyDamageComponent();

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
	bool CheckHitRule(const FHitContext& InHitContext) const;
	bool ResolveDamageSpec(const FHitContext& InHitContext,FDamageSpec& OutDamageSpec) const;
	bool ComputeDamageResult(const FHitContext& InHitContext, const FDamageSpec& InDamageSpec, FDamageResult& OutDamageResult) const;
	bool ApplyDamageToTarget(const FHitContext& InHitContext, const FDamageSpec& InDamageSpec, const FDamageResult& InDamageResult) const;

private:
	FDamageSpecKey BuildSpecKey(const FHitContext& InHitContext) const;

private:
	void PrintApplyDamageContextInfo(const FHitContext& InHitContext, const FDamageSpec& InDamageSpec, const FDamageResult& InDamageResult);

private:
	void PrintOverlapContextInfo(const FOverlapContext& InOverlapContext);
	void PrintHitContextInfo(const FAttachmentContext& InAttachmentContext, const FEquipmentContext& InEquipmentContext, const FActionContext& InActionContext);
	void PrintDamageSpecInfo(const FDamageSpec& InDamageSpec);
	void PrintDamageResultInfo(const FDamageResult& InDamageResult);
};
