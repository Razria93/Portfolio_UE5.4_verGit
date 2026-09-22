#include "Component/CDefenseComponent.h"

#include "ProjectGlobal.h"

#include "Component/CMovementComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"
#include "Reaction/CReaction.h"
#include "Action/CAction.h"
#include "Notify/CAnimNotifyState_DodgeWindow.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UCDefenseComponent::UCDefenseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Component Reference

void UCDefenseComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	if (IsValid(HealthComp_Injected)) HealthComp_Injected->OnDeadStateChanged.RemoveAll(this);
	if (IsValid(HealthComp_Injected)) HealthComp_Injected->OnHealthValuesChanged.RemoveAll(this);
	ClearDodgeState();
	ClearCounterRush();
	PendingPerfectTarget.Reset();
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	MovementComp_Injected = InReferences.MovementComponent;
	ActionComp_Injected = InReferences.ActionComponent;
	HealthComp_Injected = InReferences.HealthComponent;
	if (IsValid(HealthComp_Injected))
	{
		HealthComp_Injected->OnHealthValuesChanged.AddUObject(this, &UCDefenseComponent::HandleOwnerHealthChanged);
		HealthComp_Injected->OnDeadStateChanged.AddUObject(this, &UCDefenseComponent::HandleOwnerDeadStateChanged);
	}
	ReactionComp_Injected = InReferences.ReactionComponent;
	PendingPerfectSerial = 0;
	PendingPerfectDodgeGeneration = 0;
	EndPerfectDodgeExecution(PerfectGeneration);

	ValidateRequiredComponentReferences();
}

// Component Lifecycle

void UCDefenseComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	if (IsValid(HealthComp_Injected)) HealthComp_Injected->OnDeadStateChanged.RemoveAll(this);
	if (IsValid(HealthComp_Injected)) HealthComp_Injected->OnHealthValuesChanged.RemoveAll(this);
	ClearDodgeState();
	ClearCounterRush();
	FinishPerfectDodgeRequest(PendingPerfectSerial);
	EndPerfectDodgeExecution(PerfectGeneration);
	Super::EndPlay(InEndPlayReason);
}

void UCDefenseComponent::HandleOwnerHealthChanged()
{
	if (IsValid(HealthComp_Injected) && HealthComp_Injected->IsAlive()) return;
	ClearDodgeState();
	FinishPerfectDodgeRequest(PendingPerfectSerial);
	EndPerfectDodgeExecution(PerfectGeneration);
	ClearCounterRush();
}

void UCDefenseComponent::HandleOwnerDeadStateChanged(EDeadState InPrevious, EDeadState InCurrent)
{
	if (InCurrent != EDeadState::Dead) return;
	ClearDodgeState();
	FinishPerfectDodgeRequest(PendingPerfectSerial);
	EndPerfectDodgeExecution(PerfectGeneration);
	ClearCounterRush();
}

void UCDefenseComponent::HandleCounterTargetDeadStateChanged(EDeadState InPrevious, EDeadState InCurrent)
{
	if (InCurrent == EDeadState::Dead) ClearCounterRush();
}

// Dodge Execution Ownership

void UCDefenseComponent::BeginDodgeExecution(uint64 InGeneration, UAnimMontage* InMontage)
{
	// Repeated startup notification must not re-arm the one-shot success window.
	if (InGeneration != 0 && DodgeGeneration == InGeneration) return;
	if (!IsValid(ActionComp_Injected) || InGeneration == 0
		|| ActionComp_Injected->GetActiveActionGeneration() != InGeneration
		|| !ActionComp_Injected->IsActiveActionType(EActionType::Dodge)
		|| !IsValid(OwnerCharacter_Injected) || !IsValid(InMontage)) return;

	UAnimInstance* animInstance = OwnerCharacter_Injected->GetMesh()->GetAnimInstance();
	FAnimMontageInstance* montageInstance = animInstance ? animInstance->GetActiveInstanceForMontage(InMontage) : nullptr;
	if (!montageInstance || !montageInstance->IsPlaying()) return;

	DodgeGeneration = InGeneration;
	DodgeMontageInstanceId = montageInstance->GetInstanceID();
	DodgeMontage = InMontage;
	bPerfectConsumed = false;
}

void UCDefenseComponent::EndDodgeExecution(uint64 InGeneration)
{
	if (InGeneration != 0 && DodgeGeneration == InGeneration) ClearDodgeState();
}

