#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CWeaponTypes.h"
#include "Type/CActionKeyTypes.h"
#include "Type/CWeaponPresentationTypes.h"
#include "CWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWeaponTypeChanged, class ACharacter*, InOwnerCharacter, EWeaponType, InPrevWeaponType, EWeaponType, InNewWeaponType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCWeaponComponent();

private:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	EWeaponType WeaponActorClassKey = EWeaponType::Max;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class ACWeaponActor> WeaponActorClass;

private:
	// Weapon Actor Runtime State
	UPROPERTY(Transient)
	EWeaponType CurrentWeaponType = EWeaponType::Unarmed;

	UPROPERTY(Transient)
	class ACWeaponActor* WeaponActor = nullptr;

	UPROPERTY(Transient)
	bool bWeaponActorDisabledForProfiling = false;

private:
	// Component References
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCCombatSignalSourceComponent* CombatSignalSourceComp_Injected = nullptr;

private:
	// Weapon Presentation Runtime State Types
	struct FWeaponActionPoseScopeState
	{
		uint32 ActiveHandle = 0;
		TWeakObjectPtr<class ACWeaponActor> ScopedWeaponActor;
		FName BaselineSocketName = NAME_None;
		FQuat BaselinePivotRotation = FQuat::Identity;
		bool bHasSocketBaseline = false;
	};

	struct FWeaponSocketTransformTransitionState
	{
		FName SourceSocketName = NAME_None;
		FName TargetSocketName = NAME_None;
		uint32 ActiveHandle = 0;
		uint32 NextHandle = 1;
		uint32 ActionPoseScopeHandle = 0;
	};

	struct FWeaponPivotRotationTransitionState
	{
		FQuat SourceRotation = FQuat::Identity;
		FQuat CurrentRotation = FQuat::Identity;
		FWeaponPivotRotationSpec RotationSpec;
		uint32 ActiveHandle = 0;
		uint32 NextHandle = 1;
		uint32 ActionPoseScopeHandle = 0;
	};

	struct FWeaponPivotRotationOverrideState
	{
		FQuat TemporaryRotationOffset = FQuat::Identity;
		FWeaponPivotRotationSpec RotationSpec;
		uint32 ActiveHandle = 0;
		uint32 NextHandle = 1;
		uint32 ActionPoseScopeHandle = 0;
	};

	// Weapon Presentation Runtime State
	// Action Pose Scope
	FWeaponActionPoseScopeState ActionPoseScopeState;

	// Socket Transform Transition
	FWeaponSocketTransformTransitionState SocketTransformTransitionState;

	// Pivot Rotation
	FQuat CommittedPivotRotation = FQuat::Identity;
	FWeaponPivotRotationTransitionState PivotRotationTransitionState;
	FWeaponPivotRotationOverrideState PivotRotationOverrideState;

public:
	FWeaponTypeChanged OnWeaponTypeChanged;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Query
	FORCEINLINE bool CheckCurrentWeaponType(EWeaponType InNewWeaponType) const { return CurrentWeaponType == InNewWeaponType; }
	FORCEINLINE EWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }

	class ACWeaponActor* GetWeaponActor() const;

public:
	// Weapon Presentation - Action Pose Scope
	bool BeginWeaponActionPoseScope(uint32 InScopeHandle);
	bool EndWeaponActionPoseScope(uint32 InScopeHandle);
	bool IsWeaponActionPoseScopeActive(uint32 InScopeHandle) const;
	uint32 GetActiveWeaponActionPoseScopeHandle() const { return ActionPoseScopeState.ActiveHandle; }

public:
	// Weapon Presentation - Socket Transform Transition
	bool BeginWeaponSocketTransformTransition(EWeaponSocketSlot InTargetSlot, uint32& OutTransitionHandle);
	bool BeginWeaponSocketTransformTransition(FName InExpectedSourceSocketName, FName InTargetSocketName, uint32& OutTransitionHandle);
	bool UpdateWeaponSocketTransformTransition(uint32 InTransitionHandle, float InAlpha);
	bool CompleteWeaponSocketTransformTransition(uint32 InTransitionHandle);
	void CancelWeaponSocketTransformTransition(uint32 InTransitionHandle);

public:
	// Weapon Presentation - Pivot Rotation Transition
	bool BeginWeaponPivotRotationTransition(const FWeaponPivotRotationSpec& InRotationSpec, uint32& OutTransitionHandle);
	bool UpdateWeaponPivotRotationTransition(uint32 InTransitionHandle, float InAlpha);
	bool CompleteWeaponPivotRotationTransition(uint32 InTransitionHandle);
	void CancelWeaponPivotRotationTransition(uint32 InTransitionHandle);

public:
	// Weapon Presentation - Pivot Rotation Override
	bool BeginWeaponPivotRotationOverride(const FWeaponPivotRotationSpec& InRotationSpec, uint32& OutOverrideHandle);
	bool UpdateWeaponPivotRotationOverride(uint32 InOverrideHandle, float InAlpha);
	void EndWeaponPivotRotationOverride(uint32 InOverrideHandle);

public:
	// Equip / Unequip Commit
	void CommitEquipWeapon();
	void CommitUnequipWeapon();

public:
	// Combat Context
	void PushActionDataKey(const FActionDataKey& InActionDataKey);
	void ClearWeaponCombatContext();

public:
	// Collision
	void OpenCollisionWindow(FName InCollisionName);
	void CloseCollisionWindow();

public:
	// Feedback - Dissolve
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presentation|Dissolve")
	void StartWeaponDissolve();

	UFUNCTION(BlueprintCallable, Category = "Weapon|Presentation|Dissolve")
	void SetWeaponDissolveAmount(float InAmount);

	UFUNCTION(BlueprintCallable, Category = "Weapon|Presentation|Dissolve")
	void FinishWeaponDissolve();

public:
	// Runtime Cleanup
	void ClearWeaponRuntimeState();

private:
	// Weapon Runtime Lifecycle
	void UninitializeWeaponRuntime();

	// Weapon Actor Lifecycle
	FCharacterComponentReferences BuildWeaponActorReferences() const;
	bool CreateWeaponActor(AActor* InOwnerCharacter, EWeaponType InWeaponType, TSubclassOf<ACWeaponActor> InWeaponActorClass);
	void DestroyWeaponActor();

	// Weapon State Helpers
	void ChangeWeaponType(EWeaponType InNewWeaponType);

	// Weapon Presentation - Action Pose Scope Helpers
	void RebaseWeaponActionPoseScopeSocket();
	void ResetWeaponActionPoseScope();

	// Weapon Presentation - Socket Transform Transition Helpers
	bool BeginWeaponSocketTransformTransitionInternal(uint32 InActionPoseScopeHandle, FName InExpectedSourceSocketName, FName InTargetSocketName, uint32& OutTransitionHandle);
	bool ClearWeaponSocketTransformTransition(bool bRestoreSourceSocket);

	// Weapon Presentation - Pivot Helpers
	bool ApplyWeaponPivotRotation();
	void ClearWeaponPivotRotationTransitionState();
	void ClearWeaponPivotRotationOverrideState();
	void ClearWeaponPivotRuntimeEffects();
	void ResetWeaponPivotRotationState();

	// Combat Context Helpers
	FWeaponContext BuildWeaponContext() const;

	// Profiling
	bool ShouldSkipWeaponActorCreationForProfiling() const;
	void SkipWeaponActorCreationForProfiling();
	bool PreserveEquipWeaponTypeWithoutActorForProfiling();
};
