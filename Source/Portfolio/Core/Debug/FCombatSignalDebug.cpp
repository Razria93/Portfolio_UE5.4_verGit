#include "Core/Debug/FCombatSignalDebug.h"
#include "Core/Debug/FLog.h"

#include "Components/PrimitiveComponent.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarCombatSignalAudit(
		TEXT("Portfolio.Debug.CombatSignalAudit"),
		0,
		TEXT("Print combat signal diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarCombatSignalDump(
		TEXT("Portfolio.Debug.CombatSignalDump"),
		0,
		TEXT("Print combat signal debug dump logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatCombatSignalDamageSpecKey(const FDamageSpecKey& InDamageSpecKey)
	{
		return FString::Printf(
			TEXT("WeaponType=%s ActionType=%s ActionIndex=%d"),
			*UEnum::GetValueAsString(InDamageSpecKey.WeaponType),
			*UEnum::GetValueAsString(InDamageSpecKey.ActionType),
			InDamageSpecKey.ActionIndex);
	}

	FString FormatCombatSignalHitContextDamageSpecKey(const FHitContext& InHitContext)
	{
		FDamageSpecKey damageSpecKey;
		damageSpecKey.WeaponType = InHitContext.WeaponContext.WeaponType;
		damageSpecKey.ActionType = InHitContext.ActionContext.ActionType;
		damageSpecKey.ActionIndex = InHitContext.ActionContext.ActionIndex;

		return FormatCombatSignalDamageSpecKey(damageSpecKey);
	}
}

// Gate

bool FCombatSignalDebug::ShouldAuditCombatSignal()
{
#if !UE_BUILD_SHIPPING
	return CVarCombatSignalAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FCombatSignalDebug::ShouldPrintCombatSignalDebug()
{
#if !UE_BUILD_SHIPPING
	return CVarCombatSignalDump.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Weapon Actor Diagnostic Hook

void FCombatSignalDebug::RecordWeaponCollisionWindowForAudit(const AActor* InOwnerActor, const AActor* InWeaponActor, FName InCollisionName, int32 InHitWindowId, int32 InCollisionCount, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|WeaponActor|%s] Owner=%s | Weapon=%s | CollisionName=%s | HitWindowId=%d | CollisionCount=%d | Reason=%s"),
		InEvent ? InEvent : TEXT("CollisionWindow"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InWeaponActor),
		*InCollisionName.ToString(),
		InHitWindowId,
		InCollisionCount,
		InReason ? InReason : TEXT("None")));
}

void FCombatSignalDebug::RecordWeaponOverlapAcceptedForAudit(const FHitContext& InHitContext, const TCHAR* InEvent)
{
	if (!ShouldAuditCombatSignal()) return;

	const FOverlapContext& overlapContext = InHitContext.OverlapContext;

	FLog::Log(FString::Printf(
		TEXT("[Combat|WeaponActor|%sAccepted] Owner=%s | Weapon=%s | OverlapComp=%s | OtherActor=%s | OtherComp=%s | HitWindowId=%d | ImpactSource=%s | %s"),
		InEvent ? InEvent : TEXT("Overlap"),
		*GetNameSafe(overlapContext.OwnerActor),
		*GetNameSafe(overlapContext.DamageCauser),
		*GetNameSafe(overlapContext.OverlappedComponent),
		*GetNameSafe(overlapContext.OtherActor),
		*GetNameSafe(overlapContext.OtherComponent),
		overlapContext.HitWindowId,
		*UEnum::GetValueAsString(InHitContext.DamageImpactInfo.Source),
		*FormatCombatSignalHitContextDamageSpecKey(InHitContext)));
}

void FCombatSignalDebug::RecordWeaponOverlapRejectedForAudit(const AActor* InOwnerActor, const AActor* InWeaponActor, const UPrimitiveComponent* InOverlappedComponent, const AActor* InOtherActor, const UPrimitiveComponent* InOtherComponent, int32 InHitWindowId, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|WeaponActor|%sRejected] Reason=%s | Owner=%s | Weapon=%s | OverlapComp=%s | OtherActor=%s | OtherComp=%s | HitWindowId=%d"),
		InEvent ? InEvent : TEXT("Overlap"),
		InReason ? InReason : TEXT("InvalidOverlap"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InWeaponActor),
		*GetNameSafe(InOverlappedComponent),
		*GetNameSafe(InOtherActor),
		*GetNameSafe(InOtherComponent),
		InHitWindowId));
}

