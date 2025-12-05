# UE5 Portfolio – Development Roadmap

## Overview

A step-by-step roadmap for building a full UE5 action RPG-style portfolio project.  
Divided into 14 major stages + extra systems.

---
---

# **Stage 1: Core Player Systems**

### **Default Level Setup (Order 0)**

 - [x] Default Level Setup
    

### **Player Setup (Order 1)**

- [x] Mesh / Camera setup
    
- [x] Place character in level
    
- [x] Basic movement + camera control
    
- [x] Implement Idle / Walk / Sprint / Jump
    

### **Basic Combat (Order 2)**

- [ ] Weapon equip / unequip
    
- [ ] Attack animations
    
- [ ] Hitbox / collision setup
    

---

# **Stage 2: Basic Combat Loop**

### **Enemy Setup (Order 3)**

- [ ] Mesh / Animations / Colliders
    

### **Basic UI (Order 4)**

- [ ] Player / Enemy HP UI
    

### **Combat System Foundation (Order 5)**

- [ ] Hit and HitReact
    
- [ ] HP system
    
- [ ] Damage application
    
- [ ] Enemy Death + Destroy
    

### **Player Hit & Death (Order 6)**

- [ ] Player Hit & React
    
- [ ] Player Death + Destroy
    

### **Targeting System (Order 7)**

- [ ] Basic Lock-on targeting
    

---

# **Stage 3: AI Implementation**

### **Team System (Order 8)**

- [ ] Player / Neutral / Enemy factions
    

### **Enemy AI – Movement (Order 9)**

*(AI Navigation, EQS/queries Based)*

- [ ] Point-to-point movement
    
- [ ] Random Query movement
    
- [ ] Patrol movement
    
- [ ] Chase movement
    

### **Enemy AI – Behavior & Positioning (Order 10)**

- [ ] Post-chase dispersal logic
    
- [ ] Combat priority assignment
    
    - [ ] Attack Wait
        
    - [ ] Guard Wait
        
    - [ ] Defense Wait
        
- [ ] Look-at-Player behavior
    

### **Enemy AI – Attacking (Order 11)**

- [ ] Attack animations
    
- [ ] Damage application
    
- [ ] Combo attacks
    
- [ ] Distance-based & random attack logic
    

### **Player Defensive Mechanics (Order 12)**

#### **Guard System**

- [ ] Guard
    
- [ ] Perfect Guard
    
- [ ] Guard Break
    

#### **Parry System**

- [ ] Parry
    
- [ ] Perfect Parry
    

#### **Dodge System**

- [ ] Dodge
    
- [ ] Perfect Dodge
    

### **Enemy Defensive Mechanics (Order 13)**

#### Guard

- [ ] Guard
    
- [ ] Guard Break
    

#### Parry

- [ ] Parry
    

#### Dodge

- [ ] Dodge
    

---

# **Stage 4: AI Expansion**

### **Friendly AI (Order 14)**

- [ ] Reuse enemy AI logic for allies
    

### **Group Combat System (Order 15)**

- [ ] Ally/Enemy identification
    
- [ ] AI vs AI combat matching system
    
- [ ] Post-kill behavior logic
    

---

# **Stage 5: Items / NPC / Interaction**

### **Interaction Systems**

- [ ] Item interaction & usage (Order 16)
    
- [ ] Inventory system (Order 17)
    

### **World / NPC Systems**

- [ ] Village area (Order 18)
    
- [ ] NPCs (Order 19)
    
- [ ] Shop system (Order 20)
    
- [ ] Quest system (Order 21)
    
- [ ] Spawn system (Order 22)
    
- [ ] Level transitions (Order 23)
    

---

# **Stage 6: Advanced Animation Systems**

### **IK Systems**

- [ ] Foot IK (Order 24)
    
- [ ] Hand IK (Order 24)
    

### **Movement System**

- [ ] ALS-style locomotion (Order 25)
    
- [ ] Parkour / environment interaction (Order 26)
    

### **Physics-Based Animation (Order 27)**

- [ ] Hit reactions
    
- [ ] Throwing
    
- [ ] Pushing
    
- [ ] Ragdoll blending (with partial skeletal override)
    

### **Environment Detection (Order 28)**

- [ ] Ground detection (linked with Foot IK)
    
- [ ] Climbable surface detection (linked with Parkour)
    
- [ ] Physics/environment interaction detection module
    

### **Cinematic Effects (Order 29)**

- [ ] Slow motion / time distortion
    
- [ ] Camera effects (shake, zoom, lock-on adjustments)
    

