#include "Component/CPlayerFeedbackComponent.h"

#include "ProjectGlobal.h"

#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"

UCPlayerFeedbackComponent::UCPlayerFeedbackComponent()
{
}

// Component Reference

void UCPlayerFeedbackComponent::InitializeReferences(APlayerController* InOwnerPlayerController)
{
	OwnerPlayerController_Injected = InOwnerPlayerController;

	ValidateRequiredComponentReferences();
}

bool UCPlayerFeedbackComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerPlayerController_Injected, TEXT("APlayerController Owner") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerPlayerController_Injected, this);
	}

	return bValid;
}

// Lifecycle

void UCPlayerFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* world = GetWorld())
	{
		if (UCWorldSubsystem_CombatFeedback* feedbackSubsystem = world->GetSubsystem<UCWorldSubsystem_CombatFeedback>())
		{
			feedbackSubsystem->OnCameraShakeRequested.AddUObject(this, &UCPlayerFeedbackComponent::HandleCameraShakeRequest);
		}
	}
}

void UCPlayerFeedbackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* world = GetWorld())
	{
		if (UCWorldSubsystem_CombatFeedback* feedbackSubsystem = world->GetSubsystem<UCWorldSubsystem_CombatFeedback>())
		{
			feedbackSubsystem->OnCameraShakeRequested.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

// Camera Shake

void UCPlayerFeedbackComponent::HandleCameraShakeRequest(const FCameraShakeRequest& InCameraShakeRequest)
{
	// Camera shake is requested only by the local player controller.
	if (!CanCameraShake(InCameraShakeRequest)) return;

	const float finalScale = ResolveCameraShake(InCameraShakeRequest);
	if (finalScale <= KINDA_SMALL_NUMBER) return;

	PlayCameraShake(InCameraShakeRequest, finalScale);
}

bool UCPlayerFeedbackComponent::CanCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const
{
	if (!IsValid(OwnerPlayerController_Injected)) return false;
	if (!OwnerPlayerController_Injected->IsLocalController()) return false;
	if (!IsValid(OwnerPlayerController_Injected->PlayerCameraManager)) return false;

	if (!IsValid(InCameraShakeRequest.CameraShakeClass)) return false;

	if (!FMath::IsFinite(InCameraShakeRequest.CameraShakeBaseScale)) return false;
	if (InCameraShakeRequest.CameraShakeBaseScale <= KINDA_SMALL_NUMBER) return false;

	return true;
}

float UCPlayerFeedbackComponent::ResolveCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const
{
	if (!IsValid(OwnerPlayerController_Injected)) return 0.f;

	APawn* pawn = OwnerPlayerController_Injected->GetPawn();
	if (!IsValid(pawn)) return 0.f;

	const bool bIsLocalTarget = IsValid(InCameraShakeRequest.TargetActor) && InCameraShakeRequest.TargetActor == pawn;
	const bool bIsLocalSource = IsValid(InCameraShakeRequest.SourceActor) && InCameraShakeRequest.SourceActor == pawn;

	switch (InCameraShakeRequest.CameraShakeAudience)
	{
	case EFeedbackAudience::Source:
		return bIsLocalSource ? InCameraShakeRequest.CameraShakeBaseScale * LocalSourceShakeScale : 0.f;

	case EFeedbackAudience::Target:
		return bIsLocalTarget ? InCameraShakeRequest.CameraShakeBaseScale * LocalTargetShakeScale : 0.f;

	case EFeedbackAudience::Both:
		if (bIsLocalTarget) return InCameraShakeRequest.CameraShakeBaseScale * LocalTargetShakeScale;
		if (bIsLocalSource) return InCameraShakeRequest.CameraShakeBaseScale * LocalSourceShakeScale;

	default:
		break;
	}

	return 0.f;
}

void UCPlayerFeedbackComponent::PlayCameraShake(const FCameraShakeRequest& InCameraShakeRequest, float InCameraShakeFinalScale) const
{
	if (!IsValid(OwnerPlayerController_Injected)) return;
	if (!IsValid(OwnerPlayerController_Injected->PlayerCameraManager)) return;

	OwnerPlayerController_Injected->PlayerCameraManager->StartCameraShake(InCameraShakeRequest.CameraShakeClass, InCameraShakeFinalScale);
}
