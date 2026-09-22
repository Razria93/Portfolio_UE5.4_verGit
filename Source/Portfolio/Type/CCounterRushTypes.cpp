#include "Type/CCounterRushTypes.h"

bool FCounterRushTuning::IsValid() const
{
	return FMath::IsFinite(Duration) && Duration > 0.f
		&& FMath::IsFinite(MaxTravelDistance) && MaxTravelDistance >= 0.f
		&& FMath::IsFinite(ExtraGap) && ExtraGap >= 0.f;
}

FCounterRushPlan FCounterRushPlan::Build(const FVector& InOrigin, const FVector& InTarget,
	float InPlayerRadius, float InTargetRadius, const FCounterRushTuning& InTuning)
{
	FCounterRushPlan plan;
	if (!InTuning.IsValid() || InOrigin.ContainsNaN() || InTarget.ContainsNaN()
		|| !FMath::IsFinite(InPlayerRadius) || InPlayerRadius < 0.f
		|| !FMath::IsFinite(InTargetRadius) || InTargetRadius < 0.f) return plan;
	plan.Origin = InOrigin;
	plan.Target = InTarget;
	plan.PlayerRadius = InPlayerRadius;
	plan.TargetRadius = InTargetRadius;
	plan.CenterSpacing = InPlayerRadius + InTargetRadius + InTuning.ExtraGap;
	if (!FMath::IsFinite(plan.CenterSpacing)) return FCounterRushPlan();
	FVector delta = InTarget - InOrigin;
	delta.Z = 0.;
	const double distance = delta.Size();
	if (!FMath::IsFinite(distance)) return FCounterRushPlan();
	plan.Direction = delta.GetSafeNormal();
	const double desiredTravel = FMath::Max(0., distance - plan.CenterSpacing);
	plan.DesiredEnd = InOrigin + plan.Direction * desiredTravel;
	plan.TravelDistance = float(FMath::Min(desiredTravel, double(InTuning.MaxTravelDistance)));
	plan.LimitedEnd = InOrigin + plan.Direction * plan.TravelDistance;
	plan.bValid = true;
	return plan;
}
