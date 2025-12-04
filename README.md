# **UE5 Portfolio – Plan Overview**

## **Keywords**

- UE5 portfolio project overview and goal summary
- Scope definition
- Timeline & milestone planning
- Tools overview
- Documentation directory structure


---
## **Details**

### **1. Project Overview**

- **Project Name:** UE5 Combat Portfolio
- **Engine Version:** Unreal Engine 5.4
- **Target Platform:** Windows PC (editor execution)
- **Project Duration:** 1 month (MVP scope) + @ (extended scope)


---

### **2. Goal Summary**

- **Genre**:
    - Third-person action combat demo
    
- **Primary Goals**:
    1. Understand and implement the essential components of a UE5 gameplay project
    2. Practice a `production-like workflow` including Git, Issues, PRs, and documentation
  

---

### **3. Scope Definition**

#### **3.1 MVP Scope (Required)**

1. **Player Character**
    - Movement / Jump / Dodge (Roll)

2. **Camera & Input**
    - Third-person camera
    - Lock-on system

3. **Combat System (Basic)**

    - Equip / Unequip a single melee weapon
    - Basic attacks + combo chain
    - Basic enemy (dummy + simple combat AI)
    - Melee hit detection (hit events + hit reactions)
    - Damage system (HP reduction, death, destruction)

4. **Presentation**

    - Attack and hit VFX (Niagara or Particle)
    - Resource UI (HP + conditional resource)
    - Damage UI
    - Additional UI required for development

5. **Level Setup**

    - Test Room (feature-specific testing area)
    - Main World (demo play environment)
  

---

#### **3.2 Extended Scope (Optional Candidates)**

1. **Advanced Combat**

    - Hit stop
    - Aerial attacks & air combos
    - Down attacks
    - Execution system
    - Parry / Guard / Perfect Dodge
    - Weapon switching (melee / ranged / magic)
    - Skill activation

2. **AI Expansion**

    - Enhanced combat behaviors (idle / alert / attack / defend / evade / retreat)
    - AI team behavior & faction recognition
    - Multiple enemy types (melee / ranged / magic)
    - Wave system
    - Mini-boss

3. **Movement & Parkour**

    - Parkour actions
    - Teleport movement

4. **Advanced Animation**

    - Foot IK / Hand IK
    - ALS-style movement system
    - 8-way directional animations

5. **Systems**

    - Item interaction & usage
    - Inventory system
    - Village / Town
    - NPCs
    - Shop system
    - Quest system
    - Spawn system
    - Extended levels & level transitions

6. **FX & UI**

    - Destroy Particle Effect
    - outline effect
    - Simple cinematic sequences
    - Menu / Options / Retry UI
 

> Extended features will be added **after the MVP** depending on available time.

---
 
### **4. Timeline & Milestones**
 
#### **Target Duration**
  
- **MVP:** 1 month (2025.12.01 ~ 2025.12.31)

- **Extended:** After MVP completion
  
#### **Weekly Plan**
  
- **Dec 1 ~ Dec 7 (W1)**

    - **M1:** Character & Combat Core (v0.1)

- **Dec 8 ~ Dec 14 (W2)**

    - **M2:** Hit Reaction, Damage, Dummy Enemy, Targeting (v0.2)

- **Dec 15 ~ Dec 21 (W3)**

    - **M3:** Enemy AI & Advanced Combat (v0.3)

- **Dec 22 ~ Dec 28 (W4)**

    - **M4:** VFX & UI (v0.4)

- **Dec 28 ~ Dec 31 (W5)**

    - **M5:** Polish & Documentation (v0.5)

  
> Detailed milestones are managed in: **P01_UE5_Portfolio_Milestones**
    
---
  
### **5. Tools Overview**
  
- **Engine:** Unreal Engine 5.4

- **IDE:** Visual Studio 2022

- **Version Control:** Git + GitHub

- **Issues / Kanban:** GitHub Issues & Projects

- **Documentation:** Obsidian
  

---
  
### **6. Documentation Directory Structure**
  
```
docs/
 ├─ 00_plan/     → Project plans, milestones, workflows
 ├─ 01_daily/    → Daily checklists
 ├─ 02_design/   → System design & diagrams
 └─ 03_notes/    → Additional notes, experiments, retrospectives
```
  

---