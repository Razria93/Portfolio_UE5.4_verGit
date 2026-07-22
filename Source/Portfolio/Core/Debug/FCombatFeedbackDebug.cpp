#include "Core/Debug/FCombatFeedbackDebug.h"
#include "Core/Debug/FLog.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarCombatFeedbackAudit(
		TEXT("Portfolio.Debug.FeedbackAudit"),
		0,
		TEXT("Print combat feedback request, channel, presentation, asset, and shared dispatch diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatActionFeedbackRequest(const FActionFeedbackRequest& InRequest)
	{
		return FString::Printf(
			TEXT("ActionType=%s | ActionIndex=%d | Timing=%s | TriggerKey=%s"),
			*UEnum::GetValueAsString(InRequest.ActionFeedbackKey.ActionType),
			InRequest.ActionFeedbackKey.ActionIndex,
			*UEnum::GetValueAsString(InRequest.ActionFeedbackTiming),
			*InRequest.TriggerKey.ToString());
	}

	FString FormatReactionFeedbackRequest(const FReactionFeedbackRequest& InRequest)
	{
		return FString::Printf(
			TEXT("ReactionType=%s | WeaponType=%s | ActionType=%s | ActionIndex=%d | Timing=%s | TriggerKey=%s"),
			*UEnum::GetValueAsString(InRequest.ReactionFeedbackKey.ReactionType),
			*UEnum::GetValueAsString(InRequest.ReactionFeedbackKey.DamageSpecKey.WeaponType),
			*UEnum::GetValueAsString(InRequest.ReactionFeedbackKey.DamageSpecKey.ActionType),
			InRequest.ReactionFeedbackKey.DamageSpecKey.ActionIndex,
			*UEnum::GetValueAsString(InRequest.ReactionFeedbackTiming),
			*InRequest.TriggerKey.ToString());
	}

	FString FormatHitFeedbackPacket(const FCombatSignalTargetPacket& InPacket)
	{
		return FString::Printf(
			TEXT("Source=%s | Target=%s | Accepted=%s | CommittedDamage=%.3f"),
			*GetNameSafe(InPacket.Context.SourceActor),
			*GetNameSafe(InPacket.Context.TargetActor),
			InPacket.Result.bAccepted ? TEXT("true") : TEXT("false"),
			InPacket.Result.CommittedDamage);
	}

	FString FormatHitStopRequest(const FHitStopRequest& InRequest)
	{
		return FString::Printf(
			TEXT("Audience=%s | Duration=%.3f | Dilation=%.3f | Source=%s | Target=%s"),
			*UEnum::GetValueAsString(InRequest.HitStopAudience),
			InRequest.HitStopDuration,
			InRequest.HitStopDilation,
			*GetNameSafe(InRequest.SourceActor),
			*GetNameSafe(InRequest.TargetActor));
	}

	FString FormatCameraShakeRequest(const FCameraShakeRequest& InRequest)
	{
		return FString::Printf(
			TEXT("Audience=%s | Class=%s | Scale=%.3f | Source=%s | Target=%s | Location=%s"),
			*UEnum::GetValueAsString(InRequest.CameraShakeAudience),
			*GetNameSafe(InRequest.CameraShakeClass.Get()),
			InRequest.CameraShakeBaseScale,
			*GetNameSafe(InRequest.SourceActor),
			*GetNameSafe(InRequest.TargetActor),
			*InRequest.EventLocation.ToCompactString());
	}
}

// Gate

bool FCombatFeedbackDebug::ShouldAuditCombatFeedback()
{
#if !UE_BUILD_SHIPPING
	return CVarCombatFeedbackAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Action Feedback Diagnostic Hook

void FCombatFeedbackDebug::RecordActionFeedbackRequestAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FActionFeedbackRequest& InRequest)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Action|RequestAccepted] Owner=%s | Component=%s | %s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*FormatActionFeedbackRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordActionFeedbackChannelMatchedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FActionFeedbackRequest& InRequest, const TCHAR* InChannel, int32 InMatchCount)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Action|ChannelMatched] Owner=%s | Component=%s | Channel=%s | MatchCount=%d | %s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		InMatchCount,
		*FormatActionFeedbackRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordActionFeedbackPresentationPlayedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InEvent)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Action|PresentationPlayed] Event=%s | Owner=%s | Component=%s | Channel=%s | Asset=%s"),
		InEvent ? InEvent : TEXT("Play"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		*GetNameSafe(InAsset)));
}

void FCombatFeedbackDebug::RecordActionFeedbackRequestRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FActionFeedbackRequest& InRequest, const TCHAR* InReason)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Action|RequestRejected] Reason=%s | Owner=%s | Component=%s | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*FormatActionFeedbackRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordActionFeedbackChannelRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FActionFeedbackRequest& InRequest, const TCHAR* InChannel, const TCHAR* InReason, int32 InMatchCount)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Action|ChannelRejected] Reason=%s | Owner=%s | Component=%s | Channel=%s | MatchCount=%d | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		InMatchCount,
		*FormatActionFeedbackRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordActionFeedbackPresentationRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InReason)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Action|PresentationRejected] Reason=%s | Owner=%s | Component=%s | Channel=%s | Asset=%s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		*GetNameSafe(InAsset)));
}