void UCDefenseComponent::ClearDodgeState()
{
	DodgeGeneration = 0;
	DodgeMontageInstanceId = INDEX_NONE;
	DodgeMontage.Reset();
	bPerfectConsumed = false;
}

bool UCDefenseComponent::TryGetDodgePosition(float& OutPosition) const
{
	OutPosition = 0.f;
	if (DodgeGeneration == 0 || !DodgeMontage.IsValid()
		|| !IsValid(OwnerCharacter_Injected) || OwnerCharacter_Injected->IsActorBeingDestroyed()
		|| !IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive()
		|| !IsValid(ActionComp_Injected)
		|| ActionComp_Injected->GetActiveActionGeneration() != DodgeGeneration
		|| !ActionComp_Injected->IsActiveActionType(EActionType::Dodge)) return false;

	const UCAction* executor = ActionComp_Injected->GetActiveActionExecutor();
	if (!IsValid(executor) || !executor->IsActive()) return false;
	UAnimInstance* animInstance = OwnerCharacter_Injected->GetMesh()->GetAnimInstance();
	const FAnimMontageInstance* montageInstance = animInstance ? animInstance->GetActiveInstanceForMontage(DodgeMontage.Get()) : nullptr;
	if (!montageInstance || !montageInstance->IsPlaying()
		|| montageInstance->GetInstanceID() != DodgeMontageInstanceId) return false;
	OutPosition = montageInstance->GetPosition();
	return true;
}

bool UCDefenseComponent::CanDodge() const
{
	float position;
	return TryGetDodgePosition(position) && (DodgeTuning.bUseMontageWindows
		? UCAnimNotifyState_DodgeWindow::Contains(DodgeMontage.Get(), EDodgeWindowKind::Invulnerability, position)
		: DodgeTuning.ContainsInvulnerabilityPosition(position));
}

EDamageDefenseOutcome UCDefenseComponent::ResolveDodgeContact()
{
	if (IsPerfectDodgeActive()) return EDamageDefenseOutcome::Dodge;
	float position;
	if (!TryGetDodgePosition(position) || !(DodgeTuning.bUseMontageWindows
		? UCAnimNotifyState_DodgeWindow::Contains(DodgeMontage.Get(), EDodgeWindowKind::Invulnerability, position)
		: DodgeTuning.ContainsInvulnerabilityPosition(position)))
		return EDamageDefenseOutcome::None;
	if (!bPerfectConsumed && (DodgeTuning.bUseMontageWindows
		? UCAnimNotifyState_DodgeWindow::Contains(DodgeMontage.Get(), EDodgeWindowKind::Perfect, position)
		: DodgeTuning.ContainsPerfectPosition(position)))
	{
		bPerfectConsumed = true;
		return EDamageDefenseOutcome::PerfectDodge;
	}
	return EDamageDefenseOutcome::Dodge;
}

void UCDefenseComponent::PreparePerfectDodge(uint64 InResultSerial, AActor* InAttacker)
{
	if (InResultSerial == 0 || !bPerfectConsumed || !CanDodge()) return;
	PendingPerfectSerial = InResultSerial;
	PendingPerfectDodgeGeneration = DodgeGeneration;
	PendingPerfectTarget = InAttacker;
}

bool UCDefenseComponent::CanStartPerfectDodge(uint64 InResultSerial) const
{
	return InResultSerial != 0 && PendingPerfectSerial == InResultSerial
		&& PendingPerfectDodgeGeneration != 0 && DodgeGeneration == PendingPerfectDodgeGeneration
		&& CanDodge();
}

void UCDefenseComponent::FinishPerfectDodgeRequest(uint64 InResultSerial)
{
	if (PendingPerfectSerial != InResultSerial) return;
	PendingPerfectSerial = 0;
	PendingPerfectDodgeGeneration = 0;
	PendingPerfectTarget.Reset();
}

bool UCDefenseComponent::CanCommitPerfectDodge(uint64 InResultSerial) const
{
	return InResultSerial != 0 && PendingPerfectSerial == InResultSerial
		&& IsValid(OwnerCharacter_Injected) && !OwnerCharacter_Injected->IsActorBeingDestroyed()
		&& IsValid(HealthComp_Injected) && HealthComp_Injected->IsAlive();
}

