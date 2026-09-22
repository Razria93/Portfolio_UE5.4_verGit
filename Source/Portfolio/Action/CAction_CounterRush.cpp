#include "Action/CAction_CounterRush.h"

#include "Component/CDefenseComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"

void UCAction_CounterRush::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	Super::InitializeReferences(InReferences);
	DefenseComp_Injected = InReferences.DefenseComponent;
	MovementComp_Injected = InReferences.MovementComponent;
}

FExecutionDecisionResult UCAction_CounterRush::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;
	result.Decision = EExecutionDecision::Reject;
	if (!IsIncomingActionType(InQuery, EActionType::CounterRush) || InQuery.Snapshot.IsDead()
		|| !IsValid(DefenseComp_Injected) || !DefenseComp_Injected->GetCounterRushTarget()
		|| !IsValid(WeaponComp_Injected) || WeaponComp_Injected->CheckCurrentWeaponType(EWeaponType::Unarmed)) return result;
	if (CanResolveIndependentRelationship(InQuery)) result.Relationship = EExecutionRelationship::Independent;
	else if (InQuery.ActivePart.IsReactionParticipant()
		&& InQuery.ActivePart.ReactionContext.ReactionDataKey.ReactionType == EReactionType::PerfectDodge)
		result.Relationship = EExecutionRelationship::Exclusive;
	else return result;
	result.Decision = EExecutionDecision::Accept;
	return result;
}

bool UCAction_CounterRush::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	return InQuery.IsValidMinimal() && IsIncomingActionType(InQuery, EActionType::CounterRush)
		&& InQuery.ActivePart.IsReactionParticipant()
		&& InQuery.ActivePart.ReactionContext.ReactionDataKey.ReactionType == EReactionType::PerfectDodge
		&& IsValid(DefenseComp_Injected) && DefenseComp_Injected->GetCounterRushTarget();
}

bool UCAction_CounterRush::Start(const FActionData& InData, uint32 InActionRequestSerial)
{
	AActor* target = IsValid(DefenseComp_Injected) ? DefenseComp_Injected->GetCounterRushTarget() : nullptr;
	if (!IsValid(target) || !IsValid(OwnerCharacter_Injected) || bIsActive
		|| !FMath::IsFinite(RushDuration) || RushDuration <= 0.f
		|| !FMath::IsFinite(MaxTravelDistance) || MaxTravelDistance < 0.f
		|| !FMath::IsFinite(StopDistance) || StopDistance < 0.f) return false;
	OpportunitySerial = DefenseComp_Injected->GetCounterRushSerial();
	TargetLocation = target->GetActorLocation();
	return Super::Start(InData, InActionRequestSerial);
}

void UCAction_CounterRush::CommitStarted()
{
	if (!bIsActive || !IsValid(DefenseComp_Injected)
		|| OpportunitySerial == 0 || OpportunitySerial != DefenseComp_Injected->GetCounterRushSerial())
	{
		Stop(EActionStopReason::Interrupted);
		return;
	}
	UCharacterMovementComponent* movement = OwnerCharacter_Injected->GetCharacterMovement();
	if (!IsValid(movement)) { Stop(EActionStopReason::Interrupted); return; }
	FVector delta = TargetLocation - OwnerCharacter_Injected->GetActorLocation();
	delta.Z = 0.f;
	const float distance = FMath::Clamp(delta.Size() - StopDistance, 0.f, MaxTravelDistance);
	if (distance > KINDA_SMALL_NUMBER)
	{
		RushSource = MakeShared<FRootMotionSource_ConstantForce>();
		RushSource->InstanceName = TEXT("CounterRush");
		RushSource->Priority = 900;
		RushSource->AccumulateMode = ERootMotionAccumulateMode::Override;
		RushSource->Force = delta.GetSafeNormal() * (distance / RushDuration);
		RushSource->Duration = RushDuration;
		RushSource->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
		// Stop horizontal drift on natural expiry without slowing an existing fall.
		RushSource->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::ClampVelocity;
		RushSource->FinishVelocityParams.ClampVelocity = 0.f;
		if (movement->ApplyRootMotionSource(RushSource) == static_cast<uint16>(ERootMotionSourceID::Invalid))
		{
			RushSource.Reset();
			Stop(EActionStopReason::Interrupted);
			return;
		}
	}
	if (!DefenseComp_Injected->ConsumeCounterRush(OpportunitySerial))
	{
		Stop(EActionStopReason::Interrupted);
		return;
	}
	if (IsValid(MovementComp_Injected))
	{
		MovementComp_Injected->TryFaceTarget(TargetLocation, 600.f, 180.f);
	}
	if (auto* feedback = OwnerCharacter_Injected->FindComponentByClass<UCReactionFeedbackComponent>())
		feedback->TransferCounterWeaponToAttack(OpportunitySerial);
}

void UCAction_CounterRush::EndCounterAttackPresentation()
{
	if (IsValid(OwnerCharacter_Injected))
		if (auto* feedback = OwnerCharacter_Injected->FindComponentByClass<UCReactionFeedbackComponent>())
			feedback->ReleaseCounterWeapon(OpportunitySerial);
}

void UCAction_CounterRush::CleanupRuntimeEffects()
{
	EndCounterAttackPresentation();
	if (RushSource.IsValid() && IsValid(OwnerCharacter_Injected))
	{
		UCharacterMovementComponent* movement = OwnerCharacter_Injected->GetCharacterMovement();
		if (IsValid(movement) && movement->GetRootMotionSourceByID(RushSource->LocalID) == RushSource)
		{
			const auto other = [this](const TSharedPtr<FRootMotionSource>& InSource) { return InSource.IsValid() && InSource != RushSource; };
			const auto& sources = movement->CurrentRootMotion;
			const bool hasOther = sources.RootMotionSources.ContainsByPredicate(other) || sources.PendingAddRootMotionSources.ContainsByPredicate(other);
			const FVector velocity = RushSource->RootMotionParams.GetRootMotionTransform().GetTranslation();
			if (!hasOther && !OwnerCharacter_Injected->IsPlayingRootMotion()
				&& RushSource->Status.HasFlag(ERootMotionSourceStatusFlags::Prepared)
				&& FVector2D(movement->Velocity).Equals(FVector2D(velocity), KINDA_SMALL_NUMBER))
			{
				movement->Velocity.X = 0.f;
				movement->Velocity.Y = 0.f;
			}
			RushSource->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;
			movement->RemoveRootMotionSourceByID(RushSource->LocalID);
		}
	}
	RushSource.Reset();
	Super::CleanupRuntimeEffects();
}

void UCAction_CounterRush::ClearRuntime()
{
	OpportunitySerial = 0;
	TargetLocation = FVector::ZeroVector;
	Super::ClearRuntime();
}
