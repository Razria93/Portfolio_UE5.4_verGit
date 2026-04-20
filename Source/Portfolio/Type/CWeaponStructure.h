#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "DamageEventId.h"
#include "Type/CHealthStructure.h"
#include "CWeaponStructure.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
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
	Equip,
	Unequip,
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
};

UENUM(BlueprintType)
enum class EApplyDamageRejectReason : uint8
{
	None = 0,

	InvalidRequest,

	InvalidAttacker,
	InvalidDamageCauser,
	InvalidTarget,
	InvalidInstigator,

	SpecNotFound,
	ComputeFailed,
	CommitFailed,

	// Reject Reason of 'CanApplyDamage'
	InvalidOwner,
	SelfTarget,
	DuplicateHitInWindow,
	FriendlyTarget,
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

UENUM(BlueprintType)
enum class EActionFeedbackTiming : uint8
{
	None,
	ActionStart,
	ActionEnd,
	TriggerOnce,
	TriggerWindowBegin,
	TriggerWindowEnd
};

enum class EActionFeedbackMatchTier : uint8
{
	None = 0,
	AnyActionAnyIndex,
	ExactActionAnyIndex,
	ExactActionExactIndex,
};

UENUM(BlueprintType)
enum class EActionVFXPlayType : uint8
{
	Once,
	Loop
};

UENUM(BlueprintType)
enum class EActionSFXPlayType : uint8
{
	Once,
	Loop
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
struct FWeaponContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EWeaponType CurrentWeaponType = EWeaponType::Max;

public:
	FWeaponContext() = default;
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
	class AActor* DamageCauser = nullptr;						// WeaponActor

	UPROPERTY(Transient)
	class UPrimitiveComponent* OverlappedComponent = nullptr;	// Hit Collision of WeaponActor

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

	UPROPERTY(Transient)
	int32 HitWindowId = INDEX_NONE;

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
	FWeaponContext WeaponContext = FWeaponContext();

	UPROPERTY(Transient)
	FEquipmentContext  EquipmentContext = FEquipmentContext();

	UPROPERTY(Transient)
	FActionContext ActionContext = FActionContext();

public:
	FHitContext() = default;
};

USTRUCT()
struct FApplyDamageHitWindowKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;

	UPROPERTY(Transient)
	int32 HitWindowId = INDEX_NONE;

	bool operator==(const FApplyDamageHitWindowKey& InOther) const
	{
		return DamageCauser == InOther.DamageCauser
			&& HitWindowId == InOther.HitWindowId;
	}
};

FORCEINLINE uint32 GetTypeHash(const FApplyDamageHitWindowKey& InOther)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(InOther.DamageCauser));
	H = HashCombine(H, GetTypeHash(InOther.HitWindowId));

	return H;
}

USTRUCT(BlueprintType)
struct FApplyDamageSpecKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType = EWeaponType::Max;

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
			&& WeaponType == InOther.WeaponType
			&& ActionType == InOther.ActionType
			&& ActionIndex == InOther.ActionIndex;
	}
};

FORCEINLINE uint32 GetTypeHash(const FApplyDamageSpecKey& InOther)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InOther.WeaponType)));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InOther.EquipmentType)));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InOther.ActionType)));
	H = HashCombine(H, GetTypeHash(InOther.ActionIndex));

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
struct FApplyDamageAmount
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	float RequestDamage = 0.f;

public:
	FApplyDamageAmount() = default;
};

USTRUCT(BlueprintType)
struct FApplyDamagePayload
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FHitContext HitContext = FHitContext();

	UPROPERTY(Transient)
	AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	AActor* DamageCauser = nullptr;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	FApplyDamageHitWindowKey HitWindowKey = FApplyDamageHitWindowKey();

	UPROPERTY(Transient)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

public:
	FApplyDamagePayload() = default;
};

USTRUCT(BlueprintType)
struct FApplyDamageContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bAccepted = true;

	UPROPERTY(Transient)
	EApplyDamageRejectReason RejectReason = EApplyDamageRejectReason::None;

	UPROPERTY(Transient)
	FHitContext HitContext = FHitContext();

	UPROPERTY(Transient)
	AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	AController* Instigator = nullptr;

	UPROPERTY(Transient)
	AActor* DamageCauser = nullptr;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	FApplyDamageHitWindowKey HitWindowKey = FApplyDamageHitWindowKey();

	UPROPERTY(Transient)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	UPROPERTY(Transient)
	FApplyDamageSpec ApplyDamageSpec = FApplyDamageSpec();

	UPROPERTY(Transient)
	FApplyDamageAmount ApplyDamageAmount = FApplyDamageAmount();

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;

public:
	FApplyDamageContext() = default;
};

USTRUCT(BlueprintType)
struct FApplyDamageResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bAccepted = true;

	UPROPERTY(Transient)
	EApplyDamageRejectReason RejectReason = EApplyDamageRejectReason::None;

	UPROPERTY(Transient)
	FApplyDamageHitWindowKey HitWindowKey = FApplyDamageHitWindowKey();

	UPROPERTY(Transient)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	UPROPERTY(Transient)
	float BaseDamage = 0.f;

	UPROPERTY(Transient)
	float RequestDamage = 0.f;

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;

