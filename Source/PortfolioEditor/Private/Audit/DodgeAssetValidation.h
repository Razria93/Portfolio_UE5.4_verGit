#pragma once

#include "CoreMinimal.h"

class AActor;
class UWeaponTrailDataAsset;
struct FReactionData;
struct FActionData;

namespace DodgeAssetValidation
{
	// Read-only authored-data checks. Asset names and art choices are not bootstrap defaults.
	bool ValidatePerfectReaction(const FReactionData& Row, FString& Error);
	bool ValidateCounterRush(const FActionData& Row, const UWeaponTrailDataAsset* Trails, FString& Error);
	bool ValidatePlayer(AActor* Defaults, bool bRequireStellarSetup, FString& Error);
}