// Reaction Feedback Diagnostic Hook

void FCombatFeedbackDebug::RecordReactionFeedbackRequestAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FReactionFeedbackRequest& InRequest)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Reaction|RequestAccepted] Owner=%s | Component=%s | %s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*FormatReactionFeedbackRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordReactionFeedbackChannelMatchedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FReactionFeedbackRequest& InRequest, const TCHAR* InChannel, int32 InMatchCount)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Reaction|ChannelMatched] Owner=%s | Component=%s | Channel=%s | MatchCount=%d | %s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		InMatchCount,
		*FormatReactionFeedbackRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordReactionFeedbackPresentationPlayedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InEvent)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Reaction|PresentationPlayed] Event=%s | Owner=%s | Component=%s | Channel=%s | Asset=%s"),
		InEvent ? InEvent : TEXT("Play"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		*GetNameSafe(InAsset)));
}

void FCombatFeedbackDebug::RecordReactionFeedbackRequestRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FReactionFeedbackRequest& InRequest, const TCHAR* InReason)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Reaction|RequestRejected] Reason=%s | Owner=%s | Component=%s | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*FormatReactionFeedbackRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordReactionFeedbackChannelRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FReactionFeedbackRequest& InRequest, const TCHAR* InChannel, const TCHAR* InReason, int32 InMatchCount)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Reaction|ChannelRejected] Reason=%s | Owner=%s | Component=%s | Channel=%s | MatchCount=%d | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		InMatchCount,
		*FormatReactionFeedbackRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordReactionFeedbackPresentationRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InReason)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Reaction|PresentationRejected] Reason=%s | Owner=%s | Component=%s | Channel=%s | Asset=%s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		*GetNameSafe(InAsset)));
}

// Hit Feedback Diagnostic Hook

void FCombatFeedbackDebug::RecordHitFeedbackRequestAcceptedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FCombatSignalTargetPacket& InPacket)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Hit|RequestAccepted] Owner=%s | Component=%s | %s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*FormatHitFeedbackPacket(InPacket)));
}

void FCombatFeedbackDebug::RecordHitFeedbackPresentationRequestedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FCombatSignalTargetPacket& InPacket, const TCHAR* InChannel)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Hit|PresentationRequested] Owner=%s | Component=%s | Channel=%s | %s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		*FormatHitFeedbackPacket(InPacket)));
}

void FCombatFeedbackDebug::RecordHitFeedbackPresentationPlayedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InEvent)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Hit|PresentationPlayed] Event=%s | Owner=%s | Component=%s | Channel=%s | Asset=%s"),
		InEvent ? InEvent : TEXT("Play"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		*GetNameSafe(InAsset)));
}

void FCombatFeedbackDebug::RecordHitFeedbackRequestRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FCombatSignalTargetPacket& InPacket, const TCHAR* InReason)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Hit|RequestRejected] Reason=%s | Owner=%s | Component=%s | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		*FormatHitFeedbackPacket(InPacket)));
}

void FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const FCombatSignalTargetPacket& InPacket, const TCHAR* InChannel, const TCHAR* InReason)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Hit|PresentationRejected] Reason=%s | Owner=%s | Component=%s | Channel=%s | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		*FormatHitFeedbackPacket(InPacket)));
}

void FCombatFeedbackDebug::RecordHitFeedbackAssetRejectedForAudit(const AActor* InOwnerActor, const UObject* InComponent, const TCHAR* InChannel, const UObject* InAsset, const TCHAR* InReason)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|Hit|AssetRejected] Reason=%s | Owner=%s | Component=%s | Channel=%s | Asset=%s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponent),
		InChannel ? InChannel : TEXT("Unknown"),
		*GetNameSafe(InAsset)));
}

// Shared Feedback Dispatch Diagnostic Hook

void FCombatFeedbackDebug::RecordCombatFeedbackHitStopRequestedForAudit(const FHitStopRequest& InRequest)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|SharedDispatch|HitStopRequested] %s"),
		*FormatHitStopRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordCombatFeedbackHitStopAppliedForAudit(const AActor* InActor, float InDuration, float InDilation)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|SharedDispatch|HitStopApplied] Owner=%s | Duration=%.3f | Dilation=%.3f"),
		*GetNameSafe(InActor),
		InDuration,
		InDilation));
}

void FCombatFeedbackDebug::RecordCombatFeedbackCameraShakeRequestedForAudit(const FCameraShakeRequest& InRequest)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|SharedDispatch|CameraShakeRequested] %s"),
		*FormatCameraShakeRequest(InRequest)));
}

void FCombatFeedbackDebug::RecordCombatFeedbackHitStopRejectedForAudit(const AActor* InActor, float InDuration, float InDilation, const TCHAR* InReason)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|SharedDispatch|HitStopRejected] Reason=%s | Owner=%s | Duration=%.3f | Dilation=%.3f"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InActor),
		InDuration,
		InDilation));
}

void FCombatFeedbackDebug::RecordCombatFeedbackRequestRejectedForAudit(const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditCombatFeedback()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatFeedback|SharedDispatch|RequestRejected] Event=%s | Reason=%s"),
		InEvent ? InEvent : TEXT("Request"),
		InReason ? InReason : TEXT("Rejected")));
}
