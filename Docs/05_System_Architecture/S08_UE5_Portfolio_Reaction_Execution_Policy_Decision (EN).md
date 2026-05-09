# Reaction Execution Policy Architecture Decision

## 1. Purpose

This document clarifies what `CReaction` local execution state/rules and `FReactionExecutionPolicy` orchestration-level policy mean inside reaction orchestration.

It reorganizes the archive documents `A05` and `A07` into a single decision flow.

The key point is that `FReactionExecutionPolicy` is not a fixed initial setting of the executor.

It is a resolved policy calculated by the orchestrator from body/runtime/context at request time.

---

## 2. Background

In reaction orchestration, the following question can appear.

```text
If whether incoming reaction can interrupt active reaction is already checked by
CReaction::WantToInterrupt()
CReaction::AllowInterruptionBy(),
why is FReactionExecutionPolicy needed?
```

This question is valid because policy is thin in the first implementation.

However, `CReaction` hook and `FReactionExecutionPolicy` are not values on the same axis.

```text
CReaction local state / rule
-> internal state and rule of the currently running reaction executor

FReactionExecutionPolicy
-> orchestration-level authority interpretation of the current request in current body/runtime/context
```

`CReaction` answers "does this execution object allow this now?"

`FReactionExecutionPolicy` expresses "what authority does this request have in the current character state?"

Neither one makes the final decision alone.

The orchestrator checks both policy and executor local rule, then decides `Start / Interrupt / Ignore / Reject`.

---

## 3. Local Execution State and Rule

`CReaction` is the actual reaction executor.

Its state values are local runtime state of the currently running reaction object.

Examples:

```text
bIsReaction
bInterruptible
bCancelable
ActiveReactionMontage_Cached
ActiveReactionData_Cached
ActiveReactionMontage section
Anim notify window
```

These values are changed by montage lifecycle and anim notify windows.

They do not describe the whole external body state.

They describe what control this reaction executor allows within its own execution phase.

`CReaction` hooks are APIs that answer orchestrator queries based on this local runtime state.

```cpp
WantToInterrupt(const FReactionQueryContext& InContext)
AllowInterruptionBy(const FReactionQueryContext& InContext)
WantToCancel(const FReactionQueryContext& InContext)
AllowCancelBy(const FReactionQueryContext& InContext)
```

Executor hooks answer these questions.

```text
Does incoming reaction executor want to interrupt the current active reaction?
Does current reaction executor allow interruption now?

Does incoming reaction executor want to cancel the current active reaction?
Does current reaction executor allow cancel now?
```

Examples:

```text
Hit reaction interruptible window is open
-> AllowInterruptionBy() can return true

Hit reaction entered recovery section
-> AllowInterruptionBy() can return false

Executor is Dead reaction
-> WantToInterrupt() can return true

Specific reaction does not want to be interrupted again by the same hit reaction
-> WantToInterrupt() or AllowInterruptionBy() can return false
```

This is close to executor-internal window / montage phase / reaction-specific rule.

Therefore, `CReaction` does not make the final orchestration decision.

It provides executor-local decision evidence needed by the orchestrator.

---

## 4. Resolved Policy

`FReactionExecutionPolicy` is not executor local state.

It is an orchestration-level policy calculated by the orchestrator for the current request.

Policy does not mean "which window is the executor in?"

Policy means "what authority does this incoming reaction request have in the current character body/runtime/context?"

Current fields:

```cpp
struct FReactionExecutionPolicy
{
	bool bCanInterrupt = false;
	bool bForceInterrupt = false;
	bool bIgnoreInterruptWindow = false;
	int32 Priority = 0;
};
```

This value is recalculated for every request.

It can reflect:

```text
incoming reaction type
incoming reaction data priority
current active reaction
dead state transition
super armor
guard / guard break
poise
external hit resolution result
character trait / buff
```

The first implementation mostly uses reaction data priority and dead reaction special case.

Examples:

```text
incoming reaction is Dead
-> grant force interrupt authority regardless of active reaction local interrupt window

current character has super armor
-> remove interrupt authority even if incoming hit reaction executor wants interruption

current character is guard broken
-> grant higher priority and window ignore authority over normal hit reaction

current poise is sufficient
-> restrict policy so hit reaction request can be ignored
```

