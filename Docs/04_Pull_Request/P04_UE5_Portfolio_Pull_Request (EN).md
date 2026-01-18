# Action input routing and AnimNotify-driven action flow

## Title

✨ feat: implement Action input routing and AnimNotify-driven action execution / light attack (#12)

## Summary

- Added an **Action** key and routed the input through `CPlayerController → CPlayer → CWeaponComponent → CAction` to establish a consistent action execution path

- Introduced `FActionData` to drive montage playback (including `PlayRate`) and movement restriction (`bCanMove`) in a data-driven way

- Implemented `UCAction` as an abstract base class and consolidated action creation/initialization and `PlayAction()` invocation inside `CWeaponComponent`

- Implemented `UCAnimNotify_Action` to trigger `Begin_PlayAction / End_PlayAction` at montage timing via AnimNotifies

- Implemented `UCAction_LightAttack` to execute a montage-based light attack from Idle and properly clean up state/movement on finish


---

## Completed Tasks

### 1. Action input binding and call chain

- Added Action input binding in `CPlayerController`

- `PressAction()` calls `CPlayer::HandleAction()`

- `CPlayer::HandleAction()` calls `CWeaponComponent::PlayAction()` when the current weapon type is Sword

- `CWeaponComponent::PlayAction()` executes `UCAction::PlayAction()`


---

### 2. Add `FActionData` structure

- Implemented `FActionData`

  - Exposed `Montage`, `PlayRate`, and `bCanMove` via `UPROPERTY(EditAnywhere)`

  - Added default initialization (e.g., `Montage = nullptr`, `PlayRate = 1.0f`, `bCanMove = true`)

- Implemented `PlayMontage()` / `Begin_PlayMontage()` / `End_PlayMontage()`

  - When `bCanMove == false`, movement is restricted via `UCMovementComponent::SetStop/SetMove`

  - If a montage is valid, `Begin_PlayMontage()` plays it via `PlayAnimMontage(Montage, PlayRate)`


---

### 3. Implement `UCAction` base class and integrate initialization in WeaponComponent

- Implemented `UCAction` as an abstract base class

  - Injected OwnerCharacter / ActionData and cached `UCStateComponent`

  - Provided `PlayAction / Begin_PlayAction / End_PlayAction` APIs

  - Handled base state transitions (e.g., enter ActionMode and return to Idle)

- Created/initialized Action in `CWeaponComponent`

  - Created the action via `NewObject<UCAction>()` using `ActionClass` (TSubclassOf)

  - Injected data via `InitializeAction(OwnerCharacter, ActionData)`


---

### 4. AnimNotify-driven Begin/End action timing

- Configured `UCAnimNotify` base

  - `FlowType (Begin/End)` can be set in the editor

  - Provided utilities to resolve `CWeaponComponent` from MeshOwner

  - Built notify display name to include FlowType via `MakeNotifyName()`

- Implemented `UCAnimNotify_Action`

  - Retrieves the current action via `CWeaponComponent::GetAction()` and calls:

    - `FlowType == Begin` → `Begin_PlayAction()`

    - `FlowType == End` → `End_PlayAction()`


---

### 5. Implement `UCAction_LightAttack`

- Implemented `UCAction_LightAttack`

  - `PlayAction()`:

    - Enters ActionMode via `Super::PlayAction()`

    - Plays montage and applies movement restriction via `ActionData.Begin_PlayMontage()`

  - `End_PlayAction()`:

    - Returns to Idle via `Super::End_PlayAction()`

    - Releases movement restriction via `ActionData.End_PlayMontage()`


---

## Test Plan

1. Run the project and play in the test level

2. Switch to Sword mode (using the existing weapon-switch key)

3. Press the Action key (e.g., `Action` binding)

4. Verify light attack behavior:

   - Montage plays correctly

   - State transitions to ActionMode during the action

   - Movement restriction behaves correctly based on `bCanMove`

5. Verify AnimNotify behavior with `UCAnimNotify_Action` (Begin/End) placed on the montage:

   - Confirm `Begin_PlayAction / End_PlayAction` are triggered at the correct notify timing


---

## Related Issues / Branch

- Branch: `feature/combat-light-attack`

- Issue: #12 


---

## Notes

- Responsibility split:

  - `CPlayerController`: input binding and pawn routing

  - `CPlayer`: intent handling (weapon/state checks before calling the component)

  - `CWeaponComponent`: owns action creation/initialization/execution (`PlayAction`)

  - `UCAction`: action execution unit (state transitions + data-driven montage)

  - `UCAnimNotify_Action`: animation-timed Begin/End triggers

- `HandleAction()` currently triggers actions only in Sword mode (can be extended per weapon type later)


---