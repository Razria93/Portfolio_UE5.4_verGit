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

void UCHitFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor_Cached = GetOwner();
	check(OwnerActor_Cached);

	OwnerCharacter_Cached = Cast<ACharacter>(OwnerActor_Cached);
	check(OwnerCharacter_Cached);
}

void UCHitFeedbackComponent::PlayHitFeedback(const FCombatSignalTargetPacket& InTakeDamagePacket)
{
	if (!CanPlayHitFeedback(InTakeDamagePacket)) return;

	// PrintHitInfo(InTakeDamagePacket);

	PlayHitStop(InTakeDamagePacket);
	PlayHitVFX(InTakeDamagePacket);
	PlayHitSFX(InTakeDamagePacket);
	PlayCameraShake(InTakeDamagePacket);
}

void UCHitFeedbackComponent::PlayHitStop(const FCombatSignalTargetPacket& InTakeDamagePacket)
{
	if (!CanPlayHitStop(InTakeDamagePacket)) return;

	UCWorldSubsystem_CombatFeedback* feedbackSubsystem = GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>();
	if (!IsValid(feedbackSubsystem)) return;

	const FHitStopRequest hitStopRequest = BuildHitStopRequest(InTakeDamagePacket);

	// FLog::Log(TEXT("[UCHitFeedbackComponent] Play HitStop"));
	// PrintHitStopRequestInfo(hitStopRequest);

	feedbackSubsystem->RequestHitStop(hitStopRequest);
}

void UCHitFeedbackComponent::PlayHitVFX(const FCombatSignalTargetPacket& InTakeDamagePacket)
{
	if (!IsValid(GetWorld())) return;
	if (!IsValid(OwnerActor_Cached)) return;

	if (!IsValid(HitVFX))
	{
		FLog::Log(TEXT("[UCHitFeedbackComponent] Invalid HitVFX."));
		return;
	}

	const FVector location = ResolveHitFeedbackLocation(InTakeDamagePacket);
	const FRotator rotation = ResolveHitFeedbackRotation(InTakeDamagePacket);

	// FLog::Log(TEXT("[UCHitFeedbackComponent] Play HitVFX"));
	// PrintHitVFXRequestInfo(HitVFX, location, rotation);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitVFX, location, rotation);
}

void UCHitFeedbackComponent::PlayHitSFX(const FCombatSignalTargetPacket& InTakeDamagePacket)
{
	if (!IsValid(OwnerActor_Cached)) return;

	if (!IsValid(HitSFX))
	{
		FLog::Log(TEXT("[UCHitFeedbackComponent] Invalid HitSFX."));
		return;
	}

	const FVector location = ResolveHitFeedbackLocation(InTakeDamagePacket);

	// FLog::Log(TEXT("[UCHitFeedbackComponent] Play HitSFX"));
	// PrintHitSFXRequestInfo(HitSFX, location);

	UGameplayStatics::PlaySoundAtLocation(this, HitSFX, location);
}

void UCHitFeedbackComponent::PlayCameraShake(const FCombatSignalTargetPacket& InTakeDamagePacket)
{
	if (!CanPlayCameraShake(InTakeDamagePacket)) return;

	UCWorldSubsystem_CombatFeedback* feedbackSubsystem = GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>();
	if (!IsValid(feedbackSubsystem)) return;

	const FCameraShakeRequest cameraShakeRequest = BuildCameraShakeRequest(InTakeDamagePacket);

	// FLog::Log(TEXT("[UCHitFeedbackComponent] PlayCameraShake"));
	// PrintCameraShakeRequestInfo(cameraShakeRequest);

	feedbackSubsystem->RequestCameraShake(cameraShakeRequest);
}

bool UCHitFeedbackComponent::CanPlayHitFeedback(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	if (!IsValid(OwnerActor_Cached)) return false;
	if (!InTakeDamagePacket.Result.bAccepted) return false;
	if (InTakeDamagePacket.Result.CommittedDamage <= KINDA_SMALL_NUMBER) return false;

	return true;
}

bool UCHitFeedbackComponent::CanPlayHitStop(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	if (!GetWorld()) return false;

	if (HitStopAudience == EFeedbackAudience::None) return false;
	if (!FMath::IsFinite(HitStopDuration)) return false;
	if (!FMath::IsFinite(HitStopDilation)) return false;
	if (HitStopDuration <= KINDA_SMALL_NUMBER) return false;
	if (HitStopDilation < 0.f) return false;

	return true;
}

