#include "Component/CHitFeedbackComponent.h"

#include "ProjectGlobal.h"

#include "Core/Debug/FCombatFeedbackDebug.h"
#include "Core/Profiling/CCombatFeedbackProfiling.h"
#include "System/Combat/CWorldSubsystem_CombatFeedback.h"
#include "Type/CCombatHitTypes.h"
#include "Type/CCombatSignalTargetTypes.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"

UCHitFeedbackComponent::UCHitFeedbackComponent()
{
}

// Component Reference

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

// Entry

void UCHitFeedbackComponent::PlayHitFeedback(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!CanPlayHitFeedback(InCombatSignalTargetPacket)) return;

	FCombatFeedbackDebug::RecordHitFeedbackRequestAcceptedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket);
	FCombatFeedbackProfiling::RecordHitFeedbackRequest();

	PlayHitStop(InCombatSignalTargetPacket);

	if (FCombatFeedbackProfiling::ShouldSkipEnemyCombatFeedback(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("Presentation"), TEXT("RuntimeLODSkipEnemyFeedback"));
		FCombatFeedbackProfiling::RecordHitFeedbackPresentationSkipped();
		return;
	}

	PlayHitVFX(InCombatSignalTargetPacket);
	PlayHitSFX(InCombatSignalTargetPacket);
	PlayCameraShake(InCombatSignalTargetPacket);
}

// Playback

void UCHitFeedbackComponent::PlayHitStop(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!CanPlayHitStop(InCombatSignalTargetPacket)) return;

	UCWorldSubsystem_CombatFeedback* feedbackSubsystem = GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>();
	if (!IsValid(feedbackSubsystem))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("HitStop"), TEXT("InvalidFeedbackSubsystem"));
		return;
	}

	const FHitStopRequest hitStopRequest = BuildHitStopRequest(InCombatSignalTargetPacket);

	FCombatFeedbackDebug::RecordHitFeedbackPresentationRequestedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("HitStop"));
	feedbackSubsystem->RequestHitStop(hitStopRequest);
}

void UCHitFeedbackComponent::PlayHitVFX(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!IsValid(GetWorld()))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("VFX"), TEXT("InvalidWorld"));
		return;
	}
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("VFX"), TEXT("InvalidOwner"));
		return;
	}

	if (!IsValid(HitVFX))
	{
		FCombatFeedbackDebug::RecordHitFeedbackAssetRejectedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), HitVFX, TEXT("InvalidAsset"));
		return;
	}

	const FVector location = ResolveHitFeedbackLocation(InCombatSignalTargetPacket);
	const FRotator rotation = ResolveHitFeedbackRotation(InCombatSignalTargetPacket);

	FCombatFeedbackProfiling::RecordHitVFX();
	FCombatFeedbackDebug::RecordHitFeedbackPresentationPlayedForAudit(OwnerCharacter_Injected, this, TEXT("VFX"), HitVFX, TEXT("Spawn"));

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitVFX, location, rotation);
}

void UCHitFeedbackComponent::PlayHitSFX(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("SFX"), TEXT("InvalidOwner"));
		return;
	}

	if (!IsValid(HitSFX))
	{
		FCombatFeedbackDebug::RecordHitFeedbackAssetRejectedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), HitSFX, TEXT("InvalidAsset"));
		return;
	}

	const FVector location = ResolveHitFeedbackLocation(InCombatSignalTargetPacket);

	FCombatFeedbackProfiling::RecordHitSFX();
	FCombatFeedbackDebug::RecordHitFeedbackPresentationPlayedForAudit(OwnerCharacter_Injected, this, TEXT("SFX"), HitSFX, TEXT("Play"));

	UGameplayStatics::PlaySoundAtLocation(this, HitSFX, location);
}

