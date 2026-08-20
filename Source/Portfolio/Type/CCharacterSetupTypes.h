#pragma once

#include "CoreMinimal.h"
#include "CCharacterSetupTypes.generated.h"

// Data / Config

USTRUCT(BlueprintType)
struct FCharacterCapsuleSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Capsule")
	float Radius = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Capsule")
	float HalfHeight = 90.0f;
};

USTRUCT(BlueprintType)
struct FCharacterMeshSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Mesh")
	FVector RelativeLocation = FVector(0.0f, 0.0f, -90.0f);

	UPROPERTY(EditAnywhere, Category = "Mesh")
	FRotator RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);
};

USTRUCT(BlueprintType)
struct FCharacterMovementSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement")
	float DefaultWalkSpeed = 600.0f;
};

USTRUCT(BlueprintType)
struct FPlayerCameraSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector SpringArmRelativeLocation = FVector(0.0f, 15.0f, 55.0f);

	UPROPERTY(EditAnywhere, Category = "Camera")
	float BoomLength = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	FVector CameraRelativeLocation = FVector::ZeroVector;
};
