#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAttachmentTypeChanged, class ACharacter*, InOwnerCharacter, EAttachmentType, InPrevAttachmentType, EAttachmentType, InNewAttachmentType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEquipmentTypeChanged, class ACharacter*, InOwnerCharacter, EEquipmentType, InPrevEquipmentType, EEquipmentType, InNewEquipmentType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCWeaponComponent();

	// === WeaponData ======================================= //
private:
	UPROPERTY(EditAnywhere, Category = "AttachmentData")
	EAttachmentType AttachmentType;

	UPROPERTY(EditAnywhere, Category = "EquipmentData")
	EEquipmentType EquipmentType;

private:
	UPROPERTY(EditAnywhere, Category = "AttachmentData")
	TSubclassOf<class ACAttachment> AttachmentClass;

	UPROPERTY(EditAnywhere, Category = "EquipmentData")
	TSubclassOf<class UCEquipment> EquipmentClass;

private:
	UPROPERTY(EditAnywhere, Category = "EquipmentData")
	FEquipmentData EquipmentData;

	UPROPERTY(EditAnywhere, Category = "EquipmentData")
	FEquipmentData UnequipmentData;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	class ACAttachment* Attachment;

	UPROPERTY(Transient)
	class UCEquipment* Equipment;

private:
	/* === State === */
	UPROPERTY(Transient)
	EAttachmentType CurrentAttachmentType_Cached;
	
	UPROPERTY(Transient)
	EEquipmentType CurrentEquipmentType_Cached;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached;

public:
	/* === [Out] Custom Delgate Events === */
	FAttachmentTypeChanged OnAttachmentTypeChanged;
	FEquipmentTypeChanged OnEquipmentTypeChanged;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* === Getter === */
	class UObject* GetAttachment();
	class UObject* GetEquipment();

public:
	/* === Getter === */
	FORCEINLINE EAttachmentType GetCurAttachmentType() { return CurrentAttachmentType_Cached; }
	FORCEINLINE EEquipmentType GetCurEquipmentType() { return CurrentEquipmentType_Cached; }

public:
	/* === Setter === */
	void SetUnarmedMode();
	void SetSwordMode();

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurAttachmentType(EAttachmentType InNewAttachmentType) { return CurrentAttachmentType_Cached == InNewAttachmentType; }

public:
	void PushContextToAttachment(const FActionContext& InActionContext);
	void ClearContextToAttachment();

private:
	bool CreateAttachment(AActor* InOwnerCharacter, EAttachmentType InAttachmentType, TSubclassOf<ACAttachment> InAttachmentClass);
	bool CreateEquipment(AActor* InOwnerCharacter, EEquipmentType InEquipmentType, TSubclassOf<UCEquipment> InEquipmentClass, const FEquipmentData& InEquipmentDatas, const FEquipmentData& InUnequipmentDatas);

private:
	void ChangeMode(EAttachmentType InNewAttachmentType);

private:
	void ChangeAttachmentType(EAttachmentType InNewAttachmentType);
	void ChangeEquipmentType(EEquipmentType InNewEquipmentType);

private:
	FAttachmentContext BuildAttachmentContext() const;
	FEquipmentContext BuildEquipmentContext() const;
};
