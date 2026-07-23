#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

#include "GameFramework/Actor.h"

#include "Core/Debug/FCombatFeedbackDebug.h"
#include "Type/CCombatFeedbackTypes.h"

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
	FCombatFeedbackDebug::RecordCombatFeedbackHitStopRequestedForAudit(InHitStopRequest);

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
		FCombatFeedbackDebug::RecordCombatFeedbackRequestRejectedForAudit(TEXT("HitStop"), TEXT("UnsupportedAudience"));
		break;
	}
}

void UCWorldSubsystem_CombatFeedback::RequestCameraShake(const FCameraShakeRequest& InCameraShakeRequest)
{
	FCombatFeedbackDebug::RecordCombatFeedbackCameraShakeRequestedForAudit(InCameraShakeRequest);
	OnCameraShakeRequested.Broadcast(InCameraShakeRequest);
}

// HitStop

void UCWorldSubsystem_CombatFeedback::ApplyHitStop(AActor* InActor, float InDuration, float InDilation)
{
	if (!IsValid(InActor))
	{
		FCombatFeedbackDebug::RecordCombatFeedbackHitStopRejectedForAudit(InActor, InDuration, InDilation, TEXT("InvalidActor"));
		return;
	}
	if (InDuration <= 0.f)
	{
		FCombatFeedbackDebug::RecordCombatFeedbackHitStopRejectedForAudit(InActor, InDuration, InDilation, TEXT("InvalidDuration"));
		return;
	}

	const TWeakObjectPtr<AActor> actorKey(InActor);

	if (!CachedTimeDilationMap.Contains(actorKey))
	{
		CachedTimeDilationMap.Add(actorKey, InActor->CustomTimeDilation);
	}

	// Slow InActor
	InActor->CustomTimeDilation = InDilation;
	FCombatFeedbackDebug::RecordCombatFeedbackHitStopAppliedForAudit(InActor, InDuration, InDilation);

	if (FTimerHandle* existingHandle = ActiveHitStopMap.Find(actorKey))
	{
		GetWorld()->GetTimerManager().ClearTimer(*existingHandle);
	}

	FTimerHandle handle;
	FTimerDelegate delegate = FTimerDelegate::CreateUObject(this, &UCWorldSubsystem_CombatFeedback::RestoreHitStop, actorKey);

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
