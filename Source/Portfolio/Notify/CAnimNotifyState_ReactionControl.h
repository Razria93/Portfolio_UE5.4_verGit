#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState.h"
#include "Type/CWeaponStructure.h"
#include "CAnimNotifyState_ReactionControl.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState_ReactionControl : public UCAnimNotifyState
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_ReactionControl();

public:
	UPROPERTY(EditAnywhere)
	EReactionControlWindowType ReactionControlWindowType = EReactionControlWindowType::None;

public:
	FString GetNotifyName_Implementation() const override;

protected:
	FString MakeNotifyName(FString InName) const;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
