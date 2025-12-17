#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CWeaponStructure.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Unarmed = 0,
	Sword,
	Max,
};

UCLASS()
class PORTFOLIO_API UCWeaponStructure : public UObject
{
	GENERATED_BODY()
	
};
