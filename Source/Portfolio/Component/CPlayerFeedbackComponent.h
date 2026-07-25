#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCombatFeedbackTypes.h"
#include "CPlayerFeedbackComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PORTFOLIO_API UCPlayerFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCPlayerFeedbackComponent();

private:
	UPROPERTY(EditAnywhere, Category = "Feedback|CameraShake")
	FPlayerCameraShakeFeedbackTuning CameraShakeTuning;

private:
	UPROPERTY(Transient)
	class APlayerController* OwnerPlayerController_Injected = nullptr;

public:
	// Component Reference
	void InitializeReferences(class APlayerController* InOwnerPlayerController);

private:
	bool ValidateRequiredComponentReferences() const;

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Camera Shake
	void HandleCameraShakeRequest(const FCameraShakeRequest& InCameraShakeRequest);

private:
	bool CanCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const;
	float ResolveCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const;
	void PlayCameraShake(const FCameraShakeRequest& InCameraShakeRequest, float InScale) const;
};
