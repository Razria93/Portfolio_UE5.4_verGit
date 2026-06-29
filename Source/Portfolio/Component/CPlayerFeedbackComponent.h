#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWorldSubSystemStructure.h"
#include "CPlayerFeedbackComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCPlayerFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCPlayerFeedbackComponent();

private:
	UPROPERTY(EditAnywhere, Category = "Feedback|CameraShake")
	float LocalTargetShakeScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Feedback|CameraShake")
	float LocalSourceShakeScale = 0.5f;

private:
	/* === Injected Objects === */
	UPROPERTY(Transient)
	class APlayerController* OwnerPlayerController_Injected = nullptr;

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Component Reference
	void InitializeReferences(class APlayerController* InOwnerPlayerController);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	void HandleCameraShakeRequest(const FCameraShakeRequest& InCameraShakeRequest);

private:
	bool CanCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const;
	float ResolveCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const;
	void PlayCameraShake(const FCameraShakeRequest& InCameraShakeRequest, float InScale) const;

private:
	void PrintCameraShakeConsumeInfo(const FCameraShakeRequest& InCameraShakeRequest, float InFinalScale) const;
};
