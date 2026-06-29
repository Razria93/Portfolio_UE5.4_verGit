#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotify.h"
#include "CAnimNotify_HealthBase.generated.h"

UCLASS(Abstract)
class PORTFOLIO_API UCAnimNotify_HealthBase : public UCAnimNotify
{
	GENERATED_BODY()

public:
	UCAnimNotify_HealthBase();

protected:
	class UCHealthComponent* GetHealthComponent(USkeletalMeshComponent* InMeshComp) const;
};
