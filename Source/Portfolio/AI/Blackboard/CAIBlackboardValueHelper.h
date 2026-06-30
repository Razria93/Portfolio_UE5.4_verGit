#pragma once

#include "CoreMinimal.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

#include "AI/Blackboard/CAIKeyRegistry.h"

namespace CAIBlackboardValueHelper
{
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

	static void MarkCustomKeyApplied(TSet<FName>& InOutPendingKeys, const FAIBlackboardKeySpec& InKeySpec, const UObject* InOwnerContext)
	{
		ensureMsgf(
			InOutPendingKeys.Remove(InKeySpec.KeyName) > 0,
			TEXT("[AIBlackboardValueHelper] Custom Blackboard key was not registered | Owner=%s | Key=%s"),
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
			TEXT("[AIBlackboardValueHelper] Missing custom Blackboard initial values | Owner=%s | Pending=%s"),
			*GetNameSafe(InOwnerContext),
			*pendingCustomKeys);

		return false;
	}

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
