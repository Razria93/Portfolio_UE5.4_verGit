#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "DamageEventId.h"
#include "Type/CStateStructure.h"
#include "Type/CHealthStructure.h"
#include "CWeaponStructure.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None = 0,	// Invalid, Unset

	Unarmed,
	Sword,

	All,		// Wildcard

	Max,		// Sentinel
};

UENUM(BlueprintType)
enum class EActionType : uint8
{
	None = 0,	// Invalid, Unset

	Idle,

	Equip,
	Unequip,

	ComboAttack,

	Guard,
	Dodge,

	All,		// Wildcard

	Max,		// Sentinel
};

UENUM(BlueprintType)
enum class EReactionType : uint8
{
	None = 0,	// Invalid, Unset

	Idle,

	Hit,
	Dead,

	All,		// Wildcard

	Max,		// Sentinel
};

UENUM(BlueprintType)
enum class EActionNotifyCommand : uint8
{
	None = 0,

	Complete,

	PushHitContext,
	ClearHitContext,

	OpenReserveChainWindow,
	CloseReserveChainWindow,
	ConsumeChain,

	Equip,
	Unequip,

	Max,
};

UENUM(BlueprintType)
enum class EReactionNotifyCommand : uint8
{
	None = 0,

	Complete,

	Max,
};

UENUM(BlueprintType)
enum class EActionEventType : uint8
{
	None = 0,

	ReserveChainWindowOpened,
	ReserveChainWindowClosed,

	ActionStarted,
	ActionCompleted,

	ActionChained,

