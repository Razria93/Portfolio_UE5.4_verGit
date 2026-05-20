# A17 UE5 Portfolio Execution Intervention Base Policy

## 1. Purpose

This document defines the base intervention policy used by action and reaction executors through `WantIntervention()` and `AllowInterventionBy()`.

The core point is that incoming-side intent and active-side permission are different rules.

## 2. Previous System Shape

When an active execution exists, orchestration builds an `FExecutionInterventionQuery`.

The query asks two different questions:

```text
Incoming side
-> Does the incoming execution want to stop the active execution?
-> WantIntervention()

Active side
-> Does the active execution allow being stopped by the incoming execution?
-> AllowInterventionBy()
```

Stop reason is used to distinguish cancellation and interruption.

```text
Cancelled
-> intentional cancellation

Interrupted
-> external or forced interruption
```

## 3. Problems And Limits

`WantIntervention()` and `AllowInterventionBy()` both return bool, but they do not mean the same thing.

Using the same window for "I want to stop another execution" and "I allow being stopped" mixes incoming and active perspectives.

Action and reaction also should not have identical base policy.

Normal actions usually do not actively stop another execution. Reactions such as hit or dead can interrupt normal actions by default.

## 4. Refactoring Direction

Action incoming-side default:

```cpp
bool UCAction::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	return false;
}
```

Specific actions such as dodge or counter should override it.

Action active-side policy can allow hit/dead reactions to interrupt normal actions by default while using explicit windows for other intervention types.

Reaction incoming-side policy can use want interrupt / want cancel windows.

Reaction active-side policy can use allow interrupt / allow cancel windows.

## 5. Future Direction

Special actions should override incoming-side rules.

```text
Dodge
-> wants to cancel active reaction

Counter
-> wants to intervene after a valid defensive condition
```

Special reactions can override active-side rules for armor, immunity, or forced death behavior.

## 6. Conclusion

Intervention policy must separate incoming intent from active permission.

`WantIntervention()` and `AllowInterventionBy()` should remain separate APIs even if some default implementations look similar.
