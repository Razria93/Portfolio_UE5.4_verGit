#include "Component/CHitFeedbackComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"

#include "Core/Profiling/CCombatFeedbackProfiling.h"
#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

#include "Type/CWeaponStructure.h"

UCHitFeedbackComponent::UCHitFeedbackComponent()
{
}

void UCHitFeedbackComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;

	ValidateRequiredComponentReferences();
}

bool UCHitFeedbackComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

void UCHitFeedbackComponent::PlayHitFeedback(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!CanPlayHitFeedback(InCombatSignalTargetPacket)) return;

	FCombatFeedbackProfiling::RecordHitFeedbackRequest();

	PlayHitStop(InCombatSignalTargetPacket);

	if (FCombatFeedbackProfiling::ShouldSkipEnemyCombatFeedback(OwnerCharacter_Injected))
	{
		FCombatFeedbackProfiling::RecordHitFeedbackPresentationSkipped();
		return;
	}

	PlayHitVFX(InCombatSignalTargetPacket);
	PlayHitSFX(InCombatSignalTargetPacket);
	PlayCameraShake(InCombatSignalTargetPacket);
}

void UCHitFeedbackComponent::PlayHitStop(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!CanPlayHitStop(InCombatSignalTargetPacket)) return;

	UCWorldSubsystem_CombatFeedback* feedbackSubsystem = GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>();
	if (!IsValid(feedbackSubsystem)) return;

	const FHitStopRequest hitStopRequest = BuildHitStopRequest(InCombatSignalTargetPacket);

	feedbackSubsystem->RequestHitStop(hitStopRequest);
}

void UCHitFeedbackComponent::PlayHitVFX(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!IsValid(GetWorld())) return;
	if (!IsValid(OwnerCharacter_Injected)) return;

	if (!IsValid(HitVFX))
	{
		FLog::Log(TEXT("[UCHitFeedbackComponent] Invalid HitVFX."));
		return;
	}

	const FVector location = ResolveHitFeedbackLocation(InCombatSignalTargetPacket);
	const FRotator rotation = ResolveHitFeedbackRotation(InCombatSignalTargetPacket);

	FCombatFeedbackProfiling::RecordHitVFX();

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitVFX, location, rotation);
}

void UCHitFeedbackComponent::PlayHitSFX(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	if (!IsValid(HitSFX))
	{
		FLog::Log(TEXT("[UCHitFeedbackComponent] Invalid HitSFX."));
		return;
	}

	const FVector location = ResolveHitFeedbackLocation(InCombatSignalTargetPacket);

	FCombatFeedbackProfiling::RecordHitSFX();

	UGameplayStatics::PlaySoundAtLocation(this, HitSFX, location);
}

void UCHitFeedbackComponent::PlayCameraShake(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!CanPlayCameraShake(InCombatSignalTargetPacket)) return;

	UCWorldSubsystem_CombatFeedback* feedbackSubsystem = GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>();
	if (!IsValid(feedbackSubsystem)) return;

	const FCameraShakeRequest cameraShakeRequest = BuildCameraShakeRequest(InCombatSignalTargetPacket);

	FCombatFeedbackProfiling::RecordCameraShakeRequest();

	feedbackSubsystem->RequestCameraShake(cameraShakeRequest);
}

bool UCHitFeedbackComponent::CanPlayHitFeedback(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!InCombatSignalTargetPacket.Result.bAccepted) return false;
	if (InCombatSignalTargetPacket.Result.CommittedDamage <= KINDA_SMALL_NUMBER) return false;

	return true;
}

bool UCHitFeedbackComponent::CanPlayHitStop(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (!GetWorld()) return false;

	if (HitStopAudience == EFeedbackAudience::None) return false;
	if (!FMath::IsFinite(HitStopDuration)) return false;
	if (!FMath::IsFinite(HitStopDilation)) return false;
	if (HitStopDuration <= KINDA_SMALL_NUMBER) return false;
	if (HitStopDilation < 0.f) return false;

	return true;
}

bool UCHitFeedbackComponent::CanPlayCameraShake(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (!GetWorld()) return false;
	if (!bEnableCameraShake) return false;
	
	if (CameraShakeAudience == EFeedbackAudience::None) return false;
	if (!IsValid(CameraShakeClass)) return false;
	if (!FMath::IsFinite(CameraShakeBaseScale)) return false;
	if (CameraShakeBaseScale <= KINDA_SMALL_NUMBER) return false;

	return true;
}

FVector UCHitFeedbackComponent::ResolveHitFeedbackLocation(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	const FDamageImpactInfo& damageHitInfo = InCombatSignalTargetPacket.Context.DamageImpactInfo;

	if (damageHitInfo.bHasHitResult)
	{
		return damageHitInfo.HitResult.ImpactPoint;
	}

	return IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->GetActorLocation() : FVector::ZeroVector;
}

FRotator UCHitFeedbackComponent::ResolveHitFeedbackRotation(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	const FDamageImpactInfo& damageHitInfo = InCombatSignalTargetPacket.Context.DamageImpactInfo;

	if (damageHitInfo.bHasHitResult)
	{
		return damageHitInfo.HitResult.ImpactNormal.Rotation();
	}

	return IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->GetActorRotation() : FRotator::ZeroRotator;
}

FHitStopRequest UCHitFeedbackComponent::BuildHitStopRequest(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	FHitStopRequest hitStopRequest;

	hitStopRequest.HitStopAudience = HitStopAudience;
	hitStopRequest.HitStopDuration = HitStopDuration;
	hitStopRequest.HitStopDilation = HitStopDilation;
	hitStopRequest.SourceActor = InCombatSignalTargetPacket.Context.SourceActor;
	hitStopRequest.TargetActor = InCombatSignalTargetPacket.Context.TargetActor;

	return hitStopRequest;
}

FCameraShakeRequest UCHitFeedbackComponent::BuildCameraShakeRequest(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	FCameraShakeRequest cameraShakeRequest;

	cameraShakeRequest.CameraShakeClass = CameraShakeClass;
	cameraShakeRequest.CameraShakeBaseScale = CameraShakeBaseScale;
	cameraShakeRequest.CameraShakeAudience = CameraShakeAudience;
	cameraShakeRequest.SourceActor = InCombatSignalTargetPacket.Context.SourceActor;
	cameraShakeRequest.TargetActor = InCombatSignalTargetPacket.Context.TargetActor;
	cameraShakeRequest.EventLocation = IsValid(InCombatSignalTargetPacket.Context.TargetActor)
		? InCombatSignalTargetPacket.Context.TargetActor->GetActorLocation()
		: (IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->GetActorLocation() : FVector::ZeroVector);

	return cameraShakeRequest;
}
