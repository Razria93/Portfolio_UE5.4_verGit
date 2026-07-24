#include "Core/Debug/FCombatResultDebug.h"

#include "Core/Debug/FLog.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarCombatResultAudit(
		TEXT("Portfolio.Debug.CombatResultAudit"),
		0,
		TEXT("Print combat result receiver, parry stack, and stagger reaction diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatCombatResultDamageSpecKey(const FDamageSpecKey& InKey)
	{
		return FString::Printf(
			TEXT("WeaponType=%s | ActionType=%s | ActionIndex=%d"),
			*UEnum::GetValueAsString(InKey.WeaponType),
			*UEnum::GetValueAsString(InKey.ActionType),
			InKey.ActionIndex);
	}

	FString FormatCombatResultPacket(const FCombatResultPacket& InPacket)
	{
		return FString::Printf(
			TEXT("Source=%s | Target=%s | Instigator=%s | DamageCauser=%s | Outcome=%s | DamageCommitted=%s | CommittedDamage=%.3f | %s"),
			*GetNameSafe(InPacket.SourceActor),
			*GetNameSafe(InPacket.TargetActor),
			*GetNameSafe(InPacket.Instigator),
			*GetNameSafe(InPacket.DamageCauser),
			*UEnum::GetValueAsString(InPacket.DefenseOutcome),
			InPacket.bDamageCommitted ? TEXT("true") : TEXT("false"),
			InPacket.CommittedDamage,
			*FormatCombatResultDamageSpecKey(InPacket.DamageSpecKey));
	}

	FString FormatReactionRequestResult(const FReactionRequestResult& InResult)
	{
		return FString::Printf(
			TEXT("Result=%s | RejectReason=%s"),
			*UEnum::GetValueAsString(InResult.ResultType),
			*UEnum::GetValueAsString(InResult.RejectReason));
	}
}

// Gate

bool FCombatResultDebug::ShouldAuditCombatResult()
{
#if !UE_BUILD_SHIPPING
	return CVarCombatResultAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Combat Result Receiver Diagnostic Hook

void FCombatResultDebug::RecordCombatResultReceivedForAudit(const AActor* InReceiverActor, const FCombatResultPacket& InPacket)
{
	if (!ShouldAuditCombatResult()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatResult|Receiver|PacketReceived] Owner=%s | Receiver=%s | %s"),
		*GetNameSafe(InReceiverActor),
		*GetNameSafe(InReceiverActor),
		*FormatCombatResultPacket(InPacket)));
}

// Parry Result Diagnostic Hook

void FCombatResultDebug::RecordParryStackUpdatedForAudit(const AActor* InReceiverActor, const FCombatResultPacket& InPacket, int32 InCount, int32 InThreshold, bool bInStaggerReady)
{
	if (!ShouldAuditCombatResult()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatResult|Parry|StackUpdated] Owner=%s | Receiver=%s | Requester=%s | Count=%d/%d | StaggerReady=%s | Outcome=%s"),
		*GetNameSafe(InReceiverActor),
		*GetNameSafe(InReceiverActor),
		*GetNameSafe(InPacket.TargetActor),
		InCount,
		InThreshold,
		bInStaggerReady ? TEXT("true") : TEXT("false"),
		*UEnum::GetValueAsString(InPacket.DefenseOutcome)));
}

void FCombatResultDebug::RecordParryStaggerReactionRequestedForAudit(const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const FReactionRequestResult& InResult)
{
	if (!ShouldAuditCombatResult()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatResult|Parry|StaggerReactionRequested] Owner=%s | Receiver=%s | Requester=%s | %s | %s"),
		*GetNameSafe(InReceiverActor),
		*GetNameSafe(InReceiverActor),
		*GetNameSafe(InPacket.TargetActor),
		*FormatReactionRequestResult(InResult),
		*FormatCombatResultPacket(InPacket)));
}

void FCombatResultDebug::RecordParryStaggerReactionRejectedForAudit(const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const TCHAR* InReason)
{
	if (!ShouldAuditCombatResult()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatResult|Parry|StaggerReactionRejected] Reason=%s | Owner=%s | Receiver=%s | Requester=%s | %s"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InReceiverActor),
		*GetNameSafe(InReceiverActor),
		*GetNameSafe(InPacket.TargetActor),
		*FormatCombatResultPacket(InPacket)));
}
