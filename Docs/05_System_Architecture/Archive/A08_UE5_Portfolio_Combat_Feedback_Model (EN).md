# Combat Feedback Model

## 1. Purpose

This document defines the responsibilities of `ActionFeedback`, `ReactionFeedback`, `DamageFeedback`, `PlayerFeedback`, and `CombatFeedbackSubsystem` in the combat feedback layer.

The reaction orchestration work also reorganized feedback responsibilities.

In particular, damage hit feedback and reaction feedback were easy to mix conceptually.

In the current structure, it is more appropriate to separate hit-position-based feedback from reaction-execution-based feedback.

This document clarifies the criteria for feedback components and provides a reference for later action feedback improvement and combat feedback expansion.

---

## 2. Feedback Layer Split

Current combat feedback can be divided as follows.

```text
ActionFeedback
-> action executor based feedback

ReactionFeedback
-> reaction executor based feedback

DamageFeedback
-> damage packet based feedback

PlayerFeedback
-> player presentation based feedback

CombatFeedbackSubsystem
-> world-level feedback execution support
```

The key is not what caused the feedback, but which context the feedback is interpreted from.

ActionFeedback is based on action execution context.

ReactionFeedback is based on reaction execution context.

DamageFeedback is based on take damage packet and damage impact metadata.

---

## 3. ActionFeedback

`ActionFeedback` is action executor based feedback.

Its main keys are as follows.

```text
ActionType
ActionIndex
Timing
TriggerKey
```

ActionFeedback is requested from action montage notifies or action execution events.

Therefore, it is directly connected to timing inside action execution.

Examples:

```text
Action start feedback
Action notify point feedback
Action notify window feedback
Action completed feedback
```

ActionFeedback represents the action itself, but it is not based on whether damage was actually applied.

In other words, attack swing VFX and actual hit VFX are not the same feedback layer.

---

## 4. ReactionFeedback

`ReactionFeedback` is reaction executor based feedback.

Its main keys are as follows.

```text
ReactionType
ApplyDamageSpecKey
ReactionFeedbackTiming
TriggerKey
```

ReactionFeedback is requested by `CReaction` based on its active reaction context.

Therefore, it is directly connected to reaction montage, reaction finish, and reaction notify timing.

Examples:

```text
ReactionStart
ReactionCompleted
ReactionInterrupted
ReactionCancelled
WindowBegin
WindowEnd
Notify
```

ReactionFeedback is not hit feedback.

ReactionFeedback represents the reaction expression that occurs after being hit.

Therefore, responsibility for selecting feedback based on hit impact point or hit normal should not be placed in ReactionFeedback.

---

## 5. DamageFeedback

`DamageFeedback` is take damage packet based feedback.

Its main context is as follows.

```text
FTakeDamagePacket
FTakeDamageContext
FTakeDamageResult
FDamageImpactInfo
```

DamageFeedback is suitable for feedback that runs after damage has actually been accepted / committed.

Examples:

```text
Hit VFX
Hit SFX
Hit stop
Camera shake
Damage number
Impact decal
```

In the current structure, `FDamageImpactInfo` should be directly consumed by DamageFeedback.

This is because hit position and hit normal belong to damage event metadata.

If ReactionFeedback represents reaction execution timing, DamageFeedback represents immediate feedback for the damage event itself.

---

## 6. PlayerFeedback

`PlayerFeedback` is player presentation based feedback.

It is closer to screen / camera / input / UI response that the player should feel, rather than the combat event itself.

Examples:

```text
local camera shake
local hit stop response
controller vibration
screen effect
player-only UI feedback
```

PlayerFeedback can overlap with combat domain feedback, but its responsibility basis is different.

DamageFeedback is based on damage events.

PlayerFeedback is based on local player presentation.

Therefore, when considering multiplayer or AI actors, keeping PlayerFeedback as a separate layer is clearer.

---

## 7. CombatFeedbackSubsystem

`CombatFeedbackSubsystem` is a world-level feedback execution support layer.

If each action / reaction / damage component implements every feedback execution method directly, duplication grows.

Therefore, common feedback execution features can be separated into a subsystem.

Examples:

```text
apply hit stop audience
apply camera shake audience
support world VFX spawn
support world SFX spawn
control duplicate feedback execution
```

The subsystem is not the layer that interprets the meaning of feedback requests.

The subsystem is a support layer that executes already interpreted feedback requests.

---

## 8. Where Feedback Requests Are Built

Feedback requests should be created by the object that best understands the corresponding execution context.

The recommended basis is as follows.

```text
CAction
-> create action feedback request

CReaction
-> create reaction feedback request

TakeDamage / DamageFeedback
-> create damage feedback request

PlayerFeedback
-> create local player feedback request
```

Components can act as bridges when needed.

For example, reaction notifies can be forwarded through `ReactionComponent` to the active reaction executor.

However, the actual reaction feedback request should be created by `CReaction`.

This is because the executor knows the current execution context and timing most accurately.

---

## 9. Difference Between ReactionFeedback and DamageFeedback

ReactionFeedback and DamageFeedback can both occur in a hit situation, but their reference context is different.

```text
DamageFeedback
-> did a damage event occur
-> where did it hit
-> how much damage was applied
-> what is the hit impact metadata

ReactionFeedback
-> which reaction is running
-> which timing did the reaction reach
-> what is the reaction notify trigger key
-> which expression should be used for reaction type / damage spec
```

Therefore, sword hit VFX is closer to DamageFeedback, while posture break dust during a hit reaction montage is closer to ReactionFeedback.

By separating them, hit event expression and reaction execution expression can be tuned independently.

---

## 10. Conclusion

Combat feedback should be separated by context rather than merged into one large feedback component.

```text
ActionFeedback
-> action execution expression

ReactionFeedback
-> reaction execution expression

DamageFeedback
-> damage event expression

PlayerFeedback
-> local player presentation expression

CombatFeedbackSubsystem
-> common execution support
```

With this structure, hit feedback and reaction feedback do not mix, and each feedback layer only depends on the context it needs.

Therefore, the feedback structure in the current reaction orchestration branch is valid because it is separated by execution context.
