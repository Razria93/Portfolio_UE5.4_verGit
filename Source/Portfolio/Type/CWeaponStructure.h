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

USTRUCT(BlueprintType)
struct FEquipmentData

{
	GENERATED_BODY()

public:
	FEquipmentData()
		: Montage(nullptr), PlayRate(1.0f), bCanMove(true)
	{
	}

public:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* Montage;

	UPROPERTY(EditAnywhere)
	float PlayRate;

	UPROPERTY(EditAnywhere)
	bool bCanMove;
};

USTRUCT()
struct FActionData
{
	GENERATED_BODY()

public:
	FActionData()
		: Montage(nullptr), PlayRate(1.0f), bCanMove(true)
	{
	}

public:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* Montage;

	UPROPERTY(EditAnywhere)
	float PlayRate;

	UPROPERTY(EditAnywhere)
	bool bCanMove;

public:
	void Begin_PlayMontage(class ACharacter* InOwnerCharacter);
	void End_PlayMontage(class ACharacter* InOwnerCharacter);
};

UCLASS()
class PORTFOLIO_API UCWeaponStructure : public UObject
{
	GENERATED_BODY()

};
