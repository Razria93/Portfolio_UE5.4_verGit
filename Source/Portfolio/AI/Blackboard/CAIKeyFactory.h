#pragma once

#include "CoreMinimal.h"

#include "AI/Blackboard/CAIKeyTypes.h"

namespace CAIKeyFactory
{
	static FAIBlackboardKeySpec MakeKey(const TCHAR* InKeyName, EAIBlackboardKeyValueType InValueType)
	{
		FAIBlackboardKeySpec keySpec;
		keySpec.KeyName = InKeyName;
		keySpec.ValueType = InValueType;
		return keySpec;
	}

	static FAIBlackboardKeySpec FixedBool(const TCHAR* InKeyName, bool InValue)
	{
		FAIBlackboardKeySpec keySpec = MakeKey(InKeyName, EAIBlackboardKeyValueType::Bool);
		keySpec.InitialValuePolicy = EAIBlackboardInitialValuePolicy::Fixed;
		keySpec.BoolDefault = InValue;
		return keySpec;
	}

	static FAIBlackboardKeySpec FixedInt(const TCHAR* InKeyName, int32 InValue)
	{
		FAIBlackboardKeySpec keySpec = MakeKey(InKeyName, EAIBlackboardKeyValueType::Int);
		keySpec.InitialValuePolicy = EAIBlackboardInitialValuePolicy::Fixed;
		keySpec.IntDefault = InValue;
		return keySpec;
	}

	static FAIBlackboardKeySpec FixedFloat(const TCHAR* InKeyName, float InValue)
	{
		FAIBlackboardKeySpec keySpec = MakeKey(InKeyName, EAIBlackboardKeyValueType::Float);
		keySpec.InitialValuePolicy = EAIBlackboardInitialValuePolicy::Fixed;
		keySpec.FloatDefault = InValue;
		return keySpec;
	}

	static FAIBlackboardKeySpec FixedVector(const TCHAR* InKeyName, const FVector& InValue)
	{
		FAIBlackboardKeySpec keySpec = MakeKey(InKeyName, EAIBlackboardKeyValueType::Vector);
		keySpec.InitialValuePolicy = EAIBlackboardInitialValuePolicy::Fixed;
		keySpec.VectorDefault = InValue;
		return keySpec;
	}

	static FAIBlackboardKeySpec FixedEnum(const TCHAR* InKeyName, uint8 InValue)
	{
		FAIBlackboardKeySpec keySpec = MakeKey(InKeyName, EAIBlackboardKeyValueType::Enum);
		keySpec.InitialValuePolicy = EAIBlackboardInitialValuePolicy::Fixed;
		keySpec.EnumDefault = InValue;
		return keySpec;
	}

	static FAIBlackboardKeySpec FixedObjectNull(const TCHAR* InKeyName)
	{
		FAIBlackboardKeySpec keySpec = MakeKey(InKeyName, EAIBlackboardKeyValueType::Object);
		keySpec.InitialValuePolicy = EAIBlackboardInitialValuePolicy::Fixed;
		return keySpec;
	}

	static FAIBlackboardKeySpec RuntimeValue(const TCHAR* InKeyName, EAIBlackboardKeyValueType InValueType)
	{
		return MakeKey(InKeyName, InValueType);
	}

	static FAIBlackboardKeySpec RuntimeBool(const TCHAR* InKeyName)
	{
		return RuntimeValue(InKeyName, EAIBlackboardKeyValueType::Bool);
	}

	static FAIBlackboardKeySpec RuntimeInt(const TCHAR* InKeyName)
	{
		return RuntimeValue(InKeyName, EAIBlackboardKeyValueType::Int);
	}

	static FAIBlackboardKeySpec RuntimeFloat(const TCHAR* InKeyName)
	{
		return RuntimeValue(InKeyName, EAIBlackboardKeyValueType::Float);
	}

	static FAIBlackboardKeySpec RuntimeVector(const TCHAR* InKeyName)
	{
		return RuntimeValue(InKeyName, EAIBlackboardKeyValueType::Vector);
	}

	static FAIBlackboardKeySpec RuntimeEnum(const TCHAR* InKeyName)
	{
		return RuntimeValue(InKeyName, EAIBlackboardKeyValueType::Enum);
	}

	static FAIBlackboardKeySpec RuntimeObject(const TCHAR* InKeyName)
	{
		return RuntimeValue(InKeyName, EAIBlackboardKeyValueType::Object);
	}

	static FAIBlackboardKeySpec FromOwnerLocation(const TCHAR* InKeyName)
	{
		FAIBlackboardKeySpec keySpec = MakeKey(InKeyName, EAIBlackboardKeyValueType::Vector);
		keySpec.InitialValuePolicy = EAIBlackboardInitialValuePolicy::FromOwnerLocation;
		return keySpec;
	}

	static FAIBlackboardKeySpec Custom(const TCHAR* InKeyName, EAIBlackboardKeyValueType InValueType)
	{
		FAIBlackboardKeySpec keySpec = MakeKey(InKeyName, InValueType);
		keySpec.InitialValuePolicy = EAIBlackboardInitialValuePolicy::Custom;
		return keySpec;
	}

	static FAIBlackboardKeySpec CustomBool(const TCHAR* InKeyName)
	{
		return Custom(InKeyName, EAIBlackboardKeyValueType::Bool);
	}

	static FAIBlackboardKeySpec CustomInt(const TCHAR* InKeyName)
	{
		return Custom(InKeyName, EAIBlackboardKeyValueType::Int);
	}

	static FAIBlackboardKeySpec CustomFloat(const TCHAR* InKeyName)
	{
		return Custom(InKeyName, EAIBlackboardKeyValueType::Float);
	}

	static FAIBlackboardKeySpec CustomVector(const TCHAR* InKeyName)
	{
		return Custom(InKeyName, EAIBlackboardKeyValueType::Vector);
	}

	static FAIBlackboardKeySpec CustomEnum(const TCHAR* InKeyName)
	{
		return Custom(InKeyName, EAIBlackboardKeyValueType::Enum);
	}

	static FAIBlackboardKeySpec CustomObject(const TCHAR* InKeyName)
	{
		return Custom(InKeyName, EAIBlackboardKeyValueType::Object);
	}
}
