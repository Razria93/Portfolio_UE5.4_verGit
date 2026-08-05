#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"

#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING
namespace
{
	TAutoConsoleVariable<int32> CVarDebugOverlayEnabled(
		TEXT("Portfolio.DebugOverlay.Enabled"),
		0,
		TEXT("Draw debug overlay evidence HUD. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayCollect(
		TEXT("Portfolio.DebugOverlay.Collect"),
		0,
		TEXT("Collect debug overlay snapshot evidence. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayPreset(
		TEXT("Portfolio.DebugOverlay.Preset"),
		0,
		TEXT("Select debug overlay display preset. 0: P0 minimum."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayEventLogLimit(
		TEXT("Portfolio.DebugOverlay.EventLogLimit"),
		DebugOverlaySnapshotStoreInternals::DefaultEventLogDisplayLimit,
		TEXT("Number of recent debug overlay event lines to display. 0-32."),
		ECVF_Default);

	TAutoConsoleVariable<FString> CVarDebugOverlayEventLogFilter(
		TEXT("Portfolio.DebugOverlay.EventLogFilter"),
		TEXT("All"),
		TEXT("Filter debug overlay event log. Values: All, Execution, Combat, AI."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayHideNoiseEvents(
		TEXT("Portfolio.DebugOverlay.HideNoiseEvents"),
		0,
		TEXT("Hide noisy debug overlay event log entries. 0: show all, 1: hide reject/ignore noise."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayHideCollisionWindowEvents(
		TEXT("Portfolio.DebugOverlay.HideCollisionWindowEvents"),
		0,
		TEXT("Hide debug overlay collision window event log entries. 0: show all, 1: hide collision window events."),
		ECVF_Default);

	FString ExtractSummaryFieldValue(const FString& InSummary, const FString& InFieldName)
	{
		TArray<FString> summaryParts;
		InSummary.ParseIntoArray(summaryParts, TEXT("|"), true);

		for (FString summaryPart : summaryParts)
		{
			summaryPart.TrimStartAndEndInline();

			const FString colonPrefix = FString::Printf(TEXT("%s:"), *InFieldName);
			if (summaryPart.StartsWith(colonPrefix, ESearchCase::IgnoreCase))
			{
				FString value = summaryPart.RightChop(colonPrefix.Len());
				value.TrimStartAndEndInline();
				return value;
			}

			const FString equalsPrefix = FString::Printf(TEXT("%s="), *InFieldName);
			if (summaryPart.StartsWith(equalsPrefix, ESearchCase::IgnoreCase))
			{
				FString value = summaryPart.RightChop(equalsPrefix.Len());
				value.TrimStartAndEndInline();
				return value;
			}
		}

		return FString();
	}

	bool IsExecutionNoiseEvent(const FDebugOverlayEventEntry& InEntry)
	{
		if (!InEntry.Category.Equals(TEXT("Execution"), ESearchCase::IgnoreCase)) return false;
		if (!InEntry.EventName.Equals(TEXT("DecisionResolved"), ESearchCase::IgnoreCase)) return false;

		const FString decision = ExtractSummaryFieldValue(InEntry.Summary, TEXT("Decision"));
		if (decision.Equals(TEXT("Reject"), ESearchCase::IgnoreCase)
			|| decision.Equals(TEXT("Ignore"), ESearchCase::IgnoreCase))
		{
			return true;
		}

		const FString rejectReason = ExtractSummaryFieldValue(InEntry.Summary, TEXT("RejectReason"));
		return !rejectReason.IsEmpty() && !rejectReason.Equals(TEXT("None"), ESearchCase::IgnoreCase);
	}

	bool IsCollisionWindowEvent(const FDebugOverlayEventEntry& InEntry)
	{
		const FString category = InEntry.Category.TrimStartAndEnd();
		if (!category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)) return false;

		const FString eventName = InEntry.EventName.TrimStartAndEnd();
		return eventName.StartsWith(TEXT("CollisionEnabled"), ESearchCase::IgnoreCase)
			|| eventName.StartsWith(TEXT("CollisionDisabled"), ESearchCase::IgnoreCase)
			|| eventName.StartsWith(TEXT("CollisionDisableIgnored"), ESearchCase::IgnoreCase);
	}

	bool IsCollisionDisableIgnoredEvent(const FDebugOverlayEventEntry& InEntry)
	{
		const FString category = InEntry.Category.TrimStartAndEnd();
		if (!category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)) return false;

		const FString eventName = InEntry.EventName.TrimStartAndEnd();
		return eventName.StartsWith(TEXT("CollisionDisableIgnored"), ESearchCase::IgnoreCase)
			|| eventName.StartsWith(TEXT("CollisionDisabledIgnored"), ESearchCase::IgnoreCase);
	}

	bool IsEventExcludedByDisplayFilters(const FDebugOverlayEventEntry& InEntry)
	{
		if (SnapshotStoreConfig::ShouldHideNoiseEvents())
		{
			if (IsExecutionNoiseEvent(InEntry)) return true;
			if (IsCollisionDisableIgnoredEvent(InEntry)) return true;
		}

		if (SnapshotStoreConfig::ShouldHideCollisionWindowEvents() && IsCollisionWindowEvent(InEntry))
		{
			return true;
		}

		return false;
	}

	bool IsTargetPacketEvent(const FDebugOverlayEventEntry& InEntry)
	{
		return InEntry.Category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)
			&& (InEntry.EventName.Contains(TEXT("TargetAccepted"), ESearchCase::IgnoreCase)
				|| InEntry.EventName.Contains(TEXT("TargetRejected"), ESearchCase::IgnoreCase));
	}

	FString GetSubjectEventRoleLabel(const FDebugOverlayEventEntry& InEntry, const FString& InSubjectName)
	{
		if (!IsTargetPacketEvent(InEntry) || InSubjectName.IsEmpty()) return FString();

		const bool bIsSource = InEntry.SourceName == InSubjectName;
		const bool bIsTarget = InEntry.TargetName == InSubjectName;
		const bool bIsOwner = InEntry.OwnerName == InSubjectName;

		if (bIsSource && bIsTarget)
		{
			return TEXT("Self");
		}

		if (bIsSource)
		{
			return TEXT("Outgoing");
		}

		if (bIsTarget || bIsOwner)
		{
			return TEXT("Incoming");
		}

		return FString();
	}
}

bool SnapshotStoreConfig::IsEnabled()
{
	return CVarDebugOverlayEnabled.GetValueOnGameThread() != 0;
}

bool SnapshotStoreConfig::IsCollecting()
{
	return CVarDebugOverlayCollect.GetValueOnGameThread() != 0;
}

bool SnapshotStoreConfig::ShouldHideNoiseEvents()
{
	return CVarDebugOverlayHideNoiseEvents.GetValueOnGameThread() != 0;
}

bool SnapshotStoreConfig::ShouldHideCollisionWindowEvents()
{
	return CVarDebugOverlayHideCollisionWindowEvents.GetValueOnGameThread() != 0;
}

int32 SnapshotStoreConfig::GetEventLogDisplayLimitRaw()
{
	return CVarDebugOverlayEventLogLimit.GetValueOnGameThread();
}

FString SnapshotStoreConfig::GetEventLogFilterRaw()
{
	return CVarDebugOverlayEventLogFilter.GetValueOnGameThread();
}

FString EventFilterPolicy::NormalizeEventLogFilter(const FString& InFilter)
{
	if (InFilter.Equals(TEXT("Execution"), ESearchCase::IgnoreCase))
	{
		return TEXT("Execution");
	}

	if (InFilter.Equals(TEXT("Combat"), ESearchCase::IgnoreCase))
	{
		return TEXT("Combat");
	}

	if (InFilter.Equals(TEXT("AI"), ESearchCase::IgnoreCase))
	{
		return TEXT("AI");
	}

	return TEXT("All");
}

int32 EventFilterPolicy::GetClampedEventLogDisplayLimit()
{
	return FMath::Clamp(
		SnapshotStoreConfig::GetEventLogDisplayLimitRaw(),
		0,
		DebugOverlaySnapshotStoreInternals::MaxEventLogDisplayLimit);
}

FString EventFilterPolicy::GetCanonicalEventLogFilter()
{
	return NormalizeEventLogFilter(SnapshotStoreConfig::GetEventLogFilterRaw());
}

bool EventFilterPolicy::ShouldIncludeEventForDisplay(const FDebugOverlayEventEntry& InEntry, const FString& InFilter, bool bApplyDisplayFilters)
{
	const FString filter = NormalizeEventLogFilter(InFilter);
	if (filter != TEXT("All"))
	{
		if (filter == TEXT("Combat"))
		{
			const bool bIsCombatCategory =
				InEntry.Category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)
				|| InEntry.Category.Equals(TEXT("CombatResult"), ESearchCase::IgnoreCase);
			if (!bIsCombatCategory) return false;
		}
		else if (!InEntry.Category.Equals(filter, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	if (bApplyDisplayFilters && IsEventExcludedByDisplayFilters(InEntry)) return false;
	return true;
}

bool EventFilterPolicy::DoesEventMatchSubject(const FDebugOverlayEventEntry& InEntry, const FString& InSubjectName)
{
	if (InSubjectName.IsEmpty()) return false;

	const bool bMatchesAnyRole =
		InEntry.OwnerName == InSubjectName
		|| InEntry.SourceName == InSubjectName
		|| InEntry.TargetName == InSubjectName;

	if (InEntry.Category.Equals(TEXT("Execution"), ESearchCase::IgnoreCase))
	{
		return InEntry.OwnerName == InSubjectName;
	}

	if (InEntry.Category.Equals(TEXT("AI"), ESearchCase::IgnoreCase))
	{
		return InEntry.OwnerName == InSubjectName
			|| InEntry.SourceName == InSubjectName;
	}

	if (InEntry.Category.Equals(TEXT("CombatResult"), ESearchCase::IgnoreCase))
	{
		return InEntry.OwnerName == InSubjectName
			|| InEntry.TargetName == InSubjectName;
	}

	if (InEntry.Category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase))
	{
		if (InEntry.EventName.Contains(TEXT("Collision"), ESearchCase::IgnoreCase))
		{
			return InEntry.OwnerName == InSubjectName
				|| InEntry.SourceName == InSubjectName;
		}

		return bMatchesAnyRole;
	}

	return bMatchesAnyRole;
}

FDebugOverlayEventEntry EventFilterPolicy::MakeSubjectDisplayEventEntry(const FDebugOverlayEventEntry& InEntry, const FString& InSubjectName)
{
	FDebugOverlayEventEntry entry = InEntry;
	const FString roleLabel = GetSubjectEventRoleLabel(entry, InSubjectName);
	if (!roleLabel.IsEmpty())
	{
		entry.EventName = FString::Printf(TEXT("%s(%s)"), *entry.EventName, *roleLabel);
	}

	return entry;
}
#endif
