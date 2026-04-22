#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_HitContext.generated.h"

UENUM()
enum class EHitContextNotifyType : uint8
{
	Push,
	Clear,
};

UCLASS()
class PORTFOLIO_API UCAnimNotify_HitContext : public UCAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify_HitContext();

private:
	UPROPERTY(EditAnywhere)
	EHitContextNotifyType NotifyType = EHitContextNotifyType::Push;

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