	ActionInterrupted,
	ActionIgnored,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionDecision : uint8
{
	None = 0,

	Reject,
	Ignore,

	Accept,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionRelationship : uint8
{
	None = 0,

	Independent,
	Sequential,
	Exclusive,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionApplyMode : uint8
{
	None = 0,

	Start,
	Reserve,
	Intervene,

	Max,
};

UENUM(BlueprintType)
enum class EObservableOverlayHandling : uint8
{
	None = 0,

	ClearGuardState,
	ClearGuardOverlay,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionDomain : uint8
{
	None = 0,

	Action,
	Reaction,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionStopReason : uint8
{
	None = 0,

	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionInterventionTiming : uint8
{
	None = 0,

	Always,
	Window,

	Max,
};

struct FExecutionParticipant;

USTRUCT(BlueprintType)
struct FExecutionInterventionParticipantFilter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Filter")
	EExecutionDomain Domain = EExecutionDomain::None;

	UPROPERTY(EditAnywhere, Category = "Filter")
	EActionType ActionType = EActionType::None;

	UPROPERTY(EditAnywhere, Category = "Filter")
	EReactionType ReactionType = EReactionType::None;

	// INDEX_NONE means any index. Reaction currently ignores Index.
	UPROPERTY(EditAnywhere, Category = "Filter")
	int32 Index = INDEX_NONE;

public:
	bool IsValidMinimal() const;

	bool MatchesAction(EActionType InActionType, int32 InIndex = INDEX_NONE) const;
	bool MatchesReaction(EReactionType InReactionType) const;
	bool MatchesParticipant(const FExecutionParticipant& InParticipant) const;

public:
	bool operator==(const FExecutionInterventionParticipantFilter& InOther) const
	{
		return Domain == InOther.Domain
			&& ActionType == InOther.ActionType
			&& ReactionType == InOther.ReactionType
			&& Index == InOther.Index;
	}
};

USTRUCT(BlueprintType)
struct FExecutionInterventionWantRule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Intervention")
	TArray<FExecutionInterventionParticipantFilter> ParticipantFilters;

public:
	bool IsValidMinimal() const
	{
		return !ParticipantFilters.IsEmpty();
	}
};

USTRUCT(BlueprintType)
struct FExecutionInterventionAllowRule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Intervention")
	EExecutionInterventionTiming Timing = EExecutionInterventionTiming::Always;

	UPROPERTY(EditAnywhere, Category = "Intervention")
	FName WindowKey = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Intervention")
	TArray<FExecutionInterventionParticipantFilter> ParticipantFilters;

public:
	bool IsValidMinimal() const
	{
		return Timing != EExecutionInterventionTiming::None
			&& Timing != EExecutionInterventionTiming::Max
			&& !ParticipantFilters.IsEmpty();
	}
};

UENUM(BlueprintType)
enum class EExecutionStopSource : uint8
{
	None = 0,

	ActionOrchestration,
	ReactionOrchestration,

	System,
	External,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionAfterStopAction : uint8
{
	None = 0,

	StopOnly,
	StartIncoming,

	Max,
};

// [NOTE] Temp
UENUM(BlueprintType)
enum class EActionStopSource : uint8
{
	None = 0,

	ActionOrchestration,
	ReactionOrchestration,

	System,
	External,

	Max,
};

// [NOTE] Temp
UENUM(BlueprintType)
enum class EActionStopReason : uint8
{
	None = 0,

	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EActionFinishReason : uint8
{
	None = 0,

	Completed,
	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EActionRequestResultType : uint8
{
	None = 0,

	Rejected,
	Ignored,

	Handled,

	Started,
	Reserved,
	Deferred,
	Intervened,

	Max,
};

UENUM(BlueprintType)
enum class EActionRequestRejectReason : uint8
{
	None = 0,

	InvalidOwner,
	InvalidRequest,
	InvalidComponent,

	Dead,

	InvalidState,
	InvalidEquipment,
	InvalidCombatAction,

	InvalidQuery,

	ActionCandidateNotFound,
	ActionDataNotFound,
	ActionExecutorNotFound,
	RejectedByExecutor,
	NoExecutableAction,

	InvalidIndependent,
	InvalidSequential,
	InvalidExclusive,

	IncomingCannotIntervene,
	ActiveCannotAcceptIntervention,
	InterventionDispatchFailed,

	ActionExecutionFailed,

	Max,
};

UENUM(BlueprintType)
enum class EDamageImpactInfoSource : uint8
{
	None = 0,

	SweepResult,
	ClosestPoint,

	Max,
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

// [NOTE] Temp
UENUM(BlueprintType)
enum class EReactionStopSource : uint8
{
	None = 0,

	ActionOrchestration,
	ReactionOrchestration,

	System,
	External,

	Max,
};

// [NOTE] Temp
UENUM(BlueprintType)
enum class EReactionStopReason : uint8
{
	None = 0,

	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EReactionFinishReason : uint8
{
	None = 0,

	Completed,
	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EReactionRequestResultType : uint8
{
	None = 0,

	Rejected,
	Ignored,

	Started,
	Intervened,

	Max,
};

UENUM(BlueprintType)
enum class EReactionRequestRejectReason : uint8
{
	None = 0,

	InvalidOwner,
	InvalidRequest,
	InvalidComponent,

	InvalidDamageResult,

	Dead,

	ReactionCandidateNotFound,
	ReactionDataNotFound,
	ReactionExecutorNotFound,
	RejectedByExecutor,
	NoExecutableReaction,

	InvalidQuery,
	InvalidIndependent,
	InvalidSequential,
	InvalidExclusive,

	IncomingCannotIntervene,
	ActiveCannotAcceptIntervention,
	InterventionDispatchFailed,
	ReactionExecutionFailed,

	Max,
};

// [TODO] Migrate to CActionFeedbackStructure
UENUM(BlueprintType)
enum class EActionFeedbackTiming : uint8
{
	None,

	Start,

	Complete,
	Interrupt,

	Chain,

	TriggerOnce,
	TriggerWindowBegin,
	TriggerWindowEnd
};

// [TODO] Migrate to CActionFeedbackStructure
enum class EActionFeedbackMatchTier : uint8
{
	None = 0,

	AnyActionAnyIndex,
	ExactActionAnyIndex,
	ExactActionExactIndex,
};

// [TODO] Migrate to CActionFeedbackStructure
UENUM(BlueprintType)
enum class EActionVFXPlayType : uint8
{
	Once,
	Loop
};

// [TODO] Migrate to CActionFeedbackStructure
UENUM(BlueprintType)
enum class EActionSFXPlayType : uint8
{
	Once,
	Loop
};

USTRUCT(BlueprintType)
struct FActionDataKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Key")
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere, Category = "Key")
	int32 ActionIndex = INDEX_NONE;

public:
	bool IsValidMinimal() const;

public:
	bool operator==(const FActionDataKey& InOther) const
	{
		return ActionType == InOther.ActionType
			&& ActionIndex == InOther.ActionIndex;
	}
};

FORCEINLINE uint32 GetTypeHash(const FActionDataKey& InKey)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InKey.ActionType)));
	H = HashCombine(H, GetTypeHash(InKey.ActionIndex));

	return H;
}

USTRUCT(BlueprintType)
struct FActionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Key")
	FActionDataKey ActionDataKey = FActionDataKey();

	UPROPERTY(EditAnywhere, Category = "Key")
	TSubclassOf<class UCAction> ActionExecutorKey = nullptr;

	UPROPERTY(EditAnywhere, Category = "Priority")
	int32 Priority = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Data")
	class UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Data")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Data")
	bool bCanMove = false;

	UPROPERTY(EditAnywhere, Category = "Intervention|Want")
	TArray<FExecutionInterventionWantRule> WantInterventionRules;

	UPROPERTY(EditAnywhere, Category = "Intervention|Allow")
	TArray<FExecutionInterventionAllowRule> AllowInterventionRules;

public:
	FActionData() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FActionExecutionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FActionDataKey ActionDataKey = FActionDataKey();

	UPROPERTY(Transient)
	FActionData ActionData = FActionData();

	UPROPERTY(Transient)
	class UCAction* ActionExecutor = nullptr;

public:
	bool IsValidMinimal() const;
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
struct FWeaponContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType = EWeaponType::Max;

public:
	FWeaponContext() = default;
};

// [TODO] Translate to FActionExecutionContext
USTRUCT(BlueprintType)
struct FActionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FActionContext() = default;
};

