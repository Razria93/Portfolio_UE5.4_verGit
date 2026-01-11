#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DamageEvents.h"
#include "DamageEventId.h"
#include "CWeaponStructure.generated.h"

UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
	Unarmed = 0,
	Sword,
	Max,
};

UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	None = 0,
	Default,
	Max,
};

UENUM(BlueprintType)
enum class EActionType : uint8
{
	Idle = 0,
	LightAttack,
	ComboAttack,
	Max,
};

UENUM(BlueprintType)
enum class ETakeDamageRejectReason : uint8
{
	None,

	InvalidTarget,
	InvalidCauser,
	InvalidInstigator,

	AlreadyDead,
	// Invulnerable,
	// FriendlyFire,

	// Blocked,
	// Parried,

	// DamageCooldown,
	ZeroDamage,
};

USTRUCT(BlueprintType)
struct FEquipmentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere)
	bool bCanMove = false;

public:
	FEquipmentData() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FActionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere)
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere)
	bool bCanMove = false;

public:
	FActionData() = default;

public:
	bool IsValidMinimal() const;

public:
	void BeginPlayMontage(class ACharacter* InOwnerCharacter);
	void EndPlayMontage(class ACharacter* InOwnerCharacter);
};

USTRUCT(BlueprintType)
struct FAttachmentContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EAttachmentType CurrentAttachmentType = EAttachmentType::Max;

public:
	FAttachmentContext() = default;
};

USTRUCT(BlueprintType)
struct FEquipmentContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EEquipmentType CurrentEquipmentType = EEquipmentType::Max;

public:
	FEquipmentContext() = default;
};

USTRUCT(BlueprintType)
struct FActionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EActionType CurrentActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FActionContext() = default;
};

USTRUCT(BlueprintType)
struct FOverlapContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class AActor* OwnerActor = nullptr;							// AttackActor

	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;						// Attachment

	UPROPERTY(Transient)
	class UPrimitiveComponent* OverlappedComponent = nullptr;	// Hit Collision of Attachment

	UPROPERTY(Transient)
	class UShapeComponent* OverlapShape = nullptr;			// Cast result: UShapeComponent (nullptr if cast fails)

	UPROPERTY(Transient)
	class AActor* OtherActor = nullptr;							// DamagedActor

	UPROPERTY(Transient)
	class UPrimitiveComponent* OtherComponent = nullptr;		// DamagedComponent

	UPROPERTY(Transient)
	int32 OtherBodyIndex = INDEX_NONE;

	UPROPERTY(Transient)
	bool bFromSweep = false;

	UPROPERTY(Transient)
	FHitResult SweepResult;

public:
	FOverlapContext() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FHitContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FOverlapContext OverlapContext = FOverlapContext();

	UPROPERTY(Transient)
	FAttachmentContext AttachmentContext = FAttachmentContext();

	UPROPERTY(Transient)
	FEquipmentContext  EquipmentContext = FEquipmentContext();

	UPROPERTY(Transient)
	FActionContext ActionContext = FActionContext();

public:
	FHitContext() = default;
};

USTRUCT(BlueprintType)
struct FApplyDamageSpecKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EAttachmentType AttachmentType = EAttachmentType::Max;

	UPROPERTY(EditAnywhere)
	EEquipmentType EquipmentType = EEquipmentType::Max;

	UPROPERTY(EditAnywhere)
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FApplyDamageSpecKey() = default;

public:
	bool operator==(const FApplyDamageSpecKey& Other) const
	{
		return EquipmentType == Other.EquipmentType
			&& AttachmentType == Other.AttachmentType
			&& ActionType == Other.ActionType
			&& ActionIndex == Other.ActionIndex;
	}

public:
	friend FORCEINLINE uint32 GetTypeHash(const FApplyDamageSpecKey& Key)
	{
		uint32 H = 0;
		H = HashCombine(H, ::GetTypeHash(static_cast<uint8>(Key.AttachmentType)));
		H = HashCombine(H, ::GetTypeHash(static_cast<uint8>(Key.EquipmentType)));
		H = HashCombine(H, ::GetTypeHash(static_cast<uint8>(Key.ActionType)));
		H = HashCombine(H, ::GetTypeHash(Key.ActionIndex));
		return H;
	}
};

USTRUCT(BlueprintType)
struct FApplyDamageSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float BaseDamage = 0.f;

public:
	FApplyDamageSpec() = default;
};

USTRUCT(BlueprintType)
struct FApplyDamageResult
{
	GENERATED_BODY()

public:
	UPROPERTY()
	float RequestDamage = 0.f;

