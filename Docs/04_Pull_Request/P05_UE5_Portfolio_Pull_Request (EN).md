# Combo Attack System Implementation with PreInput-Based Input Window

## Title

✨ feat: implement combo attack action using pre-input buffering and AnimNotify-based input window (#14)

## Summary

- Refactored the existing single `FActionData` structure into `TArray<FActionData>`  
  to support data-driven multi-hit (combo) actions

- Implemented `UCAction_ComboAttack` to manage  
  combo stages (Index) and chained attacks based on pre-input buffering

- Clearly separated `PlayAction()` flow into first invocation and re-entry, converting re-invoked inputs into buffered PreInput instead of executing immediately

- Added `UCAnimNotify_PreInput` to explicitly control the Combo Input Window via AnimNotify

- Adjusted existing `UCAction / UCAnimNotify_Action` structures to better support combo actions


---

## Completed Tasks

### 1. FActionData Array Conversion (Combo-Oriented Structure)

- Converted `FActionData` from a single data structure to `TArray<FActionData>`

- Enabled per-combo-step configuration for Montage / PlayRate / movement control

- Updated `UCAction::InitializeAction()` to inject and manage multiple `FActionData` entries


---

### 2. UCAction_ComboAttack Implementation

- Implemented `UCAction_ComboAttack`

  - Introduced an `Index` variable to track combo stages

  - Added PreInput state flags
    - `bEnablePreInput` : indicates whether the input window is active
    - `bExistPreInput`  : indicates whether a buffered input exists

- `PlayAction()` flow separation

  - **Re-invocation**
    - When `PlayAction()` is re-invoked while the input window is active  
      → the input is buffered as PreInput instead of being executed immediately

  - **First invocation**
    - After validating Idle state and weapon/state conditions  
      → executes the first combo montage

- `Next_PlayAction()`

  - Advances to the next combo stage only when `bExistPreInput == true`

  - Increments the Index and plays the next `FActionData` montage

- `End_PlayAction()`

  - Resets the Index and PreInput states when the combo finishes


---

### 3. AnimNotify-Based Combo Input Window Control

- Implemented `UCAnimNotify_PreInput`

- Explicitly defined input-acceptable time windows (Input Window) within combo montages

- `FlowType (Begin / End)` behavior

  - `Begin` : calls `UCAction_ComboAttack::OnEnablePreInput()`
  - `End`   : calls `UCAction_ComboAttack::OffEnablePreInput()`

- Ensures that only inputs within the combo window are recognized as valid PreInput


---

### 4. Action / AnimNotify Structure Adjustments

- Partially adjusted `UCAction` and `UCAnimNotify_Action` structures

- Refined interfaces to support action extensions such as ComboAttack

- Clarified responsibility boundaries by moving combo execution logic fully into Action classes


---

## Test Instructions

1. Launch the project and enter the test level

2. Verify that the Sword weapon is equipped

3. Press the Action key and confirm the first attack executes correctly

4. Combo test

   - While the attack montage is playing,  
     press the Action key again within the  
     `UCAnimNotify_PreInput (Begin ~ End)` window

   - Verify that the next combo montage plays correctly

5. Input Window validation

   - Confirm that inputs outside the combo window do not advance the combo


---

## Related Issues / Branch

- Branch: `feature/combat-combo-attack`

- Issue: #14


---

## Notes

- **Input Window**
  - A time window during which player input is considered meaningful
  - Explicitly opened and closed via AnimNotify

- **PreInput Buffering**
  - Stores player input instead of executing it immediately,
    then consumes it at the next combo transition point

- Responsibility separation

  - `CPlayerController / CPlayer`  
    → input routing and intent handling

  - `CWeaponComponent`  
    → action ownership and execution entry point

  - `UCAction_ComboAttack`  
    → combo flow, pre-input handling, and stage management

  - `UCAnimNotify_PreInput`  
    → animation-timing-based input window control


---