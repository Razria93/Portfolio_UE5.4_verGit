#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState.h"
#include "Type/CWeaponStructure.h"
#include "CAnimNotifyState_Reaction.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState_Reaction : public UCAnimNotifyState
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_Reaction();

public:
	UPROPERTY(EditAnywhere)
	EReactionWindowType ReactionWindowType = EReactionWindowType::None;

public:
	FString GetNotifyName_Implementation() const override;

protected:
	FString MakeNotifyName(FString InName) const;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
