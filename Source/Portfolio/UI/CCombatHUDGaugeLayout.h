#pragma once

#include "CoreMinimal.h"

// Shared visual geometry; cells never quantize gameplay resource values.
namespace CombatHUDGauge
{
	constexpr float CellSize = 5.f;
	constexpr float CellGap = 2.5f;
	constexpr float UnitGap = 5.f;
	constexpr int32 HealthRows = 3;
	constexpr float PlayerHealthPerColumn = 100.f;
	constexpr float EnergyPerCell = 50.f;
	constexpr float ShieldPerColumn = 40.f;
	constexpr int32 ShieldUnitColumns = 10;
	constexpr int32 BossGroups = 5;
	// Fit five equal SH groups in the 880-unit budget. Each separator replaces one HP column pitch.
	constexpr int32 BossUnitColumns = (int32((880.f + CellGap) / (CellSize + CellGap)) - (BossGroups - 1)) / BossGroups;
	constexpr int32 BossHealthColumns = BossGroups * BossUnitColumns + BossGroups - 1;

	inline float UnitFill(float Current, float Maximum, float PerUnit, int32 Index)
	{
		return FMath::IsFinite(Current) && FMath::IsFinite(Maximum) && Maximum > 0.f && PerUnit > 0.f
			? FMath::Clamp(FMath::Clamp(Current, 0.f, Maximum) / PerUnit - Index, 0.f, 1.f) : 0.f;
	}
	inline int32 ResourceColumns(float Maximum, float PerColumn, int32 Limit)
	{
		return FMath::IsFinite(Maximum) && Maximum > 0.f && PerColumn > 0.f
			? FMath::CeilToInt(FMath::Clamp(Maximum / PerColumn, 0.f, float(Limit))) : 0;
	}

	struct FPixelMetrics
	{
		int32 Cell, Gap, GroupGap;
		explicit FPixelMetrics(float Scale)
		{
			// Do not exceed the original pitch: keep the fixed column count inside its width budget.
			const int32 pitch = FMath::Max(2, FMath::FloorToInt((CellSize + CellGap) * Scale + 0.0001f));
			Gap = FMath::Clamp(FMath::FloorToInt(CellGap * Scale + 0.4999f), 1, pitch - 1);
			Cell = FMath::Min(FMath::Max(1, FMath::RoundToInt(CellSize * Scale)), pitch - Gap);
			GroupGap = Gap + FMath::Max(0, FMath::FloorToInt((UnitGap - CellGap) * Scale));
		}
		int32 ColumnX(int32 Column, int32 UnitColumns, bool bBoss = false) const
		{
			return Column * (Cell + Gap) + (UnitColumns > 0 ? Column / UnitColumns * (bBoss ? Cell + Gap : GroupGap - Gap) : 0);
		}
	};

	inline float Height(int32 Rows) { return Rows * CellSize + FMath::Max(0, Rows - 1) * CellGap; }
	inline float ColumnX(int32 Column, int32 UnitColumns, bool bBoss = false)
	{
		return Column * (CellSize + CellGap)
			+ (UnitColumns > 0 ? Column / UnitColumns * (bBoss ? CellSize + CellGap : UnitGap - CellGap) : 0.f);
	}
	inline int32 ColumnsForWidth(float Width, int32 UnitColumns)
	{
		if (UnitColumns <= 0) return FMath::Max(1, FMath::FloorToInt((Width + CellGap) / (CellSize + CellGap)));
		const float unitWidth = UnitColumns * CellSize + (UnitColumns - 1) * CellGap;
		return FMath::Max(1, FMath::FloorToInt((Width + UnitGap) / (unitWidth + UnitGap))) * UnitColumns;
	}
	inline float ColumnFill(float Fraction, int32 Column, int32 Columns)
	{
		return FMath::IsFinite(Fraction) ? FMath::Clamp(Fraction * Columns - Column, 0.f, 1.f) : 0.f;
	}
}
