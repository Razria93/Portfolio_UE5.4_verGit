#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_WeaponSocketTransformTransitionBase.h"
#include "CAnimNotifyState_WeaponSocketTransformTransition.generated.h"

UCLASS(meta = (DisplayName = "Weapon Socket Transform Transition"))
class PORTFOLIO_API UCAnimNotifyState_WeaponSocketTransformTransition : public UCAnimNotifyState_WeaponSocketTransformTransitionBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Socket Transform Transition")
	FName SourceSocketName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Socket Transform Transition")
	FName TargetSocketName = NAME_None;

protected:
	virtual FString GetNotifyName_Implementation() const override;

protected:
	virtual bool IsConfigurationValid() const override;

protected:
	virtual bool BeginTransition(class UCWeaponComponent* InWeaponComp, uint32& OutTransitionHandle) const override;
};
