# Combat Feedback and Damage Impact Architecture Decision

## 1. Purpose

This document organizes the combat feedback layers and the damage impact metadata flow.

It reorganizes the archive documents `A06` and `A08` into a single decision flow.

The key point is to separate `DamageFeedback` and `ReactionFeedback`, and clarify where `FDamageImpactInfo` is created and consumed.

---

## 2. Background

In the previous feedback structure, multiple visual/audio responses during hit situations could easily look like one feedback flow.

Examples:

```text
hit VFX / hit SFX at the moment of damage
posture dust / stagger feedback during reaction montage
hit stop
camera shake
reaction start / completed / interrupted feedback
```

All of these may happen during combat, but their base context is different.

For example, hit VFX is naturally interpreted from damage event impact position, while posture dust during reaction montage is naturally interpreted from reaction lifecycle timing.

Therefore, feedback should be separated by the event/context that drives it, instead of being mixed into one component.

This document focuses on two questions.

```text
How should DamageFeedback and ReactionFeedback be separated?
Where should hit position metadata for DamageFeedback be created and delivered?
```

---

## 3. Decision

Combat feedback is separated into the following layers.

```text
ActionFeedback
-> feedback based on action execution context

ReactionFeedback
-> feedback based on reaction execution context

DamageFeedback
-> feedback based on take damage packet / damage impact metadata

PlayerFeedback
-> feedback based on local player presentation

CombatFeedbackSubsystem
-> world-level feedback execution support
```

In the reaction orchestration scope, separating `ReactionFeedback` and `DamageFeedback` is especially important.

```text
DamageFeedback
-> whether damage event was accepted / committed
-> where the hit occurred
-> how to execute hit VFX / hit SFX / hit stop / camera shake

ReactionFeedback
-> which reaction is currently running
-> which timing the reaction has reached
-> which expression matches reaction type / damage spec / trigger key
```

Therefore, sword hit VFX is closer to `DamageFeedback`, while posture break dust during hit reaction montage is closer to `ReactionFeedback`.

Summary:

```text
DamageFeedback
-> expression at the moment of damage event
-> hit impact point / damage accepted result / hit stop matter

ReactionFeedback
-> expression while reaction executor is running
-> reaction type / timing / trigger key / montage notify matter
```

Both may be related to being hit, but they are not the same responsibility.

---

## 4. DamageImpactInfo

`FDamageImpactInfo` is added to improve hit feedback location.

`FDamageImpactInfo` is not damage result.

It is impact metadata attached to a damage event.

Purpose:

```text
Send hit impact metadata with damage event
Allow DamageFeedback to play VFX / SFX at impact point instead of actor location
Allow TakeDamagePacket to carry both damage result and impact metadata
```

Current delivery flow:

```text
ACWeaponActor
-> FHitContext.DamageImpactInfo
-> FApplyDamagePayload
-> FApplyDamageContext
-> FDefaultDamageEvent
-> FTakeDamagePayload
-> FTakeDamageContext
-> FTakeDamagePacket.Context.DamageImpactInfo
-> UCDamageFeedbackComponent
```

Responsibilities:

```text
ACWeaponActor
-> creates DamageImpactInfo from overlap / sweep information

ApplyDamage / TakeDamage pipeline
-> does not calculate DamageImpactInfo, only transports it through payload/context/packet

UCDamageFeedbackComponent
-> consumes TakeDamagePacket.Context.DamageImpactInfo to resolve feedback location and rotation
```

If damage processing layers start recalculating collision metadata, ApplyDamage / TakeDamage responsibilities become unnecessarily large.

DamageImpactInfo should be created near hit detection, and damage pipeline should only deliver it.

---

## 5. Impact Point Calculation

Current melee hit detection is weapon collision overlap based.

In overlap-based structure, `SweepResult` from `OnComponentBeginOverlap()` does not always provide a meaningful impact point.

Especially when weapon actor moves through socket / attachment / animation transform, `bFromSweep == false` can naturally happen.

Current implementation resolves impact point in this order.

```text
1. If bFromSweep == true, prefer SweepResult
2. If bFromSweep == false, use GetClosestPointOnCollision() fallback
```

If `SweepResult` is available, engine-provided hit point / impact normal is preferred.

If `SweepResult` is not valid, closest point fallback based on target collision is used.

Fallback means:

```text
Weapon collision center
-> target collision surface closest point
```

This is not exact blade contact point, but it provides a more natural hit position fallback than actor location.

However, this fallback is only a better approximation inside the current overlap structure.

It does not guarantee exact weapon contact point, weapon normal, bone, physical material, or surface direction.

---

## 6. Weapon Trail Trace

Weapon Trail Trace is not included in the current implementation scope.

Trail trace is not just changing VFX location.

It is closer to changing melee hit detection model and hit metadata generation.

Concept:

```text
Define sample points on weapon
Trace between previous position and current position for each sample point
Read impact point / normal / bone / surface data from trace result
```

Costs to consider:

```text
sample socket setup
previous location storage
tick or notify tick trace
duplicate hit target filtering
trace radius / sample count tuning
debug draw / trace channel policy
fast swing / low frame rate compensation
```

For now, the chosen first implementation is overlap + `FDamageImpactInfo` + closest point fallback.

Trail trace should be reviewed as a separate hit detection model when:

```text
guard / parry / weapon clash depends on exact weapon contact direction
hit normal and weapon swing direction are used in gameplay judgment
bone / physical material / surface based feedback is needed
fast swing needs more stable contact point than overlap
```

---

## 7. Current Implementation

Current implementation:

```text
ACWeaponActor::BuildDamageImpactInfo()
-> prefer SweepResult
-> use GetClosestPointOnCollision fallback

UCDamageFeedbackComponent
-> PlayDamageFeedback()
-> ResolveHitFeedbackLocation()
-> ResolveHitFeedbackRotation()
-> PlayHitVFX()
-> PlayHitSFX()
-> PlayHitStop()
-> PlayCameraShake()

UCReactionFeedbackComponent
-> matches feedback by reaction type / damage spec / timing / trigger key

UCReaction
-> creates reaction feedback request from active reaction context
```

`DamageFeedback` directly consumes `TakeDamagePacket.Context.DamageImpactInfo`.

`ReactionFeedback` does not directly consume `FDamageImpactInfo`.

At the current stage, impact metadata is input for damage feedback, not reaction feedback.

If reaction feedback needs hit location later, whether to include impact metadata in reaction context should be decided separately.

---

## 8. Consequences

Benefits:

```text
Hit feedback and reaction execution feedback are not mixed
DamageFeedback works from damage event metadata
ReactionFeedback works from reaction lifecycle timing
Impact position can be more natural than actor location based hit VFX
ApplyDamage / TakeDamage layers do not own collision calculation responsibility
Future trail trace can reuse the delivery structure without major changes
```

Notes:

```text
ClosestPoint fallback is not exact blade contact point
ImpactNormal is an approximation in fallback mode
Precise surface / bone / weapon clash judgment needs trail trace
If ReactionFeedback needs hit location, separate context extension is needed
```

---

## 9. Follow-up

Follow-up candidates:

```text
Design weapon-specific trace profile
Review trail trace based hit detection model
Review DamageFeedback data asset separation
Organize authoring workflow for ReactionFeedback and DamageFeedback
Define hit normal direction policy clearly
```

---

## 10. Related Documents

Related detailed documents:

```text
A06_UE5_Portfolio_Weapon_Trail_Trace_Model
A08_UE5_Portfolio_Combat_Feedback_Model
```

---
