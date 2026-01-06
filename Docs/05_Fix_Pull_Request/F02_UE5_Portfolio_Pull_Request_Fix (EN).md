## Title

🐛 fix(apply-damage): add omitted files from PR #19 (#16)

## Summary

- Added files that were unintentionally left out of the previously merged PR #19 because they were still stashed
    
- Restored build/config completeness by including only the omitted files without changing any feature logic
    

## Changes

- Added: `Source/Portfolio/Character/Enemy/CEnemy.h`
    
- Added: `Source/Portfolio/Character/Enemy/CEnemy.cpp`
    
- Added: `Source/Portfolio/Component/CApplyDamageComponent.cpp`
    
- Added: `Source/Portfolio/Type/CWeaponStructure.h`
    

## Test Plan

- [x] Confirm the editor/project builds successfully
    
- [x] Run a quick sanity check for the ApplyDamage flow (e.g., verify trigger logs / compilation includes)
    

## Related

- branch: `fix/apply-damage-omitted-stash-file`
    
- Follow-up: PR #19
    
- Issue: #16
    

---
