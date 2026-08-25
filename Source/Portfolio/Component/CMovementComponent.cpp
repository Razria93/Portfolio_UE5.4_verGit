#include "Component/CMovementComponent.h"

#include "ProjectGlobal.h"

#include "AI/RuntimeLOD/CAIMovementRuntimeLODPolicy.h"
#include "Component/CHealthComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CStateComponent.h"
#include "Core/Debug/FMovementDebug.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Construction

UCMovementComponent::UCMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

// Component Reference

void UCMovementComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	CharacterMovementComp_Injected = IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->GetCharacterMovement() : nullptr;
	StateComp_Injected = InReferences.StateComponent;
	HealthComp_Injected = InReferences.HealthComponent;
	BalanceComp_Injected = InReferences.BalanceComponent;
	if (IsValid(BalanceComp_Injected))
	{
		BalanceComp_Injected->OnBalanceLifecycleStateChanged.RemoveAll(this);
		BalanceComp_Injected->OnBalanceLifecycleStateChanged.AddUObject(this, &UCMovementComponent::HandleBalanceLifecycleStateChanged);
	}

	ValidateRequiredComponentReferences();
	ApplyMovementRotationMode(CurrentMovementRotationMode);
}

// Lifecycle

void UCMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	UpdateRuntimeLODMovementMode();
}

void UCMovementComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	if (IsValid(BalanceComp_Injected))
	{
		BalanceComp_Injected->OnBalanceLifecycleStateChanged.RemoveAll(this);
	}

	Super::EndPlay(InEndPlayReason);
}

void UCMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(OwnerCharacter_Injected) || !IsValid(CharacterMovementComp_Injected)) return;

	UpdateRuntimeLODMovementMode();

	CalculateSpeed();
	CalculateDirection();

	bIsFalling = CharacterMovementComp_Injected->IsFalling();
}

// Query: Movement Arbitration

// Final movement gate shared by player input and AI movement intent.
bool UCMovementComponent::CanAcceptMovementIntent() const
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive()) return false;
	if (!bIsMovementEnabled) return false;
	if (bRuntimeLODMovementIntentBlocked) return false;
	if (IsValid(BalanceComp_Injected) && BalanceComp_Injected->IsBalanceLifecycleBlocking()) return false;

	if (IsValid(StateComp_Injected))
	{
		const EExecutionState executionState = StateComp_Injected->GetCurrentExecutionState();

		if (executionState == EExecutionState::Reaction) return false;
	}

	return true;
}

// Gameplay Movement Permission

void UCMovementComponent::SetMovementEnabled(const bool bEnabled)
{
	bIsMovementEnabled = bEnabled;
}

// Movement Input Handling

void UCMovementComponent::HandleMoveInput(const FVector2D& InAxis2D)
{
	EExecutionState executionState = EExecutionState::Max;
	if (IsValid(StateComp_Injected))
	{
		executionState = StateComp_Injected->GetCurrentExecutionState();
	}

	if (!CanAcceptMovementIntent())
	{
		const TCHAR* reason = TEXT("Rejected");
		if (!IsValid(OwnerCharacter_Injected))
		{
			reason = TEXT("InvalidOwner");
		}
		else if (!IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive())
		{
			reason = TEXT("DeadState");
		}
		else if (!bIsMovementEnabled)
		{
			reason = TEXT("MovementDisabled");
		}
		else if (bRuntimeLODMovementIntentBlocked)
		{
			reason = TEXT("RuntimeLODIntentBlocked");
		}
		else if (IsValid(BalanceComp_Injected) && BalanceComp_Injected->IsBalanceLifecycleBlocking())
		{
			reason = TEXT("BalanceLifecycleBlocking");
		}
		else if (executionState == EExecutionState::Reaction)
		{
			reason = TEXT("ReactionState");
		}

		FMovementDebug::RecordMovementInputRejectedForAudit(OwnerCharacter_Injected, this, InAxis2D, reason, executionState, bIsMovementEnabled, bRuntimeLODMovementIntentBlocked);
		return;
	}
	if (InAxis2D.IsNearlyZero())
	{
		FMovementDebug::RecordMovementInputRejectedForAudit(OwnerCharacter_Injected, this, InAxis2D, TEXT("ZeroAxis"), executionState, bIsMovementEnabled, bRuntimeLODMovementIntentBlocked);
		return;
	}
	if (!IsValid(OwnerCharacter_Injected))
	{
		FMovementDebug::RecordMovementInputRejectedForAudit(OwnerCharacter_Injected, this, InAxis2D, TEXT("InvalidOwner"), executionState, bIsMovementEnabled, bRuntimeLODMovementIntentBlocked);
		return;
	}

	const FRotator controlRot = OwnerCharacter_Injected->GetControlRotation();
	const FRotator yawRot = FRotator(0.f, controlRot.Yaw, 0.f);

	const FVector forwardDirection = FRotationMatrix(yawRot).GetUnitAxis(EAxis::X);
	const FVector rightDirection = FRotationMatrix(yawRot).GetUnitAxis(EAxis::Y);

	OwnerCharacter_Injected->AddMovementInput(forwardDirection, InAxis2D.Y);
	OwnerCharacter_Injected->AddMovementInput(rightDirection, InAxis2D.X);
	FMovementDebug::RecordMovementInputAcceptedForAudit(OwnerCharacter_Injected, this, InAxis2D, CurrentMovementGait);
}

