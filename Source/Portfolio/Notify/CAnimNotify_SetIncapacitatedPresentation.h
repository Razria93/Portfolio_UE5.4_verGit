#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ReactionBase.h"
#include "Type/CBalanceTypes.h"
#include "CAnimNotify_SetIncapacitatedPresentation.generated.h"

// Reusable reaction-montage notify for switching the persistent full-body
// incapacity presentation underneath an opaque montage pose.
UCLASS()
class PORTFOLIO_API UCAnimNotify_SetIncapacitatedPresentation : public UCAnimNotify_ReactionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_SetIncapacitatedPresentation();
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category = "Presentation")
	EIncapacitatedPresentation TargetPresentation = EIncapacitatedPresentation::None;
};
