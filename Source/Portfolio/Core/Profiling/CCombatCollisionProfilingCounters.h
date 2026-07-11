#pragma once

#include "CoreMinimal.h"

class FCombatCollisionProfilingCounters
{
public:
	static void RecordCollisionNotifyBegin();
	static void RecordCollisionNotifyEnd();
	static void RecordActionCollisionWindowBegin();
	static void RecordActionCollisionWindowEnd();
	static void RecordWeaponComponentOpenCollisionWindow();
	static void RecordWeaponComponentCloseCollisionWindow();
	static void RecordHitWindowOpen();
	static void RecordHitWindowClose();
	static void RecordHitWindowOverlap();
	static void RecordHitProcessing();
	static void RecordCombatSignal();
	static void RecordCombatSignalCueNotify();
	static void RecordActionCombatSignalCue();
	static void RecordAICombatSignalCueRequest();
	static void RecordCombatSignalCueRequest();
	static void RecordCombatSignalCueSend();

	static void FlushToCsv();
};
