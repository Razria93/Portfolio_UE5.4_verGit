#include "Core/Profiling/CCombatCollisionProfilingCounters.h"

#include "ProfilingDebugging/CsvProfiler.h"

namespace
{
#if !UE_BUILD_SHIPPING
	// Collision Notify Counter
	int32 CollisionNotifyBeginCount = 0;
	int32 CollisionNotifyEndCount = 0;

	// Action Collision Window Counter
	int32 ActionCollisionWindowBeginCount = 0;
	int32 ActionCollisionWindowEndCount = 0;

	// Weapon Component Counter
	int32 WeaponComponentOpenCollisionWindowCount = 0;
	int32 WeaponComponentCloseCollisionWindowCount = 0;

	// Hit Window Counter
	int32 HitWindowOpenCount = 0;
	int32 HitWindowCloseCount = 0;
	int32 HitWindowOverlapCount = 0;
	int32 HitProcessingCount = 0;

	// Combat Signal Counter
	int32 CombatSignalCount = 0;
	int32 CombatSignalCueNotifyCount = 0;
	int32 ActionCombatSignalCueCount = 0;
	int32 AICombatSignalCueRequestCount = 0;
	int32 CombatSignalCueRequestCount = 0;
	int32 CombatSignalCueSendCount = 0;
#endif
}

// Collision Notify Counter

void FCombatCollisionProfilingCounters::RecordCollisionNotifyBegin()
{
#if !UE_BUILD_SHIPPING
	++CollisionNotifyBeginCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordCollisionNotifyEnd()
{
#if !UE_BUILD_SHIPPING
	++CollisionNotifyEndCount;
#endif
}

// Action Collision Window Counter

void FCombatCollisionProfilingCounters::RecordActionCollisionWindowBegin()
{
#if !UE_BUILD_SHIPPING
	++ActionCollisionWindowBeginCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordActionCollisionWindowEnd()
{
#if !UE_BUILD_SHIPPING
	++ActionCollisionWindowEndCount;
#endif
}

// Weapon Component Counter

void FCombatCollisionProfilingCounters::RecordWeaponComponentOpenCollisionWindow()
{
#if !UE_BUILD_SHIPPING
	++WeaponComponentOpenCollisionWindowCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordWeaponComponentCloseCollisionWindow()
{
#if !UE_BUILD_SHIPPING
	++WeaponComponentCloseCollisionWindowCount;
#endif
}

// Hit Window Counter

void FCombatCollisionProfilingCounters::RecordHitWindowOpen()
{
#if !UE_BUILD_SHIPPING
	++HitWindowOpenCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordHitWindowClose()
{
#if !UE_BUILD_SHIPPING
	++HitWindowCloseCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordHitWindowOverlap()
{
#if !UE_BUILD_SHIPPING
	++HitWindowOverlapCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordHitProcessing()
{
#if !UE_BUILD_SHIPPING
	++HitProcessingCount;
#endif
}

// Combat Signal Counter

void FCombatCollisionProfilingCounters::RecordCombatSignal()
{
#if !UE_BUILD_SHIPPING
	++CombatSignalCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordCombatSignalCueNotify()
{
#if !UE_BUILD_SHIPPING
	++CombatSignalCueNotifyCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordActionCombatSignalCue()
{
#if !UE_BUILD_SHIPPING
	++ActionCombatSignalCueCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordAICombatSignalCueRequest()
{
#if !UE_BUILD_SHIPPING
	++AICombatSignalCueRequestCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordCombatSignalCueRequest()
{
#if !UE_BUILD_SHIPPING
	++CombatSignalCueRequestCount;
#endif
}

void FCombatCollisionProfilingCounters::RecordCombatSignalCueSend()
{
#if !UE_BUILD_SHIPPING
	++CombatSignalCueSendCount;
#endif
}

// Flush

void FCombatCollisionProfilingCounters::FlushToCsv()
{
#if !UE_BUILD_SHIPPING
	// Collision Notify Counter
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CollisionNotify_Begin_FlushCount, CollisionNotifyBeginCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CollisionNotify_End_FlushCount, CollisionNotifyEndCount, ECsvCustomStatOp::Accumulate);

	// Action Collision Window Counter
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionCollisionWindow_Begin_FlushCount, ActionCollisionWindowBeginCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionCollisionWindow_End_FlushCount, ActionCollisionWindowEndCount, ECsvCustomStatOp::Accumulate);

	// Weapon Component Counter
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_WeaponComponent_OpenCollisionWindow_FlushCount, WeaponComponentOpenCollisionWindowCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_WeaponComponent_CloseCollisionWindow_FlushCount, WeaponComponentCloseCollisionWindowCount, ECsvCustomStatOp::Accumulate);

	// Hit Window Counter
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitWindow_Open_FlushCount, HitWindowOpenCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitWindow_Close_FlushCount, HitWindowCloseCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitWindow_Overlap_FlushCount, HitWindowOverlapCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitProcessing_FlushCount, HitProcessingCount, ECsvCustomStatOp::Accumulate);

	// Combat Signal Counter
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CombatSignal_FlushCount, CombatSignalCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CombatSignalCue_Notify_FlushCount, CombatSignalCueNotifyCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionCombatSignalCue_FlushCount, ActionCombatSignalCueCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_AICombatSignalCue_Request_FlushCount, AICombatSignalCueRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CombatSignalCue_Request_FlushCount, CombatSignalCueRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_CombatSignalCue_Send_FlushCount, CombatSignalCueSendCount, ECsvCustomStatOp::Accumulate);

	// Collision Notify Counter
	CollisionNotifyBeginCount = 0;
	CollisionNotifyEndCount = 0;

	// Action Collision Window Counter
	ActionCollisionWindowBeginCount = 0;
	ActionCollisionWindowEndCount = 0;

	// Weapon Component Counter
	WeaponComponentOpenCollisionWindowCount = 0;
	WeaponComponentCloseCollisionWindowCount = 0;

	// Hit Window Counter
	HitWindowOpenCount = 0;
	HitWindowCloseCount = 0;
	HitWindowOverlapCount = 0;
	HitProcessingCount = 0;

	// Combat Signal Counter
	CombatSignalCount = 0;
	CombatSignalCueNotifyCount = 0;
	ActionCombatSignalCueCount = 0;
	AICombatSignalCueRequestCount = 0;
	CombatSignalCueRequestCount = 0;
	CombatSignalCueSendCount = 0;
#endif
}
