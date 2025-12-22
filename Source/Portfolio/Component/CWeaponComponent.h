#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWeaponTypeChanged, class ACharacter*, InOwnerCharacter, EWeaponType, InPrevWeaponType, EWeaponType, InNewWeaponType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCWeaponComponent();

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACAttachment> AttachmentClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCEquipment> EquipmentClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCAction> ActionClass;

private:
	UPROPERTY(EditAnywhere)
	FEquipmentData EquipmentData;		// Inject to UCEquipment

	UPROPERTY(EditAnywhere) 
	FEquipmentData UnequipmentData;		// Inject to UCEquipment

	UPROPERTY(EditAnywhere)
	TArray<FActionData> ActionDatas;	// Inject to UCAction

private:
	UPROPERTY(Transient)
	class ACAttachment* Attachment;
	
	UPROPERTY(Transient)
	class UCEquipment* Equipment;

	UPROPERTY(Transient)
	class UCAction* Action;

private:
	EWeaponType CurrentWeaponType;

private:
	// Cached
	class ACharacter* OwnerCharacter_Cached;

public:
	// Delegate
	FWeaponTypeChanged OnWeaponTypeChanged;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	class ACAttachment* GetAttachment();
	class UCEquipment* GetEquipment();
	class UCAction* GetAction();

public:
	FORCEINLINE EWeaponType GetCurType() { return CurrentWeaponType; }

public:
	void SetUnarmedMode();
	void SetSwordMode();

public:
	void PlayAction();

public:
	FORCEINLINE bool CheckCurType(EWeaponType InNewWeaponType) { return CurrentWeaponType == InNewWeaponType; }

private:
	void ChangeWeaponType(EWeaponType InNewWeaponType);
	void ChangeWeaponMode(EWeaponType InNewWeaponType);

private:
	void CreateAttachment(AActor* InOwnerCharacter);
	void CreateEquipment(AActor* InOwnerCharacter);
	void CreateAction(AActor* InOwnerCharacter);
};
