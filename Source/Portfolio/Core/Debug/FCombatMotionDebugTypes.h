#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatKnockbackTypes.h"
#include "Type/CActionKeyTypes.h"

class AActor;

// Recorded Decisions
struct FCombatKnockbackDebugRecord
{
	FName Stage;
	FName Result;
	FName Reason;
	uint64 Generation = 0;
	double WorldTime = 0.0;
	TWeakObjectPtr<AActor> OtherActor;
	FCombatKnockbackContext Context;
	FVector Origin = FVector::ZeroVector;
	FVector End = FVector::ZeroVector;
	FVector DrawOrigin = FVector::ZeroVector;
	FVector DrawEnd = FVector::ZeroVector;
	float SourceTime = 0.f;
	int32 SourceId = INDEX_NONE;
	bool bHasGeometry = false;
};

struct FActionFacingDebugRecord
{
	FName Result;
	FName Reason;
	uint64 Generation = 0;
	double WorldTime = 0.0;
	FActionDataKey ActionKey;
	TWeakObjectPtr<AActor> Target;
	FVector Origin = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	FVector DrawOrigin = FVector::ZeroVector;
	float BeforeYaw = 0.f;
	float AfterYaw = 0.f;
	float Distance = 0.f;
	float Angle = 0.f;
	float MaxDistance = 0.f;
	float MaxAngle = 0.f;
	bool bHasGeometry = false;
};

// Actor History
struct FCombatKnockbackDebugHistory
{
	uint64 LatestGeneration = 0;
	FCombatKnockbackDebugRecord Last;
	FCombatKnockbackDebugRecord Motion;
};
