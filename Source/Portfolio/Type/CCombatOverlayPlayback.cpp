#include "Type/CCombatOverlayPlayback.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

bool FCombatOverlayPlayback::Start(UObject* InOuter, const TArray<UMeshComponent*>& InMeshes,
	UMaterialInterface* InMaterial, FName InWeightParameter)
{
	Restore();
	if (!IsValid(InOuter) || !IsValid(InMaterial) || InWeightParameter.IsNone()) return false;
	float unused = 0.f;
	// Refuse incompatible materials rather than silently displaying a full-strength overlay.
	if (!InMaterial->GetScalarParameterValue(FMaterialParameterInfo(InWeightParameter), unused)) return false;
	WeightParameter = InWeightParameter;
	TSet<UMeshComponent*> visited;
	for (UMeshComponent* mesh : InMeshes)
	{
		if (!IsValid(mesh) || visited.Contains(mesh)) continue;
		visited.Add(mesh);
		UMaterialInstanceDynamic* instance = UMaterialInstanceDynamic::Create(InMaterial, InOuter);
		if (!instance) continue;
		instance->SetScalarParameterValue(WeightParameter, 0.f);
		FCombatOverlayLease& lease = Leases.AddDefaulted_GetRef();
		lease.Mesh = mesh;
		lease.Previous = mesh->GetOverlayMaterial();
		lease.Instance = instance;
		mesh->SetOverlayMaterial(instance);
	}
	return !Leases.IsEmpty();
}

void FCombatOverlayPlayback::SetWeight(float InWeight)
{
	const float weight = FMath::IsFinite(InWeight) ? FMath::Clamp(InWeight, 0.f, 1.f) : 0.f;
	for (int32 i = Leases.Num() - 1; i >= 0; --i)
	{
		FCombatOverlayLease& lease = Leases[i];
		if (!lease.Mesh.IsValid() || lease.Mesh->GetOverlayMaterial() != lease.Instance)
		{
			// Another system took ownership. Do not retake it on the following tick.
			Leases.RemoveAtSwap(i);
			continue;
		}
		lease.Instance->SetScalarParameterValue(WeightParameter, weight);
	}
}

void FCombatOverlayPlayback::Restore()
{
	for (const FCombatOverlayLease& lease : Leases)
		if (lease.Mesh.IsValid() && lease.Mesh->GetOverlayMaterial() == lease.Instance)
			lease.Mesh->SetOverlayMaterial(lease.Previous);
	Leases.Reset();
	WeightParameter = NAME_None;
}
