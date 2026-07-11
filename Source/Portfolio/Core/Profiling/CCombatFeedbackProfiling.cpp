#include "Core/Profiling/CCombatFeedbackProfiling.h"

#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "Character/Enemy/CEnemy.h"

namespace
{
	TAutoConsoleVariable<int32> CVarDisableEnemyCombatFeedback(
		TEXT("Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback"),
		0,
		TEXT("Disable Enemy combat feedback presentation for runtime LOD profiling. 0: play feedback, 1: skip Enemy feedback presentation."),
		ECVF_Default);

	int32 ActionFeedbackRequestCount = 0;
	int32 ActionFeedbackSkippedCount = 0;
	int32 ActionTrailCount = 0;
	int32 ActionVFXCount = 0;
	int32 ActionSFXCount = 0;

	int32 ReactionFeedbackRequestCount = 0;
	int32 ReactionFeedbackSkippedCount = 0;
	int32 ReactionVFXCount = 0;
	int32 ReactionSFXCount = 0;

	int32 HitFeedbackRequestCount = 0;
	int32 HitFeedbackPresentationSkippedCount = 0;
	int32 HitVFXCount = 0;
	int32 HitSFXCount = 0;
	int32 CameraShakeRequestCount = 0;
}

bool FCombatFeedbackProfiling::ShouldSkipEnemyCombatFeedback(const AActor* InOwnerActor)
{
	if (CVarDisableEnemyCombatFeedback.GetValueOnGameThread() == 0) return false;

	return IsValid(InOwnerActor) && InOwnerActor->IsA<ACEnemy>();
}

void FCombatFeedbackProfiling::RecordActionFeedbackRequest() { ++ActionFeedbackRequestCount; }
void FCombatFeedbackProfiling::RecordActionFeedbackSkipped() { ++ActionFeedbackSkippedCount; }
void FCombatFeedbackProfiling::RecordActionTrail() { ++ActionTrailCount; }
void FCombatFeedbackProfiling::RecordActionVFX() { ++ActionVFXCount; }
void FCombatFeedbackProfiling::RecordActionSFX() { ++ActionSFXCount; }

void FCombatFeedbackProfiling::RecordReactionFeedbackRequest() { ++ReactionFeedbackRequestCount; }
void FCombatFeedbackProfiling::RecordReactionFeedbackSkipped() { ++ReactionFeedbackSkippedCount; }
void FCombatFeedbackProfiling::RecordReactionVFX() { ++ReactionVFXCount; }
void FCombatFeedbackProfiling::RecordReactionSFX() { ++ReactionSFXCount; }

void FCombatFeedbackProfiling::RecordHitFeedbackRequest() { ++HitFeedbackRequestCount; }
void FCombatFeedbackProfiling::RecordHitFeedbackPresentationSkipped() { ++HitFeedbackPresentationSkippedCount; }
void FCombatFeedbackProfiling::RecordHitVFX() { ++HitVFXCount; }
void FCombatFeedbackProfiling::RecordHitSFX() { ++HitSFXCount; }
void FCombatFeedbackProfiling::RecordCameraShakeRequest() { ++CameraShakeRequestCount; }

void FCombatFeedbackProfiling::FlushToCsv()
{
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_Request_FlushCount, ActionFeedbackRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_Skipped_FlushCount, ActionFeedbackSkippedCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_Trail_FlushCount, ActionTrailCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_VFX_FlushCount, ActionVFXCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ActionFeedback_SFX_FlushCount, ActionSFXCount, ECsvCustomStatOp::Accumulate);

	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ReactionFeedback_Request_FlushCount, ReactionFeedbackRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ReactionFeedback_Skipped_FlushCount, ReactionFeedbackSkippedCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ReactionFeedback_VFX_FlushCount, ReactionVFXCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_ReactionFeedback_SFX_FlushCount, ReactionSFXCount, ECsvCustomStatOp::Accumulate);

	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_Request_FlushCount, HitFeedbackRequestCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_PresentationSkipped_FlushCount, HitFeedbackPresentationSkippedCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_VFX_FlushCount, HitVFXCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_SFX_FlushCount, HitSFXCount, ECsvCustomStatOp::Accumulate);
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_HitFeedback_CameraShakeRequest_FlushCount, CameraShakeRequestCount, ECsvCustomStatOp::Accumulate);

	ActionFeedbackRequestCount = 0;
	ActionFeedbackSkippedCount = 0;
	ActionTrailCount = 0;
	ActionVFXCount = 0;
	ActionSFXCount = 0;

	ReactionFeedbackRequestCount = 0;
	ReactionFeedbackSkippedCount = 0;
	ReactionVFXCount = 0;
	ReactionSFXCount = 0;

	HitFeedbackRequestCount = 0;
	HitFeedbackPresentationSkippedCount = 0;
	HitVFXCount = 0;
	HitSFXCount = 0;
	CameraShakeRequestCount = 0;
}
