## Title

🐛 fix(apply-damage): add omitted file from PR #19 (#16)

## Summary

- Added the missing header file `Source/Portfolio/Type/DamageEventId.h` that was unintentionally left out of the previous merged PR.

- This change restores build/config completeness without modifying any feature logic.

## Changes

- Added: `Source/Portfolio/Type/DamageEventId.h`

## Test Plan

- [x] Confirm the editor/project builds successfully

- [x] Run a quick sanity check for the ApplyDamage flow (e.g., verify trigger logs / compilation includes)

## Related
- Follow-up: PR #19
- Issue: #16


---