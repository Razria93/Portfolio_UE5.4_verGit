#include "Core/Debug/FCombatParticipationDebug.h"

#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarCombatParticipationDebugEnabled(TEXT("Portfolio.DebugOverlay.CombatParticipation.Enabled"), 0, TEXT("Enable Combat Participation world debug and Overlay details. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarCombatParticipationDrawWorldText(TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldText"), 1, TEXT("Draw Combat Participation state text above Enemy actors. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarCombatParticipationDrawWorldRing(TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldRing"), 1, TEXT("Draw Combat Participation state rings below Enemy actors. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarCombatParticipationShowOverlayDetails(TEXT("Portfolio.DebugOverlay.CombatParticipation.ShowOverlayDetails"), 1, TEXT("Show Combat Participation details in Debug Overlay. 0: disabled, 1: enabled."), ECVF_Default);
#endif

	FString FormatRole(ECombatRole InRole)
	{
		switch (InRole)
		{
		case ECombatRole::Engage: return TEXT("Engage");
		case ECombatRole::Alert: return TEXT("Alert");
		case ECombatRole::Observe: return TEXT("Observe");
		default: return TEXT("None");
		}
	}

	FString FormatAdmission(EEngageAdmissionKind InAdmission)
	{
		switch (InAdmission)
		{
		case EEngageAdmissionKind::GeneralBase: return TEXT("GeneralBase");
		case EEngageAdmissionKind::HitReactiveExtra: return TEXT("HitReactiveExtra");
		default: return TEXT("None");
		}
	}

	FString FormatEvidence(const FCombatParticipationDebugEntry& InEntry)
	{
		if (InEntry.bHasPerceptionEvidence && InEntry.bHasHitReactiveEvidence) return TEXT("Perception + HitReactive");
		if (InEntry.bHasPerceptionEvidence) return TEXT("Perception");
		if (InEntry.bHasHitReactiveEvidence) return TEXT("HitReactive");
		return TEXT("None");
	}

	FString FormatAssignmentState(const FCombatParticipationDebugEntry& InEntry)
	{
		TArray<FString> states;
		if (InEntry.bHasAssignmentLock) states.Add(TEXT("AssignmentLock"));
		return states.IsEmpty() ? TEXT("None") : FString::Join(states, TEXT(" + "));
	}

	FColor ResolveRoleColor(const FCombatParticipationDebugEntry& InEntry)
	{
		if (InEntry.CombatRole == ECombatRole::Engage) return FColor::Orange;
		if (InEntry.CombatRole == ECombatRole::Alert) return FColor::Yellow;
		if (InEntry.CombatRole == ECombatRole::Observe) return FColor::Blue;
		return FColor::Silver;
	}
}

bool FCombatParticipationDebug::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarCombatParticipationDebugEnabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FCombatParticipationDebug::ShouldDrawWorldText()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarCombatParticipationDrawWorldText.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FCombatParticipationDebug::ShouldDrawWorldRing()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarCombatParticipationDrawWorldRing.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FCombatParticipationDebug::ShouldShowOverlayDetails()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarCombatParticipationShowOverlayDetails.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

FCombatParticipationDebugOverlayDetails FCombatParticipationDebug::BuildOverlayDetails(const FCombatParticipationDebugSnapshot& InSnapshot, const AActor* InParticipantActor)
{
	FCombatParticipationDebugOverlayDetails details;
	if (!InSnapshot.bHasSnapshot || !IsValid(InParticipantActor)) return details;

	for (const FCombatParticipationDebugEntry& entry : InSnapshot.Entries)
	{
		if (entry.ParticipantActor != InParticipantActor) continue;

		details.bHasSnapshot = true;
		details.RoleText = FormatRole(entry.CombatRole);
		details.AdmissionText = FormatAdmission(entry.EngageAdmission);
		details.EvidenceText = FormatEvidence(entry);
		details.TargetText = GetNameSafe(entry.TargetActor);
		details.AssignmentRevisionText = FString::FromInt(entry.AssignmentRevision);
		details.RetentionText = FormatAssignmentState(entry);
		return details;
	}

	return details;
}

TArray<FString> FCombatParticipationDebug::BuildWorldSummaryLines(const FCombatParticipationDebugSnapshot& InSnapshot)
{
	TArray<FString> lines;
	if (!InSnapshot.bHasSnapshot || InSnapshot.TargetSummaries.IsEmpty()) return lines;

	for (const FCombatParticipationDebugTargetSummary& summary : InSnapshot.TargetSummaries)
	{
		lines.Add(FString::Printf(
			TEXT("Target: %s | Engage: %d / %d (Base %d/%d | Extra %d/%d) | Alert: %d / %d | Observe: %d / %d"),
			*GetNameSafe(summary.TargetActor),
			summary.EngageCount, summary.TotalEngageCap,
			summary.GeneralBaseEngageCount, summary.GeneralBaseEngageCap,
			summary.HitReactiveExtraEngageCount, summary.HitReactiveExtraEngageCap,
			summary.AlertCount, summary.AlertCap,
			summary.ObserveCount, summary.ObserveCap));
	}

	return lines;
}

void FCombatParticipationDebug::DrawWorldDebug(UWorld* InWorld, const FCombatParticipationDebugSnapshot& InSnapshot)
{
	if (!IsEnabled() || !IsValid(InWorld) || !InSnapshot.bHasSnapshot) return;

	for (const FCombatParticipationDebugEntry& entry : InSnapshot.Entries)
	{
		if (!IsValid(entry.ParticipantActor)) continue;

		const FVector location = entry.ParticipantActor->GetActorLocation();
		const FColor color = ResolveRoleColor(entry);
		if (ShouldDrawWorldRing())
		{
			DrawDebugCircle(InWorld, location + FVector(0.f, 0.f, 8.f), 42.f, 24, color, false, 0.f, 0, 2.f, FVector::ForwardVector, FVector::RightVector, false);
			if (entry.bHasAssignmentLock) DrawDebugCircle(InWorld, location + FVector(0.f, 0.f, 10.f), 48.f, 24, FColor::White, false, 0.f, 0, 1.f, FVector::ForwardVector, FVector::RightVector, false);
		}

		if (ShouldDrawWorldText())
		{
			const FString text = FString::Printf(TEXT("[%s | %s]\nEvidence: %s\nTarget: %s | Rev: %d | State: %s"),
				*FormatRole(entry.CombatRole), *FormatAdmission(entry.EngageAdmission), *FormatEvidence(entry), *GetNameSafe(entry.TargetActor), entry.AssignmentRevision,
				*FormatAssignmentState(entry));
			DrawDebugString(InWorld, location + FVector(0.f, 0.f, 120.f), text, nullptr, color, 0.f, false, 1.f);
		}
	}
}