void UCMovementComponent::HandleWalkInput()
{
	if (!CanAcceptMovementIntent()) return;
	SetMovementGait(EMovementGait::Walk);
}

void UCMovementComponent::HandleRunInput()
{
	if (!CanAcceptMovementIntent()) return;
	SetMovementGait(EMovementGait::Run);
}

void UCMovementComponent::HandleSprintInput()
{
	if (!CanAcceptMovementIntent()) return;
	SetMovementGait(EMovementGait::Sprint);
}

void UCMovementComponent::HandleJumpInput()
{
	if (!CanAcceptMovementIntent()) return;
	if (!IsValid(OwnerCharacter_Injected)) return;

	OwnerCharacter_Injected->Jump();
}

void UCMovementComponent::HandleJumpInputReleased()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	OwnerCharacter_Injected->StopJumping();
}

// Movement Policy

void UCMovementComponent::ApplyMovementGaitOverride(EMovementGait InGait)
{
	if (!bHasMovementGaitOverride)
	{
		CachedMovementGaitBeforeOverride = CurrentMovementGait;
		bHasMovementGaitOverride = true;
	}

	ApplyMovementGait(InGait);
}

void UCMovementComponent::ClearMovementGaitOverride()
{
	if (!bHasMovementGaitOverride) return;

	bHasMovementGaitOverride = false;
	ApplyMovementGait(CachedMovementGaitBeforeOverride);
}

void UCMovementComponent::SetMovementRotationMode(EMovementRotationMode InRotationMode)
{
	if (InRotationMode == EMovementRotationMode::None || InRotationMode == EMovementRotationMode::Max) return;
	if (CurrentMovementRotationMode == InRotationMode) return;

	CurrentMovementRotationMode = InRotationMode;
	ApplyMovementRotationMode(CurrentMovementRotationMode);
}

// Component Reference Validation

