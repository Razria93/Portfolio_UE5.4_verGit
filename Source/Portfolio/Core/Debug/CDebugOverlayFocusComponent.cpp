#include "Core/Debug/CDebugOverlayFocusComponent.h"

#include "GameFramework/Actor.h"

namespace
{
	FString FormatDebugOverlayFocusSource(EDebugOverlayFocusSource InSource)
	{
		switch (InSource)
		{
		case EDebugOverlayFocusSource::NearestEnemy:
			return TEXT("FocusComponent.NearestEnemy");
		case EDebugOverlayFocusSource::RecentCombat:
			return TEXT("FocusComponent.RecentCombat");
		case EDebugOverlayFocusSource::WorldScanFallback:
			return TEXT("FocusComponent.WorldScanFallback");
		case EDebugOverlayFocusSource::GameplayTarget:
			return TEXT("FocusComponent.GameplayTarget");
		case EDebugOverlayFocusSource::EditorSelection:
			return TEXT("FocusComponent.EditorSelection");
		case EDebugOverlayFocusSource::None:
		default:
			return TEXT("None");
		}
	}

	FString FormatDebugOverlayFocusCommandType(EDebugOverlayFocusCommandType InCommandType)
	{
		switch (InCommandType)
		{
		case EDebugOverlayFocusCommandType::SelectNearestTarget:
			return TEXT("SelectNearestTarget");
		case EDebugOverlayFocusCommandType::SelectActorTarget:
			return TEXT("SelectActorTarget");
		case EDebugOverlayFocusCommandType::SelectRecentCombatTarget:
			return TEXT("SelectRecentCombatTarget");
		case EDebugOverlayFocusCommandType::ClearTarget:
			return TEXT("ClearTarget");
		case EDebugOverlayFocusCommandType::None:
		default:
			return TEXT("None");
		}
	}

	FString FormatDebugOverlayFocusCommandStatus(EDebugOverlayFocusCommandStatus InStatus)
	{
		switch (InStatus)
		{
		case EDebugOverlayFocusCommandStatus::Selected:
			return TEXT("Selected");
		case EDebugOverlayFocusCommandStatus::Cleared:
			return TEXT("Cleared");
		case EDebugOverlayFocusCommandStatus::InvalidContext:
			return TEXT("InvalidContext");
		case EDebugOverlayFocusCommandStatus::NoEnemy:
			return TEXT("NoEnemy");
		case EDebugOverlayFocusCommandStatus::OutOfRange:
			return TEXT("OutOfRange");
		case EDebugOverlayFocusCommandStatus::NoActorName:
			return TEXT("NoActorName");
		case EDebugOverlayFocusCommandStatus::NoActor:
			return TEXT("NoActor");
		case EDebugOverlayFocusCommandStatus::NotEnemy:
			return TEXT("NotEnemy");
		case EDebugOverlayFocusCommandStatus::NoRecentCombat:
			return TEXT("NoRecentCombat");
		case EDebugOverlayFocusCommandStatus::None:
		default:
			return TEXT("None");
		}
	}

	FString FormatDebugOverlayFocusCommandResult(const FDebugOverlayFocusCommandResult& InResult)
	{
		if (!InResult.SummaryTextOverride.IsEmpty())
		{
			return InResult.SummaryTextOverride;
		}

		if (InResult.CommandType == EDebugOverlayFocusCommandType::None
			&& InResult.Status == EDebugOverlayFocusCommandStatus::None)
		{
			return FString();
		}

		TArray<FString> fields;
		fields.Reserve(5);
		fields.Add(FormatDebugOverlayFocusCommandType(InResult.CommandType));
		fields.Add(FormatDebugOverlayFocusCommandStatus(InResult.Status));

		if (!InResult.ActorName.IsEmpty())
		{
			fields.Add(FString::Printf(TEXT("Target: %s"), *InResult.ActorName));
		}

		if (InResult.Distance > 0.f)
		{
			fields.Add(FString::Printf(TEXT("Distance: %.0f"), InResult.Distance));
		}

		if (InResult.Radius > 0.f)
		{
			fields.Add(FString::Printf(TEXT("Radius: %.0f"), InResult.Radius));
		}

		return FString::Join(fields, TEXT(" | "));
	}
}

UCDebugOverlayFocusComponent::UCDebugOverlayFocusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Focus Query
bool UCDebugOverlayFocusComponent::HasDebugOverlayFocus() const
{
	return DebugOverlayFocusActor.IsValid();
}

bool UCDebugOverlayFocusComponent::HasDebugOverlayFocusCommandResult() const
{
	return !DebugOverlayFocusCommandResult.SummaryTextOverride.IsEmpty()
		|| DebugOverlayFocusCommandResult.CommandType != EDebugOverlayFocusCommandType::None
		|| DebugOverlayFocusCommandResult.Status != EDebugOverlayFocusCommandStatus::None;
}

AActor* UCDebugOverlayFocusComponent::GetDebugOverlayFocusActor() const
{
	return DebugOverlayFocusActor.Get();
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusActorText() const
{
	return HasDebugOverlayFocus()
		? FString::Printf(TEXT("Selected: %s"), *GetNameSafe(DebugOverlayFocusActor.Get()))
		: FString(TEXT("None"));
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusModeText() const
{
	return HasDebugOverlayFocus()
		? FormatDebugOverlayFocusSource(DebugOverlayFocusSource)
		: FString(TEXT("None"));
}

const FDebugOverlayFocusCommandResult& UCDebugOverlayFocusComponent::GetDebugOverlayFocusCommandResult() const
{
	return DebugOverlayFocusCommandResult;
}

FString UCDebugOverlayFocusComponent::GetDebugOverlayFocusCommandResultText() const
{
	return FormatDebugOverlayFocusCommandResult(DebugOverlayFocusCommandResult);
}

// Focus Mutation
void UCDebugOverlayFocusComponent::SetDebugOverlayFocus(AActor* InFocusActor, EDebugOverlayFocusSource InSource)
{
	if (!IsValid(InFocusActor) || InSource == EDebugOverlayFocusSource::None)
	{
		ClearDebugOverlayFocus();
		return;
	}

	DebugOverlayFocusActor = InFocusActor;
	DebugOverlayFocusSource = InSource;
}

void UCDebugOverlayFocusComponent::ClearDebugOverlayFocus()
{
	DebugOverlayFocusActor.Reset();
	DebugOverlayFocusSource = EDebugOverlayFocusSource::None;
}

void UCDebugOverlayFocusComponent::SetDebugOverlayFocusCommandResult(const FDebugOverlayFocusCommandResult& InResult)
{
	DebugOverlayFocusCommandResult = InResult;
}

void UCDebugOverlayFocusComponent::ClearDebugOverlayFocusCommandResult()
{
	DebugOverlayFocusCommandResult = FDebugOverlayFocusCommandResult();
}
