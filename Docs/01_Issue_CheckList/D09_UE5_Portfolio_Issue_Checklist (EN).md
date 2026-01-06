# UE5 Portfolio – Issue Checklist

## Title

**M2-04: Implement TakeDamage receiver (override + temporary verification feedback)**

### Date

- **Day 9**

- **Date : 2026.01.06**


---

### Objective

- Implement the target-side receive entry by overriding `CEnemy::TakeDamage(...)`, and validate that dispatch from `CApplyDamageComponent` (M2-03) reaches the receiver correctly.

- Validate explicit branching of `FDamageEvent` via `DamageEvent.GetTypeID()`.

- Keep `CEnemy::TakeDamage(...)` limited to minimal validation only, then delegate processing to `UCTakeDamageComponent`.

- Perform target-state-based mitigation/adjustment in `UCTakeDamageComponent::HandleTakeDamage(...)`.

- Apply the final outcome through `CHealthComponent` and `CReactionComponent` (minimal HP reduction + minimal hit reaction for this milestone).


---

### Branch

- feature/combat-take-damage


---

### TODO List

#### 1. CEnemy::TakeDamage receive entry (override)

- [ ] Override `CEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)`

- [ ] Perform minimal validation only (null/self/amount/required pointers), then delegate immediately

- [ ] Explicitly branch event types via `DamageEvent.GetTypeID()`  
  - Handle default path (default event)  
  - Include project custom event id path (if used)

- [ ] Delegate call: resolve `UCTakeDamageComponent` and forward to `HandleTakeDamage(...)` only


#### 2. UCTakeDamageComponent setup and processing pipeline

- [ ] Create `UCTakeDamageComponent` and ensure it is attached/initialized on Enemy

- [ ] Define the component entry point: `HandleTakeDamage(...)`
  - Input: `DamageAmount`, `DamageEvent`, `EventInstigator`, `DamageCauser`  
  - Output: final applied damage (or a processing result struct)

- [ ] Define target-state-based mitigation/validation inside `HandleTakeDamage(...)`
  - Examples: invincibility/guard/defense/status restrictions (minimal implementation only for this milestone)


#### 3. Health processing (minimal integration with CHealthComponent)

- [ ] Create `CHealthComponent` or connect an existing component

- [ ] Finalize HP reduction API (e.g., `TakeDamageToHealthPoint(float FinalDamage)`)

- [ ] Connect HP reduction and dead flag handling (HP <= 0)

- [ ] Validate via logs (before/after values)


#### 4. Reaction processing (minimal integration with CReactionComponent)

- [ ] Create `CReactionComponent` or connect an existing component

- [ ] Finalize minimal hit reaction API  
  - e.g., `OnHitReact(const FHitReactContext&)` or `PlayHitReaction(...)`

- [ ] Implement minimal reaction only for this milestone  
  - Required: log output  


#### 5. Structured logs and debug output (verification feedback)

- [ ] Enforce “one receive log per TakeDamage call” to prevent duplicate spam

- [ ] Standardize log format and minimum fields  
  - Target, DamageAmount  
  - EventInstigator, DamageCauser  
  - DamageEvent TypeID (+ Custom Id / SpecKey if available)


#### 6. Integration validation scenarios (with M2-03)

- [ ] Verify `CApplyDamageComponent::ApplyDamageToTarget(...)` → `CEnemy::TakeDamage(...)` linkage

- [ ] Verify `CEnemy::TakeDamage(...)` → `UCTakeDamageComponent::HandleTakeDamage(...)` delegation

- [ ] Verify `UCTakeDamageComponent` → `CHealthComponent` / `CReactionComponent` calls

- [ ] Validate call frequency per intended hit event (check unintended duplicates within a single overlap window)

- [ ] Validate multi-target scenarios (multiple Enemies placed in the test level)


---

### Notes
- 


---
