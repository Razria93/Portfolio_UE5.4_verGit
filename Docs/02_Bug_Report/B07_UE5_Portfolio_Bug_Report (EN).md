# UE5 Portfolio Bug Report (EN)

## Title

**M4-B07: AI ComboAttack repeats only the first hit and never chains to the next combo step**

### Date

- **2026.04.29**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/action-orchestration`


---

## Summary

- After starting `ComboAttack`, AI failed to continue into the next combo step and kept repeating only the first hit.

- The Player could already advance through the existing `Chain` path, but AI was not reusing the same flow, so combo chaining broke for AI.

- This was fixed by connecting Notify / Action / ActionComponent / Enemy through an action event callback path and making AI reissue the same combat request so that the Action could resolve it as `Chain`.

- The structure was also tightened so that the chain window is not opened on the final combo step.


---

## Environment

- Engine: Unreal Engine 5.4

- Branch:

  - `feature/action-orchestration`

- Related Code:

  - `Source/Portfolio/Action/CAction_ComboAttack.cpp`
  - `Source/Portfolio/Action/CAction_ComboAttack.h`
  - `Source/Portfolio/Action/CAction.cpp`
  - `Source/Portfolio/Action/CAction.h`
  - `Source/Portfolio/Component/CActionComponent.cpp`
  - `Source/Portfolio/Component/CActionComponent.h`
  - `Source/Portfolio/Character/Enemy/CEnemy.cpp`
  - `Source/Portfolio/Character/Enemy/CEnemy.h`
  - `Source/Portfolio/Notify/CAnimNotify_ChainWindow.cpp`
  - `Source/Portfolio/Notify/CAnimNotify_ChainWindow.h`

- Related Asset:

  - `Content/02_Controller/02_Enemy/AI/BehaviorTree/State/Sub/SBT_Attack.uasset`
  - `Content/04_Montage/Sword/M_Attack_Sword_0.uasset`
  - `Content/04_Montage/Sword/M_Attack_Sword_1.uasset`
  - `Content/04_Montage/Sword/M_Attack_Sword_2.uasset`


---

## Reproduction Steps

1. Let the AI start `ComboAttack` during combat.

2. Allow the first hit montage to progress into the timing window where combo follow-up should occur.

3. Check whether the AI continues to the next combo step.


---

## Expected Result

- AI should receive a `Chain` decision through the same `ComboAttack` flow used by the Player.

- At the chain window timing, the same combat request should be reissued, and the combo should advance through `ApplyChain()` and `AdvanceCombo()`.

- The result should be a continuous 1-hit, 2-hit, 3-hit combo flow.


---

## Actual Result

- AI could start the first hit, but failed to continue to the next combo step.

- Even after the chain timing window, the combo did not advance and instead restarted from the first hit again.

- In practice, the AI combo looked like repeated first-hit playback instead of a linear combo chain.


---

## Cause

- AI combo continuation was not reusing the Action-internal `Chain` path.

- The Player reissued the same action request and received a `Chain` decision, but AI did not route Notify timing back into that same execution path.

- The remaining `PreInput`-style semantics also made Player combo and AI combo behavior diverge.


---

## Fix

The structure was reorganized as follows.

```text
AnimNotify
-> UCAction_ComboAttack::OpenChainWindow()
-> UCAction::EmitActionEvent(...)
-> UCActionComponent::OnActionEvent.Broadcast(...)
-> ACEnemy::OnActionEvent(...)
-> ACEnemy::RequestChainCombatAction(...)
-> ACEnemy::HandleAICombatAction(...)
-> UCAction::DecideExecution()
-> Chain
```

- The old `PreInput` wording was reorganized around `ChainWindow`.

- When AI receives `ChainWindowOpened`, it now reissues the same combat request through `HandleAICombatAction()`.

- The actual chain decision remains inside `UCAction_ComboAttack::DecideExecution()`.

- `CanAdvanceCombo()` now prevents `OpenChainWindow()` from opening on the final combo step.


---

## Verification Result

- Confirmed that AI now advances correctly to the second and third combo steps after starting `ComboAttack`.

- Confirmed that both Player and AI use the same `Chain` execution path.

- Confirmed that no unnecessary chain follow-up request is generated on the final combo step.


---

## Follow-Up Cleanup

- If combo branches are introduced later, follow-up policy can branch on `ActionType + ActionIndex`.

- The current structure is sufficient for linear combo chaining, while branch combo / enqueue / interrupt remain future extension points.


---

## Notes

- The key fix was not to build a separate AI-only combo system, but to make AI reuse the same Action-driven chain path that the Player already used.

- Notify only calls Action methods, Action exposes events through ActionComponent, and Enemy only reconnects chain timing back into the existing combat request path.


---
