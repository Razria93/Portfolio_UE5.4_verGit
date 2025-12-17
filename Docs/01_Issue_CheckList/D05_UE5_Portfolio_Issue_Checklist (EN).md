# UE5 Portfolio – Issue Checklist

## Title

**M1-05: Implement light attack core system**

### Date

- **Day 5**

- **Date : 2025.12.18**


---

### Objective

- Implement input and trigger logic for the first light attack

- Set up light attack animation and montage playback flow

- Configure hit timing via AnimNotifies and define the first attack state / input flow


### Branch

- feature/combat-light-attack

---

### TODO List

#### 1. Attack Action

- [ ] Add and bind the LightAttack input action

- [ ] Implement the LightAttack trigger function in CPlayerCharacter

- [ ] Enforce “weapon equipped” as a prerequisite (block attack when unequipped)


#### 2. Animation Logic

- [ ] Prepare an animation or montage for the first light attack

- [ ] Implement the flow: LightAttack input → montage playback (in C++ or Blueprint)

- [ ] Add AnimNotifies to define hit-check timing during the attack


#### 3. Combat State / Input Handling

- [ ] Switch ECharacterState to Attack when the attack starts

- [ ] Apply initial movement / rotation / equip input restrictions while in Attack

- [ ] Ensure a proper return to Idle/Move state when the montage ends or conditions are met


---

### Notes

- 


---