#pragma once

#include "CoreMinimal.h"

class FCombatFeedbackProfiling
{
public:
	// Gate
	static bool ShouldSkipEnemyCombatFeedback(const class AActor* InOwnerActor);

	// Action Counter
	static void RecordActionFeedbackRequest();
	static void RecordActionFeedbackSkipped();
	static void RecordActionTrail(bool bActive);
	static void RecordActionVFX();
	static void RecordActionSFX();

	// Reaction Counter
	static void RecordReactionFeedbackRequest();
	static void RecordReactionFeedbackSkipped();
	static void RecordReactionVFX();
	static void RecordReactionSFX();

	// Hit Counter
	static void RecordHitFeedbackRequest();
	static void RecordHitFeedbackPresentationSkipped();
	static void RecordHitVFX();
	static void RecordHitSFX();
	static void RecordCameraShakeRequest();

	// Flush
	static void FlushToCsv();
};