void UCDefenseComponent::BeginPerfectDodgeExecution(uint64 InGeneration, UAnimMontage* InMontage, uint64 InResultSerial)
{
	if (InGeneration == 0 || !CanCommitPerfectDodge(InResultSerial)
		|| !IsValid(ReactionComp_Injected) || ReactionComp_Injected->GetActiveReactionGeneration() != InGeneration
		|| !ReactionComp_Injected->IsActiveReactionType(EReactionType::PerfectDodge)
		|| !IsValid(OwnerCharacter_Injected) || !IsValid(InMontage)) return;
	UAnimInstance* anim = OwnerCharacter_Injected->GetMesh()->GetAnimInstance();
	FAnimMontageInstance* instance = anim ? anim->GetActiveInstanceForMontage(InMontage) : nullptr;
	if (!instance || !instance->IsPlaying()) return;
	PerfectGeneration = InGeneration;
	PerfectMontageInstanceId = instance->GetInstanceID();
	PerfectMontage = InMontage;
	PerfectAttacker = PendingPerfectTarget;
	ClearCounterRush();
	UCHealthComponent* targetHealth = PendingPerfectTarget.IsValid() ? PendingPerfectTarget->FindComponentByClass<UCHealthComponent>() : nullptr;
	if (IsValid(targetHealth) && targetHealth->IsAlive())
	{
		CounterTarget = PendingPerfectTarget;
		CounterTargetHealth = targetHealth;
		CounterTargetDeathHandle = targetHealth->OnDeadStateChanged.AddUObject(this, &UCDefenseComponent::HandleCounterTargetDeadStateChanged);
		CounterSerial = InResultSerial;
	}
	FinishPerfectDodgeRequest(InResultSerial);
}

void UCDefenseComponent::EndPerfectDodgeExecution(uint64 InGeneration, bool bCompleted)
{
	if (InGeneration == 0 || PerfectGeneration != InGeneration) return;
	if (bCompleted || bCounterTransition)
		CounterExpiresAt = FPlatformTime::Seconds() + FMath::Max(0.f, DodgeTuning.CounterGraceSeconds);
	else
		ClearCounterRush();
	PerfectGeneration = 0;
	PerfectMontageInstanceId = INDEX_NONE;
	PerfectMontage.Reset();
	PerfectAttacker.Reset();
}

AActor* UCDefenseComponent::GetCounterRushTarget() const
{
	AActor* target = CounterTarget.Get();
	if (CounterSerial == 0 || !IsValid(target) || target->IsActorBeingDestroyed()
		|| !IsValid(OwnerCharacter_Injected) || OwnerCharacter_Injected->IsActorBeingDestroyed()
		|| !IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive()) return nullptr;
	const UCHealthComponent* targetHealth = target->FindComponentByClass<UCHealthComponent>();
	if (!IsValid(targetHealth) || !targetHealth->IsAlive()) return nullptr;
	if (!FMath::IsFinite(DodgeTuning.CounterMaxDistance) || DodgeTuning.CounterMaxDistance <= 0.f
		|| FVector::DistSquared(target->GetActorLocation(), OwnerCharacter_Injected->GetActorLocation())
			> FMath::Square(DodgeTuning.CounterMaxDistance)) return nullptr;
	if (PerfectGeneration != 0)
	{
		if (!IsPerfectDodgeActive()) return nullptr;
		UAnimInstance* anim = OwnerCharacter_Injected->GetMesh()->GetAnimInstance();
		const FAnimMontageInstance* instance = anim ? anim->GetMontageInstanceForID(PerfectMontageInstanceId) : nullptr;
		if (!instance || !FMath::IsFinite(DodgeTuning.CounterConnectStart)
			|| instance->GetPosition() < DodgeTuning.CounterConnectStart) return nullptr;
	}
	else if (CounterExpiresAt <= FPlatformTime::Seconds()) return nullptr;
	return target;
}

uint64 UCDefenseComponent::GetCounterRushSerial() const
{
	return GetCounterRushTarget() ? CounterSerial : 0;
}

bool UCDefenseComponent::IsCounterRushPresentationPending(uint64 InSerial) const
{
	if (InSerial == 0 || CounterSerial != InSerial || !CounterTarget.IsValid()
		|| CounterTarget->IsActorBeingDestroyed() || !CounterTargetHealth.IsValid() || !CounterTargetHealth->IsAlive()
		|| !IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive()) return false;
	return PerfectGeneration != 0 ? IsPerfectDodgeActive() : CounterExpiresAt > FPlatformTime::Seconds();
}

