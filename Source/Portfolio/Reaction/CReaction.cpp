#include "Reaction/CReaction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CReactionComponent.h"


void UCReaction::InitializeReaction(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	check(OwnerCharacter_Injected);

	OwnerReactionComp_Injected = InOwnerReactionComponent;
	check(OwnerReactionComp_Injected);

	MovementComp_Cached = Cast<UCMovementComponent>(OwnerCharacter_Injected->GetComponentByClass(UCMovementComponent::StaticClass()));
	check(MovementComp_Cached);

	StateComp_Cached = Cast<UCStateComponent>(OwnerCharacter_Injected->GetComponentByClass(UCStateComponent::StaticClass()));
	check(StateComp_Cached);
}

bool UCReaction::Begin(const FReactionData& reactionData)
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(OwnerReactionComp_Injected)) return false;
	if (!IsValid(reactionData.Montage)) return false;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return false;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance)) return false;

	const float playRate = FMath::Max(0.01f, reactionData.PlayRate);

	// Play Montage
	const float duration = animInstance->Montage_Play(reactionData.Montage, playRate);
	if (duration <= 0.f) return false;

	bIsActive = true;
	ChangeStateToReaction();
	ActiveMontage_Cached = reactionData.Montage;

	bInterruptibleNow = false;
	bCancelableNow = false;

	ChangeMovementToImmovable(reactionData.bCanMove);

	FOnMontageEnded montageEnd;
	
	const uint32 thisPlaySerial = ++Serial_CurrentPlay;
	CachedSerial_ActivePlay = thisPlaySerial;
	
	montageEnd.BindUObject(this, &UCReaction::OnMontageEnd, thisPlaySerial); // Capture Serial at this time
	animInstance->Montage_SetEndDelegate(montageEnd, ActiveMontage_Cached);

	return true;
}

void UCReaction::Stop(EReactionStopReason InStopReason, const UCReaction* InNewReaction)
{
	if (!bIsActive) return;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (IsValid(animInstance) && IsValid(ActiveMontage_Cached))
	{
		// Stop Montage
		animInstance->Montage_Stop(0.1f, ActiveMontage_Cached);

		FLog::Log(FString::Printf(TEXT("[Reaction::Stop] Reason=%s | Active=%s | Montage=%s | New=%s"),
			*UEnum::GetValueAsString(InStopReason),
			*GetNameSafe(this),
			*GetNameSafe(ActiveMontage_Cached),
			*GetNameSafe(InNewReaction)
		));
	}

	// [Interrupted / Canceled / Force Events]
	// Call `End(true)` here for safety
	End(true);
}

void UCReaction::End(bool bInterrupted)
{
	if (!bIsActive) return;

	// Clean up inner state CReaction (Not outer state)
	bIsActive = false;
	ActiveMontage_Cached = nullptr;

	bInterruptibleNow = false;
	bCancelableNow = false;

	if (IsValid(OwnerReactionComp_Injected))
	{
		OwnerReactionComp_Injected->OnReactionEnd(this, bInterrupted);
	}
}

void UCReaction::OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial)
{
	// Serial Token Guard
	if (InSerial != CachedSerial_ActivePlay) return;

	// Montage Guard
	if (InAnimMontage != ActiveMontage_Cached) return;

	End(bInterrupted);
}

void UCReaction::ChangeMovementToImmovable(bool bCanMove)
{
	if (!IsValid(MovementComp_Cached)) return;

	if (bCanMove == false)
	{
		// Apply: Reaction | Restore: Component
		MovementComp_Cached->SetStop();
	}
}

void UCReaction::ChangeStateToReaction()
{
	if (!IsValid(StateComp_Cached)) return;

	// Apply: Reaction | Restore: Component
	StateComp_Cached->SetReactionMode();
}

void UCReaction::PrintReactionExecutorRuntimeInfo() const
{
	FLog::Log(TEXT("----- ReactionRuntime Info ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActiveMontage"), *GetNameSafe(ActiveMontage_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bIsActive"), bIsActive ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bInterruptibleNow"), bInterruptibleNow ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bCancelableNow"), bCancelableNow ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_CurrentPlay"), Serial_CurrentPlay));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_ActivePlay"), CachedSerial_ActivePlay));
	FLog::Log(TEXT("---------------------------------"));
}
