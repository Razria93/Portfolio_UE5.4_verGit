#include "DataAsset/CWeaponTrailDataAsset.h"

const FWeaponTrailDefinition* UWeaponTrailDataAsset::FindTrailDefinition(FName InTriggerKey) const
{
	if (InTriggerKey.IsNone()) return nullptr;

	for (const FWeaponTrailDefinition& definition : TrailDefinitions)
	{
		if (definition.TriggerKey == InTriggerKey)
		{
			return &definition;
		}
	}

	return nullptr;
}
