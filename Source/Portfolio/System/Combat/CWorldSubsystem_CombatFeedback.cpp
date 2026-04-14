#include "System/Combat/CWorldSubsystem_CombatFeedback.h"
#include "ProjectGlobal.h"

#include "GameFramework/Actor.h"

#include "Type/CWorldSubSystemStructure.h"

void UCWorldSubsystem_CombatFeedback::RequestHitStop(const FHitStopRequest& InHitStopRequest)
{
	FLog::Log(TEXT("[UCWorldSubsystem_CombatFeedback] Request HitStop"));

	switch (InHitStopRequest.HitStopAudience)
	{
	case EFeedbackAudience::Source:
		ApplyHitStop(InHitStopRequest.SourceActor, InHitStopRequest.HitStopDuration, InHitStopRequest.HitStopDilation);
		break;

	case EFeedbackAudience::Target:
		ApplyHitStop(InHitStopRequest.TargetActor, InHitStopRequest.HitStopDuration, InHitStopRequest.HitStopDilation);
		break;

	case EFeedbackAudience::Both:
		ApplyHitStop(InHitStopRequest.SourceActor, InHitStopRequest.HitStopDuration, InHitStopRequest.HitStopDilation);
		ApplyHitStop(InHitStopRequest.TargetActor, InHitStopRequest.HitStopDuration, InHitStopRequest.HitStopDilation);
		break;

	default:
		break;
	}
}

void UCWorldSubsystem_CombatFeedback::RequestCameraShake(const FCameraShakeRequest& InCameraShakeRequest)
{
	OnCameraShakeRequested.Broadcast(InCameraShakeRequest);
}

void UCWorldSubsystem_CombatFeedback::ApplyHitStop(AActor* InActor, float InDuration, float InDilation)
{
	if (!IsValid(InActor)) return;
	if (InDuration <= 0.f) return;

	if (!CachedTimeDilationMap.Contains(InActor))
	{
		CachedTimeDilationMap.Add(InActor, InActor->CustomTimeDilation);
	}

	// Slow InActor
	InActor->CustomTimeDilation = InDilation;

	if (FTimerHandle* existingHandle = ActiveHitStopMap.Find(InActor))
	{
		GetWorld()->GetTimerManager().ClearTimer(*existingHandle);
	}

	FTimerHandle handle;
	FTimerDelegate delegate = FTimerDelegate::CreateUObject(this, &UCWorldSubsystem_CombatFeedback::RestoreHitStop, InActor);

	FLog::Log(TEXT("[UCWorldSubsystem_CombatFeedback] ApplyHitStop"));
	PrintHitStopConsumeInfo(InActor, InDuration, InDilation);

	GetWorld()->GetTimerManager().SetTimer(handle, delegate, InDuration, false);
	ActiveHitStopMap.Add(InActor, handle);
}

void UCWorldSubsystem_CombatFeedback::RestoreHitStop(AActor* InActor)
{
	if (IsValid(InActor))
	{
		const float* cachedDilation = CachedTimeDilationMap.Find(InActor);
		InActor->CustomTimeDilation = cachedDilation ? *cachedDilation : 1.f;
	}

	FLog::Log(TEXT("[UCWorldSubsystem_CombatFeedback] RestoreHitStop"));
	PrintHitStopConsumeInfo(InActor, 1.f, 0.f);

	// Restore InActor
	ActiveHitStopMap.Remove(InActor);
	CachedTimeDilationMap.Remove(InActor);
}

void UCWorldSubsystem_CombatFeedback::PrintHitStopConsumeInfo(AActor* InActor, float InDuration, float InDilation) const
{
	FLog::Log(TEXT("===== HitStop Consume Info ======"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Actor"), *GetNameSafe(InActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("Duration"), InDuration));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("Dilation"), InDilation));
	FLog::Log(TEXT("================================="));
}
