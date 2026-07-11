#pragma once

#include "CoreMinimal.h"

class FCombatFeedbackProfiling
{
public:
	static bool ShouldSkipEnemyCombatFeedback(const class AActor* InOwnerActor);

	static void RecordActionFeedbackRequest();
	static void RecordActionFeedbackSkipped();
	static void RecordActionTrail();
	static void RecordActionVFX();
	static void RecordActionSFX();

	static void RecordReactionFeedbackRequest();
	static void RecordReactionFeedbackSkipped();
	static void RecordReactionVFX();
	static void RecordReactionSFX();

	static void RecordHitFeedbackRequest();
	static void RecordHitFeedbackPresentationSkipped();
	static void RecordHitVFX();
	static void RecordHitSFX();
	static void RecordCameraShakeRequest();

	static void FlushToCsv();
};
