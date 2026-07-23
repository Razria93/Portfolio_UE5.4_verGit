#pragma once

#include "CoreMinimal.h"
#include "Type/CWeaponTypes.h"
#include "Type/CActionTypes.h"
#include "Type/CReactionTypes.h"
#include "Type/CExecutionRuleTypes.h"
#include "Type/CActionDataTypes.h"
#include "Type/CReactionDataTypes.h"
#include "Type/CCombatHitTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CCombatSignalSourceTypes.h"
#include "Type/CCombatSignalTargetTypes.h"
#include "Type/CStateTypes.h"
#include "Type/CHealthTypes.h"
#include "CWeaponStructure.generated.h"

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
enum class EExecutionStopReason : uint8
{
	None = 0,

	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EObservableOverlayEventType : uint8
{
	None = 0,

	GuardInputPressed,
	GuardInputReleased,

	GuardInStarted,
	GuardOutStarted,

	SwitchToGuard,
	AllowGuardStart,

	GuardLifecycleCompleted,
	GuardLifecycleInterrupted,

	Max,
};

USTRUCT(BlueprintType)
struct FObservableOverlayEventContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EObservableOverlayEventType EventType = EObservableOverlayEventType::None;

public:
	FObservableOverlayEventContext() = default;

	explicit FObservableOverlayEventContext(EObservableOverlayEventType InEventType)
		: EventType(InEventType)
	{
	}

public:
	bool IsValidMinimal() const
	{
		return EventType != EObservableOverlayEventType::None && EventType != EObservableOverlayEventType::Max;
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
	RejectedByOverlay,

	ActionExecutionFailed,

	Max,
};

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
	RejectedByOverlay,
	ReactionExecutionFailed,

	Max,
};

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

	bool HasGuardRuntimeState() const
	{
		return !bCanStartGuard || bWantsGuarding || HasGuardOverlay();
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

	UPROPERTY(Transient)
	FExecutionParticipant IncomingPart = FExecutionParticipant();

	UPROPERTY(Transient)
	FExecutionParticipant ActivePart = FExecutionParticipant();

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
			&& StopReason != EExecutionStopReason::Max
			&& IncomingPart.IsValidMinimal()
			&& ActivePart.IsValidMinimal();
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
