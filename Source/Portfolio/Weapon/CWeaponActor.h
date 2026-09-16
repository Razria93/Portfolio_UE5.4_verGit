#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/HitContextProvider.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CWeaponTypes.h"
#include "Type/CCombatHitTypes.h"
#include "CWeaponActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponActorCollisionEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponActorCollisionDisabled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_EightParams(FWeaponActorBeginOverlap, AActor*, InAttackerActor, AActor*, InDamageCauser, UShapeComponent*, InAttackCollision, AActor*, InTargetActor, UPrimitiveComponent*, InHitComponent, int32, InOtherBodyIndex, bool, InbFromSweep, const FHitResult&, InSweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponActorEndOverlap, AActor*, InAttackerActor, AActor*, InTargetActor);

struct FActionFeedbackRequest;

UCLASS()
class PORTFOLIO_API ACWeaponActor : public AActor, public IHitContextProvider
{
	GENERATED_BODY()

public:
	ACWeaponActor();

public:
	UPROPERTY(EditAnywhere, Category = "Weapon|SocketName")
	FName SocketName_Holster;

	UPROPERTY(EditAnywhere, Category = "Weapon|SocketName")
	FName SocketName_Hand;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Pivot")
	FName PivotSocketName = TEXT("HandGrip");

private:
	UPROPERTY(Transient)
	EWeaponType WeaponType;

private:
	UPROPERTY(Transient)
	bool bHitWindowOpened = false;

	UPROPERTY(Transient)
	int32 CurrentHitWindowId = INDEX_NONE;

private:
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* ActorRootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Pivot", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* PivotComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Pivot", meta = (AllowPrivateAccess = "true"))
	class USceneComponent* ContentRootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Mesh", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* WeaponMeshComponent = nullptr;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Feedback|Trail", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWeaponTrailDataAsset> WeaponTrailDataAsset = nullptr;

private:
	TMap<FName, TArray<TObjectPtr<class UWeaponTrailComponent>>> TrailComponents_Cached;

private:
	UPROPERTY(Transient)
	FTransform PivotRestTransform = FTransform::Identity;

	UPROPERTY(Transient)
	bool bHasValidPivot = false;

private:
	UPROPERTY(Transient)
	FOverlapContext LastOverlapContext_Cached;

	UPROPERTY(Transient)
	FWeaponContext LastWeaponContext_Cached;

	UPROPERTY(Transient)
	FActionDataKey LastActionDataKey_Cached;

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCCombatSignalSourceComponent* CombatSignalSourceComp_Injected = nullptr;

private:
	UPROPERTY(Transient)
	TArray<class UShapeComponent*> Collisions_Cached;

public:
	FWeaponActorCollisionEnabled OnWeaponActorCollisionEnabled;
	FWeaponActorCollisionDisabled OnWeaponActorCollisionDisabled;

	FWeaponActorBeginOverlap OnWeaponActorBeginOverlap;
	FWeaponActorEndOverlap OnWeaponActorEndOverlap;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredReferences() const;

public:
	// Initial State
	void ApplyInitialWeaponState(EWeaponType InWeaponType);

protected:
	// Lifecycle
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Query
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE bool IsHitWindowOpened() const { return bHitWindowOpened; }
	FORCEINLINE int32 GetCurrentHitWindowId() const { return CurrentHitWindowId; }

public:
	// Weapon State
	void ChangeWeaponType(EWeaponType InWeaponType);

public:
	// Weapon Attachment
	void AttachToHandSocket();
	void AttachToHolsterSocket();

	bool AttachToSocketSlot(EWeaponSocketSlot InSlot);
	bool AttachToOwnerSocket(FName InSocketName);

	bool ResolveSocketName(EWeaponSocketSlot InSlot, FName& OutSocketName) const;
	bool GetCurrentOwnerSocketName(FName& OutSocketName) const;

	bool GetOwnerSocketWorldTransform(FName InSocketName, FTransform& OutWorldTransform) const;
	bool SetActorRootWorldTransform(const FTransform& InWorldTransform);

public:
	// Weapon Pivot
	bool ApplyPivotRotation(const FQuat& InRotation);
	void ResetPivotRotation();
	bool HasValidPivot() const { return bHasValidPivot; }

public:
	// Collision Notify Events
	void CollisionEnabled(FName InName);
	void CollisionDisabled();

public:
	// Collision Overlap Delegate Events
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	// Hit Context - Provider Query
	virtual const FOverlapContext& GetLastOverlapContext() const override;
	virtual const FWeaponContext& GetLastWeaponContext() const override;
	virtual const FActionDataKey& GetLastActionDataKey() const override;

public:
	// Hit Context - Provider Mutation
	virtual void SetLastOverlapContext(const FOverlapContext& InOverlapContext) override;
	virtual void SetLastWeaponContext(const FWeaponContext& InWeaponContext) override;
	virtual void SetLastActionDataKey(const FActionDataKey& InActionDataKey) override;

public:
	// Feedback - Trail
	bool HandleTrailFeedback(const FActionFeedbackRequest& InActionFeedbackRequest);
	void DeactivateAllTrails();

public:
	// Feedback - Dissolve
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Presentation|Dissolve")
	void ReceiveWeaponDissolveStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Presentation|Dissolve")
	void ReceiveWeaponDissolveAmount(float InAmount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Presentation|Dissolve")
	void ReceiveWeaponDissolveFinished();

private:
	// Weapon Pivot Initialization
	bool InitializePivotHierarchy();
	void ResetPivotHierarchy();

	// Collision Lifecycle
	void InitializeCollisionComponents();
	void ClearCollisionComponents();

	// Hit Context Helpers
	FOverlapContext BuildOverlapContext(AActor* InOwnerActor, AActor* InDamageCauser, UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) const;
	FHitImpactContext BuildHitImpactContext(const FOverlapContext& InOverlapContext) const;
	FHitContext BuildHitContext(const FOverlapContext& InOverlapContext) const;

	// Feedback - Trail Lifecycle
	void InitializeTrailState();
	void ClearTrailState();

	// Feedback - Trail Runtime Helpers
	void CreateTrailComponents();
	void DestroyTrailComponents();
	const TArray<TObjectPtr<class UWeaponTrailComponent>>* FindTrailComponents(FName InTriggerKey) const;
	static void SetTrailComponentActive(class UWeaponTrailComponent* InTrailComponent, bool bEnable);
};
