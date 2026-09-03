#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"
#include "Core/Debug/FDebugOverlayEventCategory.h"

#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING
namespace
{
	// ===== CVars =====

	TAutoConsoleVariable<int32> CVarDebugOverlayHUDVisible(
		TEXT("Portfolio.DebugOverlay.HUDVisible"),
		0,
		TEXT("Show the Debug Overlay HUD and world diagnostics. 0: hidden, 1: visible."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayCaptureEnabled(
		TEXT("Portfolio.DebugOverlay.CaptureEnabled"),
		0,
		TEXT("Capture future Debug Overlay Event Log entries and Actor histories. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayEventLogLimit(
		TEXT("Portfolio.DebugOverlay.EventLogLimit"),
		DebugOverlaySnapshotStoreInternals::DefaultEventLogDisplayLimit,
		TEXT("Number of recent debug overlay event lines to display. 0-32."),
		ECVF_Default);

	TAutoConsoleVariable<FString> CVarDebugOverlayEventLogFilter(
		TEXT("Portfolio.DebugOverlay.EventLogFilter"),
		TEXT("All"),
		TEXT("Filter debug overlay event log. Values: All, ActionReaction, ExecutionSession, Combat, AI, Balance, Death, Facing."),
		ECVF_Default);

	TAutoConsoleVariable<FString> CVarDebugOverlayEventLogScope(
		TEXT("Portfolio.DebugOverlay.EventLogScope"),
		TEXT("World"),
		TEXT("Scope debug overlay event log. Values: World, FocusedEnemy."),
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

	// ===== Summary Parsing =====

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

	// ===== Display Filter Helpers =====

	bool IsActionReactionNoiseEvent(const FDebugOverlayEventEntry& InEntry)
	{
		if (!InEntry.Category.Equals(DebugOverlayEventCategory::ActionReaction, ESearchCase::IgnoreCase)) return false;
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
			if (IsActionReactionNoiseEvent(InEntry)) return true;
			if (IsCollisionDisableIgnoredEvent(InEntry)) return true;
		}

		if (SnapshotStoreConfig::ShouldHideCollisionWindowEvents() && IsCollisionWindowEvent(InEntry))
		{
			return true;
		}

		return false;
	}

}

// ===== Runtime Config Accessors =====

bool SnapshotStoreConfig::IsHudVisible()
{
	return CVarDebugOverlayHUDVisible.GetValueOnGameThread() != 0;
}

bool SnapshotStoreConfig::IsCollecting()
{
	return CVarDebugOverlayCaptureEnabled.GetValueOnGameThread() != 0;
}

int32 SnapshotStoreConfig::GetEventLogDisplayLimitRaw()
{
	return CVarDebugOverlayEventLogLimit.GetValueOnGameThread();
}

FString SnapshotStoreConfig::GetEventLogFilterRaw()
{
	return CVarDebugOverlayEventLogFilter.GetValueOnGameThread();
}

FString SnapshotStoreConfig::GetEventLogScopeRaw()
{
	return CVarDebugOverlayEventLogScope.GetValueOnGameThread();
}

bool SnapshotStoreConfig::ShouldHideNoiseEvents()
{
	return CVarDebugOverlayHideNoiseEvents.GetValueOnGameThread() != 0;
}

bool SnapshotStoreConfig::ShouldHideCollisionWindowEvents()
{
	return CVarDebugOverlayHideCollisionWindowEvents.GetValueOnGameThread() != 0;
}

// ===== Event Filter Policy =====

FString EventFilterPolicy::NormalizeEventLogFilter(const FString& InFilter)
{
	if (InFilter.Equals(DebugOverlayEventCategory::ActionReaction, ESearchCase::IgnoreCase))
	{
		return DebugOverlayEventCategory::ActionReaction;
	}

	if (InFilter.Equals(DebugOverlayEventCategory::ExecutionSession, ESearchCase::IgnoreCase))
	{
		return DebugOverlayEventCategory::ExecutionSession;
	}

	if (InFilter.Equals(DebugOverlayEventCategory::Combat, ESearchCase::IgnoreCase))
	{
		return DebugOverlayEventCategory::Combat;
	}

	if (InFilter.Equals(DebugOverlayEventCategory::AI, ESearchCase::IgnoreCase))
	{
		return DebugOverlayEventCategory::AI;
	}

	if (InFilter.Equals(DebugOverlayEventCategory::Balance, ESearchCase::IgnoreCase))
	{
		return DebugOverlayEventCategory::Balance;
	}

	if (InFilter.Equals(DebugOverlayEventCategory::Death, ESearchCase::IgnoreCase))
	{
		return DebugOverlayEventCategory::Death;
	}

	if (InFilter.Equals(DebugOverlayEventCategory::Facing, ESearchCase::IgnoreCase))
	{
		return DebugOverlayEventCategory::Facing;
	}

	return TEXT("All");
}

FString EventFilterPolicy::GetCanonicalEventLogFilter()
{
	return NormalizeEventLogFilter(SnapshotStoreConfig::GetEventLogFilterRaw());
}

FString EventFilterPolicy::NormalizeEventLogScope(const FString& InScope)
{
	return InScope.Equals(TEXT("FocusedEnemy"), ESearchCase::IgnoreCase)
		? TEXT("FocusedEnemy")
		: TEXT("World");
}

FString EventFilterPolicy::GetCanonicalEventLogScope()
{
	return NormalizeEventLogScope(SnapshotStoreConfig::GetEventLogScopeRaw());
}

int32 EventFilterPolicy::GetClampedEventLogDisplayLimit()
{
	return FMath::Clamp(
		SnapshotStoreConfig::GetEventLogDisplayLimitRaw(),
		0,
		DebugOverlaySnapshotStoreInternals::MaxEventLogDisplayLimit);
}

bool EventFilterPolicy::ShouldIncludeEventForDisplay(const FDebugOverlayEventEntry& InEntry, const FString& InFilter, bool bApplyDisplayFilters)
{
	const FString filter = NormalizeEventLogFilter(InFilter);
	if (filter != TEXT("All"))
	{
		if (!InEntry.Category.Equals(filter, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	if (bApplyDisplayFilters && IsEventExcludedByDisplayFilters(InEntry)) return false;
	return true;
}

// ===== Subject Matching =====

#endif
