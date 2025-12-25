# UE5 Portfolio – Issue Checklist

## Title

**M2-02: Implement Dummy Enemy base & HP structure**

### Date

- **Day 7**

- **Date : 2025.12.23


---

### Objective

- Implement a Enemy character for hit testing

- Design a basic HP/Stat structure that can be shared with the player

- Prepare a “hit target container” for the Hit/Damage system


### Branch

- feature/combat-hit-collision
- feature/combat-hit-damage


---

### TODO List

#### 1. Dummy Enemy base setup

- [x] Create a C++ or BP-based enemy character (`CDummy`)

- [x] Configure Mesh / Capsule defaults

- [x] Place multiple Dummy Enemies in the test level


#### 2. HP / Stat structure

- [ ] Define an HP/Stat structure (or component) for the enemy

- [ ] Design it so it can be shared or made compatible with the player later

- [ ] Implement HP reduction and a simple “is dead” flag when HP ≤ 0


#### 3. Hit receive preparation

- [ ] Connect an interface or component to receive Damage/Hit (only the entry point; full logic will be done in M2-03)

- [ ] Add a simple temporary reaction (log, color change, etc.) to verify that the enemy can receive hit events


---

### Notes
- 


---