bool UCMovementComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ CharacterMovementComp_Injected, TEXT("UCharacterMovementComponent") },
		{ StateComp_Injected, TEXT("UCStateComponent") },
		{ HealthComp_Injected, TEXT("UCHealthComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Runtime LOD Update

void UCMovementComponent::UpdateRuntimeLODMovementMode()
{
	if (!FAIMovementRuntimeLODPolicy::IsEnemyMovementRuntimeLODTarget(OwnerCharacter_Injected)) return;

	const int32 requestedMovementMode = FAIMovementRuntimeLODPolicy::GetEnemyMovementMode(OwnerCharacter_Injected);
	if (RuntimeLODMovementState.AppliedMode == requestedMovementMode) return;

	const int32 previousMovementMode = RuntimeLODMovementState.AppliedMode;

	ApplyRuntimeLODMovementMode(requestedMovementMode);
	RuntimeLODMovementState.AppliedMode = requestedMovementMode;

	FMovementDebug::RecordRuntimeLODMovementModeAppliedForAudit(OwnerCharacter_Injected, this, previousMovementMode, requestedMovementMode, bIsMovementEnabled, bRuntimeLODMovementIntentBlocked);
}

void UCMovementComponent::ApplyRuntimeLODMovementMode(const int32 InMovementMode)
{
	const bool bShouldBlockMovementIntent = FAIMovementRuntimeLODPolicy::ShouldBlockMovementIntent(InMovementMode);
	const bool bWasMovementIntentBlocked = bRuntimeLODMovementIntentBlocked;

	SetRuntimeLODMovementIntentBlocked(bShouldBlockMovementIntent);

	if (!bWasMovementIntentBlocked && bShouldBlockMovementIntent)
	{
		StopActiveAIMovement();
	}
}

void UCMovementComponent::SetRuntimeLODMovementIntentBlocked(const bool bBlocked)
{
	if (bRuntimeLODMovementIntentBlocked == bBlocked) return;

	bRuntimeLODMovementIntentBlocked = bBlocked;

	if (bBlocked)
	{
		FMovementDebug::RecordRuntimeLODMovementIntentBlockedForAudit(OwnerCharacter_Injected, this, TEXT("RuntimeLODMode"));
	}
	else
	{
		FMovementDebug::RecordRuntimeLODMovementIntentAllowedForAudit(OwnerCharacter_Injected, this, TEXT("RuntimeLODMode"));
	}
}

void UCMovementComponent::StopActiveAIMovement()
{
	AAIController* aiController = IsValid(OwnerCharacter_Injected) ? Cast<AAIController>(OwnerCharacter_Injected->GetController()) : nullptr;
	if (!IsValid(aiController)) return;

	aiController->StopMovement();
}

// Balance Lifecycle Event

void UCMovementComponent::HandleBalanceLifecycleStateChanged(const EBalanceLifecycleState InPreviousState, const EBalanceLifecycleState InNewState)
{
	if (InPreviousState != EBalanceLifecycleState::Accumulating) return;
	if (InNewState == EBalanceLifecycleState::Accumulating) return;

	StopActiveAIMovement();
}

// Gait Implementation

void UCMovementComponent::SetMovementGait(EMovementGait InNewMovementGait)
{
	if (bHasMovementGaitOverride)
	{
		// Keep override speed active, but remember the base gait to restore later.
		CachedMovementGaitBeforeOverride = InNewMovementGait;
		return;
	}

	ApplyMovementGait(InNewMovementGait);
}

void UCMovementComponent::ApplyMovementGait(EMovementGait InNewMovementGait)
{
	if (!IsValid(CharacterMovementComp_Injected))
	{
		FMovementDebug::RecordMovementGaitRejectedForAudit(OwnerCharacter_Injected, this, InNewMovementGait, TEXT("InvalidMovementComponent"));
		return;
	}
	if (InNewMovementGait == EMovementGait::None || InNewMovementGait == EMovementGait::Max)
	{
		FMovementDebug::RecordMovementGaitRejectedForAudit(OwnerCharacter_Injected, this, InNewMovementGait, TEXT("InvalidGait"));
		return;
	}

	const float* speed = GaitSpeedMap.Find(InNewMovementGait);

	if (!speed)
	{
		FMovementDebug::RecordMovementGaitRejectedForAudit(OwnerCharacter_Injected, this, InNewMovementGait, TEXT("MissingGaitSpeed"));
		return;
	}

	CurrentMovementGait = InNewMovementGait;
	CharacterMovementComp_Injected->MaxWalkSpeed = *speed;
	FMovementDebug::RecordMovementGaitAppliedForAudit(OwnerCharacter_Injected, this, InNewMovementGait, *speed);
}

// Rotation Implementation

void UCMovementComponent::ApplyMovementRotationMode(EMovementRotationMode InRotationMode)
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(CharacterMovementComp_Injected)) return;

	switch (InRotationMode)
	{
	case EMovementRotationMode::OrientToMovement:
		CharacterMovementComp_Injected->bOrientRotationToMovement = true;
		CharacterMovementComp_Injected->bUseControllerDesiredRotation = false;
		OwnerCharacter_Injected->bUseControllerRotationYaw = false;
		break;

	case EMovementRotationMode::ControllerDesired:
		CharacterMovementComp_Injected->bOrientRotationToMovement = false;
		CharacterMovementComp_Injected->bUseControllerDesiredRotation = true;
		OwnerCharacter_Injected->bUseControllerRotationYaw = false;
		break;

	case EMovementRotationMode::FixedFacing:
		CharacterMovementComp_Injected->bOrientRotationToMovement = false;
		CharacterMovementComp_Injected->bUseControllerDesiredRotation = false;
		OwnerCharacter_Injected->bUseControllerRotationYaw = false;
		break;

	default:
		break;
	}
}

// Runtime State Refresh

void UCMovementComponent::CalculateSpeed()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(CharacterMovementComp_Injected)) return;

	CurrentSpeed = OwnerCharacter_Injected->GetVelocity().Size2D();
}

void UCMovementComponent::CalculateDirection()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(CharacterMovementComp_Injected)) return;

	if (CurrentSpeed < KINDA_SMALL_NUMBER)
	{
		CurrentDirection = 0.f;
		return;
	}
	else
	{
		const FVector velocityNormal = OwnerCharacter_Injected->GetVelocity().GetSafeNormal2D();
		const FVector forwardNormal = OwnerCharacter_Injected->GetActorForwardVector().GetSafeNormal2D();

		float angleRad = FMath::Acos(FVector::DotProduct(forwardNormal, velocityNormal));
		float angleDeg = FMath::RadiansToDegrees(angleRad);

		// Determine left or right from the movement input basis.
		FVector crossProduct = FVector::CrossProduct(forwardNormal, velocityNormal);
		if (crossProduct.Z < 0)
		{
			angleDeg *= -1.f;
		}

		CurrentDirection = angleDeg;
	}
}
