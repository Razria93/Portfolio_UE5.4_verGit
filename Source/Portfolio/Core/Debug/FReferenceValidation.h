#pragma once

#include "CoreMinimal.h"

struct FRequiredReference
{
	const UObject* Object = nullptr;
	const TCHAR* Label = TEXT("");
};

class FReferenceValidation
{
public:
	static bool EnsureRequiredReference(const UObject* InObject, const TCHAR* InLabel, const UObject* InOwner, const UObject* InContext)
	{
		return ensureMsgf(IsValid(InObject), TEXT("Missing required %s | Owner=%s | This=%s"), InLabel, *GetNameSafe(InOwner), *GetNameSafe(InContext));
	}
};