bool UCDefenseComponent::BeginCounterRushTransition()
{
	if (bCounterTransition || !GetCounterRushTarget()) return false;
	bCounterTransition = true;
	return true;
}

void UCDefenseComponent::EndCounterRushTransition()
{
	bCounterTransition = false;
}

bool UCDefenseComponent::ConsumeCounterRush(uint64 InSerial)
{
	if (InSerial == 0 || InSerial != GetCounterRushSerial()) return false;
	ClearCounterRush();
	return true;
}

void UCDefenseComponent::ClearCounterRush()
{
	if (CounterTargetHealth.IsValid()) CounterTargetHealth->OnDeadStateChanged.Remove(CounterTargetDeathHandle);
	CounterTargetDeathHandle.Reset();
	CounterTargetHealth.Reset();
	CounterTarget.Reset();
	CounterSerial = 0;
	CounterExpiresAt = 0.;
	bCounterTransition = false;
}

bool UCDefenseComponent::IsPerfectDodgeActive() const
{
	if (PerfectGeneration == 0 || !PerfectMontage.IsValid()
		|| !IsValid(OwnerCharacter_Injected) || OwnerCharacter_Injected->IsActorBeingDestroyed()
		|| !IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive()
		|| !IsValid(ReactionComp_Injected) || ReactionComp_Injected->GetActiveReactionGeneration() != PerfectGeneration
		|| !ReactionComp_Injected->IsActiveReactionType(EReactionType::PerfectDodge)) return false;
	const UCReaction* executor = ReactionComp_Injected->GetActiveReactionExecutor();
	if (!IsValid(executor) || !executor->IsActive()) return false;
	UAnimInstance* anim = OwnerCharacter_Injected->GetMesh()->GetAnimInstance();
	const FAnimMontageInstance* instance = anim ? anim->GetMontageInstanceForID(PerfectMontageInstanceId) : nullptr;
	return instance && instance->IsPlaying();
}

// Component Reference Validation

