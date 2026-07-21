#pragma once

#include "CoreMinimal.h"
#include "AI/RuntimeLOD/CAIRuntimeLODTierResolver.h"

class FAIStateRuntimeLODProfiling
{
public:
	static bool ShouldAuditStateRuntimeLOD();
	static void RecordResolvedTierForProfiling(EAIRuntimeLODTier InTier);
};
