#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CDefenseTuningTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "Type/CHealthTypes.h"
#include "Type/CObservableOverlayTypes.h"
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
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Injected = nullptr;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Defense|Guard")
	FDefenseGuardTuning GuardTuning;

	UPROPERTY(EditDefaultsOnly, Category = "Defense|Dodge")
	FDefenseDodgeTuning DodgeTuning;

	// Ownership is checked against both the action generation and montage instance.
	uint64 DodgeGeneration = 0;
	int32 DodgeMontageInstanceId = INDEX_NONE;
	TWeakObjectPtr<class UAnimMontage> DodgeMontage;
	bool bPerfectConsumed = false;
	uint64 PendingPerfectSerial = 0;
	uint64 PendingPerfectDodgeGeneration = 0;
	uint64 PerfectGeneration = 0;
	int32 PerfectMontageInstanceId = INDEX_NONE;
	TWeakObjectPtr<UAnimMontage> PerfectMontage;
	TWeakObjectPtr<AActor> PendingPerfectTarget;
	TWeakObjectPtr<AActor> CounterTarget;
	TWeakObjectPtr<AActor> PerfectAttacker;
	TWeakObjectPtr<UCHealthComponent> CounterTargetHealth;
	FDelegateHandle CounterTargetDeathHandle;
	uint64 CounterSerial = 0;
	double CounterExpiresAt = 0.;
	bool bCounterTransition = false;
	uint64 ReservedCounterSerial = 0;
	uint64 ReservedPerfectGeneration = 0;
	bool bCounterOpportunityOpen = false;

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
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Component Lifecycle
	void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Query
	FORCEINLINE bool CanStartGuard() const { return bCanStartGuard; }
	FORCEINLINE bool WantsGuarding() const { return bWantsGuarding; }
	FORCEINLINE bool IsGuardingPose() const { return bIsGuardingPose; }
	FORCEINLINE bool CanGuard() const { return bCanGuard; }
	FORCEINLINE bool CanParry() const { return bCanParry; }
	FORCEINLINE bool HasGuardOverlay() const { return bIsGuardingPose || bCanGuard || bCanParry; }
	FORCEINLINE bool HasGuardRuntimeState() const { return !bCanStartGuard || bWantsGuarding || HasGuardOverlay(); }
	FORCEINLINE float GetGuardDamageTakenMultiplier() const { return GuardTuning.GuardDamageTakenMultiplier; }
	bool CanDodge() const;
	bool IsPerfectDodgeActive() const;
	bool CanStartPerfectDodge(uint64 InResultSerial) const;
	bool CanCommitPerfectDodge(uint64 InResultSerial) const;
	AActor* GetCounterRushTarget() const;
	uint64 GetCounterRushSerial() const;
	// Presentation lifetime, not input eligibility: includes the pre-connect preparation phase.
	bool IsCounterRushPresentationPending(uint64 InSerial) const;
	AActor* GetPerfectDodgeAttacker() const { return PerfectGeneration != 0 ? PerfectAttacker.Get() : nullptr; }

public:
	// Dodge Execution Ownership
	void BeginDodgeExecution(uint64 InGeneration, UAnimMontage* InMontage);
	void EndDodgeExecution(uint64 InGeneration);
	void ClearDodgeState();
	EDamageDefenseOutcome ResolveDodgeContact();
	void PreparePerfectDodge(uint64 InResultSerial, AActor* InAttacker = nullptr);
	void FinishPerfectDodgeRequest(uint64 InResultSerial);
	void BeginPerfectDodgeExecution(uint64 InGeneration, UAnimMontage* InMontage, uint64 InResultSerial);
	void EndPerfectDodgeExecution(uint64 InGeneration, bool bCompleted = false);
	bool BeginCounterRushTransition();
	void EndCounterRushTransition();
	// Point markers may only mutate the current PerfectDodge montage instance.
	bool SetCounterOpportunityWindow(bool bOpen, uint64 InGeneration, int32 InMontageInstanceId);
	bool ConsumeCounterRush(uint64 InSerial);
	void ClearCounterRush();

private:
	bool TryGetDodgePosition(float& OutPosition) const;
	void HandleOwnerHealthChanged();
	void HandleOwnerDeadStateChanged(EDeadState InPrevious, EDeadState InCurrent);
	void HandleCounterTargetDeadStateChanged(EDeadState InPrevious, EDeadState InCurrent);

public:
	// Overlay Policy
	void WriteOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const override;
	bool CanApplyOverlayEvent(const FObservableOverlayEventContext& InContext) const override;
	bool ApplyOverlayEvent(const FObservableOverlayEventContext& InContext) override;
	bool CanApplyOverlayHandling(EObservableOverlayHandling InHandling) const override;
	bool ApplyOverlayHandling(EObservableOverlayHandling InHandling) override;

public:
	// Guard Event Entry
	void HandleGuardInputPressed();
	void HandleGuardInputReleased();

	void HandleGuardInStarted();
	void HandleGuardOutStarted();

	void HandleSwitchToGuard();
	void HandleAllowGuardStart();

	void HandleGuardLifecycleCompleted();
	void HandleGuardLifecycleInterrupted();

public:
	// Guard State Cleanup
	void ClearGuardState();
	void ClearGuardOverlay();
	void RestoreGuardOverlay();

private:
	// Guard State Primitive
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
	// Movement Gait Override
	void ApplyGuardMovementGaitOverride();
	void ClearMovementGaitOverride();
};
