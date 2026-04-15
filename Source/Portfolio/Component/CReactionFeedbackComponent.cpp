#include "Component/CReactionFeedbackComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"

#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

#include "Type/CWeaponStructure.h"

UCReactionFeedbackComponent::UCReactionFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCReactionFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor_Cached = GetOwner();
	check(OwnerActor_Cached);

	OwnerCharacter_Cached = Cast<ACharacter>(OwnerActor_Cached);
	check(OwnerCharacter_Cached);
}

void UCReactionFeedbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCReactionFeedbackComponent::PlayDamageFeedback(const FTakeDamagePacket& InTakeDamagePacket)
{
	if (!CanPlayDamageFeedback(InTakeDamagePacket)) return;

	PlayHitStop(InTakeDamagePacket);
	PlayHitVFX(InTakeDamagePacket);
	PlayHitSFX(InTakeDamagePacket);
	PlayCameraShake(InTakeDamagePacket);
}

void UCReactionFeedbackComponent::PlayHitStop(const FTakeDamagePacket& InTakeDamagePacket)
{
	if (!CanPlayHitStop(InTakeDamagePacket)) return;

	UCWorldSubsystem_CombatFeedback* feedbackSubsystem = GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>();
	if (!IsValid(feedbackSubsystem)) return;

	const FHitStopRequest hitStopRequest = BuildHitStopRequest(InTakeDamagePacket);

	FLog::Log(TEXT("[UCReactionFeedbackComponent] Play HitStop"));
	PrintHitStopRequestInfo(hitStopRequest);

	feedbackSubsystem->RequestHitStop(hitStopRequest);
}

void UCReactionFeedbackComponent::PlayHitVFX(const FTakeDamagePacket& InTakeDamagePacket)
{
	if (!IsValid(GetWorld())) return;
	if (!IsValid(OwnerActor_Cached)) return;

	if (!IsValid(HitVFX))
	{
		FLog::Log(TEXT("[UCReactionFeedbackComponent] Invalid HitVFX."));
		return;
	}

	const FVector location = OwnerActor_Cached->GetActorLocation();
	const FRotator rotation = OwnerActor_Cached->GetActorRotation();

	FLog::Log(TEXT("[UCReactionFeedbackComponent] Play HitVFX"));
	PrintHitVFXRequestInfo(HitVFX, location, rotation);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitVFX, location, rotation);
}

void UCReactionFeedbackComponent::PlayHitSFX(const FTakeDamagePacket& InTakeDamagePacket)
{
	if (!IsValid(OwnerActor_Cached)) return;

	if (!IsValid(HitSFX))
	{
		FLog::Log(TEXT("[UCReactionFeedbackComponent] Invalid HitSFX."));
		return;
	}

	const FVector location = OwnerActor_Cached->GetActorLocation();

	FLog::Log(TEXT("[UCReactionFeedbackComponent] Play HitSFX"));
	PrintHitSFXRequestInfo(HitSFX, location);

	UGameplayStatics::PlaySoundAtLocation(this, HitSFX, location);
}

void UCReactionFeedbackComponent::PlayCameraShake(const FTakeDamagePacket& InTakeDamagePacket)
{
	if (!CanPlayCameraShake(InTakeDamagePacket)) return;

	UCWorldSubsystem_CombatFeedback* feedbackSubsystem = GetWorld()->GetSubsystem<UCWorldSubsystem_CombatFeedback>();
	if (!IsValid(feedbackSubsystem)) return;

	const FCameraShakeRequest cameraShakeRequest = BuildCameraShakeRequest(InTakeDamagePacket);

	FLog::Log(TEXT("[UCReactionFeedbackComponent] PlayCameraShake"));
	PrintCameraShakeRequestInfo(cameraShakeRequest);

	feedbackSubsystem->RequestCameraShake(cameraShakeRequest);
}

bool UCReactionFeedbackComponent::CanPlayDamageFeedback(const FTakeDamagePacket& InTakeDamagePacket) const
{
	if (!IsValid(OwnerActor_Cached)) return false;
	if (!InTakeDamagePacket.Result.bAccepted) return false;
	if (InTakeDamagePacket.Result.CommittedDamage <= KINDA_SMALL_NUMBER) return false;

	return true;
}

