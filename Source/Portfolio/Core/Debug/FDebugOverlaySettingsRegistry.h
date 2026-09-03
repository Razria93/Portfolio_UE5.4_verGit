#pragma once

#include "CoreMinimal.h"

enum class EDebugOverlaySettingType : uint8
{
	Bool,
	Int,
	Float,
	Enum,
};

struct PORTFOLIO_API FDebugOverlayEnumOption
{
	FString Value;
	FString DisplayName;
};

struct PORTFOLIO_API FDebugOverlaySettingDefinition
{
	FName Category;
	FString CVarName;
	FString DisplayName;
	FString HelpText;
	EDebugOverlaySettingType Type = EDebugOverlaySettingType::Bool;
	FString DefaultValue;
	FString ParentGateCVarName;
	float MinValue = 0.f;
	float MaxValue = 1.f;
	TArray<FDebugOverlayEnumOption> EnumOptions;
};

struct PORTFOLIO_API FDebugOverlaySettingsCategory
{
	FName Id;
	FString DisplayName;
	FString Description;
};

class PORTFOLIO_API FDebugOverlaySettingsRegistry
{
public:
	static const TArray<FDebugOverlaySettingsCategory>& GetCategories();
	static const TArray<FDebugOverlaySettingDefinition>& GetSettings();
};
