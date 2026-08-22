#include "Core/Debug/FCombatParticipationDebug.h"

#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarCombatParticipationDebugEnabled(TEXT("Portfolio.DebugOverlay.CombatParticipation.Enabled"), 0, TEXT("Enable Combat Participation debug data and world visualization. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarCombatParticipationDrawWorldText(TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldText"), 1, TEXT("Draw Combat Participation state text above Enemy actors. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarCombatParticipationDrawWorldRing(TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldRing"), 1, TEXT("Draw Combat Participation state rings below Enemy actors. 0: disabled, 1: enabled."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarCombatParticipationDrawHitReactiveEvidenceAnchor(TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawHitReactiveEvidenceAnchor"), 0, TEXT("Draw each live HitReactive Evidence anchor point, Target connection and 2D radius. 0: disabled, 1: enabled."), ECVF_Default);
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

	FString FormatPerceptionEvidenceLifetime(const FCombatParticipationDebugEntry& InEntry)
	{
		if (!InEntry.bHasPerceptionEvidence) return FString();
		if (!InEntry.bHasPerceptionEvidenceLifetimeState) return TEXT("Context unavailable");
		if (InEntry.bHasPerceptionLOS) return TEXT("LOS");

		return FString::Printf(TEXT("Memory %.1fs"), InEntry.PerceptionMemoryRemainingSeconds);
	}

	FString FormatHitReactiveEvidenceLifetime(const FCombatParticipationDebugEntry& InEntry)
	{
		if (!InEntry.bHasHitReactiveEvidence) return FString();
		if (!InEntry.bHasStartedHitReactivePostReactionTTL) return TEXT("Awaiting reaction");

		return FString::Printf(TEXT("TTL %.1fs"), InEntry.HitReactivePostReactionTTLRemainingSeconds);
	}

	FString FormatAssignmentState(const FCombatParticipationDebugEntry& InEntry)
	{
		TArray<FString> states;
		if (InEntry.bHasAssignmentLock) states.Add(TEXT("AssignmentLock"));
		return states.IsEmpty() ? TEXT("None") : FString::Join(states, TEXT(" + "));
	}

	FColor ResolveRoleColor(const FCombatParticipationDebugEntry& InEntry)
	{
		if (InEntry.CombatRole == ECombatRole::Engage)
		{
			if (InEntry.bHasPerceptionEvidence && InEntry.bHasHitReactiveEvidence) return FColor::Red;
			if (InEntry.bHasHitReactiveEvidence) return FColor::Magenta;
			return FColor::Orange;
		}
		if (InEntry.CombatRole == ECombatRole::Alert) return FColor::Yellow;
		if (InEntry.CombatRole == ECombatRole::Observe) return FColor::Blue;
		return FColor::Silver;
	}

	void DrawHitReactiveEvidenceAnchor(UWorld* InWorld, const FCombatParticipationDebugEntry& InEntry)
	{
		if (!IsValid(InWorld) || !InEntry.bHasHitReactiveEvidence || !InEntry.bHasHitReactiveEvidenceAnchor || !IsValid(InEntry.TargetActor)) return;

		const FColor anchorColor = FColor::Cyan;
		const FVector anchorLocation = InEntry.HitReactiveEvidenceAnchorLocation;
		const FVector targetLocation = InEntry.TargetActor->GetActorLocation();
		DrawDebugPoint(InWorld, anchorLocation + FVector(0.f, 0.f, 8.f), 16.f, anchorColor, false, 0.f);
		DrawDebugLine(InWorld, anchorLocation + FVector(0.f, 0.f, 8.f), targetLocation + FVector(0.f, 0.f, 16.f), anchorColor, false, 0.f, 0, 1.5f);
		DrawDebugCircle(InWorld, anchorLocation + FVector(0.f, 0.f, 4.f), InEntry.HitReactiveEvidenceAnchorRadius, 48, anchorColor, false, 0.f, 0, 1.5f, FVector::ForwardVector, FVector::RightVector, false);
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

bool FCombatParticipationDebug::ShouldDrawHitReactiveEvidenceAnchor()
{
#if !UE_BUILD_SHIPPING
	return IsEnabled() && CVarCombatParticipationDrawHitReactiveEvidenceAnchor.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

FCombatParticipationDebugOverlayDetails FCombatParticipationDebug::BuildOverlayDetails(const FCombatParticipationDebugSnapshot& InSnapshot, const AActor* InParticipantActor)
{
	FCombatParticipationDebugOverlayDetails details;
	if (!IsEnabled() || !InSnapshot.bHasSnapshot || !IsValid(InParticipantActor)) return details;

	for (const FCombatParticipationDebugEntry& entry : InSnapshot.Entries)
	{
		if (entry.ParticipantActor != InParticipantActor) continue;

		details.bHasSnapshot = true;
		details.RoleText = FormatRole(entry.CombatRole);
		details.AdmissionText = FormatAdmission(entry.EngageAdmission);
		details.EvidenceText = FormatEvidence(entry);
		details.PerceptionLifetimeText = FormatPerceptionEvidenceLifetime(entry);
		details.HitReactiveLifetimeText = FormatHitReactiveEvidenceLifetime(entry);
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
	if (!IsEnabled() || !InSnapshot.bHasSnapshot || InSnapshot.TargetSummaries.IsEmpty()) return lines;

	for (int32 summaryIndex = 0; summaryIndex < InSnapshot.TargetSummaries.Num(); ++summaryIndex)
	{
		const FCombatParticipationDebugTargetSummary& summary = InSnapshot.TargetSummaries[summaryIndex];
		if (summaryIndex > 0)
		{
			lines.Add(TEXT(""));
		}

		lines.Add(FString::Printf(TEXT("Target: %s"), *GetNameSafe(summary.TargetActor)));
		lines.Add(FString::Printf(TEXT("Engage: %d / %d"), summary.EngageCount, summary.TotalEngageCap));
		lines.Add(FString::Printf(TEXT("  - Base: %d / %d"), summary.GeneralBaseEngageCount, summary.GeneralBaseEngageCap));
		lines.Add(FString::Printf(TEXT("  - HitReactive Extra: %d / %d"), summary.HitReactiveExtraEngageCount, summary.HitReactiveExtraEngageCap));
		lines.Add(FString::Printf(TEXT("Alert: %d / %d"), summary.AlertCount, summary.AlertCap));
		lines.Add(FString::Printf(TEXT("Observe: %d / %d"), summary.ObserveCount, summary.ObserveCap));
	}

	return lines;
}

void FCombatParticipationDebug::DrawWorldDebug(UWorld* InWorld, const FCombatParticipationDebugSnapshot& InSnapshot)
{
	if (!IsEnabled() || !IsValid(InWorld) || !InSnapshot.bHasSnapshot) return;

	TMap<const AActor*, int32> worldTextRowByParticipant;
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
		if (ShouldDrawHitReactiveEvidenceAnchor()) DrawHitReactiveEvidenceAnchor(InWorld, entry);

		if (ShouldDrawWorldText())
		{
			TArray<FString> textLines;
			textLines.Add(FString::Printf(TEXT("[%s | %s]"), *FormatRole(entry.CombatRole), *FormatAdmission(entry.EngageAdmission)));
			textLines.Add(FString::Printf(TEXT("Evidence: %s"), *FormatEvidence(entry)));

			const FString perceptionLifetimeText = FormatPerceptionEvidenceLifetime(entry);
			if (!perceptionLifetimeText.IsEmpty()) textLines.Add(FString::Printf(TEXT("Perception: %s"), *perceptionLifetimeText));

			const FString hitReactiveLifetimeText = FormatHitReactiveEvidenceLifetime(entry);
			if (!hitReactiveLifetimeText.IsEmpty()) textLines.Add(FString::Printf(TEXT("HitReactive: %s"), *hitReactiveLifetimeText));

			textLines.Add(FString::Printf(TEXT("Target: %s | Rev: %d | State: %s"), *GetNameSafe(entry.TargetActor), entry.AssignmentRevision, *FormatAssignmentState(entry)));

			int32& worldTextRow = worldTextRowByParticipant.FindOrAdd(entry.ParticipantActor);
			const FVector textLocation = location + FVector(0.f, 0.f, 120.f + (worldTextRow++ * 96.f));
			DrawDebugString(InWorld, textLocation, FString::Join(textLines, TEXT("\n")), nullptr, color, 0.f, false, 1.f);
		}
	}
}
