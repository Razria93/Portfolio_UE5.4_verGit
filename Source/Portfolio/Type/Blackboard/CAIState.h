#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "Type/CAIStateStructure.h"
#include "CAIState.generated.h"

UCLASS()
class PORTFOLIO_API UCAIState : public UBlackboardKeyType_Enum
{
	GENERATED_BODY()
	
public:
	UCAIState();
};
