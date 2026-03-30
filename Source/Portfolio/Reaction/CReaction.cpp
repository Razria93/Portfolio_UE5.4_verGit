#include "Reaction/CReaction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CReactionComponent.h"
#include "CReaction_Hit.h"


void UCReaction::Initialize(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	check(OwnerCharacter_Injected);

	OwnerReactionComp_Injected = InOwnerReactionComponent;
	check(OwnerReactionComp_Injected);
}

bool UCReaction::IsValidMinimal()
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(OwnerReactionComp_Injected)) return false;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return false;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance)) return false;

	return true;
}

bool UCReaction::Begin(const FReactionData& InReactionData)
{
	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	UAnimInstance* animInstance = meshComp->GetAnimInstance();

	const float playRate = FMath::Max(0.01f, InReactionData.PlayRate);

	// Play Montage (Use Montage/PlayRate in InReactionData)
	const float duration = animInstance->Montage_Play(InReactionData.Montage, playRate);
	if (duration <= 0.f) return false;

	bIsActive = true;
	ActiveMontage_Cached = InReactionData.Montage;

	const uint32 thisPlaySerial = ++Serial_CurrentPlay;
	CachedSerial_ActivePlay = thisPlaySerial;

	FOnMontageEnded montageEnd;
	montageEnd.BindUObject(this, &UCReaction::OnMontageEnd, thisPlaySerial); // Capture Serial at this time
	animInstance->Montage_SetEndDelegate(montageEnd, ActiveMontage_Cached);

	return true;
}

void UCReaction::Stop(EReactionStopReason InStopReason)
{
	if (!bIsActive) return;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (IsValid(animInstance) && IsValid(ActiveMontage_Cached))
	{
		// Stop Montage
		animInstance->Montage_Stop(0.1f, ActiveMontage_Cached);

		PrintStopReasonInfo(InStopReason);
		PrintReactionExecutorRuntimeInfo();
	}

	// [Interrupted / Canceled / Force Events]
	// Call `End(true)` here for safety
	End(true);
}

void UCReaction::End(bool bInterrupted)
{
	if (!bIsActive) return;

	Clear();

	if (IsValid(OwnerReactionComp_Injected))
	{
		OwnerReactionComp_Injected->OnReactionEnd(this, bInterrupted);
	}
}

void UCReaction::Clear()
{
	bIsActive = false;
	ActiveMontage_Cached = nullptr;

	bInterruptible = false;
	bCancelable = false;
}

void UCReaction::PrintReactionExecutorRuntimeInfo_Public() const
{
	PrintReactionExecutorRuntimeInfo();
}

void UCReaction::OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial)
{
	// Serial Token Guard
	if (InSerial != CachedSerial_ActivePlay) return;

	// Montage Guard
	if (InAnimMontage != ActiveMontage_Cached) return;

	End(bInterrupted);
}

void UCReaction::PrintReactionExecutorRuntimeInfo() const
{
	FLog::Log(TEXT("----- ReactionRuntime Info ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActiveMontage"), *GetNameSafe(ActiveMontage_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bIsActive"), bIsActive ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bInterruptible"), bInterruptible ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bCancelable"), bCancelable ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_CurrentPlay"), Serial_CurrentPlay));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_ActivePlay"), CachedSerial_ActivePlay));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReaction::PrintStopReasonInfo(EReactionStopReason InStopReason) const
{
	FLog::Log(FString::Printf(TEXT("[Reaction::Stop] Reason = %s | ActiveReaction = %s"), *UEnum::GetValueAsString(InStopReason), *GetNameSafe(this)));
}