USTRUCT(BlueprintType)
struct FDamageImpactInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bHasHitResult = false;


	UPROPERTY(Transient)
	EDamageImpactInfoSource Source = EDamageImpactInfoSource::None;

	UPROPERTY(Transient)
	FHitResult HitResult = FHitResult();

public:
	FDamageImpactInfo() = default;

public:
	bool IsValidMinimal() const
	{
		return bHasHitResult
			&& Source != EDamageImpactInfoSource::None
			&& Source != EDamageImpactInfoSource::Max;
	}
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
	FActionContext ActionContext = FActionContext();

	UPROPERTY(Transient)
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

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
	EActionType ActionType = EActionType::Max;

	UPROPERTY(EditAnywhere)
	int32 ActionIndex = INDEX_NONE;

public:
	FApplyDamageSpecKey() = default;

public:
	bool IsValidMinimal() const
	{
		return WeaponType != EWeaponType::None
			&& WeaponType != EWeaponType::Max
			&& ActionType != EActionType::None
			&& ActionType != EActionType::Max;
	}

public:
	bool operator==(const FApplyDamageSpecKey& InOther) const
	{
		return WeaponType == InOther.WeaponType
			&& ActionType == InOther.ActionType
			&& ActionIndex == InOther.ActionIndex;
	}
};

FORCEINLINE uint32 GetTypeHash(const FApplyDamageSpecKey& InOther)
{
	uint32 H = 0;

	H = HashCombine(H, GetTypeHash(static_cast<uint8>(InOther.WeaponType)));
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
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

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
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

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

	// Damage MetaData
	UPROPERTY(Transient)
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

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
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

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
	FDamageImpactInfo DamageImpactInfo = FDamageImpactInfo();

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
	bool IsValidMinimal() const;

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

	UPROPERTY(EditAnywhere, Category = "Priority")
	int32 Priority = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Data")
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Data")
	float PlayRate = 1.f;

	UPROPERTY(EditAnywhere, Category = "Data")
	bool bCanMove = false;

	UPROPERTY(EditAnywhere, Category = "Intervention|Want")
	TArray<FExecutionInterventionWantRule> WantInterventionRules;

	UPROPERTY(EditAnywhere, Category = "Intervention|Allow")
	TArray<FExecutionInterventionAllowRule> AllowInterventionRules;

public:
	FReactionData() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FReactionExecutionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FReactionDataKey ReactionDataKey = FReactionDataKey();

	UPROPERTY(Transient)
	FReactionData ReactionData = FReactionData();

	UPROPERTY(Transient)
	class UCReaction* ReactionExecutor = nullptr;

public:
	FReactionExecutionContext() = default;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FGuardObservableOverlaySnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bWantsGuarding = false;

	UPROPERTY(Transient)
	bool bIsGuardingPose = false;

	UPROPERTY(Transient)
	bool bCanGuard = false;

	UPROPERTY(Transient)
	bool bCanParry = false;

	UPROPERTY(Transient)
	bool bCanStartGuard = true;

public:
	bool HasGuardOverlay() const
	{
		return bIsGuardingPose || bCanGuard || bCanParry;
	}
};

