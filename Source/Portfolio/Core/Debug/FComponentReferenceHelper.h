#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Core/Debug/FLog.h"
#include "Type/CCharacterComponentReferenceStructure.h"

class FComponentReferenceHelper
{
public:
	template <typename TComponent>
	static void RecoverIfInvalid(AActor* InOwnerActor, TComponent*& InOutComponent)
	{
		if (IsValid(InOutComponent)) return;
		if (!IsValid(InOwnerActor)) return;

		TComponent* resolvedComponent = InOwnerActor->FindComponentByClass<TComponent>();

		if (!ensureMsgf(
			IsValid(resolvedComponent),
			TEXT("[ComponentReferenceRecovery] Failed | Owner=%s | Component=%s"),
			*GetNameSafe(InOwnerActor),
			*GetNameSafe(TComponent::StaticClass())
		))
		{
			return;
		}

		InOutComponent = resolvedComponent;

		FLog::Log(FString::Printf(
			TEXT("[ComponentReferenceRecovery] Recovered | Owner=%s | Component=%s | Resolved=%s"),
			*GetNameSafe(InOwnerActor),
			*GetNameSafe(TComponent::StaticClass()),
			*GetNameSafe(resolvedComponent)
		));
	}

	template <typename TComponent>
	static void InjectIfValid(TComponent* InComponent, const FCharacterComponentReferences& InReferences)
	{
		if (!IsValid(InComponent)) return;

		InComponent->InitializeReferences(InReferences);
	}
};
