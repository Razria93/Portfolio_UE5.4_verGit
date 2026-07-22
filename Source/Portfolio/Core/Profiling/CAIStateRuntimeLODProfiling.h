#pragma once

#include "CoreMinimal.h"
#include "AI/RuntimeLOD/CAIRuntimeLODTierResolver.h"

class FAIStateRuntimeLODProfiling
{
public:
	// Gate
	static bool ShouldAuditStateRuntimeLOD();

	// Counter
	static void RecordResolvedTier(EAIRuntimeLODTier InTier);
};
