#pragma once

#include "CoreMinimal.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

#include "AI/Blackboard/CAIKeyRegistry.h"

namespace CAIBlackboardValueHelper
{
	// Value Set
	static void SetBoolIfChanged(UBlackboardComponent* InBlackboardComp, const FName& InKeyName, bool InValue)
	{
		if (!IsValid(InBlackboardComp)) return;
		if (InBlackboardComp->GetValueAsBool(InKeyName) == InValue) return;

		InBlackboardComp->SetValueAsBool(InKeyName, InValue);
	}

	static void SetIntIfChanged(UBlackboardComponent* InBlackboardComp, const FName& InKeyName, int32 InValue)
	{
		if (!IsValid(InBlackboardComp)) return;
		if (InBlackboardComp->GetValueAsInt(InKeyName) == InValue) return;

		InBlackboardComp->SetValueAsInt(InKeyName, InValue);
	}

	static void SetFloatIfChanged(UBlackboardComponent* InBlackboardComp, const FName& InKeyName, float InValue)
	{
		if (!IsValid(InBlackboardComp)) return;
		if (FMath::IsNearlyEqual(InBlackboardComp->GetValueAsFloat(InKeyName), InValue)) return;

		InBlackboardComp->SetValueAsFloat(InKeyName, InValue);
	}

	static void SetVectorIfChanged(UBlackboardComponent* InBlackboardComp, const FName& InKeyName, const FVector& InValue)
	{
		if (!IsValid(InBlackboardComp)) return;
		if (InBlackboardComp->GetValueAsVector(InKeyName).Equals(InValue)) return;

		InBlackboardComp->SetValueAsVector(InKeyName, InValue);
	}

	static void SetEnumIfChanged(UBlackboardComponent* InBlackboardComp, const FName& InKeyName, uint8 InValue)
	{
		if (!IsValid(InBlackboardComp)) return;
		if (InBlackboardComp->GetValueAsEnum(InKeyName) == InValue) return;

		InBlackboardComp->SetValueAsEnum(InKeyName, InValue);
	}

	static void SetObjectIfChanged(UBlackboardComponent* InBlackboardComp, const FName& InKeyName, UObject* InValue)
	{
		if (!IsValid(InBlackboardComp)) return;
		if (InBlackboardComp->GetValueAsObject(InKeyName) == InValue) return;

		InBlackboardComp->SetValueAsObject(InKeyName, InValue);
	}

	// Initial Value Apply
	static void ApplyFixedValue(UBlackboardComponent* InBlackboardComp, const FAIBlackboardKeySpec& InKeySpec)
	{
		if (!IsValid(InBlackboardComp)) return;

		switch (InKeySpec.ValueType)
		{
		case EAIBlackboardKeyValueType::Bool:
			InBlackboardComp->SetValueAsBool(InKeySpec.KeyName, InKeySpec.BoolDefault);
			break;

		case EAIBlackboardKeyValueType::Int:
			InBlackboardComp->SetValueAsInt(InKeySpec.KeyName, InKeySpec.IntDefault);
			break;

		case EAIBlackboardKeyValueType::Float:
			InBlackboardComp->SetValueAsFloat(InKeySpec.KeyName, InKeySpec.FloatDefault);
			break;

		case EAIBlackboardKeyValueType::Vector:
			InBlackboardComp->SetValueAsVector(InKeySpec.KeyName, InKeySpec.VectorDefault);
			break;

		case EAIBlackboardKeyValueType::Enum:
			InBlackboardComp->SetValueAsEnum(InKeySpec.KeyName, InKeySpec.EnumDefault);
			break;

		case EAIBlackboardKeyValueType::Object:
			InBlackboardComp->ClearValue(InKeySpec.KeyName);
			break;
		}
	}

	static void ApplyOwnerLocationValue(UBlackboardComponent* InBlackboardComp, const APawn* InOwnerPawn, const FAIBlackboardKeySpec& InKeySpec)
	{
		if (!IsValid(InBlackboardComp)) return;
		if (!IsValid(InOwnerPawn)) return;

		InBlackboardComp->SetValueAsVector(InKeySpec.KeyName, InOwnerPawn->GetActorLocation());
	}

