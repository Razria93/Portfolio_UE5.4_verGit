#include "Component/CTargetLockAssistComponent.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Character/Player/CPlayer.h"
#include "Component/CMovementComponent.h"
#include "Component/CTargetingComponent.h"

#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UCTargetLockAssistComponent::UCTargetLockAssistComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

// ===== Component Reference =====

void UCTargetLockAssistComponent::InitializeReferences(APlayerController* InOwnerPlayerController, UCTargetingComponent* InTargetingComponent)
{
	if (IsValid(TargetingComponent_Injected))
	{
		TargetingComponent_Injected->OnTargetChanged.RemoveAll(this);
	}

	OwnerPlayerController_Injected = InOwnerPlayerController;
	TargetingComponent_Injected = InTargetingComponent;

	if (IsValid(TargetingComponent_Injected))
	{
		TargetingComponent_Injected->OnTargetChanged.AddUObject(this, &UCTargetLockAssistComponent::HandleTargetChanged);
	}

	ValidateRequiredReferences();
	ApplyCurrentTargetPolicy();
}

void UCTargetLockAssistComponent::SetControlledPlayer(ACPlayer* InControlledPlayer)
{
	if (ControlledPlayer.Get() == InControlledPlayer)
	{
		ApplyCurrentTargetPolicy();
		return;
	}

	RestoreControlledPlayerPolicy();
	ControlledPlayer = InControlledPlayer;
	ApplyCurrentTargetPolicy();
}

void UCTargetLockAssistComponent::ClearControlledPlayer()
{
	RestoreControlledPlayerPolicy();
	ControlledPlayer.Reset();
}

// ===== Lifecycle =====

void UCTargetLockAssistComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	if (IsValid(TargetingComponent_Injected))
	{
		TargetingComponent_Injected->OnTargetChanged.RemoveAll(this);
	}

	ClearControlledPlayer();
	TargetingComponent_Injected = nullptr;
	OwnerPlayerController_Injected = nullptr;

	Super::EndPlay(InEndPlayReason);
}

void UCTargetLockAssistComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCameraTracking(DeltaTime);
}

// ===== Query =====

bool UCTargetLockAssistComponent::IsTargetLockActive() const
{
	return IsValid(ControlledPlayer.Get())
		&& IsValid(TargetingComponent_Injected)
		&& TargetingComponent_Injected->HasTarget();
}

bool UCTargetLockAssistComponent::ShouldSuppressLookInput() const
{
	return IsTargetLockActive();
}

// ===== Validation =====

bool UCTargetLockAssistComponent::ValidateRequiredReferences() const
{
	const FRequiredReference requiredReferences[] =
	{
		{ OwnerPlayerController_Injected, TEXT("APlayerController Owner") },
		{ TargetingComponent_Injected, TEXT("UCTargetingComponent") },
	};

	bool bValid = true;
	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerPlayerController_Injected, this);
	}

	return bValid;
}

// ===== Target State =====

void UCTargetLockAssistComponent::HandleTargetChanged(ACEnemy* InPreviousTarget, ACEnemy* InNewTarget)
{
	ApplyCurrentTargetPolicy();
}

void UCTargetLockAssistComponent::ApplyCurrentTargetPolicy()
{
	ACPlayer* controlledPlayer = ControlledPlayer.Get();
	if (!IsValid(controlledPlayer)) return;

	UCMovementComponent* movementComp = controlledPlayer->GetMovementComp();
	if (!IsValid(movementComp)) return;

	const EMovementRotationMode rotationMode = IsTargetLockActive()
		? EMovementRotationMode::ControllerDesired
		: EMovementRotationMode::OrientToMovement;

	movementComp->SetMovementRotationMode(rotationMode);
}

void UCTargetLockAssistComponent::RestoreControlledPlayerPolicy()
{
	ACPlayer* controlledPlayer = ControlledPlayer.Get();
	if (!IsValid(controlledPlayer)) return;

	UCMovementComponent* movementComp = controlledPlayer->GetMovementComp();
	if (!IsValid(movementComp)) return;

	movementComp->SetMovementRotationMode(EMovementRotationMode::OrientToMovement);
}

