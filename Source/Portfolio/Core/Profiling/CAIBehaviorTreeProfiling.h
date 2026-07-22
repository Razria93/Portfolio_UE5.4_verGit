#pragma once

#include "CoreMinimal.h"

enum class EBTServiceIntervalPreset : uint8;

class FAIBehaviorTreeProfiling
{
public:
	// Counter
	static void RecordUpdateAIContextTickForProfiling();
	static void RecordUpdateAIIntentStateTickForProfiling();
	static void RecordUpdateEngageContextTickForProfiling();
	static void RecordAIIntentIntervalPresetForProfiling(EBTServiceIntervalPreset InPreset);
};
