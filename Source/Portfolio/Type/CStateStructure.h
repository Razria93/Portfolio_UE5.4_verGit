#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CStateStructure.generated.h"

UENUM(BlueprintType)
enum class EStateType : uint8
{
	Idle = 0,
	Equip,
	Unequip,
	Max,
};

UCLASS()
class PORTFOLIO_API UCStateStructure : public UObject
{
	GENERATED_BODY()

};