bool UCDefenseComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ MovementComp_Injected, TEXT("UCMovementComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

	// Overlay Policy
void UCDefenseComponent::WriteOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const
{
	OutSnapshot.Guard.bCanStartGuard = bCanStartGuard;
	OutSnapshot.Guard.bWantsGuarding = bWantsGuarding;
	OutSnapshot.Guard.bIsGuardingPose = bIsGuardingPose;
	OutSnapshot.Guard.bCanGuard = bCanGuard;
	OutSnapshot.Guard.bCanParry = bCanParry;
}

bool UCDefenseComponent::CanApplyOverlayEvent(const FObservableOverlayEventContext& InContext) const
{
	switch (InContext.EventType)
	{
	case EObservableOverlayEventType::GuardInputPressed:
	case EObservableOverlayEventType::GuardInputReleased:
	case EObservableOverlayEventType::GuardInStarted:
	case EObservableOverlayEventType::GuardOutStarted:
	case EObservableOverlayEventType::SwitchToGuard:
	case EObservableOverlayEventType::AllowGuardStart:
	case EObservableOverlayEventType::GuardLifecycleCompleted:
	case EObservableOverlayEventType::GuardLifecycleInterrupted:
		return true;

	default:
		return false;
	}
}

bool UCDefenseComponent::ApplyOverlayEvent(const FObservableOverlayEventContext& InContext)
{
	if (!CanApplyOverlayEvent(InContext)) return false;

	switch (InContext.EventType)
	{
	case EObservableOverlayEventType::GuardInputPressed:
		HandleGuardInputPressed();
		return true;

	case EObservableOverlayEventType::GuardInputReleased:
		HandleGuardInputReleased();
		return true;

	case EObservableOverlayEventType::GuardInStarted:
		HandleGuardInStarted();
		return true;

	case EObservableOverlayEventType::GuardOutStarted:
		HandleGuardOutStarted();
		return true;

	case EObservableOverlayEventType::SwitchToGuard:
		HandleSwitchToGuard();
		return true;

	case EObservableOverlayEventType::AllowGuardStart:
		HandleAllowGuardStart();
		return true;

	case EObservableOverlayEventType::GuardLifecycleCompleted:
		HandleGuardLifecycleCompleted();
		return true;

	case EObservableOverlayEventType::GuardLifecycleInterrupted:
		HandleGuardLifecycleInterrupted();
		return true;

	default:
		return false;
	}
}

bool UCDefenseComponent::CanApplyOverlayHandling(EObservableOverlayHandling InHandling) const
{
	switch (InHandling)
	{
	case EObservableOverlayHandling::None:
		return true;

	case EObservableOverlayHandling::ClearGuardState:
		return true;

	case EObservableOverlayHandling::ClearGuardOverlay:
		return true;

	default:
		return false;
	}
}

bool UCDefenseComponent::ApplyOverlayHandling(EObservableOverlayHandling InHandling)
{
	if (!CanApplyOverlayHandling(InHandling)) return false;

	switch (InHandling)
	{
	case EObservableOverlayHandling::None:
		return true;

	case EObservableOverlayHandling::ClearGuardState:
		ClearGuardState();
		return true;

	case EObservableOverlayHandling::ClearGuardOverlay:
		ClearGuardOverlay();
		return true;

	default:
		return false;
	}
}

// Guard Event Entry
void UCDefenseComponent::HandleGuardInputPressed()
{
	BeginGuardIntent();
}

void UCDefenseComponent::HandleGuardInputReleased()
{
	EndGuardIntent();
}

void UCDefenseComponent::HandleGuardInStarted()
{
	BlockGuardStart();

	BeginGuardPose();
	CloseGuardWindow();
	OpenParryWindow();

	ApplyGuardMovementGaitOverride();
}

void UCDefenseComponent::HandleGuardOutStarted()
{
	BlockGuardStart();

	EndGuardPose();
	CloseGuardWindow();
	CloseParryWindow();

	ClearMovementGaitOverride();
}

void UCDefenseComponent::HandleSwitchToGuard()
{
	CloseParryWindow();

	if (WantsGuarding())
	{
		OpenGuardWindow();
	}
	else
	{
		CloseGuardWindow();
	}
}

void UCDefenseComponent::HandleAllowGuardStart()
{
	AllowGuardStart();
}

void UCDefenseComponent::HandleGuardLifecycleCompleted()
{
	ClearGuardState();
}

void UCDefenseComponent::HandleGuardLifecycleInterrupted()
{
	ClearGuardState();
}

// Guard State Cleanup
void UCDefenseComponent::ClearGuardState()
{
	AllowGuardStart();
	EndGuardIntent();

	EndGuardPose();
	CloseGuardWindow();
	CloseParryWindow();

	ClearMovementGaitOverride();
}

void UCDefenseComponent::ClearGuardOverlay()
{
	EndGuardPose();
	CloseGuardWindow();
	CloseParryWindow();

	ClearMovementGaitOverride();
}

void UCDefenseComponent::RestoreGuardOverlay()
{
	if (WantsGuarding())
	{
		BeginGuardPose();
		OpenGuardWindow();
		CloseParryWindow();

		ApplyGuardMovementGaitOverride();
	}
}

// Guard State Primitive
void UCDefenseComponent::AllowGuardStart()
{
	bCanStartGuard = true;
}

void UCDefenseComponent::BlockGuardStart()
{
	bCanStartGuard = false;
}

void UCDefenseComponent::BeginGuardIntent()
{
	bWantsGuarding = true;
}

void UCDefenseComponent::EndGuardIntent()
{
	bWantsGuarding = false;
}

void UCDefenseComponent::BeginGuardPose()
{
	bIsGuardingPose = true;
}

void UCDefenseComponent::EndGuardPose()
{
	bIsGuardingPose = false;
}

void UCDefenseComponent::OpenGuardWindow()
{
	bCanGuard = true;
}

void UCDefenseComponent::CloseGuardWindow()
{
	bCanGuard = false;
}

void UCDefenseComponent::OpenParryWindow()
{
	bCanParry = true;
}

void UCDefenseComponent::CloseParryWindow()
{
	bCanParry = false;
}

// Movement Gait Override
void UCDefenseComponent::ApplyGuardMovementGaitOverride()
{
	if (!IsValid(MovementComp_Injected)) return;

	MovementComp_Injected->ApplyMovementGaitOverride(EMovementGait::Walk);
}

void UCDefenseComponent::ClearMovementGaitOverride()
{
	if (!IsValid(MovementComp_Injected)) return;

	MovementComp_Injected->ClearMovementGaitOverride();
}
