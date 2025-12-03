#include "Core/Debug/FLog.h"

DEFINE_LOG_CATEGORY_STATIC(Custom_FLog, Display, All);

// === Console Log ===
void FLog::Log(int32 InValue)
{
	UE_LOG(Custom_FLog, Display, TEXT("%d"), InValue);
}

void FLog::Log(float InValue)
{
	UE_LOG(Custom_FLog, Display, TEXT("%f"), InValue);
}

void FLog::Log(const FString& InValue)
{
	if (InValue.IsEmpty())
	{
		UE_LOG(Custom_FLog, Warning, TEXT("InValue is Empty [String]"));
		return;
	}
	UE_LOG(Custom_FLog, Display, TEXT("%s"), *InValue);
}

void FLog::Log(const FVector& InValue)
{
	UE_LOG(Custom_FLog, Display, TEXT("%s"), *InValue.ToString());
}

void FLog::Log(const FRotator& InValue)
{
	UE_LOG(Custom_FLog, Display, TEXT("%s"), *InValue.ToString());
}

void FLog::Log(const UObject* InValue)
{
	if (!IsValid(InValue))
	{
		UE_LOG(Custom_FLog, Warning, TEXT("Invalid Object [UObject*]"));
		return;
	}

	UE_LOG(Custom_FLog, Display, TEXT("%s (Valid)"), *InValue->GetName());
}

// === Screen Debug Message ===
void FLog::Print(int InValue, int32 InKey, float InDuration, const FColor& InColor)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, FString::FromInt(InValue));
}

void FLog::Print(float InValue, int32 InKey, float InDuration, const FColor& InColor)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, FString::SanitizeFloat(InValue));
}

void FLog::Print(const FString& InValue, int32 InKey, float InDuration, const FColor& InColor)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, InValue);
}

void FLog::Print(const FVector& InValue, int32 InKey, float InDuration, const FColor& InColor)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, InValue.ToString());
}

void FLog::Print(const FRotator& InValue, int32 InKey, float InDuration, const FColor& InColor)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, InValue.ToString());
}

void FLog::Print(const UObject* InValue, int32 InKey, float InDuration, const FColor& InColor)
{
	if (!GEngine)
		return;

	FString Message = InValue ? FString::Printf(TEXT("%s [Valid]"), *InValue->GetName())
	                          : TEXT("Null Object");

	GEngine->AddOnScreenDebugMessage(InKey, InDuration, InColor, Message);
}