#pragma once

#include "CoreMinimal.h"
#include "Type/CActionFeedbackTypes.h"
#include "Type/CReactionFeedbackTypes.h"
#include "Type/CCombatSignalTargetTypes.h"
#include "Type/CCombatFeedbackTypes.h"

class PORTFOLIO_API FCombatFeedbackDebug
{
public:
	// Gate
	static bool ShouldAuditCombatFeedback();

public:
	// Action Feedback Diagnostic Hook
	static void RecordActionFeedbackRequestAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FActionFeedbackRequest& InRequest);
	static void RecordActionFeedbackChannelMatchedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FActionFeedbackRequest& InRequest, const TCHAR* InChannel, int32 InMatchCount);
	static void RecordActionFeedbackPresentationPlayedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InEvent);
	static void RecordActionFeedbackRequestRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FActionFeedbackRequest& InRequest, const TCHAR* InReason);
	static void RecordActionFeedbackChannelRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FActionFeedbackRequest& InRequest, const TCHAR* InChannel, const TCHAR* InReason, int32 InMatchCount = 0);
	static void RecordActionFeedbackPresentationRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InReason);

public:
	// Reaction Feedback Diagnostic Hook
	static void RecordReactionFeedbackRequestAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FReactionFeedbackRequest& InRequest);
	static void RecordReactionFeedbackChannelMatchedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FReactionFeedbackRequest& InRequest, const TCHAR* InChannel, int32 InMatchCount);
	static void RecordReactionFeedbackPresentationPlayedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InEvent);
	static void RecordReactionFeedbackRequestRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FReactionFeedbackRequest& InRequest, const TCHAR* InReason);
	static void RecordReactionFeedbackChannelRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FReactionFeedbackRequest& InRequest, const TCHAR* InChannel, const TCHAR* InReason, int32 InMatchCount = 0);
	static void RecordReactionFeedbackPresentationRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InReason);

public:
	// Hit Feedback Diagnostic Hook
	static void RecordHitFeedbackRequestAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FCombatSignalTargetPacket& InPacket);
	static void RecordHitFeedbackPresentationRequestedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FCombatSignalTargetPacket& InPacket, const TCHAR* InChannel);
	static void RecordHitFeedbackPresentationPlayedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InEvent);
	static void RecordHitFeedbackRequestRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FCombatSignalTargetPacket& InPacket, const TCHAR* InReason);
	static void RecordHitFeedbackPresentationRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FCombatSignalTargetPacket& InPacket, const TCHAR* InChannel, const TCHAR* InReason);
	static void RecordHitFeedbackAssetRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InReason);

public:
	// Shared Feedback Dispatch Diagnostic Hook
	static void RecordCombatFeedbackHitStopRequestedForAudit(const FHitStopRequest& InRequest);
	static void RecordCombatFeedbackHitStopAppliedForAudit(const AActor* InActor, float InDuration, float InDilation);
	static void RecordCombatFeedbackCameraShakeRequestedForAudit(const FCameraShakeRequest& InRequest);
	static void RecordCombatFeedbackHitStopRejectedForAudit(const AActor* InActor, float InDuration, float InDilation, const TCHAR* InReason);
	static void RecordCombatFeedbackRequestRejectedForAudit(const TCHAR* InEvent, const TCHAR* InReason);
};
