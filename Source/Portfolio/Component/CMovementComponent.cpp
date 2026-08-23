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

// Runtime LOD

void UCMovementComponent::UpdateRuntimeLODMovementMode()
{
	if (!FAIMovementRuntimeLODPolicy::IsEnemyMovementRuntimeLODTarget(OwnerCharacter_Injected)) return;

	const int32 requestedMovementMode = FAIMovementRuntimeLODPolicy::GetEnemyMovementMode(OwnerCharacter_Injected);

	EnsureRuntimeLODMovementOriginalStateCached();

	if (RuntimeLODMovementState.AppliedMode != requestedMovementMode)
	{
		const int32 previousMovementMode = RuntimeLODMovementState.AppliedMode;
		ApplyRuntimeLODMovementMode(requestedMovementMode);
		RuntimeLODMovementState.AppliedMode = requestedMovementMode;
		FMovementDebug::RecordRuntimeLODMovementModeAppliedForAudit(OwnerCharacter_Injected, this, previousMovementMode, requestedMovementMode, IsComponentTickEnabled(), bCanMove, bRuntimeLODMovementIntentBlocked);
	}

	if (FAIMovementRuntimeLODPolicy::ShouldBlockMovementIntent(requestedMovementMode))
	{
		BlockRuntimeLODMovementIntent();
	}
}

void UCMovementComponent::EnsureRuntimeLODMovementOriginalStateCached()
{
	if (RuntimeLODMovementState.bOriginalStateCached) return;

	RuntimeLODMovementState.bOriginalMovementComponentTickEnabled = IsComponentTickEnabled();
	RuntimeLODMovementState.bOriginalStateCached = true;
}

void UCMovementComponent::ApplyRuntimeLODMovementMode(int32 InMovementMode)
{
	if (FAIMovementRuntimeLODPolicy::ShouldDisableMovementStateRefresh(InMovementMode))
	{
		ApplyRuntimeLODMovementStateRefreshDisabled();
		return;
	}

	if (FAIMovementRuntimeLODPolicy::ShouldBlockMovementIntent(InMovementMode))
	{
		ApplyRuntimeLODMovementIntentBlocked();
		return;
	}

	ApplyRuntimeLODMovementDefault();
}

void UCMovementComponent::ApplyRuntimeLODMovementDefault()
{
	RestoreRuntimeLODMovementStateRefresh();
	AllowRuntimeLODMovementIntent();
}

void UCMovementComponent::ApplyRuntimeLODMovementStateRefreshDisabled()
{
	AllowRuntimeLODMovementIntent();
	DisableRuntimeLODMovementStateRefresh();
}

void UCMovementComponent::ApplyRuntimeLODMovementIntentBlocked()
{
	RestoreRuntimeLODMovementStateRefresh();
	BlockRuntimeLODMovementIntent();
	StopActiveAIMovement();
}

void UCMovementComponent::RestoreRuntimeLODMovementStateRefresh()
{
	SetComponentTickEnabled(RuntimeLODMovementState.bOriginalMovementComponentTickEnabled);
}

void UCMovementComponent::DisableRuntimeLODMovementStateRefresh()
{
	SetComponentTickEnabled(false);
}

// Movement State

void UCMovementComponent::AllowRuntimeLODMovementIntent()
{
	if (bRuntimeLODMovementIntentBlocked)
	{
		FMovementDebug::RecordRuntimeLODMovementIntentAllowedForAudit(OwnerCharacter_Injected, this, TEXT("RuntimeLODMode"));
	}

	ClearMovementIntentBlockForRuntimeLOD();
	SetMove();
}

void UCMovementComponent::BlockRuntimeLODMovementIntent()
{
	if (!bRuntimeLODMovementIntentBlocked)
	{
		FMovementDebug::RecordRuntimeLODMovementIntentBlockedForAudit(OwnerCharacter_Injected, this, TEXT("RuntimeLODMode"));
	}

	BlockMovementIntentForRuntimeLOD();
}

void UCMovementComponent::StopActiveAIMovement()
{
	AAIController* aiController = IsValid(OwnerCharacter_Injected) ? Cast<AAIController>(OwnerCharacter_Injected->GetController()) : nullptr;
	if (!IsValid(aiController)) return;

	aiController->StopMovement();
}

void UCMovementComponent::HandleBalanceLifecycleStateChanged(const EBalanceLifecycleState InPreviousState, const EBalanceLifecycleState InNewState)
{
	if (InPreviousState != EBalanceLifecycleState::Accumulating) return;
	if (InNewState == EBalanceLifecycleState::Accumulating) return;

	StopActiveAIMovement();
}

// Movement Arbitration

