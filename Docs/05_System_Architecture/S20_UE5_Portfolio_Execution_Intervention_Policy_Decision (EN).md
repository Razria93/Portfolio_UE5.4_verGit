# S20 UE5 Portfolio Execution Intervention Policy Decision

## 1. Purpose

This document defines why executor intervention policy should split `WantIntervention()` and `AllowInterventionBy()`.

The core point is that incoming-side intent and active-side permission are different questions.

## 2. Previous System Shape

Cancel and interrupt were initially understood as:

```text
Cancel
-> intentional stop

Interrupt
-> external or forced stop
```

Interruptible and cancelable windows represented whether a running action or reaction could be stopped.

## 3. Problems And Limits

Cancel / interrupt cannot be mapped directly to one API or one window.

Dodge stopping a hit reaction is closer to intentional cancellation.

Hit reaction stopping an attack action is closer to interruption from the active action perspective.

The following questions must be separated:

```text
WantIntervention
-> does incoming execution want to stop active execution?

AllowInterventionBy
-> does active execution allow being stopped by incoming execution?
```

## 4. Refactoring Direction

Executors should expose:

```cpp
virtual bool WantIntervention(const FExecutionInterventionQuery& InQuery) const;
virtual bool AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const;
```

Default action policy may return false for `WantIntervention()`.

Special actions such as dodge or counter can override it.

Active-side policy can allow or reject intervention based on windows, reaction type, armor, or forced death behavior.

## 5. Trial And Error

Interrupt and cancel windows looked sufficient at first.

Cross-domain execution made it clear that the executor that wants to stop another execution and the executor that allows being stopped are different participants.

Without this split, special actions such as dodge, counter, guard, and parry push too many conditions into one side.

## 6. Conclusion

Intervention policy must separate incoming and active perspectives.

`WantIntervention()` and `AllowInterventionBy()` should remain separate APIs.
