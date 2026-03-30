#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_EndEnemyAttack.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_EndEnemyAttack : public UCAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify_EndEnemyAttack();

private:
	FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
