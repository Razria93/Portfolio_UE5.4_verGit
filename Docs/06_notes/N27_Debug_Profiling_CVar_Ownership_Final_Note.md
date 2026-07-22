# N27. Debug Profiling CVar Ownership Final Note

## Purpose

This note records the final ownership rule after the debug log, diagnostic hook, profiling audit, and CSV counter cleanup.

Related notes:

```text
N23_Debug_Log_And_Diagnostic_Code_Policy_Note.md
N24_Debug_Log_Cleanup_Inventory_Note.md
N25_Diagnostic_Log_Gating_And_Audit_Category_Plan_Note.md
N26_Diagnostic_Log_Full_Audit_Inventory_Note.md
```

N23 keeps the general policy.
N24 keeps cleanup inventory.
N25 keeps helper/gating design.
N26 keeps full audit inventory.
This note fixes the final ownership boundary so future cleanup does not mix debug, profiling, and runtime policy concerns again.

---

## Final Ownership Rule

```text
1. Debug / Diagnostic output CVar
   Owner: Core/Debug helper
   Responsibility: CVar, gate, message format, Output Log call, non-shipping no-op

2. Debug Dump CVar
   Owner: Core/Debug helper
   Responsibility: verbose context/payload/result dump gate and print formatting

3. Profiling audit / CSV counter CVar
   Owner: Core/Profiling helper
   Responsibility: CVar, gate, CSV counter, profiling summary, non-shipping no-op

4. Runtime policy / tuning CVar
   Owner: owning policy/system cpp
   Responsibility: actual gameplay policy selection or tuning value
```

Runtime policy/tuning CVar can stay in the owning policy/system file when it changes actual game behavior.
However, audit/counter output must not be implied by a policy selector CVar.

---

## Mixed CVar Rule

Do not let one CVar control both runtime behavior and diagnostic/profiling output.

Correct split:

```text
StatePolicyMode
-> selects RuntimeLOD policy source only

StatePolicyAudit
-> controls state tier CSV profiling counter only
```

Incorrect split:

```text
StatePolicyMode
-> selects policy source
-> also controls audit/counter output
```

This rule also applies to animation, movement, BT interval, perception, combat feedback, and combat collision profiling.

---

## CSV Macro Rule

```text
CSV_CUSTOM_STAT_GLOBAL
-> event / counter recording
-> direct macro calls live in Core/Profiling helper implementation
-> gameplay / BT / anim code calls Record...ForProfiling() only

CSV_SCOPED_TIMING_STAT_GLOBAL
-> RAII scope timing
-> stays in the measured scope body
-> do not hide it behind a helper, because that can change the measured range
```

Final scan expectation:

```text
Core/Profiling outside direct CSV_CUSTOM_STAT_GLOBAL count: 0
Core/Profiling outside direct CSV_SCOPED_TIMING_STAT_GLOBAL: allowed only at measured scope entry
```

---

## Current Applied Result

Core/Debug owns:

```text
AI combat BT audit gates
AI perception audit summaries
CombatEngage assignment audit output format
Combat signal / result / feedback diagnostic output
Action / reaction / movement / overlay / component reference diagnostic output
```

Core/Profiling owns:

```text
DisableEnemyPerception
DisableEnemyHitProcessing
DisableEnemyWeaponActor
DisableEnemyCombatFeedback
AnimationRefreshAudit
StatePolicyAudit
AI animation refresh CSV counters
AI behavior tree CSV counters
AI state RuntimeLOD tier CSV counters
Combat collision CSV counters
Combat feedback CSV counters
```

Runtime policy/system owners keep:

```text
EngageAssignmentWarmupTime
EngageAssignmentEngageCap
EngageAssignmentAlertCap
EnemyMeshMode
EnemyActorTickMode
EnemyMovementMode
EnemyAnimationMode
EnemyAnimationReducedRefreshInterval
BTUpdateIntervalMode
StatePolicyMode
```

These remaining CVars are policy/tuning controls, not debug output gates.

---

## Review Checklist

Use this checklist when adding or reviewing a new CVar:

```text
1. Does it print text or control diagnostic output?
   -> Core/Debug helper

2. Does it emit CSV counters, profiling summaries, or profiling audit output?
   -> Core/Profiling helper

3. Does it select gameplay policy or tuning behavior?
   -> owning policy/system cpp

4. Does it do more than one of the above?
   -> split it before merging

5. Is it CSV_CUSTOM_STAT_GLOBAL?
   -> direct call must be inside Core/Profiling

6. Is it CSV_SCOPED_TIMING_STAT_GLOBAL?
   -> keep it at the measured scope entry
```
