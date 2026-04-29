# UE5 Portfolio – Issue Checklist

## Title

**M02-02: Implement Dummy Enemy base (hit test target)**

### Date

- **Day 7**

- **Date : 2025.12.23**


---

### Objective

- Implement a minimal Enemy character to be used as a stable hit test target in the test level

- Configure Mesh / Capsule / MovementComponent defaults and ensure consistent Collision/Overlap behavior

- Provide a clean baseline for follow-up issues (M2-03~05) to integrate Hit/Damage/HP/UI


---

### Branch

- feature/combat-hit-collision


---

### TODO List

#### 1. Enemy base setup

- [x] Create an Enemy character (C++ or BP-based) (`CEnemy`)

- [x] Configure Mesh / Capsule / MovementComponent defaults

- [x] Define the default Collision/Overlap policy for hit testing


#### 2. Hit test readiness (visibility)

- [x] Add a minimal target identifier output (name/tag/log)

- [x] Add a temporary log to confirm overlap/hit entry (verification only)


---

### Notes
- 


---