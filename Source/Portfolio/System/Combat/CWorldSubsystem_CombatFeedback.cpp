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
	const bool bHitStop = !bRealTime && InSerial == 0;
	const TCHAR* rejection = !IsValid(InActor) ? TEXT("InvalidActor")
		: !GetWorld() ? TEXT("InvalidWorld")
		: (!FMath::IsFinite(InDuration) || InDuration <= 0.f) ? TEXT("InvalidDuration")
		: (!FMath::IsFinite(InDilation) || InDilation < 0.f || InDilation > 1.f) ? TEXT("InvalidDilation") : nullptr;
	if (rejection)
	{
		if (bHitStop) FCombatFeedbackDebug::RecordCombatFeedbackHitStopRejectedForAudit(InActor, InDuration, InDilation, rejection);
		return;
	}
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
	// Emit once per accepted request, not once per refresh/tick. Slow leases are not HitStop requests.
	if (bHitStop) FCombatFeedbackDebug::RecordCombatFeedbackHitStopAppliedForAudit(InActor, InDuration, InDilation);
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

uint64 UCWorldSubsystem_CombatFeedback::RequestPerfectDodgeSlow(AActor* InPlayer, AActor* InAttacker,
	const FCombatPresentationEnvelope& InEnvelope, float InDilation)
{
	if (!InEnvelope.IsValid() || InEnvelope.Duration() <= 0. || !FMath::IsFinite(InDilation)
		|| InDilation < 0.f || InDilation > 1.f || (!IsValid(InPlayer) && !IsValid(InAttacker))) return 0;
	const uint64 serial = ++NextSlowSerial;
	AddEnvelopeTimeEffect(InPlayer, InEnvelope, InDilation, serial);
	if (InAttacker != InPlayer) AddEnvelopeTimeEffect(InAttacker, InEnvelope, InDilation, serial);
	return serial;
}

void UCWorldSubsystem_CombatFeedback::AddEnvelopeTimeEffect(AActor* InActor,
	const FCombatPresentationEnvelope& InEnvelope, float InDilation, uint64 InSerial)
{
	if (!IsValid(InActor) || !GetWorld()) return;
	RefreshTimeEffects();
	FActorTimeEffects* state = ActorTimeEffects.Find(InActor);
	if (!state)
	{
		state = &ActorTimeEffects.Add(InActor);
		state->Baseline = InActor->CustomTimeDilation;
	}
	FTimeEffect effect;
	effect.Dilation = InDilation;
	effect.bRealTime = effect.bEnvelope = true;
	effect.OwnerSerial = InSerial;
	const double now = FPlatformTime::Seconds();
	effect.ExpiresAt = now + InEnvelope.Duration();
	effect.Envelope.Start(InEnvelope, now);
	effect.CurveKeepAlive.Reset(InEnvelope.BlendCurve);
	state->Effects.Add(MoveTemp(effect));
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
		for (const FTimeEffect& effect : state.Effects)
		{
			const float requested = effect.bEnvelope
				? FMath::Lerp(state.Baseline, FMath::Min(state.Baseline, effect.Dilation), effect.Envelope.Evaluate(realNow))
				: effect.Dilation;
			dilation = FMath::Min(dilation, requested);
		}
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
