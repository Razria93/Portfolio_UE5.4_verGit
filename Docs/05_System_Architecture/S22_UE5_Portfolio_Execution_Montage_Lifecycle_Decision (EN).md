# S22 UE5 Portfolio Execution Montage Lifecycle Decision

## 1. Purpose

This document defines how Complete, Stop, and MontageEnd responsibilities should be separated in action / reaction executor lifecycle.

The core point is to distinguish natural completion, external stop requests, and engine callbacks.

## 2. Previous System Shape

Montage-based execution follows:

```text
Start
-> Montage_Play
-> Notify or MontageEnd
-> Complete / Stop / Finish
```

Complete notify, stop request, and montage end delegate can all participate in execution ending.

```text
MontageEnd
-> engine callback

Complete
-> natural completion

Stop
-> external stop request
```

## 3. Problems And Limits

MontageEnd can still be called after Stop.

If both Stop and MontageEnd finish the execution, duplicate cleanup can happen.

If only the delegate is trusted, explicit stop request effects may remain unclear.

Executor owns montage lifecycle detail, while component owns active context and execution state.

## 4. Refactoring Direction

Responsibilities should be:

```text
Executor
-> montage play / stop / complete
-> notify command handling
-> stop reason recording
-> notify component about finish

Component
-> active context owner
-> execution state update
-> active context clear
-> fallback end handling
```

Completion concepts:

```text
Complete
-> natural completion

Stop
-> external stop request
-> includes interrupted / cancelled / ignored reason

MontageEnd
-> engine callback
-> must avoid duplicate cleanup after handled stop
```

## 5. Trial And Error

It was unclear whether `Stop()` should only stop the montage or synchronously finish the execution.

Because `Montage_Stop` can call the end delegate after blend-out, finishing inside Stop can later be followed by another MontageEnd callback.

However, waiting only for the delegate can leave active state unclear after an explicit stop request.

## 6. Conclusion

Use the following principle:

```text
Complete
-> natural end

Stop
-> external interruption/cancellation

MontageEnd
-> engine callback that must avoid duplicate finish
```

Executor handles montage lifecycle. Component handles active runtime state.
