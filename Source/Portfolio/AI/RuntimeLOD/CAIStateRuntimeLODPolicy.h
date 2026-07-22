#pragma once

#include "CoreMinimal.h"
#include "AI/RuntimeLOD/CAIRuntimeLODTierResolver.h"

class FAIStateRuntimeLODPolicy
{
public:
	static int32 GetStatePolicyMode();
	static bool ShouldUseStateBasedPolicy();
};