void FCombatSignalDebug::RecordWeaponOverlapIgnoredForAudit(const AActor* InOwnerActor, const AActor* InWeaponActor, const UPrimitiveComponent* InOverlappedComponent, const AActor* InOtherActor, const UPrimitiveComponent* InOtherComponent, int32 InHitWindowId, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|WeaponActor|%sIgnored] Reason=%s | Owner=%s | Weapon=%s | OverlapComp=%s | OtherActor=%s | OtherComp=%s | HitWindowId=%d"),
		InEvent ? InEvent : TEXT("Overlap"),
		InReason ? InReason : TEXT("IgnoredOverlap"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InWeaponActor),
		*GetNameSafe(InOverlappedComponent),
		*GetNameSafe(InOtherActor),
		*GetNameSafe(InOtherComponent),
		InHitWindowId));
}

// Weapon Actor Debug Dump

void FCombatSignalDebug::PrintWeaponHitContextDebug(const FHitContext& InHitContext)
{
	if (!ShouldPrintCombatSignalDebug()) return;

	const FOverlapContext& overlapContext = InHitContext.OverlapContext;

	FLog::Log(FString::Printf(
		TEXT("[Combat|WeaponActor|HitContextDump] Owner=%s | Weapon=%s | OverlapComp=%s | OtherActor=%s | OtherComp=%s | BodyIndex=%d | FromSweep=%s | HitWindowId=%d | ImpactValid=%s | ImpactSource=%s | %s"),
		*GetNameSafe(overlapContext.OwnerActor),
		*GetNameSafe(overlapContext.DamageCauser),
		*GetNameSafe(overlapContext.OverlappedComponent),
		*GetNameSafe(overlapContext.OtherActor),
		*GetNameSafe(overlapContext.OtherComponent),
		overlapContext.OtherBodyIndex,
		overlapContext.bFromSweep ? TEXT("true") : TEXT("false"),
		overlapContext.HitWindowId,
		InHitContext.DamageImpactInfo.bHasHitResult ? TEXT("true") : TEXT("false"),
		*UEnum::GetValueAsString(InHitContext.DamageImpactInfo.Source),
		*FormatCombatSignalHitContextDamageSpecKey(InHitContext)));
}

// Source Diagnostic Hook

void FCombatSignalDebug::RecordSourceHitRequestRejectedForAudit(const FHitContext& InHitContext, const TCHAR* InReason)
{
	if (!ShouldAuditCombatSignal()) return;

	const FOverlapContext& overlapContext = InHitContext.OverlapContext;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalSource|Rejected] Reason=%s | Source=%s | Target=%s | DamageCauser=%s | HitWindowId=%d | %s"),
		InReason ? InReason : TEXT("InvalidRequest"),
		*GetNameSafe(overlapContext.OwnerActor),
		*GetNameSafe(overlapContext.OtherActor),
		*GetNameSafe(overlapContext.DamageCauser),
		overlapContext.HitWindowId,
		*FormatCombatSignalHitContextDamageSpecKey(InHitContext)));
}

void FCombatSignalDebug::RecordSourceRejectedForAudit(const FCombatSignalSourceContext& InContext)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalSource|Rejected] Reason=%s | Source=%s | Target=%s | DamageCauser=%s | Instigator=%s | HitWindowId=%d | %s | RequestDamage=%.3f | CommittedDamage=%.3f"),
		*UEnum::GetValueAsString(InContext.RejectReason),
		*GetNameSafe(InContext.SourceActor),
		*GetNameSafe(InContext.TargetActor),
		*GetNameSafe(InContext.DamageCauser),
		*GetNameSafe(InContext.Instigator),
		InContext.HitWindowKey.HitWindowId,
		*FormatCombatSignalDamageSpecKey(InContext.DamageSpecKey),
		InContext.DamageAmount.RequestDamage,
		InContext.CommittedDamage));
}

void FCombatSignalDebug::RecordSourceAcceptedForAudit(const FCombatSignalSourceContext& InContext)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalSource|Accepted] Source=%s | Target=%s | DamageCauser=%s | Instigator=%s | HitWindowId=%d | %s | RequestDamage=%.3f | CommittedDamage=%.3f"),
		*GetNameSafe(InContext.SourceActor),
		*GetNameSafe(InContext.TargetActor),
		*GetNameSafe(InContext.DamageCauser),
		*GetNameSafe(InContext.Instigator),
		InContext.HitWindowKey.HitWindowId,
		*FormatCombatSignalDamageSpecKey(InContext.DamageSpecKey),
		InContext.DamageAmount.RequestDamage,
		InContext.CommittedDamage));
}

