#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "DamageEventId.h"
#include "CWeaponStructure.generated.h"

UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
	Unarmed = 0,
	Sword,
	All,
	Max,
};

UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	None = 0,
	Default,
	All,
	Max,
};

UENUM(BlueprintType)
enum class EActionType : uint8
{
	Idle = 0,
	LightAttack,
	ComboAttack,
	All,
	Max,
};

UENUM(BlueprintType)
enum class EReactionType : uint8
{
	None = 0,
	Hit,
	Dead,
};

UENUM(BlueprintType)
enum class ETakeDamageRejectReason : uint8
{
	None = 0,

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

UENUM(BlueprintType)
enum class EReactionStopReason : uint8
{
	None = 0,
	Interrupted,
	Cancelled,
};

UENUM(BlueprintType)
enum class EReactionWindowType : uint8
{
	None = 0,

	// [System-Driven] 
	// Current reaction replaced by a new, stronger Reaction (ex. Hit Stun)
	Interruptible,

	// [Player-Driven] 
	// Current reaction canceled by a conscious Player Action (ex. Parry/Dodge)
	Cancelable,

	// [Ignore All] 
	// Solid state. Current reaction ignores any incoming Reactions.
	ImmuneToReaction,
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
	bool operator==(const FApplyDamageSpecKey& InOther) const
	{
		return EquipmentType == InOther.EquipmentType
			&& AttachmentType == InOther.AttachmentType
			&& ActionType == InOther.ActionType
			&& ActionIndex == InOther.ActionIndex;
	}
};

FORCEINLINE uint32 GetTypeHash(const FApplyDamageSpecKey& InKey)
{
	uint32 H = 0;
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.AttachmentType)));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.EquipmentType)));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionType)));
	H = HashCombine(H, GetTypeHash(InKey.ActionIndex));
	return H;
}

/***
 * [EN]
 * USTRUCT Set/Map key checklist:
 * 1) operator==
 * 2) GetTypeHash
 *
 * GetTypeHash notes:
 * - Prefer a normal overload at namespace/global scope (avoid hidden-friend in the struct).
 * - Avoid ::GetTypeHash(...); call GetTypeHash(...) to keep ADL available.
 ***/

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
	// Resolved objects [Set BuildContext]
	UPROPERTY(VisibleAnywhere)
	class AActor* DamagedActor = nullptr;

	UPROPERTY(VisibleAnywhere)
	class AController* Instigator = nullptr;

	UPROPERTY(VisibleAnywhere)
	class AActor* DamageCauser = nullptr;

	// Damage MetaData [Set BuildContext]
	UPROPERTY(VisibleAnywhere)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	// Query Acceptable [Set EvaluateTakeDamage]
	UPROPERTY(VisibleAnywhere)
	bool bAccepted = true;

	UPROPERTY(VisibleAnywhere)
	ETakeDamageRejectReason RejectReason = ETakeDamageRejectReason::None;

	// Pre-state Snapshot [Set EvaluateTakeDamage]
	UPROPERTY(VisibleAnywhere)
	float HealthPointBefore = 0.f;

	UPROPERTY(VisibleAnywhere)
	bool bWasDeadBefore = false;

	// DamageAmounts [Set EvaluateTakeDamage & CommitTakeDamage]
	UPROPERTY(VisibleAnywhere)
	float RequestedDamage = 0.f;		// Raw incoming damage requested by Apply pipeline. (ex. [skill] 100)

	UPROPERTY(VisibleAnywhere)
	float MitigatedDamage = 0.f;		// Post-mitigation damage after target defenses. (ex. [guard/resistance] 100 -> 70)

	UPROPERTY(VisibleAnywhere)
	float FinalTakenDamage = 0.f;		// Final damage decided by Take evaluation rules. (ex. [clamp max/min damage-limit] 70 -> 60)

	UPROPERTY(VisibleAnywhere)
	float FinalAppliedDamage = 0.f;		// Actual HP loss committed to Health. (ex. [shield absorbs] 60 -> HP: -30 / SP: -30)

	// Post-state Snapshot [Set BuildResult]
	UPROPERTY(VisibleAnywhere)
	float HealthPointAfter = 0.f;

	UPROPERTY(VisibleAnywhere)
	bool bIsDeadAfter = false;

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

USTRUCT(BlueprintType)
struct FTakeDamageResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	bool bAccepted = true;

	UPROPERTY(VisibleAnywhere)
	ETakeDamageRejectReason RejectReason = ETakeDamageRejectReason::None;

	// Damage MetaData
	UPROPERTY(VisibleAnywhere)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	// Damage Amount
	UPROPERTY(VisibleAnywhere)
	float RequestDamage = 0.f;

	UPROPERTY(VisibleAnywhere)
	float MitigatedDamage = 0.f;

	UPROPERTY(VisibleAnywhere)
	float FinalTakenDamage = 0.f;

	UPROPERTY(VisibleAnywhere)
	float FinalAppliedDamage = 0.f;

	UPROPERTY(VisibleAnywhere)
	bool bKilled = false;

	// Dispatch flags
	UPROPERTY(VisibleAnywhere)
	bool bTriggerHitReaction = true;

	UPROPERTY(VisibleAnywhere)
	bool bTriggerDeathReaction = true;

public:
	FTakeDamageResult() = default;
};

USTRUCT(BlueprintType)
struct FReactionDataKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	UPROPERTY(EditAnywhere)
	EReactionType ReactionType = EReactionType::None;

public:
	FReactionDataKey() = default;

public:
	bool operator==(const FReactionDataKey& InOther) const
	{
		return ReactionType == InOther.ReactionType && ApplyDamageSpecKey == InOther.ApplyDamageSpecKey;
	}
};

FORCEINLINE uint32 GetTypeHash(const FReactionDataKey& InKey)
{
	uint32 H = 0;
	H = HashCombine(H, GetTypeHash(InKey.ApplyDamageSpecKey));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ReactionType)));
	return H;
}

USTRUCT(BlueprintType)
struct FReactionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Key")
	FReactionDataKey ReactionDataKey = FReactionDataKey();

	UPROPERTY(EditAnywhere, Category = "Key")
	TSubclassOf<class UCReaction> ReactionExecutorKey;

	UPROPERTY(EditAnywhere, Category = "Reaction")
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Reaction")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Reaction")
	bool bCanMove = false;

	// UPROPERTY(EditAnywhere, Category = "Policy")
	// int32 priority = 0;

public:
	FReactionData() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FReactionQueryContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class UCReaction* CurrentReactionExecutor = nullptr;

	UPROPERTY(Transient)
	class UCReaction* IncomingReactionExecutor = nullptr;

	UPROPERTY(Transient)
	FReactionData CurrentReactionData = FReactionData();

	UPROPERTY(Transient)
	FReactionData IncomingReactionData = FReactionData();

public:
	FReactionQueryContext() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FReactionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FReactionData ReactionData = FReactionData();

	UPROPERTY(Transient)
	class UCReaction* ReactionExecutor = nullptr;

public:
	FReactionContext() = default;

public:
	bool IsValidMinimal() const;
};