#include "Core/Profiling/CCombatCollisionProfilingCounters.h"

#include "ProfilingDebugging/CsvProfiler.h"

namespace
{
	int32 CollisionNotifyBeginCount = 0;
	int32 CollisionNotifyEndCount = 0;
	int32 ActionCollisionWindowBeginCount = 0;
	int32 ActionCollisionWindowEndCount = 0;
	int32 WeaponComponentOpenCollisionWindowCount = 0;
	int32 WeaponComponentCloseCollisionWindowCount = 0;
	int32 HitWindowOpenCount = 0;
	int32 HitWindowCloseCount = 0;
	int32 HitWindowOverlapCount = 0;
	int32 HitProcessingCount = 0;
	int32 CombatSignalCount = 0;
	int32 CombatSignalCueNotifyCount = 0;
	int32 ActionCombatSignalCueCount = 0;
	int32 AICombatSignalCueRequestCount = 0;
	int32 CombatSignalCueRequestCount = 0;
	int32 CombatSignalCueSendCount = 0;
}

void FCombatCollisionProfilingCounters::RecordCollisionNotifyBegin() { ++CollisionNotifyBeginCount; }
void FCombatCollisionProfilingCounters::RecordCollisionNotifyEnd() { ++CollisionNotifyEndCount; }
void FCombatCollisionProfilingCounters::RecordActionCollisionWindowBegin() { ++ActionCollisionWindowBeginCount; }
void FCombatCollisionProfilingCounters::RecordActionCollisionWindowEnd() { ++ActionCollisionWindowEndCount; }
void FCombatCollisionProfilingCounters::RecordWeaponComponentOpenCollisionWindow() { ++WeaponComponentOpenCollisionWindowCount; }
void FCombatCollisionProfilingCounters::RecordWeaponComponentCloseCollisionWindow() { ++WeaponComponentCloseCollisionWindowCount; }
void FCombatCollisionProfilingCounters::RecordHitWindowOpen() { ++HitWindowOpenCount; }
void FCombatCollisionProfilingCounters::RecordHitWindowClose() { ++HitWindowCloseCount; }
void FCombatCollisionProfilingCounters::RecordHitWindowOverlap() { ++HitWindowOverlapCount; }
void FCombatCollisionProfilingCounters::RecordHitProcessing() { ++HitProcessingCount; }
void FCombatCollisionProfilingCounters::RecordCombatSignal() { ++CombatSignalCount; }
void FCombatCollisionProfilingCounters::RecordCombatSignalCueNotify() { ++CombatSignalCueNotifyCount; }
void FCombatCollisionProfilingCounters::RecordActionCombatSignalCue() { ++ActionCombatSignalCueCount; }
void FCombatCollisionProfilingCounters::RecordAICombatSignalCueRequest() { ++AICombatSignalCueRequestCount; }
void FCombatCollisionProfilingCounters::RecordCombatSignalCueRequest() { ++CombatSignalCueRequestCount; }
void FCombatCollisionProfilingCounters::RecordCombatSignalCueSend() { ++CombatSignalCueSendCount; }

void FCombatCollisionProfilingCounters::FlushToCsv()
{
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CollisionNotify_Begin_FlushCount, CollisionNotifyBeginCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CollisionNotify_End_FlushCount, CollisionNotifyEndCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionCollisionWindow_Begin_FlushCount, ActionCollisionWindowBeginCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionCollisionWindow_End_FlushCount, ActionCollisionWindowEndCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_WeaponComponent_OpenCollisionWindow_FlushCount, WeaponComponentOpenCollisionWindowCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_WeaponComponent_CloseCollisionWindow_FlushCount, WeaponComponentCloseCollisionWindowCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitWindow_Open_FlushCount, HitWindowOpenCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitWindow_Close_FlushCount, HitWindowCloseCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitWindow_Overlap_FlushCount, HitWindowOverlapCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitProcessing_FlushCount, HitProcessingCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CombatSignal_FlushCount, CombatSignalCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CombatSignalCue_Notify_FlushCount, CombatSignalCueNotifyCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionCombatSignalCue_FlushCount, ActionCombatSignalCueCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_AICombatSignalCue_Request_FlushCount, AICombatSignalCueRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CombatSignalCue_Request_FlushCount, CombatSignalCueRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CombatSignalCue_Send_FlushCount, CombatSignalCueSendCount, ECsvCustomStatOp::Accumulate);

	CollisionNotifyBeginCount = 0;
	CollisionNotifyEndCount = 0;
	ActionCollisionWindowBeginCount = 0;
	ActionCollisionWindowEndCount = 0;
	WeaponComponentOpenCollisionWindowCount = 0;
	WeaponComponentCloseCollisionWindowCount = 0;
	HitWindowOpenCount = 0;
	HitWindowCloseCount = 0;
	HitWindowOverlapCount = 0;
	HitProcessingCount = 0;
	CombatSignalCount = 0;
	CombatSignalCueNotifyCount = 0;
	ActionCombatSignalCueCount = 0;
	AICombatSignalCueRequestCount = 0;
	CombatSignalCueRequestCount = 0;
	CombatSignalCueSendCount = 0;
}
