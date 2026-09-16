#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ReactionBase.h"
#include "Type/CBalanceTypes.h"
#include "CAnimNotify_SetIncapacitatedPresentation.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_SetIncapacitatedPresentation : public UCAnimNotify_ReactionBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Presentation")
	EIncapacitatedPresentation TargetPresentation = EIncapacitatedPresentation::None;

public:
	UCAnimNotify_SetIncapacitatedPresentation();

protected:
	virtual FString GetNotifyName_Implementation() const override;

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
