#pragma once

#include "CoreMinimal.h"
#include "CCombatOverlayPlayback.generated.h"

/** One mesh's overlay lease. A replacement by another effect is never overwritten. */
USTRUCT()
struct PORTFOLIO_API FCombatOverlayLease
{
	GENERATED_BODY()
	UPROPERTY(Transient)
	TWeakObjectPtr<class UMeshComponent> Mesh;
	UPROPERTY(Transient)
	TObjectPtr<class UMaterialInterface> Previous = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<class UMaterialInstanceDynamic> Instance = nullptr;
};

/** Store as a UPROPERTY on its owning component so previous materials and MIDs stay alive. */
USTRUCT()
struct PORTFOLIO_API FCombatOverlayPlayback
{
	GENERATED_BODY()
	UPROPERTY(Transient)
	TArray<FCombatOverlayLease> Leases;
	FName WeightParameter = NAME_None;

	bool Start(UObject* InOuter, const TArray<class UMeshComponent*>& InMeshes,
		class UMaterialInterface* InMaterial, FName InWeightParameter);
	void SetWeight(float InWeight);
	void Restore();
};
