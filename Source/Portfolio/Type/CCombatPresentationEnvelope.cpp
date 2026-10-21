#include "Type/CCombatPresentationEnvelope.h"

bool FCombatPresentationEnvelope::IsValid() const
{
	return FMath::IsFinite(Delay) && Delay >= 0.f
		&& FMath::IsFinite(FadeIn) && FadeIn >= 0.f
		&& FMath::IsFinite(Hold) && Hold >= 0.f
		&& FMath::IsFinite(FadeOut) && FadeOut >= 0.f;
}

float FCombatPresentationEnvelope::Blend(float InProgress) const
{
	if (!FMath::IsFinite(InProgress) || InProgress <= 0.f) return 0.f;
	if (InProgress >= 1.f) return 1.f;
	const float value = BlendCurve ? BlendCurve->GetFloatValue(InProgress)
		: InProgress * InProgress * (3.f - 2.f * InProgress);
	return FMath::IsFinite(value) ? FMath::Clamp(value, 0.f, 1.f) : InProgress;
}

float FCombatPresentationEnvelope::Evaluate(double InElapsed, bool bSustain) const
{
	if (!IsValid() || !FMath::IsFinite(InElapsed) || InElapsed < Delay) return 0.f;
	double t = InElapsed - Delay;
	if (t < FadeIn) return Blend(float(t / FadeIn));
	t -= FadeIn;
	if (bSustain || t < Hold) return 1.f;
	t -= Hold;
	return FadeOut > 0.f && t < FadeOut ? 1.f - Blend(float(t / FadeOut)) : 0.f;
}

bool FCombatPresentationEnvelopeState::Start(const FCombatPresentationEnvelope& InProfile, double InNow, bool bInSustain)
{
	Reset();
	if (!InProfile.IsValid() || !FMath::IsFinite(InNow)) return false;
	Profile = InProfile;
	StartedAt = InNow;
	bSustain = bInSustain;
	bRunning = true;
	return true;
}

void FCombatPresentationEnvelopeState::Release(double InNow)
{
	if (!bRunning || bReleased || !FMath::IsFinite(InNow)) return;
	ReleasedWeight = Evaluate(InNow);
	ReleasedAt = InNow;
	bReleased = true;
}

float FCombatPresentationEnvelopeState::Evaluate(double InNow) const
{
	if (!bRunning || !FMath::IsFinite(InNow)) return 0.f;
	if (!bReleased) return Profile.Evaluate(InNow - StartedAt, bSustain);
	if (Profile.FadeOut <= 0.f) return 0.f;
	return ReleasedWeight * (1.f - Profile.Blend(float((InNow - ReleasedAt) / Profile.FadeOut)));
}

bool FCombatPresentationEnvelopeState::IsFinished(double InNow) const
{
	if (!bRunning || !FMath::IsFinite(InNow)) return true;
	if (bReleased) return InNow >= ReleasedAt + Profile.FadeOut;
	return !bSustain && InNow >= StartedAt + Profile.Duration();
}