	// TODO:
	// FVector ImpactPoint;
	// FVector HitNormal;

public:
	FApplyDamageResult() = default;
};

USTRUCT(BlueprintType)
struct FDefaultDamageEvent : public FDamageEvent
{
	GENERATED_BODY()

public:
	UPROPERTY() 
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	UPROPERTY()
	FApplyDamageSpec ApplyDamageSpec = FApplyDamageSpec();

	UPROPERTY()
	FApplyDamageResult ApplyDamageResult = FApplyDamageResult();

public:
	static const int32 ClassID = (int32)EDamageEventTypeId::DefaultDamage;

public:
	FDefaultDamageEvent() = default;

public:
	virtual int32 GetTypeID() const override { return ClassID; }
	virtual bool IsOfType(int32 InID) const override { return InID == ClassID || FDamageEvent::IsOfType(InID); }
};

USTRUCT(BlueprintType)
struct FTakeDamagePayload
{
	GENERATED_BODY()

public:
	// ObjectData
	UPROPERTY(VisibleAnywhere)
	class AActor* DamagedActor = nullptr;

	UPROPERTY(VisibleAnywhere)
	class AController* EventInstigator = nullptr;

	UPROPERTY(VisibleAnywhere)
	class AActor* DamageCauser = nullptr;

	// Damage MetaData
	UPROPERTY(VisibleAnywhere)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	UPROPERTY(VisibleAnywhere)
	FApplyDamageSpec ApplyDamageSpec = FApplyDamageSpec();

	UPROPERTY(VisibleAnywhere)
	FApplyDamageResult ApplyDamageResult = FApplyDamageResult();

	//Damage AmountData
	UPROPERTY(VisibleAnywhere)
	float RequestedDamage = 0.f;

public:
	FTakeDamagePayload() = default;
};

USTRUCT(BlueprintType)
struct FTakeDamageContext
{
	GENERATED_BODY()

public:
	// Resolved objects
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AActor> DamagedActor = nullptr;

	UPROPERTY(VisibleAnywhere)
	class AController* Instigator = nullptr;

	UPROPERTY(VisibleAnywhere)
	class AActor* DamageCauser = nullptr;

	// Pre-state snapshot
	UPROPERTY(VisibleAnywhere)
	bool bWasDead = false;

	UPROPERTY(VisibleAnywhere)
	float HealthPoint_Before = 0.f;

	UPROPERTY(VisibleAnywhere)
	float HealthPoint_After = 0.f;

	// DamageAmounts (derived)
	UPROPERTY(VisibleAnywhere)
	float RequestedDamage = 0.f;		// Raw incoming damage requested by Apply pipeline. (ex. [skill] 100)

	UPROPERTY(VisibleAnywhere)
	float MitigatedDamage = 0.f;		// Post-mitigation damage after target defenses. (ex. [guard/resistance] 100 -> 70)

	UPROPERTY(VisibleAnywhere)
	float FinalTakenDamage = 0.f;		// Final damage decided by Take evaluation rules. (ex. [clamp max/min damage-limit] 70 -> 60)

	UPROPERTY(VisibleAnywhere)
	float FinalAppliedDamage = 0.f;		// Actual HP loss committed to Health. (ex. [shield absorbs] 60 -> HP: -30 / SP: -30)

	// TODO:
	// - HitBoneName
	// - HitDirection
	// - HitImpulseVector
	// - TeamId / Attribute
	// - StateSnapshot
	// - Cached Component (Minimal)

public:
	FTakeDamageContext() = default;
};

USTRUCT()
struct FTakeDamageResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	bool bAccepted = true;

	UPROPERTY(VisibleAnywhere)
	ETakeDamageRejectReason RejectReason = ETakeDamageRejectReason::None;

	UPROPERTY(VisibleAnywhere)
	bool bKilled = false;

	// Damage Amount
	UPROPERTY(VisibleAnywhere)
	float RequestDamage = 0.f;

	UPROPERTY(VisibleAnywhere)
	float MitigatedDamage = 0.f;

	UPROPERTY(VisibleAnywhere)
	float FinalTakenDamage = 0.f;

	UPROPERTY(VisibleAnywhere)
	float FinalAppliedDamage = 0.f;

	// Dispatch flags
	UPROPERTY()
	bool bTriggerHitReaction = true;

	UPROPERTY()
	bool bTriggerDeathReaction = true;

public:
	FTakeDamageResult() = default;
};

UCLASS()
class PORTFOLIO_API UCWeaponStructure : public UObject
{
	GENERATED_BODY()

};
