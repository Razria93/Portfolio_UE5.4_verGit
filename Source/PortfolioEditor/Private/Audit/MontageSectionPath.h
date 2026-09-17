#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"

namespace ExecutionMontageAudit
{
	// Static authored path only; runtime section overrides are outside this audit.
	struct FSectionPath
	{
		TArray<int32> Sections;
		FString Error;
		bool bLoops = false;

		bool HasLinearTimeline() const
		{
			if (bLoops) return false;
			for (int32 Index = 1; Index < Sections.Num(); ++Index)
				if (Sections[Index] != Sections[Index - 1] + 1) return false;
			return true;
		}

		bool ContainsTime(const UAnimMontage* Montage, float Time) const
		{
			return Sections.Contains(Montage->GetSectionIndexFromPosition(Time));
		}
	};

	inline FSectionPath BuildSectionPath(const UAnimMontage* Montage, FName StartSection, float PlayRate)
	{
		FSectionPath Result;
		if (!FMath::IsFinite(PlayRate) || !FMath::IsFinite(Montage->RateScale)
			|| PlayRate * Montage->RateScale <= 0.f)
		{
			Result.Error = TEXT("Non-positive or invalid effective play rate requires manual path review.");
			return Result;
		}
		int32 Section = StartSection.IsNone() ? Montage->GetSectionIndexFromPosition(0.f) : Montage->GetSectionIndex(StartSection);
		if (Section == INDEX_NONE)
		{
			Result.Error = TEXT("Missing configured start section (or no section at playback start).");
			return Result;
		}
		while (Section != INDEX_NONE)
		{
			if (Result.Sections.Contains(Section))
			{
				Result.bLoops = true;
				break;
			}
			Result.Sections.Add(Section);
			const FName Next = Montage->CompositeSections[Section].NextSectionName;
			Section = Next.IsNone() ? INDEX_NONE : Montage->GetSectionIndex(Next);
			if (!Next.IsNone() && Section == INDEX_NONE)
			{
				Result.Error = FString::Printf(TEXT("Authored next section '%s' does not exist."), *Next.ToString());
				break;
			}
		}
		return Result;
	}
}
