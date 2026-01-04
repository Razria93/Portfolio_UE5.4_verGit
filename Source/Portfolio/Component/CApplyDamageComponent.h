#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CApplyDamageComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCApplyDamageComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	TMap<FName, FDamageSpecData> DamageSpecData;

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
	bool ValidateRequest(const FHitContext& InHitContext) const;
	bool CheckHitRule(const FHitContext& InHitContext) const;

private:
	bool FindDamageSpecData(const FHitContext& InHitContext) const;

private:
	void PrintApplyDamageContextInfo(const FHitContext& InHitContext);

	void PrintOverlapContextInfo(const FOverlapContext& InOverlapContext);
	void Print_HitContextInfo(const FAttachmentContext& InAttachmentContext, const FEquipmentContext& InEquipmentContext, const FActionContext& InActionContext);
};
