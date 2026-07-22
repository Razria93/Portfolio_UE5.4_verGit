#include "Core/Profiling/CCombatFeedbackProfiling.h"

#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "Character/Enemy/CEnemy.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarDisableEnemyCombatFeedback(
		TEXT("Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback"),
		0,
		TEXT("Disable Enemy combat feedback presentation for runtime LOD profiling. 0: play feedback, 1: skip Enemy feedback presentation."),
		ECVF_Default);

	// Action Counter
	int32 ActionFeedbackRequestCount = 0;
	int32 ActionFeedbackSkippedCount = 0;
	int32 ActionTrailCount = 0;
	int32 ActionTrailClearCount = 0;
	int32 ActionVFXCount = 0;
	int32 ActionSFXCount = 0;

	// Reaction Counter
	int32 ReactionFeedbackRequestCount = 0;
	int32 ReactionFeedbackSkippedCount = 0;
	int32 ReactionVFXCount = 0;
	int32 ReactionSFXCount = 0;

	// Hit Counter
	int32 HitFeedbackRequestCount = 0;
	int32 HitFeedbackPresentationSkippedCount = 0;
	int32 HitVFXCount = 0;
	int32 HitSFXCount = 0;
	int32 CameraShakeRequestCount = 0;
#endif
}

// Gate

bool FCombatFeedbackProfiling::ShouldSkipEnemyCombatFeedback(const AActor* InOwnerActor)
{
#if !UE_BUILD_SHIPPING
	if (CVarDisableEnemyCombatFeedback.GetValueOnGameThread() == 0) return false;

	return IsValid(InOwnerActor) && InOwnerActor->IsA<ACEnemy>();
#else
	return false;
#endif
}

// Action Counter

void FCombatFeedbackProfiling::RecordActionFeedbackRequest()
{
#if !UE_BUILD_SHIPPING
	++ActionFeedbackRequestCount;
#endif
}

void FCombatFeedbackProfiling::RecordActionFeedbackSkipped()
{
#if !UE_BUILD_SHIPPING
	++ActionFeedbackSkippedCount;
#endif
}

void FCombatFeedbackProfiling::RecordActionTrail(bool bActive)
{
#if !UE_BUILD_SHIPPING
	if (bActive)
	{
		++ActionTrailCount;
		return;
	}

	++ActionTrailClearCount;
#endif
}

void FCombatFeedbackProfiling::RecordActionVFX()
{
#if !UE_BUILD_SHIPPING
	++ActionVFXCount;
#endif
}

void FCombatFeedbackProfiling::RecordActionSFX()
{
#if !UE_BUILD_SHIPPING
	++ActionSFXCount;
#endif
}

// Reaction Counter

void FCombatFeedbackProfiling::RecordReactionFeedbackRequest()
{
#if !UE_BUILD_SHIPPING
	++ReactionFeedbackRequestCount;
#endif
}

void FCombatFeedbackProfiling::RecordReactionFeedbackSkipped()
{
#if !UE_BUILD_SHIPPING
	++ReactionFeedbackSkippedCount;
#endif
}

void FCombatFeedbackProfiling::RecordReactionVFX()
{
#if !UE_BUILD_SHIPPING
	++ReactionVFXCount;
#endif
}

void FCombatFeedbackProfiling::RecordReactionSFX()
{
#if !UE_BUILD_SHIPPING
	++ReactionSFXCount;
#endif
}

// Hit Counter

void FCombatFeedbackProfiling::RecordHitFeedbackRequest()
{
#if !UE_BUILD_SHIPPING
	++HitFeedbackRequestCount;
#endif
}

void FCombatFeedbackProfiling::RecordHitFeedbackPresentationSkipped()
{
#if !UE_BUILD_SHIPPING
	++HitFeedbackPresentationSkippedCount;
#endif
}

void FCombatFeedbackProfiling::RecordHitVFX()
{
#if !UE_BUILD_SHIPPING
	++HitVFXCount;
#endif
}

void FCombatFeedbackProfiling::RecordHitSFX()
{
#if !UE_BUILD_SHIPPING
	++HitSFXCount;
#endif
}

void FCombatFeedbackProfiling::RecordCameraShakeRequest()
{
#if !UE_BUILD_SHIPPING
	++CameraShakeRequestCount;
#endif
}

// Flush

void FCombatFeedbackProfiling::FlushToCsv()
{
#if !UE_BUILD_SHIPPING
	// Action Counter
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_Request_FlushCount, ActionFeedbackRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_Skipped_FlushCount, ActionFeedbackSkippedCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_Trail_FlushCount, ActionTrailCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_TrailClear_FlushCount, ActionTrailClearCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_VFX_FlushCount, ActionVFXCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_SFX_FlushCount, ActionSFXCount, ECsvCustomStatOp::Accumulate);

	// Reaction Counter
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ReactionFeedback_Request_FlushCount, ReactionFeedbackRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ReactionFeedback_Skipped_FlushCount, ReactionFeedbackSkippedCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ReactionFeedback_VFX_FlushCount, ReactionVFXCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ReactionFeedback_SFX_FlushCount, ReactionSFXCount, ECsvCustomStatOp::Accumulate);

	// Hit Counter
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_Request_FlushCount, HitFeedbackRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_PresentationSkipped_FlushCount, HitFeedbackPresentationSkippedCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_VFX_FlushCount, HitVFXCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_SFX_FlushCount, HitSFXCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_CameraShakeRequest_FlushCount, CameraShakeRequestCount, ECsvCustomStatOp::Accumulate);

	// Action Counter
	ActionFeedbackRequestCount = 0;
	ActionFeedbackSkippedCount = 0;
	ActionTrailCount = 0;
	ActionTrailClearCount = 0;
	ActionVFXCount = 0;
	ActionSFXCount = 0;

	// Reaction Counter
	ReactionFeedbackRequestCount = 0;
	ReactionFeedbackSkippedCount = 0;
	ReactionVFXCount = 0;
	ReactionSFXCount = 0;

	// Hit Counter
	HitFeedbackRequestCount = 0;
	HitFeedbackPresentationSkippedCount = 0;
	HitVFXCount = 0;
	HitSFXCount = 0;
	CameraShakeRequestCount = 0;
#endif
}
