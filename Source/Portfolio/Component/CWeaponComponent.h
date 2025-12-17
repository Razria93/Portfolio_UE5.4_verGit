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


private:
	class ACAttachment* Attachment;

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

public:
	void SetSwordMode();

public:
	FORCEINLINE bool CheckCurType(EWeaponType InNewWeaponType) { return CurrentWeaponType == InNewWeaponType; }

private:
	void ChangeWeaponType(EWeaponType InNewWeaponType);
	void ChangeWeaponMode(EWeaponType InNewWeaponType);
	
private:
	void CreateAttachment(AActor* InOwnerCharacter);
}; 
