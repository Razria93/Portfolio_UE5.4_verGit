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

- [x] Add `MoveForward` / `MoveRight` functions to `CPlayerCharacter`

- [x] Register `MoveForward` / `MoveRight` Axis mappings in **Project Settings > Input**

- [x] Verify basic movement using WASD input

- [x] Implement Jump / Land (bind `Jump` / `StopJumping`)

- [x] Set initial values for `MaxWalkSpeed`


#### 2. Basic Animation Setup

- [x] Select animation assets for Idle / Walk / Run (temporary assets are fine)

- [x] Create an AnimBlueprint for `BP_CPlayerCharacter`

- [x] Create a Speed-based Idle/Walk/Run BlendSpace

- [x] Build a basic Locomotion State (Idle/Move) in the AnimBP

- [x] From `CPlayerCharacter`, update the Speed variable passed into the AnimBP


#### 3. Git / Issues

- [x] Create a GitHub Issue for Day 3 (Title: `M1-03: CPlayerCharacter Movement/Jump & Basic AnimBP Setup`)

- [x] Commit in small units of work (include the Issue number in commit messages)

- [x] Copy this D03 checklist into Obsidian and check off items during actual progress


---

### Notes

- **Component-Based Movement Architecture**  
    Refactored movement logic by introducing `UCMovementComponent` to centralize all movement-related calculations and clearly separate responsibilities between systems
    
- **Animation Asset Migration Issue & Resolution (M1-B01: #7)**  
	- Due to Unreal Engine version differences, the original animations could not be migrated directly and resulted in **skeleton compatibility errors and retargeting failures**  
	- To resolve this, only the **Mesh and Skeleton** from the previous project were imported as the minimal required assets, and all existing animations were **retargeted to the new Mesh/Skeleton setup**, which resolved the compatibility issues
    
- **Planned Locomotion Enhancements**  
    After the basic locomotion setup, the system will be expanded with 8-way directional locomotion, motion distance-driven correction, and precise foot synchronization for higher-quality locomotion
    
- **ALS Integration Consideration**  
    Considering partial adoption of ALS (Advanced Locomotion System) patterns


---