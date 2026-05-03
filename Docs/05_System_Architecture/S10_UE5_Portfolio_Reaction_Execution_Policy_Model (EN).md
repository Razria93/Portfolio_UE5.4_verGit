# Reaction Execution Policy Model

## 1. Purpose

This document clarifies the difference between the runtime state owned by `CReaction`  
and the policy state produced by `ResolveReactionPolicy()`.

The main goals are:

- Clarify that `CReaction` hooks are local rules of the execution object.
- Clarify that `FReactionExecutionPolicy` is not a fixed initial policy of the executor.
- Define `FReactionExecutionPolicy` as a resolved policy computed for the current request.
- Explain why these two layers are not redundant even if the first implementation keeps policy thin.


---

## 2. Core Question

In Reaction Orchestration, the following question can arise.

```text
If CReaction::WantToInterrupt() and CReaction::AllowInterruptionBy()
already decide whether an incoming reaction can interrupt an active reaction,
why does ResolveReactionPolicy() need to exist?
```

This question is valid in the first implementation pass.

If `FReactionExecutionPolicy` only contains the following fields,  
it can look redundant with `CReaction` hooks.

```cpp
struct FReactionExecutionPolicy
{
	bool bCanInterruptToActive = false;
	int32 Priority = 0;
};
```

However, these values do not answer the same question.

```text
CReaction hooks are local rules owned by the execution object
FReactionExecutionPolicy is a resolved policy interpreted by the orchestrator for the current request
```


---

## 3. CReaction State and Hooks

`CReaction` is the actual reaction execution object.

Therefore, the state owned by `CReaction` represents the local runtime state of the currently executing reaction object.

Examples:

```cpp
bool bIsActive;
bool bInterruptible;
bool bCancelable;
UAnimMontage* ActiveMontage_Cached;
```

These values are mainly changed by montage lifecycle and anim notify windows.

```text
The reaction becomes active when its montage starts
bInterruptible becomes true when the interruptible notify window opens
bCancelable becomes true when the cancelable notify window opens
State is cleared when the montage ends or the reaction is stopped
```

`CReaction` hooks also make decisions from this local runtime state.

```cpp
CReaction::WantToInterrupt()
CReaction::AllowInterruptionBy()
CReaction::WantToCancel()
CReaction::AllowCancelBy()
```

These hooks answer questions such as:

```text
Does this reaction executor allow interruption right now?
Does this incoming reaction executor want to interrupt?
Does this reaction executor allow cancel right now?
```

Therefore, `CReaction` hooks are local rules inside the execution object.


---

## 4. ResolveReactionPolicy State

`ResolveReactionPolicy()` does not return the initial settings of a `CReaction` executor.

`ResolveReactionPolicy()` interprets how the current request may be handled  
inside the current body/runtime/context state.

`FReactionExecutionPolicy` means:

```text
How much execution authority does this incoming reaction request have
under the current character state?
```

This policy is computed per request.

It may reflect:

```text
current body state
current active reaction
incoming damage / reaction context
dead state
super armor state
guard / guard break state
poise state
external hit resolution result
character trait / buff
```

Therefore, `FReactionExecutionPolicy` is not original reaction data,  
and it is not the local state of a `CReaction` executor.

It is an orchestration-level authority interpretation created by the orchestrator for the current request.


---

## 5. Responsibility Difference

The difference between the two layers is:

```text
CReaction hook
- Local execution rule owned by the reaction executor
- Focused on montage window, interruptible, cancelable, and executor runtime flags
- Describes what the execution object currently allows

FReactionExecutionPolicy
- Describes what authority the current request has under the current character state
- Orchestration-level policy resolved from body/runtime/context
- Describes how the orchestrator should evaluate this request
```

In short, `CReaction` hooks are internal execution-object decisions,  
while `FReactionExecutionPolicy` is a higher-level decision input that considers request and body state together.


---

## 6. Examples

### Dead Reaction

The current active reaction may not be inside an interruptible window.

However, if the incoming reaction is `Dead`, death is a higher-level state than a normal hit reaction.

In that case, policy may mean:

```cpp
policy.bForceInterrupt = true;
policy.bIgnoreInterruptWindow = true;
```

The orchestrator can force interruption above executor hooks.

### Super Armor

The current character may be in a super armor state.

The incoming hit reaction executor may want to interrupt.

However, if the body state is super armor, the orchestrator policy may prevent the incoming hit reaction  
from becoming an interrupt candidate.

```cpp
policy.bCanInterruptToActive = false;
```

The orchestration layer can block the request even if the executor wants interruption.

### Guard Break

If guard break has occurred, a specific stagger reaction may need to be forced.

In that case, policy may mean:

```cpp
policy.bForceInterrupt = true;
policy.bIgnoreInterruptWindow = true;
```

The incoming reaction can be evaluated with different authority from a normal hit reaction.


---

## 7. Current Implementation Guideline

In the first implementation pass, body state, super armor, guard, poise, and hit resolution result  
are not deeply integrated yet.

Therefore, `FReactionExecutionPolicy` can remain thin.

```cpp
struct FReactionExecutionPolicy
{
	bool bCanInterruptToActive = false;
	int32 Priority = 0;
};
```

At this stage, `bCanInterruptToActive` does not decide final interrupt availability by itself.

Its current meaning should be limited to:

```text
Can the orchestrator evaluate this request as an active-reaction interrupt candidate?
```

The final decision happens in this order.

```text
1. Check resolved policy
2. Compare priority
3. Check current CReaction::AllowInterruptionBy()
4. Check incoming CReaction::WantToInterrupt()
```

The policy does not replace `CReaction` hooks.  
It defines request-level authority before the decision reaches `CReaction` hooks.


---

## 8. Conclusion

`CReaction` state and hooks are local rules inside the execution object.

`FReactionExecutionPolicy` is a resolved policy created by the orchestrator  
from the current request and current body/runtime/context.

The difference is:

```text
CReaction
- Local state and rule of the currently executing reaction object
- Focused on animation windows and executor runtime flags

FReactionExecutionPolicy
- Execution authority interpretation for the current request
- Orchestration-level policy resolved from body/runtime/context
```

The policy may look redundant while it stays thin.  
However, once body state, hit resolution, guard, poise, or super armor is introduced,  
the policy becomes the layer that carries higher-level decisions that executor hooks cannot express alone.

