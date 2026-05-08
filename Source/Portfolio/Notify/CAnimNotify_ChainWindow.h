#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify_ActionBase.h"
#include "CAnimNotify_ChainWindow.generated.h"

UENUM()
enum class EChainWindowNotifyType : uint8
{
	Opened,
	Closed,
};

UCLASS()
class PORTFOLIO_API UCAnimNotify_ChainWindow : public UCAnimNotify_ActionBase
{
	GENERATED_BODY()

public:
	UCAnimNotify_ChainWindow();

private:
	UPROPERTY(EditAnywhere)
	EChainWindowNotifyType NotifyType = EChainWindowNotifyType::Opened;

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
