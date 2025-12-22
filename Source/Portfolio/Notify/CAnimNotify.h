#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify.generated.h"

UENUM()
enum class EAnimNotifyFlow : uint8
{
	Begin, End, Next, Max,
};

UCLASS()
class PORTFOLIO_API UCAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UCAnimNotify();

protected:
	UPROPERTY(EditAnywhere)
	EAnimNotifyFlow FlowType = EAnimNotifyFlow::Max;

protected:
	FString MakeNotifyName(FString InName) const;

protected:
	class UCWeaponComponent* GetWeaponComponent(class USkeletalMeshComponent* MeshComp);
};