void FCombatSignalDebug::RecordCueRejectedForAudit(const FCombatSignal& InSignal, const TCHAR* InReason)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalCue|Rejected] Reason=%s | CueTag=%s | Source=%s | Target=%s | Causer=%s"),
		InReason ? InReason : TEXT("InvalidCue"),
		*InSignal.CueTag.ToString(),
		*GetNameSafe(InSignal.Header.SourceActor),
		*GetNameSafe(InSignal.Header.TargetActor),
		*GetNameSafe(InSignal.Header.SignalCauser)));
}

void FCombatSignalDebug::RecordCueAcceptedForAudit(const FCombatSignal& InSignal)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalCue|Accepted] CueTag=%s | Source=%s | Target=%s | Causer=%s"),
		*InSignal.CueTag.ToString(),
		*GetNameSafe(InSignal.Header.SourceActor),
		*GetNameSafe(InSignal.Header.TargetActor),
		*GetNameSafe(InSignal.Header.SignalCauser)));
}

// Source Debug Dump

void FCombatSignalDebug::PrintSourceContextDebug(const FCombatSignalSourceContext& InContext)
{
	if (!ShouldPrintCombatSignalDebug()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalSource|ContextDump] Accepted=%s | Reason=%s | Source=%s | Target=%s | DamageCauser=%s | Instigator=%s | HitWindowId=%d | %s | BaseDamage=%.3f | RequestDamage=%.3f | CommittedDamage=%.3f"),
		InContext.bAccepted ? TEXT("true") : TEXT("false"),
		*UEnum::GetValueAsString(InContext.RejectReason),
		*GetNameSafe(InContext.SourceActor),
		*GetNameSafe(InContext.TargetActor),
		*GetNameSafe(InContext.DamageCauser),
		*GetNameSafe(InContext.Instigator),
		InContext.HitWindowKey.HitWindowId,
		*FormatCombatSignalDamageSpecKey(InContext.DamageSpecKey),
		InContext.DamageSpec.BaseDamage,
		InContext.DamageAmount.RequestDamage,
		InContext.CommittedDamage));
}

// Target Diagnostic Hook

void FCombatSignalDebug::RecordTargetDamageRequestRejectedForAudit(float InDamageAmount, const FDamageEvent& InDamageEvent, AController* InEventInstigator, AActor* InDamageCauser, const TCHAR* InReason)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalTarget|Rejected] Reason=%s | DamageAmount=%.3f | DamageEventType=%d | Instigator=%s | DamageCauser=%s"),
		InReason ? InReason : TEXT("InvalidDamageRequest"),
		InDamageAmount,
		InDamageEvent.GetTypeID(),
		*GetNameSafe(InEventInstigator),
		*GetNameSafe(InDamageCauser)));
}

void FCombatSignalDebug::RecordTargetRejectedForAudit(const FCombatSignalTargetPacket& InPacket)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalTarget|Rejected] Reason=%s | Outcome=%s | Source=%s | Target=%s | DamageCauser=%s | Instigator=%s | %s | RequestDamage=%.3f | MitigatedDamage=%.3f | FinalTakenDamage=%.3f | CommittedDamage=%.3f"),
		*UEnum::GetValueAsString(InPacket.Result.RejectReason),
		*UEnum::GetValueAsString(InPacket.Result.DefenseOutcome),
		*GetNameSafe(InPacket.Context.SourceActor),
		*GetNameSafe(InPacket.Context.TargetActor),
		*GetNameSafe(InPacket.Context.DamageCauser),
		*GetNameSafe(InPacket.Context.Instigator),
		*FormatCombatSignalDamageSpecKey(InPacket.Result.DamageSpecKey),
		InPacket.Result.RequestDamage,
		InPacket.Result.MitigatedDamage,
		InPacket.Result.FinalTakenDamage,
		InPacket.Result.CommittedDamage));
}

