#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_Collision.generated.h"

UENUM()
enum class ECollisionNotifyType : uint8
{
	Enabled,
	Disabled,
};

UCLASS()
class PORTFOLIO_API UCAnimNotify_Collision : public UCAnimNotify
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	ECollisionNotifyType NotifyType = ECollisionNotifyType::Enabled;

private:
	UPROPERTY(EditAnywhere)
	FName CollisionName;

public:
	UCAnimNotify_Collision();

private:
	FString GetNotifyName_Implementation() const override;
	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