	static void InitializeValues(UBlackboardComponent* InBlackboardComp, const APawn* InOwnerPawn, TSet<FName>& OutPendingKeys)
	{
		if (!IsValid(InBlackboardComp)) return;

		for (const FAIBlackboardKeySpec& keySpec : CAIKeyRegistry::GetKeySpecs())
		{
			switch (keySpec.InitialValuePolicy)
			{
			case EAIBlackboardInitialValuePolicy::Fixed:
				ApplyFixedValue(InBlackboardComp, keySpec);
				break;

			case EAIBlackboardInitialValuePolicy::FromOwnerLocation:
				ApplyOwnerLocationValue(InBlackboardComp, InOwnerPawn, keySpec);
				break;

			case EAIBlackboardInitialValuePolicy::Custom:
				OutPendingKeys.Add(keySpec.KeyName);
				break;

			case EAIBlackboardInitialValuePolicy::None:
			default:
				break;
			}
		}
	}

	// Custom Value Apply
	static void MarkCustomKeyApplied(TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, const UObject* InOwnerContext)
	{
		ensureMsgf(
			InOutPendingKeys.Remove(InKeySpec.KeyName) > 0,
			TEXT("[AI|Blackboard|CustomKeyNotRegistered] Reason=MissingPendingKey | Owner=%s | Key=%s"),
			*GetNameSafe(InOwnerContext),
			*InKeySpec.KeyName.ToString());
	}

	static void ApplyCustomBool(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, bool InValue, const UObject* InOwnerContext)
	{
		if (!IsValid(InBlackboardComp)) return;

		InBlackboardComp->SetValueAsBool(InKeySpec.KeyName, InValue);
		MarkCustomKeyApplied(InOutPendingKeys, InKeySpec, InOwnerContext);
	}

	static void ApplyCustomInt(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, int32 InValue, const UObject* InOwnerContext)
	{
		if (!IsValid(InBlackboardComp)) return;

		InBlackboardComp->SetValueAsInt(InKeySpec.KeyName, InValue);
		MarkCustomKeyApplied(InOutPendingKeys, InKeySpec, InOwnerContext);
	}

	static void ApplyCustomFloat(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, float InValue, const UObject* InOwnerContext)
	{
		if (!IsValid(InBlackboardComp)) return;

		InBlackboardComp->SetValueAsFloat(InKeySpec.KeyName, InValue);
		MarkCustomKeyApplied(InOutPendingKeys, InKeySpec, InOwnerContext);
	}

	static void ApplyCustomVector(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, const FVector& InValue, const UObject* InOwnerContext)
	{
		if (!IsValid(InBlackboardComp)) return;

		InBlackboardComp->SetValueAsVector(InKeySpec.KeyName, InValue);
		MarkCustomKeyApplied(InOutPendingKeys, InKeySpec, InOwnerContext);
	}

	static void ApplyCustomEnum(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, uint8 InValue, const UObject* InOwnerContext)
	{
		if (!IsValid(InBlackboardComp)) return;

		InBlackboardComp->SetValueAsEnum(InKeySpec.KeyName, InValue);
		MarkCustomKeyApplied(InOutPendingKeys, InKeySpec, InOwnerContext);
	}

	static void ApplyCustomObject(UBlackboardComponent* InBlackboardComp, TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, UObject* InValue, const UObject* InOwnerContext)
	{
		if (!IsValid(InBlackboardComp)) return;

		InBlackboardComp->SetValueAsObject(InKeySpec.KeyName, InValue);
		MarkCustomKeyApplied(InOutPendingKeys, InKeySpec, InOwnerContext);
	}

	// Validation
	static bool ValidateCustomKeysApplied(const TSet<FName>& InPendingKeys, const UObject* InOwnerContext)
	{
		if (InPendingKeys.IsEmpty()) return true;

		TArray<FString> pendingKeyNames;
		for (const FName& keyName : InPendingKeys)
		{
			pendingKeyNames.Add(keyName.ToString());
		}

		const FString pendingCustomKeys = FString::Join(pendingKeyNames, TEXT(", "));

		ensureMsgf(false,
			TEXT("[AI|Blackboard|CustomInitialValuesMissing] Reason=PendingCustomKeys | Owner=%s | Missing=%s"),
			*GetNameSafe(InOwnerContext),
			*pendingCustomKeys);

		return false;
	}

	// Clear
	static void ClearValues(UBlackboardComponent* InBlackboardComp)
	{
		if (!IsValid(InBlackboardComp)) return;

		for (const FAIBlackboardKeySpec& keySpec : CAIKeyRegistry::GetKeySpecs())
		{
			if (!keySpec.bClearOnRuntimeTeardown) continue;

			InBlackboardComp->ClearValue(keySpec.KeyName);
		}
	}
}
