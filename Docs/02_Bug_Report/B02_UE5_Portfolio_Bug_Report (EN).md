# UE5 Portfolio – Bug Report (EN)

## Title

**M01-B02: Fix Editor Crash from USTRUCT reference parameters**

### Date

- **Day 5**

- **2025.12.16**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/character-weapon-equip`


---

## Summary

- Fixed an **editor crash in the Property Editor** caused by using `const&` parameters with a reflected `USTRUCT` (`FEquipmentData`) in initialization / function parameters.


---

## Environment

- Engine: Unreal Engine 5.4
  
- Editor area: Details Panel / Property Editor (editing struct data)
  
- Affected data: `FEquipmentData` (montage, play rate, movement flag)


---

## Steps to Reproduce

1. Define `FEquipmentData` as a `USTRUCT` used for editor-exposed settings.
    
2. Implement `FEquipmentData` through a `UPROPERTY()`.
    
3. Use a function signature that takes the struct as a **reference parameter**, e.g.:
    
    - `InitializeEquipment(ACharacter* InOwnerCharacter, const FEquipmentData& InEquipData, const FEquipmentData& InUnequipData)`
	
4. Open the editor Details panel and interact with the properties related to `FEquipmentData`.
    
5. The editor intermittently crashes.


---

## Expected vs Actual

**Expected**

- Editing `FEquipmentData` in the Details panel should be stable and never crash the editor.

**Actual**

- The editor crashes during Property Editor evaluation with an access violation when `FEquipmentData` is passed as `const&` through reflected/editor paths.


---

## Root Cause

- Passing `USTRUCT` parameters by `const&` in a context that is touched by **reflection/editor evaluation** can lead to **lifetime issues**:
  the Details panel / property system may construct temporary struct instances or evaluate values in a way that does not guarantee the referenced memory stays valid for the full call chain.
	
- As a result, a `const&` parameter may end up referencing memory that is no longer valid (dangling reference), which can trigger crashes in the Property Editor.


---

## Fix

1. Marked the struct as editor-safe and properly reflectable:
    
    - `USTRUCT(BlueprintType)` + `UPROPERTY` fields inside `FEquipmentData`
        
2. Changed initialization/function parameters from reference to pass-by-value:
    
    - `const FEquipmentData&` → `FEquipmentData`
        
3. Verified that `FEquipmentData` can be edited in the Details panel without causing crashes.


---

## Verification

1. Open the editor and select the actor/asset where `FEquipmentData` is exposed.
    
2. In the Details panel:
    
    - Modify montage / play rate / movement flag values
        
    - Recompile BP / refresh selection (if applicable)
        
3. Confirm:
    
    - No Property Editor crash occurs
        
    - Updated values apply correctly at runtime (equip/unequip flow still works)


---

## Notes

- For **editor/reflection-facing APIs**, prefer:
    
    - pass-by-value for `USTRUCT` parameters, or use a stable owning container (e.g., store data in a `UPROPERTY` member and pass pointers/IDs), rather than relying on references.
        
- If a struct is intended for editor use, ensure it is:
    
    - `USTRUCT(BlueprintType)` and its members are `UPROPERTY`-annotated.


---