#include "Component/CHitFeedbackComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"

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

	// PrintHitInfo(InCombatSignalTargetPacket);

	PlayHitStop(InCombatSignalTargetPacket);
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

	// FLog::Log(TEXT("[UCHitFeedbackComponent] Play HitStop"));
	// PrintHitStopRequestInfo(hitStopRequest);

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

	// FLog::Log(TEXT("[UCHitFeedbackComponent] Play HitVFX"));
	// PrintHitVFXRequestInfo(HitVFX, location, rotation);

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

	// FLog::Log(TEXT("[UCHitFeedbackComponent] Play HitSFX"));
	// PrintHitSFXRequestInfo(HitSFX, location);

	UGameplayStatics::PlaySoundAtLocation(this, HitSFX, location);
}

void UCHitFeedbackComponent::PlayCameraShake(const FCombatSignalTargetPacket& InCombatSignalTargetPacket)
{
	if (!CanPlayCameraShake(InCombatSignalTargetPacket)) return;

	UCWorldSubsystem_CombatFeedback* feedbackSubsystem = GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>();
	if (!IsValid(feedbackSubsystem)) return;

	const FCameraShakeRequest cameraShakeRequest = BuildCameraShakeRequest(InCombatSignalTargetPacket);

	// FLog::Log(TEXT("[UCHitFeedbackComponent] PlayCameraShake"));
	// PrintCameraShakeRequestInfo(cameraShakeRequest);

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

void UCHitFeedbackComponent::PrintHitStopRequestInfo(const FHitStopRequest& InHitStopRequest) const
{
	FLog::Log(TEXT("====== HitStop Request Info ====="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("HitStopAudience"), *UEnum::GetValueAsString(InHitStopRequest.HitStopAudience)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("SourceActor"), *GetNameSafe(InHitStopRequest.SourceActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InHitStopRequest.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("HitStopDuration"), InHitStopRequest.HitStopDuration));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("HitStopDilation"), InHitStopRequest.HitStopDilation));
	FLog::Log(TEXT("================================="));
}

void UCHitFeedbackComponent::PrintHitVFXRequestInfo(UNiagaraSystem* InHitVFX, const FVector& InLocation, const FRotator& InRotation) const
{
	FLog::Log(TEXT("========== HitVFX Info =========="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Asset"), *GetNameSafe(InHitVFX)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerCharacter"), *GetNameSafe(OwnerCharacter_Injected)));
	FLog::Log(FString::Printf(TEXT("%-20s: X=%.2f Y=%.2f Z=%.2f"), TEXT("Location"), InLocation.X, InLocation.Y, InLocation.Z));
	FLog::Log(FString::Printf(TEXT("%-20s: P=%.2f Y=%.2f R=%.2f"), TEXT("Rotation"), InRotation.Pitch, InRotation.Yaw, InRotation.Roll));
	FLog::Log(TEXT("================================="));
}

void UCHitFeedbackComponent::PrintHitSFXRequestInfo(USoundBase* InHitSFX, const FVector& InLocation) const
{
	FLog::Log(TEXT("========= HitSFX Info ========="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Asset"), *GetNameSafe(InHitSFX)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerCharacter"), *GetNameSafe(OwnerCharacter_Injected)));
	FLog::Log(FString::Printf(TEXT("%-20s: X = %.2f Y = %.2f Z = %.2f"), TEXT("Location"), InLocation.X, InLocation.Y, InLocation.Z));
	FLog::Log(TEXT("================================="));
}

void UCHitFeedbackComponent::PrintCameraShakeRequestInfo(const FCameraShakeRequest& InCameraShakeRequest) const
{
	FLog::Log(TEXT("=== CameraShake Request Info ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Audience"), *UEnum::GetValueAsString(InCameraShakeRequest.CameraShakeAudience)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Class"), *GetNameSafe(InCameraShakeRequest.CameraShakeClass)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("SourceActor"), *GetNameSafe(InCameraShakeRequest.SourceActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InCameraShakeRequest.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.2f"), TEXT("CameraShakeBaseScale"), InCameraShakeRequest.CameraShakeBaseScale));
	FLog::Log(TEXT("================================="));
}

void UCHitFeedbackComponent::PrintHitInfo(const FCombatSignalTargetPacket& InCombatSignalTargetPacket) const
{
	if (!InCombatSignalTargetPacket.Context.DamageImpactInfo.bHasHitResult)
	{
		FLog::Log(TEXT("[HitFeedback] HitInfo: None"));
		return;
	}

	const FHitResult& hitResult = InCombatSignalTargetPacket.Context.DamageImpactInfo.HitResult;

	FLog::Log(TEXT("======== Damage Hit Info ========"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ImpactPoint"), *hitResult.ImpactPoint.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ImpactNormal"), *hitResult.ImpactNormal.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Location"), *hitResult.Location.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Normal"), *hitResult.Normal.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("BoneName"), *hitResult.BoneName.ToString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Component"), *GetNameSafe(hitResult.GetComponent())));
	FLog::Log(TEXT("================================="));
}