Policy reflects character state, combat state, and damage interpretation result above executor-local rules.

---

## 5. Decision

Keep `FReactionExecutionPolicy`.

At the current stage, keep it as a thin resolved policy.

Final interrupt availability is not decided by policy alone.

Current judgment order:

```text
1. Check incoming context validity
2. Check whether current active context exists
3. Check incoming policy bCanInterrupt
4. Compare policy priority and current reaction priority
5. If not bIgnoreInterruptWindow, check current executor AllowInterruptionBy()
6. If not bForceInterrupt, check incoming executor WantToInterrupt()
7. Resolve final decision as Start / Interrupt / Ignore / Reject
```

Policy does not replace executor hooks.

Policy organizes what higher-level authority the request has before reaching executor hooks.

Executor local rule verifies whether the policy-allowed candidate is also allowed by executor internal state.

The final decision is made by `ReactionOrchestrator`.

---

## 6. Examples

### Dead Reaction

Dead reaction is a higher body state transition than normal hit reaction.

Therefore, force interrupt is needed even if active reaction interruptible window is closed.

```cpp
policy.bCanInterrupt = true;
policy.bForceInterrupt = true;
policy.bIgnoreInterruptWindow = true;
policy.Priority = TNumericLimits<int32>::Max();
```

In this case, the orchestrator can ignore the window check by policy even if current executor `AllowInterruptionBy()` returns false.

Dead override is an orchestration policy above executor-local window.

### Super Armor

If current body state is super armor, orchestration can block incoming hit reaction even if incoming executor wants interruption.

```cpp
policy.bCanInterrupt = false;
```

In this case, even if incoming executor `WantToInterrupt()` returns true, the orchestrator can reject or ignore before local rule query.

Super armor restricts incoming reaction authority above executor-local intent.

### Guard Break

If character is guard broken, a specific stagger reaction may ignore normal interrupt window.

```cpp
policy.bCanInterrupt = true;
policy.bForceInterrupt = true;
policy.bIgnoreInterruptWindow = true;
```

Even if current executor has not opened interruptible window yet, body/runtime state can grant stronger authority to incoming reaction.

### Normal Hit Reaction

Normal hit reaction may not receive special force authority from policy.

```cpp
policy.bCanInterrupt = true;
policy.bForceInterrupt = false;
policy.bIgnoreInterruptWindow = false;
policy.Priority = InContext.ReactionData.Priority;
```

In this case, both current executor `AllowInterruptionBy()` and incoming executor `WantToInterrupt()` matter.

Normal hit reaction becomes Interrupt decision only when both orchestration policy and executor-local rule allow it.

---

## 7. Current Implementation

Current implementation:

```text
ResolveReactionPolicy()
-> sets ReactionData.Priority to policy.Priority
-> sets default bCanInterrupt to true
-> if Dead reaction, sets force interrupt / ignore interrupt window / max priority

CanInterruptActiveReaction()
-> checks policy and priority first
-> checks CReaction local hooks
-> returns final interrupt availability
```

Current policy is thin, but it becomes an important extension point when guard / parry / poise / super armor are added.

---

## 8. Consequences

Benefits:

```text
Executor local rule and orchestration-level policy are separated
Higher states such as Dead reaction can be handled above executor hooks
Body-state based policies such as super armor / guard / poise have a place to live
The current implementation stays thin while preserving an extension point
```

Notes:

```text
Policy may look duplicated with executor hooks while it is thin
bCanInterrupt is not final interrupt availability, but authority to be evaluated as an interrupt candidate
force / ignore window policy becomes clearer when actual body state extensions are added
```

---

## 9. Follow-up

Follow-up candidates:

```text
Reflect guard / parry / dodge / counter results into reaction policy
Connect super armor / poise / guard break state to ResolveReactionPolicy
Define cancel policy separately
Review whether action orchestration also needs a similar resolved policy layer
```

---

## 10. Related Documents

Related detailed documents:

```text
A05_UE5_Portfolio_Reaction_Execution_Policy_Model
A07_UE5_Portfolio_Reaction_Lifecycle_Model
```

---
