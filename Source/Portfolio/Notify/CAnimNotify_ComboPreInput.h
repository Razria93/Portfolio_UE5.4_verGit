#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_ComboPreInput.generated.h"

UENUM()
enum class EPreInputNotifyType : uint8
{
	Enabled,
	Disabled,
};

UCLASS()
class PORTFOLIO_API UCAnimNotify_ComboPreInput : public UCAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify_ComboPreInput();

private:
	UPROPERTY(EditAnywhere)
	EPreInputNotifyType NotifyType = EPreInputNotifyType::Enabled;

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
