# UE5 Portfolio – Issue Checklist

### Date

- **Day 3**
    
- **2025.12.03**
    

---

### Objective

- Implement basic CPlayerCharacter movement logic (Idle/Walk/Run)
    
- Implement jump and do a first pass on movement-related parameter tuning
    
- Build a basic AnimBP-driven locomotion (Idle/Move) state machine skeleton
    

### Branch

- `feature/character-move-core`
    

---

### TODO List

#### 1. Movement / Jump Logic

- [ ] Add `MoveForward` / `MoveRight` functions to `CPlayerCharacter`

- [ ] Register `MoveForward` / `MoveRight` Axis mappings in **Project Settings > Input**

- [ ] Verify basic movement using WASD input

- [ ] Implement Jump / Land (bind `Jump` / `StopJumping`)

- [ ] Set initial values for `MaxWalkSpeed` / `JumpZVelocity` / `AirControl`


#### 2. Basic Animation Setup

- [ ] Select animation assets for Idle / Walk / Run (temporary assets are fine)

- [ ] Create an AnimBlueprint for `BP_CPlayerCharacter`

- [ ] Create a Speed-based Idle/Walk/Run BlendSpace

- [ ] Build a basic Locomotion State (Idle/Move) in the AnimBP

- [ ] From `CPlayerCharacter`, update the Speed variable passed into the AnimBP


#### 3. Git / Issues

- [ ] Create a GitHub Issue for Day 3 (Title: `M1-03: CPlayerCharacter Movement/Jump & Basic AnimBP Setup`)

- [ ] Commit in small units of work (include the Issue number in commit messages)

- [ ] Copy this D03 checklist into Obsidian and check off items during actual progress


---

### Notes

- 


---