# Implement weapon equip/unequip system with AnimNotify-driven flow

## Title

✨ feat: implement weapon equip/unequip system and AnimNotify-driven flow (#9, #10)

## Summary

- This PR drives the **weapon equip/unequip flow** via **Montage + AnimNotify timing**, and integrates it with `CStateComponent` (state), `CMovementComponent` (movement control), and `CAttachment` (socket attachment).
    
- It also prevents an **editor (Property Editor) crash** by marking `FEquipmentData` as `USTRUCT(BlueprintType)` and switching USTRUCT parameters from `const&` to **pass-by-value**, eliminating **USTRUCT lifetime / dangling-reference risks** during Property Editor evaluation.
    

---

## Completed Tasks

### 1. weapon type / state type change flow (foundation)

- Implemented weapon type/mode switching in `CWeaponComponent`
    
    - Broadcasts changes via `FWeaponTypeChanged` delegate
        
- Implemented state type/mode management in `CStateComponent`
    
    - Broadcasts changes via `FStateTypeChanged` delegate
        
- Connected player input routing in `CPlayer`
    
    - `HandleSword()` → `CWeaponComponent::SetSwordMode`
        
- Updated `CAnimInstance` to react to weapon type changes via delegate binding
    
    - Binds `OnWeaponTypeChanged` and updates AnimBP variables
        

---

### 2. equipment system (equip/unequip lifecycle)

- Implemented `CEquipment` lifecycle
    
    - `Equip`, `Begin_Equip`, `End_Equip`
        
    - `Unequip`, `Begin_Unequip`, `End_Unequip`
        
- Added movement control during equip/unequip
    
    - Implemented `CMovementComponent::SetStop` / `SetMove`
        
    - Uses `FEquipmentData::bCanMove` to determine movement lock/unlock behavior
        
- Extended equip-related state definitions
    
    - Added `Equip` (and optionally `Unequip`) to `CStateStructure`
        
- Extended equip data definitions
    
    - Added `FEquipmentData` to `CWeaponStructure`
        
    - Exposed `EquipmentData` and `UnequipmentData` in `CWeaponComponent`
        
    - `CWeaponComponent` creates/initializes `CEquipment` and provides `GetEquipment()` accessor
        

---

### 3. attachment + socket setup (hand/holster)

- Extended `CAttachment` socket attachment logic
    
    - Supports hand/holster sockets
        
    - Allows socket name configuration via editor/BP data
        
- Bound `CEquipment` events to `CAttachment` to switch sockets at equip/unequip timing
    
    - `OnEquipmentBeginEquip` → attach to hand
        
    - `OnEquipmentBeginUnequip` → attach to holster
        

---

### 4. animation-timed equip/unequip (AnimNotify)

- Implemented AnimNotify integration
    
    - `CAnimNotify_Equip` → `CEquipment::Begin_Equip` / `End_Equip`
        
    - `CAnimNotify_Unequip` → `CEquipment::Begin_Unequip` / `End_Unequip`
        
- Added/updated related montage and blend assets
    
    - draw/sheath montages
        
    - upper/full-body layered blending (based on current project setup)
        

---

### 5. editor crash fix (Property Editor stability)

- Applied `USTRUCT(BlueprintType)` to `FEquipmentData` and exposed fields via `UPROPERTY`
    
- Replaced `const FEquipmentData&` initialization parameters with pass-by-value
    
- Prevented Property Editor crashes by removing **USTRUCT lifetime (dangling reference) risks** during evaluation/render/clone paths
    

---

## How to Test

1. Run the project and play in the test level
    
2. Trigger weapon input (e.g., `PressSword` binding)
    
3. Verify equip flow:
    
    - draw montage plays
        
    - state switches to equip
        
    - movement stop/move behaves as configured by data
        
    - attachment moves to the hand socket at the AnimNotify timing
        
4. Verify unequip flow:
    
    - sheath montage plays
        
    - attachment returns to the holster socket at the AnimNotify timing
        
    - state returns to idle
        
5. In BP/Details panel, edit `EquipmentData` and confirm:
    
    - no editor crash occurs
        

---

## Related Issue / Branch

- Branch: `feature/character-weapon-equip`
    
- Issues:
    
    - #9 (equip/unequip system integration)
        
    - #10 (fix issue caused by using `const&` in USTRUCT parameters)
        

---

## Notes

- Responsibility split:
    
    - `CPlayerController / CPlayer`: input handling and routing
        
    - `CWeaponComponent`: creates and owns `CAttachment` / `CEquipment`
        
    - `CEquipment`: state transitions, movement control, montage-timed flow control
        
    - `AnimNotify`: triggers equip/unequip begin/end timing
        
- `FEquipmentData` is now structured for safe editor usage and can be reused across other weapons.

---
