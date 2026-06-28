#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ActionBase.h"
#include "CAnimNotify_CombatSignalCue.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotify_CombatSignalCue : public UCAnimNotify_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_CombatSignalCue();

public:
	UPROPERTY(EditAnywhere, Category = "CombatSignal")
	FName CueTag = NAME_None;

private:
	FString GetNotifyName_Implementation() const override;

private:
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	AActor* ResolveCueTargetActor(const ACharacter* InOwnerCharacter) const;
};
