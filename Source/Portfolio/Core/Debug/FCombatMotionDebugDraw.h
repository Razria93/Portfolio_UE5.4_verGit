#pragma once

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

namespace CombatMotionDebugDraw
{
	constexpr float GroundClearance = 8.f;
	constexpr float FacingRadius = 110.f;
	constexpr float FacingArcRadius = 70.f;
	constexpr float FacingAngleTolerance = 1.f;

	inline FVector FootAnchor(const AActor* Actor, const FVector& ActorLocation)
	{
		const ACharacter* character = Cast<ACharacter>(Actor);
		const UCapsuleComponent* capsule = character ? character->GetCapsuleComponent() : nullptr;
		const float halfHeight = capsule ? capsule->GetScaledCapsuleHalfHeight() : 0.f;
		return ActorLocation - FVector(0, 0, halfHeight) + FVector(0, 0, GroundClearance);
	}

	inline void Ring(UWorld* World, const FVector& Center, float Radius, const FColor& Color, uint8 Depth)
	{
		DrawDebugCircle(World, Center, Radius, 24, Color, false, -1.f, Depth, 2.f,
			FVector::ForwardVector, FVector::RightVector, false);
	}

	inline void DashedLine(UWorld* World, const FVector& Start, const FVector& End, uint8 Depth)
	{
		const FVector delta = End - Start;
		const double length = delta.Size();
		if (!FMath::IsFinite(length) || length <= UE_SMALL_NUMBER) return;
		const int32 segments = FMath::Clamp(FMath::CeilToInt(FMath::Min(length / 10.0, 128.0)), 1, 128);
		for (int32 i = 0; i < segments; ++i)
		{
			const FVector a = Start + delta * (static_cast<double>(i) / segments);
			const FVector b = Start + delta * ((i + 0.55) / segments);
			DrawDebugLine(World, a, b, FColor::Yellow, false, -1.f, Depth, 2.f);
		}
	}

	inline void FacingArc(UWorld* World, const FVector& Origin, float BeforeYaw, float DeltaYaw, uint8 Depth)
	{
		FVector previous = Origin + FRotator(0, BeforeYaw, 0).Vector() * FacingArcRadius;
		const int32 segments = FMath::Clamp(FMath::CeilToInt(FMath::Abs(DeltaYaw) / 5.f), 1, 36);
		for (int32 i = 1; i <= segments; ++i)
		{
			const FVector next = Origin + FRotator(0, BeforeYaw + DeltaYaw * i / segments, 0).Vector() * FacingArcRadius;
			if (i == segments)
				DrawDebugDirectionalArrow(World, previous, next, 6.f, FColor::Cyan, false, -1.f, Depth, 2.f);
			else
				DrawDebugLine(World, previous, next, FColor::Cyan, false, -1.f, Depth, 2.f);
			previous = next;
		}
	}
}
#endif
