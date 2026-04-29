# Reorganize Action Orchestration and Strengthen AI Combo / Reaction Integration

## Title

`♻️ refactor: reorganize action orchestration and strengthen ai combo / reaction integration`

## Summary

- This PR reorganizes the **shared execution flow for Player / AI around the Action Orchestration structure**, and strengthens **AI combo chaining** and **Reaction takeover** so they work inside the existing action execution flow.

- The main direction is as follows.

	- explicitly organize the `Intent -> Request -> Resolve -> Execute` flow.
	
	- align Player and AI to reuse the same **combat request path** and the same **combo chain execution path**.
	
	- keep combat blackboard state focused on **combat availability / lifecycle state**, rather than chain-window timing itself.
	
	- document the **failure-handling / rollback policy** for action execution failure and align it with the current execution structure.

- This PR also fixes the following issues that came up during development.

	- after a C++ class rename, the existing Weapon Blueprint failed to load its parent class

	- AI repeated only the first hit in `ComboAttack` and failed to chain into the next combo step
	
	- when hit during an active combo action, Reaction left action / state / blackboard out of sync and broke the combat flow


---

## Completed Items

### 1. Reorganized AI combat intent / blackboard structure

- Reorganized the AI combat decision flow around an **intent-driven state** model

- Reorganized AI combat blackboard keys around **combat-action state**

- Explicitly renamed the old `AIState` as `AIIntentState` and reorganized related names and structure
  
### 2. Reorganized combat start / cooldown ownership

- Removed the old cooldown commit task and reorganized the flow so cooldown is committed only when the combat action is actually `Started`

- `StartCombatAction` now only handles the following:

	- requesting combat action start
	
	- verifying success
	
	- committing `NextCombatActionTime` on success

### 3. Built the action event bridge

- Made the `Action -> ActionComponent -> Enemy callback` path explicit

- Based on that `callback`, AI can trigger additional behavior

- Organized the flow so AI can receive subsequent **combo chain** requests through `OnActionEvent(...)`

### 4. Unified combo chain execution flow between Player and AI

- Reorganized **combo chain** semantics around `ChainWindow`

- Routed AI combo chain requests back through the existing combat request path

- Kept the actual chain decision inside `UCAction_ComboAttack`, as before

- As a result, Player and AI now reuse the same combo chain execution flow

### 5. Strengthened Reaction takeover safety

- Added the structure so that if an active action exists when reaction starts, it is aborted first

- Reflected reaction state in combat availability calculation

- Added the minimum safety structure required so the combat flow can continue again after reaction, even when hit during an active combo action

### 6. Organized the failure-handling / rollback policy

- Documented the shared failure-handling rules for the action orchestration structure

- Core policy:

	- `Reject / Ignore` must not leave observable state changes
	
	- commit is allowed only for `Start / Chain / Enqueue / Interrupt`
	
	- cleanup responsibility remains in action lifecycle endpoints such as `Abort()` / `Complete()`
	  
	- blackboard state values are updated from actual start/end signals or calculated conditions

### 7. Strengthened Blueprint parent class migration safety

- Fixed the case where the existing Weapon Blueprint parent class failed to load after renaming `ACAttachment` to `ACWeaponActor`

- Added a `CoreRedirects` class redirect so the parent class path stored by the existing Blueprint can resolve correctly to the new C++ parent class

- Verified that the existing Weapon Blueprint is restored correctly after restarting the editor and reloading the Blueprint

### 8. Updated docs and bug reports

- `S03`: Action Orchestration State Model design

- `S04`: Action Orchestration implementation plan

- `S05`: AI Action Event Bridge / combo chain structure

- `B06`: bug report for existing Weapon Blueprint parent class load failure after a C++ class rename

- `B07`: bug report for AI repeating only the first combo hit

- `B08`: bug report for broken combat flow after reaction takeover


---

## Test Method

1. Verify that Player input executes correctly through the `Orchestrator -> ActionComponent -> Action` path

2. Verify that `NextCombatActionTime` is updated only when combat action start succeeds

3. Verify that Player `ComboAttack` still chains the same way as before

4. Verify that AI `ComboAttack` chains correctly into the next combo step after the first hit

5. Verify that the final combo step does not open an unnecessary chain window

6. Verify that combat flow recovers after reaction takeover when hit during an active combo action

7. Verify that reject / ignore paths do not leave observable state changes behind when an action request fails

---

## Related Issue / Branch

- Branch: `feature/action-orchestration`

- Related work:

  - `M05-01: Reorganize Action Orchestration and strengthen AI Combo / Reaction integration`

- Related bug reports:

  - `B06_UE5_Portfolio_Bug_Report (KR/EN)`

  - `B07_UE5_Portfolio_Bug_Report (KR/EN)`

  - `B08_UE5_Portfolio_Bug_Report (KR/EN)`


---
