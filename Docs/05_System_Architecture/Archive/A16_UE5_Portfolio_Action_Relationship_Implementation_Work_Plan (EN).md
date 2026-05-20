# A16 UE5 Portfolio Action Relationship Implementation Work Plan

## 1. Purpose

This document records the implementation plan for applying the action execution relationship model to code.

A15 defines the structural direction. This document lists the enum, struct, API, and executor naming changes required to apply it.

## 2. Current Code Shape

The current action orchestration flow is close to:

```text
RequestAction
-> ResolveActionCandidate
-> ResolveActionContext
-> BuildExecutionDecisionQuery
-> Executor ResolveExecutionDecision
-> BuildActionExecutionResult
-> ResolveInterventionDirective
-> DispatchActionDecision
```

The current decision values still carry mixed meanings.

```text
Executable
Chainable
Reject
Ignore
```

## 3. Modification Direction

`EExecutionDecision` should express only request acceptance.

```cpp
enum class EExecutionDecision : uint8
{
	None = 0,

	Reject,
	Ignore,
	Accept,

	Max,
};
```

Execution relationship should be added separately.

```cpp
enum class EExecutionRelationship : uint8
{
	None = 0,

	Independent,
	Sequential,
	Exclusive,

	Max,
};
```

Execution apply mode should also be separate.

```cpp
enum class EExecutionApplyMode : uint8
{
	None = 0,

	Start,
	Reserve,
	Intervene,
	StopOnly,

	Max,
};
```

## 4. Struct Changes

`FExecutionDecisionResult` should own decision, relationship, and apply mode.

```cpp
struct FExecutionDecisionResult
{
	EExecutionDecision Decision = EExecutionDecision::None;
	EExecutionRelationship Relationship = EExecutionRelationship::None;
	EExecutionApplyMode ApplyMode = EExecutionApplyMode::None;

	bool IsAccepted() const
	{
		return Decision == EExecutionDecision::Accept;
	}

	bool RequiresIntervention() const
	{
		return ApplyMode == EExecutionApplyMode::Intervene;
	}

	bool RequiresReserve() const
	{
		return ApplyMode == EExecutionApplyMode::Reserve;
	}
};
```

`FActionExecutionResult` should wrap the common decision result and the resolved action context.

## 5. API Changes

The orchestrator should split accepted execution handling by relationship.

```text
ResolveActionExecutionResult
-> ResolveImmediateActionStart
-> ResolveSequentialActionReserve
-> ResolveExclusiveActionIntervention
```

The action component should consume result modes rather than infer them from old decision values.

Combo executor APIs should be renamed toward reserve / consume semantics.

```text
ReserveChain
ConsumeChain
CanReserveChain
CanConsumeChain
```

## 6. Recommended Work Order

1. Stabilize reaction-side structure first.
2. Introduce execution relationship and apply mode enums.
3. Update action execution result.
4. Split immediate / sequential / exclusive handling.
5. Rename chain APIs to reserve / consume.
6. Align reaction-side result structure later.

## 7. Conclusion

The implementation should not keep extending `Executable` and `Chainable`.

The action pipeline should explicitly model decision, relationship, and apply mode.
