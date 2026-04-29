# UE5 Portfolio – Issue Checklist

## Title

**M01-04: implement weapon equip/unequip system and socket-based attachment**

### Date

- **Day 4**

- **Date : 2025.12.10**


---

### Objective

- Implement a character weapon **equip/unequip** system

- Handle weapon **attach/detach** using skeleton sockets

- Add a basic **equip state flag** and connect it to the character state flow


---

### Branch

- `feature/character-weapon-equip`


---

### TODO List

#### 1. System Setup

- [x] Define weapon equip state (Equipped / Unequipped)

- [x] Create weapon sockets on the character skeleton (Hand / Holster)

- [x] Add weapon reference variable and `Equip` / `Unequip` / `Toggle` functions to `CPlayerCharacter`


#### 2. Animation Integration

- [x] Prepare equip/unequip montage (or select animation sections)

- [x] Play equip/unequip animation based on input or state changes

- [x] Sync socket switching (Hand ↔ Holster) at AnimNotify timing


#### 3. Character Integration

- [x] Add and bind equip input action (toggle style, etc.)

- [x] Update internal character state (flag/enum) on equip/unequip

- [x] Design for future combat constraints (e.g., disable attack when unequipped)


---

### Notes

- **Related Issue**: `M1-B02: Fix Editor Crash from USTRUCT reference parameters` (#10)


---
---