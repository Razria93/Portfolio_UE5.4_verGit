#include "Component/CPlayerFeedbackComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"

#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

UCPlayerFeedbackComponent::UCPlayerFeedbackComponent()
{
}

void UCPlayerFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPlayerController_Cached = Cast<APlayerController>(GetOwner());
	check(OwnerPlayerController_Cached);

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

void UCPlayerFeedbackComponent::HandleCameraShakeRequest(const FCameraShakeRequest& InCameraShakeRequest)
{
	// [NOTE] Only the local player controller.
	if (!CanCameraShake(InCameraShakeRequest)) return;

	const float finalScale = ResolveCameraShake(InCameraShakeRequest);
	if (finalScale <= KINDA_SMALL_NUMBER) return;

	FLog::Log(TEXT("[UCPlayerFeedbackComponent] Handle CameraShakeRequest"));
	PrintCameraShakeConsumeInfo(InCameraShakeRequest, finalScale);

	PlayCameraShake(InCameraShakeRequest, finalScale);
}

bool UCPlayerFeedbackComponent::CanCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const
{
	if (!IsValid(OwnerPlayerController_Cached)) return false;
	if (!OwnerPlayerController_Cached->IsLocalController()) return false;
	if (!IsValid(OwnerPlayerController_Cached->PlayerCameraManager)) return false;

	if (!IsValid(InCameraShakeRequest.CameraShakeClass)) return false;

	if (!FMath::IsFinite(InCameraShakeRequest.CameraShakeBaseScale)) return false;
	if (InCameraShakeRequest.CameraShakeBaseScale <= KINDA_SMALL_NUMBER) return false;

	return true;
}

float UCPlayerFeedbackComponent::ResolveCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const
{
	if (!IsValid(OwnerPlayerController_Cached)) return 0.f;

	APawn* pawn = OwnerPlayerController_Cached->GetPawn();
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
	if (!IsValid(OwnerPlayerController_Cached)) return;
	if (!IsValid(OwnerPlayerController_Cached->PlayerCameraManager)) return;

	OwnerPlayerController_Cached->PlayerCameraManager->StartCameraShake(InCameraShakeRequest.CameraShakeClass, InCameraShakeFinalScale);
}

void UCPlayerFeedbackComponent::PrintCameraShakeConsumeInfo(const FCameraShakeRequest& InCameraShakeRequest, float InCameraShakeFinalScale) const
{
	FLog::Log(TEXT("======== CameraShake Info ======="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CameraShakeAudience"), *UEnum::GetValueAsString(InCameraShakeRequest.CameraShakeAudience)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CameraShakeClass"), *GetNameSafe(InCameraShakeRequest.CameraShakeClass)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("SourceActor"), *GetNameSafe(InCameraShakeRequest.SourceActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InCameraShakeRequest.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.2f"), TEXT("CameraShakeBaseScale"), InCameraShakeRequest.CameraShakeBaseScale));
	FLog::Log(FString::Printf(TEXT("%-20s: %.2f"), TEXT("CameraShakeFinalScale"), InCameraShakeFinalScale));
	FLog::Log(TEXT("================================="));
}