// Final movement gate for axis input accepted by the orchestrator.
bool UCMovementComponent::CanAcceptMoveInput() const
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive()) return false;
	if (!bCanMove) return false;
	if (IsValid(BalanceComp_Injected) && BalanceComp_Injected->IsBalanceLifecycleBlocking()) return false;

	if (IsValid(StateComp_Injected))
	{
		const EExecutionState executionState = StateComp_Injected->GetCurrentExecutionState();

		if (executionState == EExecutionState::Reaction) return false;
	}

	return true;
}

void UCMovementComponent::BlockMovementIntentForRuntimeLOD()
{
	bRuntimeLODMovementIntentBlocked = true;
	bCanMove = false;
}

void UCMovementComponent::ClearMovementIntentBlockForRuntimeLOD()
{
	bRuntimeLODMovementIntentBlocked = false;
}

// Movement Input

void UCMovementComponent::OnMove(const FVector2D& InAxis2D)
{
	EExecutionState executionState = EExecutionState::Max;
	if (IsValid(StateComp_Injected))
	{
		executionState = StateComp_Injected->GetCurrentExecutionState();
	}

	if (!CanAcceptMoveInput())
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
		else if (bRuntimeLODMovementIntentBlocked)
		{
			reason = TEXT("RuntimeLODIntentBlocked");
		}
	else if (!bCanMove)
		{
		reason = TEXT("CannotMove");
	}
	else if (IsValid(BalanceComp_Injected) && BalanceComp_Injected->IsBalanceLifecycleBlocking())
	{
		reason = TEXT("BalanceLifecycleBlocking");
	}
		else if (executionState == EExecutionState::Reaction)
		{
			reason = TEXT("ReactionState");
		}

		FMovementDebug::RecordMovementInputRejectedForAudit(OwnerCharacter_Injected, this, InAxis2D, reason, executionState, bCanMove, bRuntimeLODMovementIntentBlocked);
		return;
	}
	if (InAxis2D.IsNearlyZero())
	{
		FMovementDebug::RecordMovementInputRejectedForAudit(OwnerCharacter_Injected, this, InAxis2D, TEXT("ZeroAxis"), executionState, bCanMove, bRuntimeLODMovementIntentBlocked);
		return;
	}
	if (!IsValid(OwnerCharacter_Injected))
	{
		FMovementDebug::RecordMovementInputRejectedForAudit(OwnerCharacter_Injected, this, InAxis2D, TEXT("InvalidOwner"), executionState, bCanMove, bRuntimeLODMovementIntentBlocked);
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

void UCMovementComponent::OnWalk()
{
	if (IsValid(BalanceComp_Injected) && BalanceComp_Injected->IsBalanceLifecycleBlocking()) return;
	SetMovementGait(EMovementGait::Walk);
}

void UCMovementComponent::OnRun()
{
	if (IsValid(BalanceComp_Injected) && BalanceComp_Injected->IsBalanceLifecycleBlocking()) return;
	SetMovementGait(EMovementGait::Run);
}

void UCMovementComponent::OnSprint()
{
	if (IsValid(BalanceComp_Injected) && BalanceComp_Injected->IsBalanceLifecycleBlocking()) return;
	SetMovementGait(EMovementGait::Sprint);
}

void UCMovementComponent::OnJump()
{
	if (IsValid(BalanceComp_Injected) && BalanceComp_Injected->IsBalanceLifecycleBlocking()) return;
	if (!IsValid(OwnerCharacter_Injected)) return;

	OwnerCharacter_Injected->Jump();
}

void UCMovementComponent::OnStopJump()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	OwnerCharacter_Injected->StopJumping();
}

// Movement Policy

void UCMovementComponent::ApplyMovementGaitOverride(EMovementGait InGait)
{
	if (!bHasMovementGaitOverride)
	{
		CachedMovementGait_BeforeOverride = CurrentMovementGait;
		bHasMovementGaitOverride = true;
	}

	ApplyMovementGait(InGait);
}

void UCMovementComponent::ClearMovementGaitOverride()
{
	if (!bHasMovementGaitOverride) return;

	bHasMovementGaitOverride = false;
	ApplyMovementGait(CachedMovementGait_BeforeOverride);
}

void UCMovementComponent::SetMovementRotationMode(EMovementRotationMode InRotationMode)
{
	if (InRotationMode == EMovementRotationMode::None || InRotationMode == EMovementRotationMode::Max) return;
	if (CurrentMovementRotationMode == InRotationMode) return;

	CurrentMovementRotationMode = InRotationMode;
	ApplyMovementRotationMode(CurrentMovementRotationMode);
}

void UCMovementComponent::SetMovementGait(EMovementGait InNewMovementGait)
{
	if (bHasMovementGaitOverride)
	{
		// Keep override speed active, but remember the base gait to restore later.
		CachedMovementGait_BeforeOverride = InNewMovementGait;
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

// Runtime State

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
