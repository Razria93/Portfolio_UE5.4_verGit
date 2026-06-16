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
	bool bWantsGuarding = false;

	UPROPERTY(Transient)
	bool bIsGuardingPose = false;

	UPROPERTY(Transient)
	bool bCanGuard = false;

	UPROPERTY(Transient)
	bool bCanParry = false;

public:
	FORCEINLINE bool WantsGuarding() const { return bWantsGuarding; }
	FORCEINLINE bool IsGuardingPose() const { return bIsGuardingPose; }
	FORCEINLINE bool CanGuard() const { return bCanGuard; }
	FORCEINLINE bool CanParry() const { return bCanParry; }
	FORCEINLINE bool HasGuardOverlay() const { return bIsGuardingPose || bCanGuard || bCanParry; }

public:
	void BeginGuardIntent();
	void EndGuardIntent();

	void BeginGuardPose();
	void EndGuardPose();

	void OpenGuardWindow();
	void CloseGuardWindow();

	void OpenParryWindow();
	void CloseParryWindow();

	void ResetGuardState();
	void ClearGuardOverlay();

public:
	void ResolveObservableOverlayDecision(const FObservableOverlayQuery& InQuery, FObservableOverlayDecision& OutDecision) const override;

private:
	bool NeedsObservableOverlayGate(EExecutionApplyMode InApplyMode) const;
	void ResolveGuardOverlayForAction(const FActionExecutionContext& InIncomingContext, FObservableOverlayDecision& OutDecision) const;
	void ResolveGuardOverlayForReaction(const FReactionExecutionContext& InIncomingContext, FObservableOverlayDecision& OutDecision) const;

public:
	void HandleGuardInStarted();
	void HandleGuardOutStarted();
	void HandleGuardOutCompleted();
	void HandleGuardInterrupted(EActionStopReason InStopReason);

public:
	void PrintGuardStateInfo() const;
};
