#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_ActionBase.h"
#include "CAnimNotifyState_Collision.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState_Collision : public UCAnimNotifyState_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_Collision();

private:
	UPROPERTY(EditAnywhere, Category = "Collision")
	FName CollisionName = NAME_None;

public:
	FString GetNotifyName_Implementation() const override;

public:
	void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	class ACWeaponActor* GetWeaponActor(USkeletalMeshComponent* InMeshComp) const;
};
