# UE5 Portfolio – Issue Checklist

## Title

**M02-03: Implement ApplyDamage pipeline (Request → CalculateDamage)**

### Date

- **Day 8**

- **Date : 2025.01.01**


---

### Objective

- Make `CApplyDamageComponent` the one place where ApplyDamage is processed.
    
- Handle all ApplyDamage requests through one entry point (`RequestApplyDamage`), and calculate damage in `CalculateDamage` using clear SpecKey/Result structs.
    
- Send the final result to the target only through `ApplyDamageToTarget`, which calls the engine `TakeDamage(...)` (the receiver is implemented in M2-04).


---

### Branch

- feature/combat-apply-damage


---

### TODO List

#### 1. ApplyDamage component base

- [x] Create `CApplyDamageComponent`

- [x] Define a single entry point API (`RequestApplyDamage(...)`)

- [x] Validate request integrity (attacker / damage causer / target / hit component, etc.) and enforce early-return on invalid inputs


#### 2. Damage calculation (CalculateDamage)

- [x] Define minimal input structures for calculation (e.g., `DamageContext`, `SpecKey`)

- [x] Implement minimal calculation path (`BaseDamage → FinalDamage`) with explicit output (`DamageResult`)

- [x] Keep calculation side-effect-free (no target mutation during calculation)


#### 3. Dispatch boundary (engine `TakeDamage` integration)

- [x] Define a single dispatch hook (`ApplyDamageToTarget(...)`) as the only place that calls `Target->TakeDamage(...)`

- [x] Verify the end-to-end call path via structured logs only (actual `TakeDamage` receive behavior is implemented in M2-04; HP/UI is handled in M2-05)


---

### Notes
- 


---