USTRUCT(BlueprintType)
struct FObservableOverlaySnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FGuardObservableOverlaySnapshot Guard = FGuardObservableOverlaySnapshot();

public:
	bool HasObservableOverlay() const
	{
		return Guard.HasGuardOverlay();
	}
};

USTRUCT(BlueprintType)
struct FExecutionSnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionState ExecutionState = EExecutionState::Idle;

public:
	UPROPERTY(Transient)
	bool bIsDead = false;

	UPROPERTY(Transient)
	FObservableOverlaySnapshot ObservableOverlay = FObservableOverlaySnapshot();

public:
	bool IsIdle() const
	{
		return ExecutionState == EExecutionState::Idle;
	}

	bool IsInAction() const
	{
		return ExecutionState == EExecutionState::Action;
	}

	bool IsInReaction() const
	{
		return ExecutionState == EExecutionState::Reaction;
	}

	bool IsDead() const
	{
		return bIsDead || ExecutionState == EExecutionState::Dead;
	}

	bool HasObservableOverlay() const
	{
		return ObservableOverlay.HasObservableOverlay();
	}
};

USTRUCT(BlueprintType)
struct FExecutionParticipant
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bIsValid = false;

	UPROPERTY(Transient)
	EExecutionDomain ParticipantDomain = EExecutionDomain::None;

	UPROPERTY(Transient)
	FActionExecutionContext ActionContext = FActionExecutionContext();

	UPROPERTY(Transient)
	FReactionExecutionContext ReactionContext = FReactionExecutionContext();

public:
	bool IsValidMinimal() const;
	bool IsActionParticipant() const;
	bool IsReactionParticipant() const;

	const FActionExecutionContext& GetActionContext() const;
	const FReactionExecutionContext& GetReactionContext() const;

	UObject* GetExecutor() const;
	int32 GetPriority() const;
};

USTRUCT(BlueprintType)
struct FObservableOverlayExecutionDecision
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionDecision Decision = EExecutionDecision::None;

	UPROPERTY(Transient)
	TArray<EObservableOverlayHandling> Handlings;

public:
	bool IsAccepted() const
	{
		return Decision == EExecutionDecision::Accept;
	}
};

USTRUCT(BlueprintType)
struct FExecutionDecisionQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionSnapshot Snapshot = FExecutionSnapshot();

	UPROPERTY(Transient)
	FExecutionParticipant IncomingPart = FExecutionParticipant();

	UPROPERTY(Transient)
	FExecutionParticipant ActivePart = FExecutionParticipant();

public:
	bool HasIncomingPart() const
	{
		return IncomingPart.IsValidMinimal();
	}

	bool HasActivePart() const
	{
		return ActivePart.IsValidMinimal();
	}
};

USTRUCT(BlueprintType)
struct FObservableOverlayQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionDecisionQuery DecisionQuery = FExecutionDecisionQuery();

	UPROPERTY(Transient)
	EExecutionApplyMode ApplyMode = EExecutionApplyMode::None;
};

USTRUCT(BlueprintType)
struct FExecutionDecisionResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionDecision Decision = EExecutionDecision::None;

	UPROPERTY(Transient)
	EExecutionRelationship Relationship = EExecutionRelationship::None;