---

# **Stage 7: VFX Implementation**

### Combat VFX 
    
- [ ] Hit effects / Critical effects (Order 30)
    
- [ ] Damage number FX (Order 31)
    
- [ ] Charging effect (Order 32)
    
- [ ] Status ailment VFX (burn / freeze / shock) (Order 33)
    
- [ ] Ultimate effect (cutscene / camera shake / large-scale particles) (Order 39)
        

### Environmental Interaction VFX
    
- [ ] Footstep / dust / water splash interaction (Order 34)
    
- [ ] Destruction VFX (debris / dust / smoke) (Order 40)
        

### Camera & Screen Effects
    
- [ ] Slow motion / shockwave effects (Order 35)
    
- [ ] Rim lighting / shield effect (outline / shield highlight) (Order 36)
    

### UI & HUD Integrated VFX

- [ ] Level-up / EXP effects (Order 37)
    
- [ ] Decal effects (bullet holes / footprints / blood) (Order 38)
    

### Special Effects VFX

- [ ] Gravity / black hole effect (vector field) (Order 41)
    
- [ ] Network-synced VFX (multiplayer synchronization) (Order 42)
        

### **Environmental VFX**

- [ ] Footstep / Dust / Water splash (Medium, Order 34)
    
- [ ] Object destruction FX (High, Order 40)
    

### **Camera & Screen VFX**

- [ ] Slow motion + shockwave (Medium, Order 35)
    
- [ ] Rim light / Shield FX (Medium, Order 36)
    

### **UI & HUD VFX**

- [ ] Level-up / XP gain FX (Medium, Order 37)
    
- [ ] Decal effects (bullet holes, footprints, blood) (Low, Order 38)
    

### **Special Effects**

- [ ] Gravity / Black Hole FX (High, Order 41)
    
- [ ] Network-synced VFX (Very High, Order 42)
    

---

# **Stage 8: Advanced Combat Features**

- [ ] Weapon switching
    
- [ ] Aerial attacks / hits
    
- [ ] Down attacks / hits
    
- [ ] Execution system
    
- [ ] Skill system
    

---

# **Stage 9: Boss Battle Implementation**

- [ ] Boss arena
    
- [ ] Boss mesh/animation setup
    
- [ ] Boss attack patterns
    
- [ ] Boss clear sequence
    

---

# **Stage 10: Combat System Expansion**

### **Minimum Features**

- [ ] Critical hit system
    
- [ ] Status effect system (Burn/Ice/Shock)
    
- [ ] Full character stats (HP/MP/Stamina)
    
- [ ] XP/Level-up system
    

### **Extended Features**

- [ ] Charged attacks
    
- [ ] Backstab / rear bonus
    
- [ ] Groggy / stagger system
    
- [ ] Precise hit reactions (per-body-part impact)
    
- [ ] Aerial combo system
    

---

# **Stage 11: Skills & Weapons**

### Minimum

- [ ] Weapon asset management (Data/Resource Management System)
    
- [ ] Skill asset management (Data/Resource Management System)
    
- [ ] Ultimate skill system
    

### Extended

- [ ] Special item effects
    
- [ ] Homing / auto-target skills
    
- [ ] Summoning skills
    

---

# **Stage 12: Movement & Parkour**

### Minimum

- [ ] Parkour / landing recovery
    

### Extended

- [ ] Stealth system
    
- [ ] Flight system
    

---

# **Stage 13: Environment & World**

### Minimum

- [ ] Time-of-day / weather (simple version)
    
- [ ] Small sample map
    

### Extended

- [ ] Nanite demo-level optimization
    

---

# **Stage 14: Presentation & UI**

### Minimum

- [ ] HUD / Menu widgets
    
- [ ] Debug UI
    
- [ ] Camera collision
    

### Extended

- [ ] Cinematics
    
- [ ] Wall transparency system
    
- [ ] Editor tools / plugins
    

---

# **Additional Systems**

### Combat Behavior Extensions

- [ ] Enemy Just Guard Attack
    
- [ ] Enemy Just Parry Attack
    
- [ ] Enemy Just Dodge Attack
    

### Player Reaction Enhancements

- [ ] Player Just Guard React
    
- [ ] Player Just Parry React
    
- [ ] Player Just Dodge React
    

### Other Systems

- [ ] AI Party System
    
- [ ] Character Switching System
    
- [ ] Pet System
    
- [ ] Mount System
    
- [ ] Assist System
    
- [ ] Command Queue System
    
- [ ] Post Process effects
    
- [ ] Camera movement presets
    

---
---