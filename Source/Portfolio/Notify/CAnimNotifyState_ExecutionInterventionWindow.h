#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState.h"
#include "Type/CWeaponStructure.h"
#include "CAnimNotifyState_ExecutionInterventionWindow.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimNotifyState_ExecutionInterventionWindow : public UCAnimNotifyState
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_ExecutionInterventionWindow();

protected:
	UPROPERTY(EditAnywhere, Category = "Window")
	FName WindowKey = NAME_None;

public:
	FString GetNotifyName_Implementation() const override;
	FLinearColor GetEditorColor() override;

public:
	void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	void HandleWindow(USkeletalMeshComponent* InMeshComp, bool bOpen) const;
};
