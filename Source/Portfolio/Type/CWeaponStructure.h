#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CWeaponStructure.generated.h"

UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
	Unarmed = 0,
	Sword,
	Max,
};

UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	None = 0,
	Sword,
	Max,
};

UENUM(BlueprintType)
enum class EActionType : uint8
{
	Idle = 0,
	LightAttack,
	ComboAttack,
	Max,
};

USTRUCT(BlueprintType)
struct FEquipmentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere)
	bool bCanMove = false;
};

USTRUCT(BlueprintType)
struct FActionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere)
	bool bCanMove = false;

public:
	void BeginPlayMontage(class ACharacter* InOwnerCharacter);
	void EndPlayMontage(class ACharacter* InOwnerCharacter);
};

USTRUCT(BlueprintType)
struct FDamageSpecData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float DamageAmount = 0.0f;
};

UCLASS()
class PORTFOLIO_API UCWeaponStructure : public UObject
{
	GENERATED_BODY()

};
