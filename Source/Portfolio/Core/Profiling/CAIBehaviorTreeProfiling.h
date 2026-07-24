#pragma once

#include "CoreMinimal.h"
#include "Type/CAIBehaviorTreeTypes.h"

class FAIBehaviorTreeProfiling
{
public:
	// Service Tick Counter
	static void RecordUpdateAIContextTick();
	static void RecordUpdateAIIntentStateTick();
	static void RecordUpdateEngageContextTick();

	// Interval Preset Counter
	static void RecordAIIntentIntervalPreset(EBTServiceIntervalPreset InPreset);
};
