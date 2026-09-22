#include "System/Combat/CWorldSubsystem_CombatFeedback.h"

#include "Core/Debug/FCombatFeedbackDebug.h"
#include "Type/CCombatFeedbackTypes.h"

#include "GameFramework/Actor.h"

// Lifecycle

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
	AddTimeEffect(InActor, InDuration, InDilation, false, 0);
}

void UCWorldSubsystem_CombatFeedback::AddTimeEffect(AActor* InActor, float InDuration, float InDilation, bool bRealTime, uint64 InSerial)
{
	if (!IsValid(InActor) || !GetWorld() || !FMath::IsFinite(InDuration) || InDuration <= 0.f
		|| !FMath::IsFinite(InDilation) || InDilation < 0.f || InDilation > 1.f) return;
	RefreshTimeEffects();
	FActorTimeEffects* state = ActorTimeEffects.Find(InActor);
	if (!state)
	{
		state = &ActorTimeEffects.Add(InActor);
		state->Baseline = InActor->CustomTimeDilation;
	}
	FTimeEffect effect;
	effect.Dilation = InDilation;
	effect.ExpiresAt = (bRealTime ? FPlatformTime::Seconds() : GetWorld()->GetTimeSeconds()) + InDuration;
	effect.bRealTime = bRealTime;
	effect.OwnerSerial = InSerial;
	state->Effects.Add(effect);
	RefreshTimeEffects();
}

uint64 UCWorldSubsystem_CombatFeedback::RequestPerfectDodgeSlow(AActor* InPlayer, AActor* InAttacker, float InDuration, float InDilation)
{
	const uint64 serial = ++NextSlowSerial;
	AddTimeEffect(InPlayer, InDuration, InDilation, true, serial);
	if (InAttacker != InPlayer) AddTimeEffect(InAttacker, InDuration, InDilation, true, serial);
	return serial;
}

void UCWorldSubsystem_CombatFeedback::CancelPerfectDodgeSlow(uint64 InSerial)
{
	if (InSerial == 0) return;
	for (auto& pair : ActorTimeEffects)
		pair.Value.Effects.RemoveAll([InSerial](const FTimeEffect& InEffect) { return InEffect.OwnerSerial == InSerial; });
	RefreshTimeEffects();
}

void UCWorldSubsystem_CombatFeedback::Tick(float InDeltaTime)
{
	RefreshTimeEffects();
}

void UCWorldSubsystem_CombatFeedback::RefreshTimeEffects()
{
	const double realNow = FPlatformTime::Seconds();
	const double gameNow = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.;
	for (auto it = ActorTimeEffects.CreateIterator(); it; ++it)
	{
		AActor* actor = it.Key().Get();
		if (!IsValid(actor)) { it.RemoveCurrent(); continue; }
		FActorTimeEffects& state = it.Value();
		state.Effects.RemoveAll([realNow, gameNow](const FTimeEffect& InEffect)
		{
			return InEffect.ExpiresAt <= (InEffect.bRealTime ? realNow : gameNow);
		});
		float dilation = state.Baseline;
		for (const FTimeEffect& effect : state.Effects) dilation = FMath::Min(dilation, effect.Dilation);
		actor->CustomTimeDilation = dilation;
		if (state.Effects.IsEmpty()) it.RemoveCurrent();
	}
}

void UCWorldSubsystem_CombatFeedback::ClearHitStop()
{
	for (const auto& pair : ActorTimeEffects)
	{
		AActor* actor = pair.Key.Get();
		if (IsValid(actor)) actor->CustomTimeDilation = pair.Value.Baseline;
	}
	ActorTimeEffects.Reset();
}

// Camera Shake

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
