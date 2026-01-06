# UE5 Portfolio – Issue Checklist

## Title

**M2-01: Implement combo attack (2–3 hit) system**

### Date

- **Day 6**

- **Date : 2025.12.21**


---

### Objective

- Implement 2–3 hit combo attacks based on the first light attack

- Define combo input timing (input window)

- Establish the initial combo flow integrated with the attack state


---

### Branch

- feature/combat-combo-attack


---

### TODO List

#### 1. Combo structure design

- [x] Design the combo structure where Light Attack 1st hit can chain into 2nd and 3rd hits

- [x] Define variables for managing combo stages (e.g. AttackIndex)

- [x] Define reset rules when there is no input or when timing fails


#### 2. Animation & montage

- [x] Prepare animations or montage sections for 2nd and 3rd hits

- [x] Configure the montage to smoothly connect 1st → 2nd → 3rd hit

- [x] Use AnimNotifies or section end events to mark the input window for the next hit


#### 3. Input handling & state integration

- [x] Implement logic for combo input (input buffering or “press again to advance to the next hit”)

- [x] Ensure combo progression is only allowed while in Attack state

- [x] Reset internal counters and return to Idle/Move when the combo finishes


---

### Notes
- 


---