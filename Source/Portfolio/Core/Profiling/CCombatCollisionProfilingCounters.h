#pragma once

#include "CoreMinimal.h"

class FCombatCollisionProfilingCounters
{
public:
	// Collision Notify Counter
	static void RecordCollisionNotifyBegin();
	static void RecordCollisionNotifyEnd();

	// Action Collision Window Counter
	static void RecordActionCollisionWindowBegin();
	static void RecordActionCollisionWindowEnd();

	// Weapon Component Counter
	static void RecordWeaponComponentOpenCollisionWindow();
	static void RecordWeaponComponentCloseCollisionWindow();

	// Hit Window Counter
	static void RecordHitWindowOpen();
	static void RecordHitWindowClose();
	static void RecordHitWindowOverlap();
	static void RecordHitProcessing();

	// Combat Signal Counter
	static void RecordCombatSignal();
	static void RecordCombatSignalCueNotify();
	static void RecordActionCombatSignalCue();
	static void RecordAICombatSignalCueRequest();
	static void RecordCombatSignalCueRequest();
	static void RecordCombatSignalCueSend();

	// Flush
	static void FlushToCsv();
};