bool UCHitFeedbackComponent::CanPlayCameraShake(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	if (!GetWorld()) return false;
	if (!bEnableCameraShake) return false;
	
	if (CameraShakeAudience == EFeedbackAudience::None) return false;
	if (!IsValid(CameraShakeClass)) return false;
	if (!FMath::IsFinite(CameraShakeBaseScale)) return false;
	if (CameraShakeBaseScale <= KINDA_SMALL_NUMBER) return false;

	return true;
}

FVector UCHitFeedbackComponent::ResolveHitFeedbackLocation(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	const FDamageImpactInfo& damageHitInfo = InTakeDamagePacket.Context.DamageImpactInfo;

	if (damageHitInfo.bHasHitResult)
	{
		return damageHitInfo.HitResult.ImpactPoint;
	}

	return IsValid(OwnerActor_Cached) ? OwnerActor_Cached->GetActorLocation() : FVector::ZeroVector;
}

FRotator UCHitFeedbackComponent::ResolveHitFeedbackRotation(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	const FDamageImpactInfo& damageHitInfo = InTakeDamagePacket.Context.DamageImpactInfo;

	if (damageHitInfo.bHasHitResult)
	{
		return damageHitInfo.HitResult.ImpactNormal.Rotation();
	}

	return IsValid(OwnerActor_Cached) ? OwnerActor_Cached->GetActorRotation() : FRotator::ZeroRotator;
}

FHitStopRequest UCHitFeedbackComponent::BuildHitStopRequest(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	FHitStopRequest hitStopRequest;

	hitStopRequest.HitStopAudience = HitStopAudience;
	hitStopRequest.HitStopDuration = HitStopDuration;
	hitStopRequest.HitStopDilation = HitStopDilation;
	hitStopRequest.SourceActor = InTakeDamagePacket.Context.SourceActor;
	hitStopRequest.TargetActor = InTakeDamagePacket.Context.TargetActor;

	return hitStopRequest;
}

FCameraShakeRequest UCHitFeedbackComponent::BuildCameraShakeRequest(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	FCameraShakeRequest cameraShakeRequest;

	cameraShakeRequest.CameraShakeClass = CameraShakeClass;
	cameraShakeRequest.CameraShakeBaseScale = CameraShakeBaseScale;
	cameraShakeRequest.CameraShakeAudience = CameraShakeAudience;
	cameraShakeRequest.SourceActor = InTakeDamagePacket.Context.SourceActor;
	cameraShakeRequest.TargetActor = InTakeDamagePacket.Context.TargetActor;
	cameraShakeRequest.EventLocation = IsValid(InTakeDamagePacket.Context.TargetActor) ? InTakeDamagePacket.Context.TargetActor->GetActorLocation() : GetOwner()->GetActorLocation();

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
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(OwnerActor_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: X=%.2f Y=%.2f Z=%.2f"), TEXT("Location"), InLocation.X, InLocation.Y, InLocation.Z));
	FLog::Log(FString::Printf(TEXT("%-20s: P=%.2f Y=%.2f R=%.2f"), TEXT("Rotation"), InRotation.Pitch, InRotation.Yaw, InRotation.Roll));
	FLog::Log(TEXT("================================="));
}

void UCHitFeedbackComponent::PrintHitSFXRequestInfo(USoundBase* InHitSFX, const FVector& InLocation) const
{
	FLog::Log(TEXT("========= HitSFX Info ========="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Asset"), *GetNameSafe(InHitSFX)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(OwnerActor_Cached)));
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

void UCHitFeedbackComponent::PrintHitInfo(const FCombatSignalTargetPacket& InTakeDamagePacket) const
{
	if (!InTakeDamagePacket.Context.DamageImpactInfo.bHasHitResult)
	{
		FLog::Log(TEXT("[HitFeedback] HitInfo: None"));
		return;
	}

	const FHitResult& hitResult = InTakeDamagePacket.Context.DamageImpactInfo.HitResult;

	FLog::Log(TEXT("======== Damage Hit Info ========"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ImpactPoint"), *hitResult.ImpactPoint.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ImpactNormal"), *hitResult.ImpactNormal.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Location"), *hitResult.Location.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Normal"), *hitResult.Normal.ToCompactString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("BoneName"), *hitResult.BoneName.ToString()));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Component"), *GetNameSafe(hitResult.GetComponent())));
	FLog::Log(TEXT("================================="));
}
