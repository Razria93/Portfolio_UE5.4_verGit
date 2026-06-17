#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "Interface/ObservableOverlayPolicy.h"
#include "CDefenseComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCDefenseComponent : public UActorComponent, public IObservableOverlayPolicy
{
	GENERATED_BODY()

public:
	UCDefenseComponent();

private:

	UPROPERTY(Transient)
	bool bCanStartGuard = true;

	UPROPERTY(Transient)
	bool bWantsGuarding = false;

	UPROPERTY(Transient)
	bool bIsGuardingPose = false;

	UPROPERTY(Transient)
	bool bCanGuard = false;

	UPROPERTY(Transient)
	bool bCanParry = false;

public:
	FORCEINLINE bool CanStartGuard() const { return bCanStartGuard; }
	FORCEINLINE bool WantsGuarding() const { return bWantsGuarding; }
	FORCEINLINE bool IsGuardingPose() const { return bIsGuardingPose; }
	FORCEINLINE bool CanGuard() const { return bCanGuard; }
	FORCEINLINE bool CanParry() const { return bCanParry; }
	FORCEINLINE bool HasGuardOverlay() const { return bIsGuardingPose || bCanGuard || bCanParry; }
	FORCEINLINE bool HasGuardRuntimeState() const { return !bCanStartGuard || bWantsGuarding || HasGuardOverlay(); }

public:
	void WriteObservableOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const override;
	bool CanApplyObservableOverlayHandling(EObservableOverlayHandling InHandling) const override;
	bool ApplyObservableOverlayHandling(EObservableOverlayHandling InHandling) override;

public:
	void HandleGuardInputPressed();
	void HandleGuardInputReleased();

	void HandleGuardInStarted();
	void HandleGuardOutStarted();

	void HandleSwitchToGuard();
	void HandleAllowGuardStart();

	void HandleGuardLifecycleCompleted();
	void HandleGuardLifecycleInterrupted();

public:
	void ClearGuardState();
	void ClearGuardOverlay();
	void RestoreGuardOverlay();

private:
	void AllowGuardStart();
	void BlockGuardStart();

	void BeginGuardIntent();
	void EndGuardIntent();

	void BeginGuardPose();
	void EndGuardPose();

	void OpenGuardWindow();
	void CloseGuardWindow();

	void OpenParryWindow();
	void CloseParryWindow();

private:
	void PrintGuardStateInfo() const;
};