public:
	FApplyDamageResult() = default;
};

USTRUCT(BlueprintType)
struct FDefaultDamageEvent : public FDamageEvent
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	UPROPERTY(Transient)
	FApplyDamageSpec ApplyDamageSpec = FApplyDamageSpec();

	UPROPERTY(Transient)
	FApplyDamageAmount ApplyDamageAmount = FApplyDamageAmount();

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
	UPROPERTY(Transient)
	class AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	class AController* EventInstigator = nullptr;

	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;

	// Damage MetaData
	UPROPERTY(Transient)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	UPROPERTY(Transient)
	FApplyDamageSpec ApplyDamageSpec = FApplyDamageSpec();

	UPROPERTY(Transient)
	FApplyDamageAmount ApplyDamageAmount = FApplyDamageAmount();

	//Damage AmountData
	UPROPERTY(Transient)
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
	UPROPERTY(Transient)
	class AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	class AController* Instigator = nullptr;

	UPROPERTY(Transient)
	class AActor* DamageCauser = nullptr;

	// Damage MetaData [Set BuildContext]
	UPROPERTY(Transient)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	// Query Acceptable [Set ValidateContext / CanTakeDamage / ComputeTakeDamage]
	UPROPERTY(Transient)
	bool bAccepted = true;

	UPROPERTY(Transient)
	ETakeDamageRejectReason RejectReason = ETakeDamageRejectReason::None;

	// Pre-state Snapshot [Set HandleDefaultDamageEvent before ValidatePolicy]
	UPROPERTY(Transient)
	float HealthPointBefore = 0.f;

	UPROPERTY(Transient)
	EDeadState DeadState_Before = EDeadState::Alive;

	// DamageAmounts [Set ComputeTakeDamage & CommitTakeDamage]
	UPROPERTY(Transient)
	float RequestedDamage = 0.f;		// Raw incoming damage requested by Apply pipeline. (ex. [skill] 100)

	UPROPERTY(Transient)
	float MitigatedDamage = 0.f;		// Post-mitigation damage after target defenses. (ex. [guard/resistance] 100 -> 70)

	UPROPERTY(Transient)
	float FinalTakenDamage = 0.f;		// Final damage decided by Take evaluation rules. (ex. [clamp max/min damage-limit] 70 -> 60)

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;		// Actual HP loss committed to Health. (ex. [shield absorbs] 60 -> HP: -30 / SP: -30)

	// Post-state Snapshot [Set BuildResult]
	UPROPERTY(Transient)
	float HealthPointAfter = 0.f;

	UPROPERTY(Transient)
	EDeadState DeadState_After = EDeadState::Alive;

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

public:
	UPROPERTY(Transient)
	bool bAccepted = true;

	UPROPERTY(Transient)
	ETakeDamageRejectReason RejectReason = ETakeDamageRejectReason::None;

	// Damage MetaData
	UPROPERTY(Transient)
	FApplyDamageSpecKey ApplyDamageSpecKey = FApplyDamageSpecKey();

	// Damage Amount
	UPROPERTY(Transient)
	float RequestDamage = 0.f;

	UPROPERTY(Transient)
	float MitigatedDamage = 0.f;

	UPROPERTY(Transient)
	float FinalTakenDamage = 0.f;

	UPROPERTY(Transient)
	float CommittedDamage = 0.f;

	UPROPERTY(Transient)
	EDeadState DeadState_Before = EDeadState::Alive;

	UPROPERTY(Transient)
	EDeadState DeadState_After = EDeadState::Alive;

public:
	FTakeDamageResult() = default;
};

USTRUCT(BlueprintType)
struct FTakeDamagePacket
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FTakeDamagePayload Payload;

	UPROPERTY(Transient)
	FTakeDamageContext Context;

	UPROPERTY(Transient)
	FTakeDamageResult Result;
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
		return ReactionType == InOther.ReactionType
			&& ApplyDamageSpecKey == InOther.ApplyDamageSpecKey;
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
	TSubclassOf<class UCReaction> ReactionExecutorKey = nullptr;

	UPROPERTY(EditAnywhere, Category = "Reaction")
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Reaction")
	float PlayRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Reaction")
	bool bCanMove = false;

	UPROPERTY(EditAnywhere, Category = "Reaction")
	int32 Priority = INDEX_NONE;

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

USTRUCT(BlueprintType)
struct FActionFeedbackKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FActionFeedbackKey() = default;

public:
	bool operator==(const FActionFeedbackKey& InOther) const
	{
		return ActionType == InOther.ActionType
			&& ActionIndex == InOther.ActionIndex;
	}
};

USTRUCT(BlueprintType)
struct FActionFeedbackRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

public:
	FActionFeedbackRequest() = default;
};

