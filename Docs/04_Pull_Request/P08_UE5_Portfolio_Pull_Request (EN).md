# Payload/Context/Result-based TakeDamage Pipeline Implementation: CEnemy / CTakeDamageComponent / CHealthComponent

## Title

✨ feat: Implement TakeDamage pipeline (CEnemy / CTakeDamageComponent / CHealthComponent) (#22)

---

## Summary

- The `AActor::TakeDamage()` override previously contained the full workflow in a single class: receive, validate, resolve, commit HP, and debug output.
    
- This structure introduces the following architectural risks.
    
    1. **Actor responsibility bloat**
        
        - If `CEnemy` owns the entire damage pipeline, the actor becomes tightly coupled to damage processing.
            
        - With inheritance-based extensions, `CEnemy` tends to accumulate special cases and coupling between related objects increases.
            
    2. **Reduced extensibility**
        
        - Adding new damageable actors often requires duplicating similar damage-processing logic.
            
	    - Actors that do not require damage computation may still be forced to implement damage-handling logic.
            
    3. **Mixed resource responsibilities**
        
        - If HP resource management (Clamp, Damage, Heal, Dead) is not separated from evaluation and policy (Reject, Resolve, Commit), adding features  increases coupling quickly (such as shield, mitigation, or state transitions).
            
- This PR restructures responsibilities as follows.
    
    1. Reduce `CEnemy` to **TakeDamage entry (override) + component routing**.
        
    2. Move the TakeDamage pipeline into `UCTakeDamageComponent`.
        
    3. Implement `UCTakeDamageComponent` as an **orchestration layer that branches by `FDamageEvent` type**.
        
    4. Implement `UCHealthComponent` as a **resource manager for HP decrease, increase, and Dead determination**.
        
    5. Standardize the TakeDamage data flow using `FTakeDamagePayload / FTakeDamageContext / FTakeDamageResult`.
        
    6. Add Debug Print helpers to inspect values and pipeline execution during development.
        

---

## Completed Tasks

### 1. CEnemy setup and TakeDamage override: separate entry and processing

- Overrode `TakeDamage(...)` in `CEnemy` (or any damageable actor) to define a clear receive entry point.
    
- Delegated processing to `UCTakeDamageComponent::RequestTakeDamage(...)` instead of performing computation and HP commit inside the actor override.
    
- Kept `CEnemy` limited to `Entry` and `Routing`.
    

---

### 2. Implement CTakeDamageComponent and provide a damage-processing API

- Exposed a public entry API in `UCTakeDamageComponent`.
    
    - `RequestTakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)`
        
- Consolidated the processing flow inside `UCTakeDamageComponent`.
    
    - `RequestTakeDamage()` → `ProcessTakeDamage()` → `HandleDamageEvent()`
        

---

### 3. Standardize DamageEvent and route by type with per-event handlers

- Defined `FDefaultDamageEvent` based on `FDamageEvent` to preserve Apply-stage data required during Take.
    
    - Includes `FApplyDamageSpecKey / FApplyDamageSpec / FApplyDamageResult`.
        
    - Sets `ClassID` using `EDamageEventTypeId::DefaultDamage(0x1001)` for stable type identification.
        
- Implemented type routing via `DamageEvent.IsOfType(FDefaultDamageEvent::ClassID)` in `ProcessTakeDamage()`.
    
    - Current handler: `HandleDefaultDamageEvent(...)`.
        
    - Extensible pattern: add `HandleXDamageEvent()` for future events such as `PointDamage`, `RadialDamage`, and `StatusEffect`.
        

---

### 4. Establish the TakeDamage pipeline using Payload/Context/Result orchestration

- Implemented an orchestration pipeline inside `HandleDefaultDamageEvent` with explicit stages.
    
    - `ValidateRequest()`
        
    - `BuildPayload()`
        
    - `BuildContext()`
        
    - `EvaluateTakeDamage()` Compute / Rule stage
        
    - `CommitTakeDamage()` Resource / State stage
        
- Standardized data responsibilities across three structures.
    
    - `FTakeDamagePayload`: raw input and Apply-stage data (`Instigator`, `Causer`, `ApplyResult`, `ApplySpec`).
        
    - `FTakeDamageContext`: resolved runtime objects, state snapshot (e.g., `bWasDead`), and intermediate damage amounts.
        
    - `FTakeDamageResult`: pipeline decision and outputs (`Accepted/Rejected`, `RejectReason`, `FinalTakeDamage`, `bKilled`).
        
- Implemented conservative instigator resolution via `ResolveInstigatorController()` with clear fallback rules.
    
    - Prefer `EventInstigator`.
        
    - Fallback to `DamageCauser`’s `InstigatorController` or `Pawn Controller`.
        
    - Owner-based proxy fallback using `DamageCauser Owner`.
        

---

### 5. Implement CHealthComponent for HP resource management

- Implemented `UCHealthComponent` to isolate HP logic from damage evaluation and policy.
    
    - Initialization: `InitializeHealth(InitMaxHP, InitCurrentHP, bFillToInitMaxHP)`.
        
    - Damage: `TakeDamage(float InTakeDamageAmount)`.
        
    - Heal: `TakeHeal(float InTakeHealAmount)`.
        
    - State: `UpdateDeadState()` for dead transition updates.
        
- Added input validation and clamping to reject negative or meaningless values.
    
- Kept delegate broadcasts as TODO and focused on stable baseline behavior and debug visibility.
    
    `OnHealthChanged` `OnDead` `OnRevived`
    

---

### 6. Add debug printing for pipeline inspection

- `UCTakeDamageComponent`
    
    - `PrintTakeDamageSummaryInfo()` prints key objects and final damage summary.
        
    - `PrintTakeDamageContextInfo()` prints detailed `Payload`, `Context`, `SpecKey`, and amount breakdown.
        
- `UCHealthComponent`
    
    - `PrintTakeDamageContextInfo()` and `PrintTakeHealContextInfo()` print HP delta, percent, and dead state.
        

---

## How to Test

1. Verify the target actor (`CEnemy`) has `UCHealthComponent` and `UCTakeDamageComponent` attached.
    
2. Trigger an Apply flow and confirm `Target->TakeDamage(...)` is called.
    
3. Confirm `CEnemy::TakeDamage(...)` delegates to `UCTakeDamageComponent::RequestTakeDamage(...)`.
    
4. Confirm event routing works.
    
    - `DamageEvent.IsOfType(FDefaultDamageEvent::ClassID)` → `HandleDefaultDamageEvent(...)`
        
5. Confirm TakeDamage summary logs print expected objects and amounts.
    
    - `DamagedActor / ResolvedInstigator / ResolvedDamageCauser`
        
    - `FinalTakeDamage`
        
6. Confirm `UCHealthComponent` logs show correct HP updates.
    
    - `PreviousHP / CurrentHP / HPDelta / HPPercent / bIsDead`
        

---

## Related Issues / Branch

- Branch: `feature/combat-take-damage`
    
- Issue: #22
    

---

## Notes

- This PR focuses on establishing a maintainable TakeDamage pipeline: **event routing + Payload/Context/Result orchestration + Health responsibility separation**.
    
- `UCTakeDamageComponent` does not execute reactions or state transitions yet. Post-commit reactions remain TODO to avoid coupling growth.
    
- Instigator resolution is implemented conservatively to match common Unreal usage patterns and to support future extensions.
    

---

## In Scope (Included in this PR)

- `CEnemy::TakeDamage(...)` override and routing to TakeDamageComponent.
    
- `UCTakeDamageComponent` with the `DefaultDamageEvent` pipeline.
    
- `FDefaultDamageEvent(EDamageEventTypeId)` and `FTakeDamagePayload/Context/Result`.
    
- `UCHealthComponent` with HP decrease/increase/dead determination logic.
    
- Debug print helpers for Take and Health stages.
    

---

## Out of Scope (Excluded from this PR)

- Finalizing `FinalTakeDamage` policy (mitigation, clamp, minimum damage, cap, rounding).
    
- Duplicate-hit prevention (AlreadyHit set) and team/invulnerable/state-based filtering policy.
    
- Hit reactions (stagger, knockback, hit-stop), animation/VFX/SFX, and state transitions.
    
- Health change/death/revive delegate broadcasting.
    

---

## Follow-ups (Next Steps)

- Implement `FinalTakeDamage` policy in `EvaluateTakeDamage()`.  
    Example: split into `Mitigated` `FinalizeTaken` `FinalizeApplied`.
    
- Expand reject policies using `ETakeDamageRejectReason`.  
    Example: `AlreadyDead` `Invulnerable` `FriendlyFire` `RuleBlocked`.
    
- Add delegate broadcasts to `UCHealthComponent`.  
    Example: `OnHealthChanged / OnDead / OnRevived`.
    
- Move post-TakeDamage handling (state transitions and reaction requests) into a dedicated component.
    
- Remove unnecessary `Tick` overrides and keep alignment with `PrimaryComponentTick` being disabled.
    

---