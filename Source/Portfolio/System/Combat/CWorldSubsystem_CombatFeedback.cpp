#include "System/Combat/CWorldSubsystem_CombatFeedback.h"
#include "ProjectGlobal.h"

#include "GameFramework/Actor.h"

#include "Type/CWorldSubSystemStructure.h"

void UCWorldSubsystem_CombatFeedback::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCWorldSubsystem_CombatFeedback::Deinitialize()
{
	ClearFeedbackRuntimeState();

	Super::Deinitialize();
}

// Request

void UCWorldSubsystem_CombatFeedback::RequestHitStop(const FHitStopRequest& InHitStopRequest)
{
	// FLog::Log(TEXT("[UCWorldSubsystem_CombatFeedback] Request HitStop"));

	switch (InHitStopRequest.HitStopAudience)
	{
	case EFeedbackAudience::Source:
	{
		ApplyHitStop(InHitStopRequest.SourceActor, InHitStopRequest.HitStopDuration, InHitStopRequest.HitStopDilation);
		break;
	}

	case EFeedbackAudience::Target:
	{
		ApplyHitStop(InHitStopRequest.TargetActor, InHitStopRequest.HitStopDuration, InHitStopRequest.HitStopDilation);
		break;
	}

	case EFeedbackAudience::Both:
	{
		ApplyHitStop(InHitStopRequest.SourceActor, InHitStopRequest.HitStopDuration, InHitStopRequest.HitStopDilation);
		ApplyHitStop(InHitStopRequest.TargetActor, InHitStopRequest.HitStopDuration, InHitStopRequest.HitStopDilation);
		break;
	}

	default:
		break;
	}
}

void UCWorldSubsystem_CombatFeedback::RequestCameraShake(const FCameraShakeRequest& InCameraShakeRequest)
{
	OnCameraShakeRequested.Broadcast(InCameraShakeRequest);
}

// HitStop

void UCWorldSubsystem_CombatFeedback::ApplyHitStop(AActor* InActor, float InDuration, float InDilation)
{
	if (!IsValid(InActor)) return;
	if (InDuration <= 0.f) return;

	const TWeakObjectPtr<AActor> actorKey(InActor);

	if (!CachedTimeDilationMap.Contains(actorKey))
	{
		CachedTimeDilationMap.Add(actorKey, InActor->CustomTimeDilation);
	}

	// Slow InActor
	InActor->CustomTimeDilation = InDilation;

	if (FTimerHandle* existingHandle = ActiveHitStopMap.Find(actorKey))
	{
		GetWorld()->GetTimerManager().ClearTimer(*existingHandle);
	}

	FTimerHandle handle;
	FTimerDelegate delegate = FTimerDelegate::CreateUObject(this, &UCWorldSubsystem_CombatFeedback::RestoreHitStop, actorKey);

	// FLog::Log(TEXT("[UCWorldSubsystem_CombatFeedback] ApplyHitStop"));
	// PrintHitStopConsumeInfo(InActor, InDuration, InDilation);

	GetWorld()->GetTimerManager().SetTimer(handle, delegate, InDuration, false);
	ActiveHitStopMap.Add(actorKey, handle);
}

void UCWorldSubsystem_CombatFeedback::RestoreHitStop(TWeakObjectPtr<AActor> InActorKey)
{
	AActor* InActor = InActorKey.Get();
	if (IsValid(InActor))
	{
		const float* cachedDilation = CachedTimeDilationMap.Find(InActorKey);
		InActor->CustomTimeDilation = cachedDilation ? *cachedDilation : 1.f;
	}

	// FLog::Log(TEXT("[UCWorldSubsystem_CombatFeedback] RestoreHitStop"));
	// PrintHitStopConsumeInfo(InActor, 1.f, 0.f);

	// Restore InActor
	ActiveHitStopMap.Remove(InActorKey);
	CachedTimeDilationMap.Remove(InActorKey);
}

void UCWorldSubsystem_CombatFeedback::ClearHitStop()
{
	if (UWorld* world = GetWorld())
	{
		FTimerManager& timerManager = world->GetTimerManager();

		for (const TPair<TWeakObjectPtr<AActor>, FTimerHandle>& pair : ActiveHitStopMap)
		{
			FTimerHandle timerHandle = pair.Value;
			timerManager.ClearTimer(timerHandle);
		}
	}

	for (const TPair<TWeakObjectPtr<AActor>, float>& pair : CachedTimeDilationMap)
	{
		AActor* actor = pair.Key.Get();
		if (!IsValid(actor)) continue;

		actor->CustomTimeDilation = pair.Value;
	}

	ActiveHitStopMap.Reset();
	CachedTimeDilationMap.Reset();
}

// CameraShake

void UCWorldSubsystem_CombatFeedback::ClearCameraShake()
{
	OnCameraShakeRequested.Clear();
}

// Runtime State

void UCWorldSubsystem_CombatFeedback::ClearFeedbackRuntimeState()
{
	ClearHitStop();
	ClearCameraShake();
}

// Debug

void UCWorldSubsystem_CombatFeedback::PrintHitStopConsumeInfo(AActor* InActor, float InDuration, float InDilation) const
{
	FLog::Log(TEXT("===== HitStop Consume Info ======"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Actor"), *GetNameSafe(InActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("Duration"), InDuration));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("Dilation"), InDilation));
	FLog::Log(TEXT("================================="));
}