bool UCReactionFeedbackComponent::CanPlayHitStop(const FTakeDamagePacket& InTakeDamagePacket) const
{
	if (!GetWorld()) return false;

	if (HitStopAudience == EFeedbackAudience::None) return false;
	if (!FMath::IsFinite(HitStopDuration)) return false;
	if (!FMath::IsFinite(HitStopDilation)) return false;
	if (HitStopDuration <= KINDA_SMALL_NUMBER) return false;
	if (HitStopDilation < 0.f) return false;

	return true;
}

bool UCReactionFeedbackComponent::CanPlayCameraShake(const FTakeDamagePacket& InTakeDamagePacket) const
{
	if (!GetWorld()) return false;
	if (!bEnableCameraShake) return false;
	
	if (CameraShakeAudience == EFeedbackAudience::None) return false;
	if (!IsValid(CameraShakeClass)) return false;
	if (!FMath::IsFinite(CameraShakeBaseScale)) return false;
	if (CameraShakeBaseScale <= KINDA_SMALL_NUMBER) return false;

	return true;
}

FHitStopRequest UCReactionFeedbackComponent::BuildHitStopRequest(const FTakeDamagePacket& InTakeDamagePacket) const
{
	FHitStopRequest hitStopRequest;

	hitStopRequest.HitStopAudience = HitStopAudience;
	hitStopRequest.HitStopDuration = HitStopDuration;
	hitStopRequest.HitStopDilation = HitStopDilation;
	hitStopRequest.SourceActor = InTakeDamagePacket.Context.SourceActor;
	hitStopRequest.TargetActor = InTakeDamagePacket.Context.TargetActor;

	return hitStopRequest;
}

FCameraShakeRequest UCReactionFeedbackComponent::BuildCameraShakeRequest(const FTakeDamagePacket& InTakeDamagePacket) const
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

void UCReactionFeedbackComponent::PrintHitStopRequestInfo(const FHitStopRequest& InHitStopRequest) const
{
	FLog::Log(TEXT("====== HitStop Request Info ====="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("HitStopAudience"), *UEnum::GetValueAsString(InHitStopRequest.HitStopAudience)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("SourceActor"), *GetNameSafe(InHitStopRequest.SourceActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InHitStopRequest.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("HitStopDuration"), InHitStopRequest.HitStopDuration));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("HitStopDilation"), InHitStopRequest.HitStopDilation));
	FLog::Log(TEXT("================================="));
}

void UCReactionFeedbackComponent::PrintHitVFXRequestInfo(UNiagaraSystem* InHitVFX, const FVector& InLocation, const FRotator& InRotation) const
{
	FLog::Log(TEXT("========== HitVFX Info =========="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Asset"), *GetNameSafe(InHitVFX)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(OwnerActor_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: X=%.2f Y=%.2f Z=%.2f"), TEXT("Location"), InLocation.X, InLocation.Y, InLocation.Z));
	FLog::Log(FString::Printf(TEXT("%-20s: P=%.2f Y=%.2f R=%.2f"), TEXT("Rotation"), InRotation.Pitch, InRotation.Yaw, InRotation.Roll));
	FLog::Log(TEXT("================================="));
}

void UCReactionFeedbackComponent::PrintHitSFXRequestInfo(USoundBase* InHitSFX, const FVector& InLocation) const
{
	FLog::Log(TEXT("========= HitSFX Info ========="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Asset"), *GetNameSafe(InHitSFX)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("OwnerActor"), *GetNameSafe(OwnerActor_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: X = %.2f Y = %.2f Z = %.2f"), TEXT("Location"), InLocation.X, InLocation.Y, InLocation.Z));
	FLog::Log(TEXT("================================="));
}

void UCReactionFeedbackComponent::PrintCameraShakeRequestInfo(const FCameraShakeRequest& InCameraShakeRequest) const
{
	FLog::Log(TEXT("=== CameraShake Request Info ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Audience"), *UEnum::GetValueAsString(InCameraShakeRequest.CameraShakeAudience)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Class"), *GetNameSafe(InCameraShakeRequest.CameraShakeClass)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("SourceActor"), *GetNameSafe(InCameraShakeRequest.SourceActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InCameraShakeRequest.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.2f"), TEXT("CameraShakeBaseScale"), InCameraShakeRequest.CameraShakeBaseScale));
	FLog::Log(TEXT("================================="));
}