void FCombatSignalDebug::RecordTargetAcceptedForAudit(const FCombatSignalTargetPacket& InPacket)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalTarget|Accepted] Outcome=%s | Source=%s | Target=%s | DamageCauser=%s | Instigator=%s | %s | RequestDamage=%.3f | MitigatedDamage=%.3f | FinalTakenDamage=%.3f | CommittedDamage=%.3f | HP=%.3f->%.3f | DeadState=%s->%s"),
		*UEnum::GetValueAsString(InPacket.Result.DefenseOutcome),
		*GetNameSafe(InPacket.Context.SourceActor),
		*GetNameSafe(InPacket.Context.TargetActor),
		*GetNameSafe(InPacket.Context.DamageCauser),
		*GetNameSafe(InPacket.Context.Instigator),
		*FormatCombatSignalDamageSpecKey(InPacket.Result.DamageSpecKey),
		InPacket.Result.RequestDamage,
		InPacket.Result.MitigatedDamage,
		InPacket.Result.FinalTakenDamage,
		InPacket.Result.CommittedDamage,
		InPacket.Context.HealthPointBefore,
		InPacket.Context.HealthPointAfter,
		*UEnum::GetValueAsString(InPacket.Result.DeadState_Before),
		*UEnum::GetValueAsString(InPacket.Result.DeadState_After)));
}

void FCombatSignalDebug::RecordTimingCueRejectedForAudit(const FCombatSignal& InSignal, const TCHAR* InReason)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalTargetCue|Rejected] Reason=%s | CueTag=%s | Source=%s | Target=%s | Causer=%s"),
		InReason ? InReason : TEXT("InvalidTimingCue"),
		*InSignal.CueTag.ToString(),
		*GetNameSafe(InSignal.Header.SourceActor),
		*GetNameSafe(InSignal.Header.TargetActor),
		*GetNameSafe(InSignal.Header.SignalCauser)));
}

void FCombatSignalDebug::RecordTimingCueAcceptedForAudit(const FCombatSignal& InSignal)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalTargetCue|Accepted] CueTag=%s | Source=%s | Target=%s | Causer=%s"),
		*InSignal.CueTag.ToString(),
		*GetNameSafe(InSignal.Header.SourceActor),
		*GetNameSafe(InSignal.Header.TargetActor),
		*GetNameSafe(InSignal.Header.SignalCauser)));
}

// Target Debug Dump

void FCombatSignalDebug::PrintTargetPacketDebug(const FCombatSignalTargetPacket& InPacket)
{
	if (!ShouldPrintCombatSignalDebug()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|SignalTarget|PacketDump] Accepted=%s | Reason=%s | Outcome=%s | Source=%s | Target=%s | DamageCauser=%s | Instigator=%s | %s | RequestDamage=%.3f | MitigatedDamage=%.3f | FinalTakenDamage=%.3f | CommittedDamage=%.3f | HP=%.3f->%.3f"),
		InPacket.Result.bAccepted ? TEXT("true") : TEXT("false"),
		*UEnum::GetValueAsString(InPacket.Result.RejectReason),
		*UEnum::GetValueAsString(InPacket.Result.DefenseOutcome),
		*GetNameSafe(InPacket.Context.SourceActor),
		*GetNameSafe(InPacket.Context.TargetActor),
		*GetNameSafe(InPacket.Context.DamageCauser),
		*GetNameSafe(InPacket.Context.Instigator),
		*FormatCombatSignalDamageSpecKey(InPacket.Result.DamageSpecKey),
		InPacket.Result.RequestDamage,
		InPacket.Result.MitigatedDamage,
		InPacket.Result.FinalTakenDamage,
		InPacket.Result.CommittedDamage,
		InPacket.Context.HealthPointBefore,
		InPacket.Context.HealthPointAfter));
}

// Shared Dispatch Diagnostic Hook

void FCombatSignalDebug::RecordCombatResultDispatchForAudit(const FCombatSignalTargetPacket& InTargetPacket, const FCombatResultPacket& InResultPacket, const AActor* InReceiverActor, const TCHAR* InEvent)
{
	if (!ShouldAuditCombatSignal()) return;

	FLog::Log(FString::Printf(
		TEXT("[Combat|ResultDispatch|%s] Outcome=%s | Source=%s | Target=%s | DamageCauser=%s | Receiver=%s | Requester=%s | CommittedDamage=%.3f"),
		InEvent ? InEvent : TEXT("Unknown"),
		*UEnum::GetValueAsString(InResultPacket.DefenseOutcome),
		*GetNameSafe(InResultPacket.SourceActor),
		*GetNameSafe(InResultPacket.TargetActor),
		*GetNameSafe(InResultPacket.DamageCauser),
		*GetNameSafe(InReceiverActor),
		*GetNameSafe(InTargetPacket.Context.TargetActor),
		InResultPacket.CommittedDamage));
}
