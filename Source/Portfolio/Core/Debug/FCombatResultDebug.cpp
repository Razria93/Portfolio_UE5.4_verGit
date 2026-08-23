#include "Core/Debug/FCombatResultDebug.h"

#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FLog.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarCombatResultAudit(
		TEXT("Portfolio.Debug.CombatResultAudit"),
		0,
		TEXT("Print combat result receiver diagnostic hook logs. 0: disabled, 1: enabled."),
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

	const UObject* ResolveCombatResultWorldContext(const AActor* InReceiverActor, const FCombatResultPacket& InPacket)
	{
		if (IsValid(InReceiverActor)) return InReceiverActor;
		if (IsValid(InPacket.TargetActor)) return InPacket.TargetActor;
		if (IsValid(InPacket.SourceActor)) return InPacket.SourceActor;
		if (IsValid(InPacket.DamageCauser)) return InPacket.DamageCauser;

		return nullptr;
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
	if (FDebugOverlaySnapshotStore::IsCollecting())
	{
		FDebugOverlaySnapshotStore::RecordCombatResult(
			ResolveCombatResultWorldContext(InReceiverActor, InPacket),
			InReceiverActor,
			InPacket,
			TEXT("PacketReceived"));
	}

	if (!ShouldAuditCombatResult()) return;

	FLog::Log(FString::Printf(
		TEXT("[CombatResult|Receiver|PacketReceived] Owner=%s | Receiver=%s | %s"),
		*GetNameSafe(InReceiverActor),
		*GetNameSafe(InReceiverActor),
		*FormatCombatResultPacket(InPacket)));
}
