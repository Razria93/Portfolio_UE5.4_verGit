#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Type/CWeaponStructure.h"
#include "ActionFeedbackRequestProvider.generated.h"

UINTERFACE(MinimalAPI)
class UActionFeedbackRequestProvider : public UInterface
{
	GENERATED_BODY()
};

class PORTFOLIO_API IActionFeedbackRequestProvider
{
	GENERATED_BODY()

public:
	virtual bool BuildActionFeedbackRequest(EActionFeedbackTiming InActionFeedbackTiming, FName InTriggerKey, FActionFeedbackRequest& OutActionFeedbackRequest) const = 0;
};
