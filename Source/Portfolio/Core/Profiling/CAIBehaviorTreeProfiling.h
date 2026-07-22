#pragma once

#include "CoreMinimal.h"

enum class EBTServiceIntervalPreset : uint8;

class FAIBehaviorTreeProfiling
{
public:
	// Service Tick Counter
	static void RecordUpdateAIContextTickForProfiling();
	static void RecordUpdateAIIntentStateTickForProfiling();
	static void RecordUpdateEngageContextTickForProfiling();

	// Interval Preset Counter
	static void RecordAIIntentIntervalPresetForProfiling(EBTServiceIntervalPreset InPreset);
};
