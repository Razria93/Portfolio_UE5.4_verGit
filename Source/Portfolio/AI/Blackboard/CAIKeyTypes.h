#pragma once

#include "CoreMinimal.h"

enum class EAIBlackboardKeyValueType : uint8
{
	Bool,
	Int,
	Float,
	Vector,
	Enum,
	Object,
};

enum class EAIBlackboardInitialValuePolicy : uint8
{
	None,
	Fixed,
	FromOwnerLocation,
	Custom,
};

struct FAIBlackboardKeySpec
{
	FName KeyName = NAME_None;
	EAIBlackboardKeyValueType ValueType = EAIBlackboardKeyValueType::Bool;
	EAIBlackboardInitialValuePolicy InitialValuePolicy = EAIBlackboardInitialValuePolicy::None;

	bool BoolDefault = false;
	int32 IntDefault = 0;
	float FloatDefault = 0.f;
	FVector VectorDefault = FVector::ZeroVector;
	uint8 EnumDefault = 0;

	bool bRequired = true;
	bool bClearOnRuntimeTeardown = true;
};
