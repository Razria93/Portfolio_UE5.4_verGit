#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACPlayerController();

private:
	// Components
	UPROPERTY(VisibleAnywhere)
	class UCPlayerFeedbackComponent* PlayerFeedbackComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCTargetingComponent* TargetingComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCDebugOverlayFocusComponent* DebugOverlayFocusComponent = nullptr;

	// Cached input
	FVector2D CachedMoveAxis2D = FVector2D::ZeroVector;

public:
	// Component Query
	FORCEINLINE class UCTargetingComponent* GetTargetingComp() const { return TargetingComponent; }

	// Debug Overlay Exec
	UFUNCTION(Exec)
	void DebugOverlaySelectNearestFocus();

	UFUNCTION(Exec)
	void DebugOverlaySelectOutlinerFocus(const FString& ActorName);

	UFUNCTION(Exec)
	void DebugOverlaySelectRecentCombatFocus();

	UFUNCTION(Exec)
	void DebugOverlayClearFocus();

protected:
	// Lifecycle
	virtual void PostInitializeComponents() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	// Look Input
	void InputLookYaw(float InAxisValue);
	void InputLookPitch(float InAxisValue);

	// Move Input
	void InputMoveForward(float InAxisValue);
	void InputMoveRight(float InAxisValue);

	// Movement Dispatch
	void FlushMoveInput();

	// Action Input
	void PressWalk();
	void ReleaseWalk();

	void PressJump();
	void ReleaseJump();

	void PressComboAction();
	void PressDodge();
	void PressGuard();
	void ReleaseGuard();
	void PressSwordToggle();
	void PressTargetLock();
};
