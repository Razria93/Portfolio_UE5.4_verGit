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
	UPROPERTY(EditAnywhere)
	float LocalTargetShakeScale = 1.0f;

	UPROPERTY(EditAnywhere)
	float LocalSourceShakeScale = 0.5f;

private:
	UPROPERTY(Transient)
	class APlayerController* OwnerPlayerController_Cached = nullptr;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void HandleCameraShakeRequest(const FCameraShakeRequest& InCameraShakeRequest);

private:
	bool CanCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const;
	float ResolveCameraShake(const FCameraShakeRequest& InCameraShakeRequest) const;
	void PlayCameraShake(const FCameraShakeRequest& InCameraShakeRequest, float InScale) const;

private:
	void PrintCameraShakeConsumeInfo(const FCameraShakeRequest& InCameraShakeRequest, float InFinalScale) const;
};
