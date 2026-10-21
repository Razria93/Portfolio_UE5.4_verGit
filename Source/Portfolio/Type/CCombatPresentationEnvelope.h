#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "CCombatPresentationEnvelope.generated.h"

/** A normalized 0..1 presentation weight; callers choose the clock explicitly. */
USTRUCT(BlueprintType)
struct PORTFOLIO_API FCombatPresentationEnvelope
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Envelope", meta = (ClampMin = "0", Units = "s"))
	float Delay = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Envelope", meta = (ClampMin = "0", Units = "s"))
	float FadeIn = .08f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Envelope", meta = (ClampMin = "0", Units = "s"))
	float Hold = .15f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Envelope", meta = (ClampMin = "0", Units = "s"))
	float FadeOut = .18f;
	// Optional progress remapping: X and Y in [0,1]. Endpoints are enforced.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Envelope")
	TObjectPtr<UCurveFloat> BlendCurve = nullptr;

	bool IsValid() const;
	float Blend(float InProgress) const;
	double Duration() const { return double(Delay) + FadeIn + Hold + FadeOut; }
	float Evaluate(double InElapsed, bool bSustain = false) const;
};

/** Per-owner playback state. Never store this mutable state on a shared Notify. */
USTRUCT()
struct PORTFOLIO_API FCombatPresentationEnvelopeState
{
	GENERATED_BODY()
	UPROPERTY(Transient)
	FCombatPresentationEnvelope Profile;
	double StartedAt = 0.;
	double ReleasedAt = 0.;
	float ReleasedWeight = 0.f;
	bool bRunning = false;
	bool bSustain = false;
	bool bReleased = false;

	bool Start(const FCombatPresentationEnvelope& InProfile, double InNow, bool bInSustain = false);
	void Release(double InNow);
	float Evaluate(double InNow) const;
	bool IsFinished(double InNow) const;
	void Reset() { *this = FCombatPresentationEnvelopeState(); }
};