USTRUCT(BlueprintType)
struct FTrailFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere)
	bool bTrailActive = false;

public:
	FTrailFeedbackData() = default;
};

USTRUCT(BlueprintType)
struct FActionVFXFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere)
	EActionVFXPlayType VFXPlayType = EActionVFXPlayType::Once;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* VFX = nullptr;

	UPROPERTY(EditAnywhere)
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere)
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere)
	FVector RelativeScale = FVector(1.f, 1.f, 1.f);

public:
	FActionVFXFeedbackData() = default;
};

USTRUCT(BlueprintType)
struct FActionSFXFeedbackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(EditAnywhere)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(EditAnywhere)
	FName TriggerKey = NAME_None;

	UPROPERTY(EditAnywhere)
	EActionSFXPlayType SFXPlayType = EActionSFXPlayType::Once;

	UPROPERTY(EditAnywhere)
	class USoundBase* SFX = nullptr;

public:
	FActionSFXFeedbackData() = default;
};

USTRUCT()
struct FActionVFXExecutionKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(Transient)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(Transient)
	FName TriggerKey = NAME_None;

	UPROPERTY(Transient)
	EActionVFXPlayType VFXPlayType = EActionVFXPlayType::Once;

	UPROPERTY(Transient)
	TObjectPtr<class UNiagaraSystem> VFX = nullptr;

	UPROPERTY(Transient)
	FName SocketName = NAME_None;

	UPROPERTY(Transient)
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	FVector RelativeScale = FVector(1.f, 1.f, 1.f);

public:
	FActionVFXExecutionKey() = default;

public:
	bool operator==(const FActionVFXExecutionKey& InOther) const
	{
		return ActionFeedbackKey == InOther.ActionFeedbackKey
			&& ActionFeedbackTiming == InOther.ActionFeedbackTiming
			&& TriggerKey == InOther.TriggerKey
			&& VFXPlayType == InOther.VFXPlayType
			&& VFX == InOther.VFX
			&& SocketName == InOther.SocketName
			&& RelativeLocation == InOther.RelativeLocation
			&& RelativeRotation == InOther.RelativeRotation
			&& RelativeScale == InOther.RelativeScale;
	}
};

FORCEINLINE uint32 GetTypeHash(const FActionVFXExecutionKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionFeedbackKey.ActionType)));
	H = HashCombine(H, GetTypeHash(InKey.ActionFeedbackKey.ActionIndex));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionFeedbackTiming)));
	H = HashCombine(H, GetTypeHash(InKey.TriggerKey));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.VFXPlayType)));
	H = HashCombine(H, GetTypeHash(InKey.VFX));
	H = HashCombine(H, GetTypeHash(InKey.SocketName));

	H = HashCombine(H, GetTypeHash(InKey.RelativeLocation.X));
	H = HashCombine(H, GetTypeHash(InKey.RelativeLocation.Y));
	H = HashCombine(H, GetTypeHash(InKey.RelativeLocation.Z));

	H = HashCombine(H, GetTypeHash(InKey.RelativeRotation.Pitch));
	H = HashCombine(H, GetTypeHash(InKey.RelativeRotation.Yaw));
	H = HashCombine(H, GetTypeHash(InKey.RelativeRotation.Roll));

	H = HashCombine(H, GetTypeHash(InKey.RelativeScale.X));
	H = HashCombine(H, GetTypeHash(InKey.RelativeScale.Y));
	H = HashCombine(H, GetTypeHash(InKey.RelativeScale.Z));

	return H;
}

USTRUCT()
struct FActionSFXExecutionKey
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FActionFeedbackKey ActionFeedbackKey = FActionFeedbackKey();

	UPROPERTY(Transient)
	EActionFeedbackTiming ActionFeedbackTiming = EActionFeedbackTiming::None;

	UPROPERTY(Transient)
	FName TriggerKey = NAME_None;

	UPROPERTY(Transient)
	EActionSFXPlayType SFXPlayType = EActionSFXPlayType::Once;

	UPROPERTY(Transient)
	TObjectPtr<class USoundBase> SFX = nullptr;

public:
	FActionSFXExecutionKey() = default;

public:
	bool operator==(const FActionSFXExecutionKey& InOther) const
	{
		return ActionFeedbackKey == InOther.ActionFeedbackKey
			&& ActionFeedbackTiming == InOther.ActionFeedbackTiming
			&& TriggerKey == InOther.TriggerKey
			&& SFXPlayType == InOther.SFXPlayType
			&& SFX == InOther.SFX;
	}
};

FORCEINLINE uint32 GetTypeHash(const FActionSFXExecutionKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionFeedbackKey.ActionType)));
	H = HashCombine(H, GetTypeHash(InKey.ActionFeedbackKey.ActionIndex));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionFeedbackTiming)));
	H = HashCombine(H, GetTypeHash(InKey.TriggerKey));
	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.SFXPlayType)));
	H = HashCombine(H, GetTypeHash(InKey.SFX));

	return H;
}