// ===== Camera Tracking =====

void UCTargetLockAssistComponent::UpdateCameraTracking(float DeltaTime)
{
	if (!IsValid(OwnerPlayerController_Injected) || !IsTargetLockActive()) return;

	ACEnemy* currentTarget = TargetingComponent_Injected->GetCurrentTarget();
	if (!IsValid(currentTarget)) return;

	FVector viewLocation = FVector::ZeroVector;
	FRotator viewRotation = FRotator::ZeroRotator;
	OwnerPlayerController_Injected->GetPlayerViewPoint(viewLocation, viewRotation);

	const FVector targetFocusLocation = currentTarget->GetActorLocation() + TargetLockAssistTuning.TargetFocusOffset;
	const FVector directionToTarget = targetFocusLocation - viewLocation;
	FRotator desiredRotation = directionToTarget.Rotation();
	desiredRotation.Pitch = ResolveDesiredLockPitch(desiredRotation.Pitch, directionToTarget.Size2D());
	desiredRotation.Roll = 0.f;

	if (IsValid(OwnerPlayerController_Injected->PlayerCameraManager))
	{
		desiredRotation.Pitch = FMath::ClampAngle(
			desiredRotation.Pitch,
			OwnerPlayerController_Injected->PlayerCameraManager->ViewPitchMin,
			OwnerPlayerController_Injected->PlayerCameraManager->ViewPitchMax);
	}

	const FRotator currentRotation = OwnerPlayerController_Injected->GetControlRotation();
	const float safeDeltaTime = FMath::Max(DeltaTime, 0.f);
	const FRotator nextRotation = TargetLockAssistTuning.CameraRotationInterpSpeed <= 0.f
		? desiredRotation
		: FMath::RInterpTo(currentRotation, desiredRotation, safeDeltaTime, TargetLockAssistTuning.CameraRotationInterpSpeed);

	OwnerPlayerController_Injected->SetControlRotation(nextRotation);
}

float UCTargetLockAssistComponent::ResolveDesiredLockPitch(float InRawTargetPitch, float InHorizontalDistance) const
{
	const float nearDistance = FMath::Max(
		FMath::Min(TargetLockAssistTuning.NearPitchHoldDistance, TargetLockAssistTuning.FullPitchTrackingDistance),
		0.f);
	const float fullTrackingDistance = FMath::Max(
		FMath::Max(TargetLockAssistTuning.NearPitchHoldDistance, TargetLockAssistTuning.FullPitchTrackingDistance),
		0.f);

	float trackingAlpha = 0.f;
	if (FMath::IsNearlyEqual(nearDistance, fullTrackingDistance))
	{
		trackingAlpha = InHorizontalDistance >= fullTrackingDistance ? 1.f : 0.f;
	}
	else
	{
		trackingAlpha = FMath::Clamp(
			FMath::GetRangePct(nearDistance, fullTrackingDistance, FMath::Max(InHorizontalDistance, 0.f)),
			0.f,
			1.f);
	}

	const float smoothTrackingAlpha = trackingAlpha * trackingAlpha * (3.f - (2.f * trackingAlpha));
	const float minLockPitch = FMath::Min(TargetLockAssistTuning.MinLockPitchDegrees, TargetLockAssistTuning.MaxLockPitchDegrees);
	const float maxLockPitch = FMath::Max(TargetLockAssistTuning.MinLockPitchDegrees, TargetLockAssistTuning.MaxLockPitchDegrees);
	const float nearLockPitch = FMath::Clamp(TargetLockAssistTuning.NearLockPitchDegrees, minLockPitch, maxLockPitch);
	const float desiredPitch = FMath::Lerp(nearLockPitch, InRawTargetPitch, smoothTrackingAlpha);

	return FMath::Clamp(desiredPitch, minLockPitch, maxLockPitch);
}
