# UE5 Portfolio – Milestones & Timeline

## **Keywords**

- Milestone definitions & version tag planning
    
- Weekly goals for a 1-month period (2025.12.01 ~ 12.31)
    
- MVP vs Extended scope
    
- Git Tag / Issue / PR workflow guideline
    

---
---
# **1. Version Tag / Milestone Overview**
## **M0 – Initial Environment & Workflow Setup**

**Duration:** 2025.12.01 (Day 1)

### **Tasks**

- [x] Install UE5.4 and create a new project
    
- [x] Set up Git/GitHub repository and `.gitignore`
    
- [x] Set up Obsidian `docs/` structure
    - `00_plan`, `01_daily`, `02_design`, `03_notes`
        
- [x] Set up GitHub Issues & Kanban board
    
- [x] Add Git/GitHub workflow docs
    
- [x] Add Plan-Overview, Milestones, Issue-Checklist docs
    

**Git Tag:** `v0.0-setup`

---

## **M1 – Character & Combat Core (v0.1)**

**Duration:** Month 1 (~ 2025.12.31)

### **Tasks**

- [x] Implement `PlayerCharacter` / `PlayerController` (C++)
    
- [x] Add third-person camera (SpringArm)
    
- [x] Basic movement (move/jump)
    
- [x] Weapon equip/unequip
    
- [x] Implement Basic attack system
    
- [x] Create “Test Room” for early combat testing
    

### **Completion Criteria**

- [x] Player can move, jump, dodge, and perform basic combos
    
- [x] Attack availability changes depending on weapon state
    

**Git Tag:** `v0.1-character-combat-core`

---

## **M2 – Hit, Damage, Dummy Enemy, Targeting (v0.2)**

**Duration:** Month 2 (~ 2026.01.31)

### **Tasks**

#### Implement hit & hit-reaction
	
- [x] Implement combo attack system
    
- [x] Add Dummy Enemy

- [x] Implement Hit detection
    
- [x] Implement Damage system (HP decrease, death, destructible object)
    
- [x] Implement hit/death reaction (player / enemy)
      

#### Implement lock-on targeting system
    
- [ ] Basic lock-on targeting system
    

### **Completion Criteria**

- [x] Functional 1v1 combat loop (player vs dummy enemy)
    
- [ ] Camera and character correctly track locked target
    

**Git Tag:** `v0.2-hit-damage-targeting`

---

## **M3 – Enemy AI & Advanced Combat (v0.3)**

**Duration:** Month 3 (~ 2026.02.28)

### **Tasks**

#### Implement Enemy AI (FSM)

- [x] Idle
    
- [x] Move
    
- [x] Chase
    
- [x] Attack
    
- [x] Hit reaction
    
- [x] Death


#### Implement Advanced Player Combat

- [ ] Guard / Guard Break
    
- [ ] Parry
    
- [ ] Dodge
    

#### Integration

- [ ] Player and enemy interact using advanced combat logic
    

### **Completion Criteria**

- [ ] Stable combat loop with guard, parry, dodge, and basic attacks
    
- [ ] Functional 1-vs-multiple enemy scenario
    

**Git Tag:** `v0.3-enemy-ai-and-advanced-combat`

---

## **M4 – VFX & UI (v0.4)**

**Duration:** Month 4 (~ 2026.03.31)

### **Tasks**

### VFX

- [ ] Attack impact particles
    
- [ ] Hit reaction particles
    
- [ ] Simple destruction effects
    

### UI

- [ ] HP bar
    
- [ ] Resource UI
    
- [ ] Damage UI
    

### Demo

- [ ] Prepare short playable demo
    

### **Completion Criteria**

- [ ] Playable demo with combat, UI, and VFX feedback
    

**Git Tag:** `v0.4-vfx-and-ui`

---

## **M5 – Combat Orchestration Structure (v0.5)**

**Duration:** Month 5 (~ 2026.04.30)

### **Tasks**

- [ ] Organize the flow from Player input to Action execution around the Orchestrator
    
- [ ] Separate state transition confirmation from Action execution
    
- [ ] Redefine responsibilities of `ActionComponent` and `CAction`
    
- [ ] Organize rollback flow when Action execution fails
    
- [ ] Prepare the execution structure for later AI / Reaction / Guard / Parry expansion
    

### **Completion Criteria**

- [ ] Player action execution flow can be explained as `Input -> Orchestrator -> State Transition -> Action Execution`
    
- [ ] `CAction` no longer owns direct state transition responsibility
    
- [ ] Existing ComboAttack / LightAttack behavior remains intact
    

**Git Tag:** `v0.5-combat-orchestration`

---

## **M6 – Polish & Documentation (v0.6)**

**Duration:** Final cleanup phase

### **Tasks**

- [ ] Capture gameplay demo video
    
- [ ] Write technical/design documents (`docs/`)
    
- [ ] Write the extended roadmap
    

### **Completion Criteria**

- [ ] Portfolio fully understandable from GitHub repo + video + docs
    

**Git Tag:** `v0.6-polish-and-docs`

---

# **2. Tag / Milestone Check Table**

| Milestone | Tag Name                      | Summary                                        | Week | Done |
| --------- | ----------------------------- | ---------------------------------------------- | ---- | :--: |
| **M0**    | –                             | Environment setup & workflow                   | W0   |      |
| **M1**    | `v0.1-character-combat-core`  | Character/Camera/Movement/Basic Combo          | W1   |      |
| **M2**    | `v0.2-hit-damage-targeting`   | Hit/Damage/Enemy/Targeting                     | W2   |      |
| **M3**    | `v0.3-ai-and-advanced-combat` | Enemy AI & Advanced Combat (Guard/Parry/Dodge) | W3   |      |
| **M4**    | `v0.4-vfx-and-ui`             | VFX/UI/Documentation                           | W4~5 |      |
| **M5**    | `v0.5-combat-orchestration`   | Combat Orchestration structure                 | W5   |      |
| **M6**    | `v0.6-polish-and-docs`        | Polish & Documentation                         | Final|      |

---
---
