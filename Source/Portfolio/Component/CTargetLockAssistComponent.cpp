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
	FRotator desiredRotation = (targetFocusLocation - viewLocation).Rotation();
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