void UCHitFeedbackComponent::PlayCameraShake(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!CanPlayCameraShake(InCombatSignalTargetPacket)) return;

	UCWorldSubsystem_CombatFeedback* feedbackSubsystem = GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>();
	if (!IsValid(feedbackSubsystem))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("CameraShake"), TEXT("InvalidFeedbackSubsystem"));
		return;
	}

	const FCameraShakeRequest cameraShakeRequest = BuildCameraShakeRequest(InCombatSignalTargetPacket);

	FCombatFeedbackProfiling::RecordCameraShakeRequest();
	FCombatFeedbackDebug::RecordHitFeedbackPresentationRequestedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("CameraShake"));

	feedbackSubsystem->RequestCameraShake(cameraShakeRequest);
}

// Query

bool UCHitFeedbackComponent::CanPlayHitFeedback(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		FCombatFeedbackDebug::RecordHitFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("InvalidOwner"));
		return false;
	}
	if (!InCombatSignalTargetPacket.Result.bAccepted)
	{
		FCombatFeedbackDebug::RecordHitFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("RejectedCombatSignal"));
		return false;
	}
	if (InCombatSignalTargetPacket.Result.CommittedDamage <= KINDA_SMALL_NUMBER)
	{
		FCombatFeedbackDebug::RecordHitFeedbackRequestRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("NoCommittedDamage"));
		return false;
	}

	return true;
}

bool UCHitFeedbackComponent::CanPlayHitStop(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (!GetWorld())
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("HitStop"), TEXT("InvalidWorld"));
		return false;
	}

	if (HitStopAudience == EFeedbackAudience::None)
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("HitStop"), TEXT("AudienceNone"));
		return false;
	}
	if (!FMath::IsFinite(HitStopDuration))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("HitStop"), TEXT("InvalidDuration"));
		return false;
	}
	if (!FMath::IsFinite(HitStopDilation))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("HitStop"), TEXT("InvalidDilation"));
		return false;
	}
	if (HitStopDuration <= KINDA_SMALL_NUMBER)
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("HitStop"), TEXT("InvalidDuration"));
		return false;
	}
	if (HitStopDilation < 0.f)
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("HitStop"), TEXT("InvalidDilation"));
		return false;
	}

	return true;
}

bool UCHitFeedbackComponent::CanPlayCameraShake(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (!GetWorld())
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("CameraShake"), TEXT("InvalidWorld"));
		return false;
	}
	if (!bEnableCameraShake)
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("CameraShake"), TEXT("Disabled"));
		return false;
	}
	
	if (CameraShakeAudience == EFeedbackAudience::None)
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("CameraShake"), TEXT("AudienceNone"));
		return false;
	}
	if (!IsValid(CameraShakeClass))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("CameraShake"), TEXT("InvalidClass"));
		return false;
	}
	if (!FMath::IsFinite(CameraShakeBaseScale))
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("CameraShake"), TEXT("InvalidScale"));
		return false;
	}
	if (CameraShakeBaseScale <= KINDA_SMALL_NUMBER)
	{
		FCombatFeedbackDebug::RecordHitFeedbackPresentationRejectedForAudit(OwnerCharacter_Injected, this, InCombatSignalTargetPacket, TEXT("CameraShake"), TEXT("InvalidScale"));
		return false;
	}

	return true;
}

// Resolve

FVector UCHitFeedbackComponent::ResolveHitFeedbackLocation(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	const FHitImpactContext& damageHitInfo = InCombatSignalTargetPacket.Context.HitImpactContext;

	if (damageHitInfo.bHasHitResult)
	{
		return damageHitInfo.HitResult.ImpactPoint;
	}

	return IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->GetActorLocation() : FVector::ZeroVector;
}

FRotator UCHitFeedbackComponent::ResolveHitFeedbackRotation(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	const FHitImpactContext& damageHitInfo = InCombatSignalTargetPacket.Context.HitImpactContext;

	if (damageHitInfo.bHasHitResult)
	{
		return damageHitInfo.HitResult.ImpactNormal.Rotation();
	}

	return IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->GetActorRotation() : FRotator::ZeroRotator;
}

// Request

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