public:
	bool IsAccepted() const
	{
		return Decision == EExecutionDecision::Accept;
	}
};

USTRUCT(BlueprintType)
struct FExecutionInterventionQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionSnapshot Snapshot = FExecutionSnapshot();

	UPROPERTY(Transient)
	FExecutionParticipant IncomingPart = FExecutionParticipant();

	UPROPERTY(Transient)
	FExecutionParticipant ActivePart = FExecutionParticipant();

	UPROPERTY(Transient)
	EExecutionStopReason StopReason = EExecutionStopReason::None;

public:
	bool IsValidMinimal() const
	{
		return IncomingPart.IsValidMinimal()
			&& ActivePart.IsValidMinimal()
			&& StopReason != EExecutionStopReason::None
			&& StopReason != EExecutionStopReason::Max;
	}

};

USTRUCT(BlueprintType)
struct FExecutionInterventionDirective
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bRequested = false;

	UPROPERTY(Transient)
	EExecutionStopSource StopSource = EExecutionStopSource::None;

	UPROPERTY(Transient)
	EExecutionDomain SourceDomain = EExecutionDomain::None;

	UPROPERTY(Transient)
	EExecutionDomain TargetDomain = EExecutionDomain::None;

	UPROPERTY(Transient)
	EExecutionStopReason StopReason = EExecutionStopReason::None;

	UPROPERTY(Transient)
	EExecutionAfterStopAction AfterStopAction = EExecutionAfterStopAction::None;

public:
	bool IsRequested() const
	{
		return bRequested;
	}

	bool IsValidRequest() const
	{
		return bRequested
			&& StopSource != EExecutionStopSource::None
			&& StopSource != EExecutionStopSource::Max
			&& SourceDomain != EExecutionDomain::None
			&& SourceDomain != EExecutionDomain::Max
			&& TargetDomain != EExecutionDomain::None
			&& TargetDomain != EExecutionDomain::Max
			&& StopReason != EExecutionStopReason::None
			&& StopReason != EExecutionStopReason::Max;
	}
};

USTRUCT(BlueprintType)
struct FActionExecutionResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionDecision Decision = EExecutionDecision::None;

	UPROPERTY(Transient)
	EExecutionRelationship Relationship = EExecutionRelationship::None;

	UPROPERTY(Transient)
	EExecutionApplyMode ApplyMode = EExecutionApplyMode::None;

	UPROPERTY(Transient)
	FActionExecutionContext ResolvedContext = FActionExecutionContext();

	UPROPERTY(Transient)
	EActionRequestRejectReason RejectReason = EActionRequestRejectReason::None;

	UPROPERTY(Transient)
	FExecutionInterventionDirective InterventionDirective = FExecutionInterventionDirective();

	UPROPERTY(Transient)
	TArray<EObservableOverlayHandling> OverlayHandlings;


public:
	bool IsAcceptedDecision() const
	{
		return Decision == EExecutionDecision::Accept;
	}

	bool RequiresIntervention() const
	{
		return InterventionDirective.IsRequested();
	}
};

USTRUCT(BlueprintType)
struct FReactionExecutionResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionDecision Decision = EExecutionDecision::None;

	UPROPERTY(Transient)
	EExecutionRelationship Relationship = EExecutionRelationship::None;

	UPROPERTY(Transient)
	EExecutionApplyMode ApplyMode = EExecutionApplyMode::None;

	UPROPERTY(Transient)
	FReactionExecutionContext ResolvedContext = FReactionExecutionContext();

	UPROPERTY(Transient)
	EReactionRequestRejectReason RejectReason = EReactionRequestRejectReason::None;

	UPROPERTY(Transient)
	FExecutionInterventionDirective InterventionDirective = FExecutionInterventionDirective();

	UPROPERTY(Transient)
	TArray<EObservableOverlayHandling> OverlayHandlings;

public:
	bool IsAcceptedDecision() const
	{
		return Decision == EExecutionDecision::Accept;
	}

	bool RequiresIntervention() const
	{
		return InterventionDirective.IsRequested();
	}
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
