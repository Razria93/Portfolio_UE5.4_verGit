#pragma once

#include "CoreMinimal.h"

class PORTFOLIO_API FLog
{
public:
	// === Print Log ===
	static void Log(int32 InValue);
	static void Log(float InValue);
	static void Log(const FString& InValue);
	static void Log(const FVector& InValue);
	static void Log(const FRotator& InValue);
	static void Log(const UObject* InValue);

	// === Print Screen ===
	static void Print(int InValue, int32 InKey, float InDuration = 10.f, const FColor& InColor = FColor::Blue);
	static void Print(float InValue, int32 InKey, float InDuration = 10.f, const FColor& InColor = FColor::Blue);
	static void Print(const FString& InValue, int32 InKey, float InDuration = 10.f, const FColor& InColor = FColor::Blue);
	static void Print(const FVector& InValue, int32 InKey, float InDuration = 10.f, const FColor& InColor = FColor::Blue);
	static void Print(const FRotator& InValue, int32 InKey, float InDuration = 10.f, const FColor& InColor = FColor::Blue);
	static void Print(const UObject* InValue, int32 InKey, float InDuration = 10.f, const FColor& InColor = FColor::Blue);
};
