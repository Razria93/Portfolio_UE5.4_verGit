# UE5 Portfolio Issue Checklist

## Title

**M04-01: Build the Combat Feedback Pipeline**

### Date

- **Day 15**
  
- **Date : 2026.04.11**


---

### Goals

- Organize the first-pass **Combat Feedback** structure around combat results and action timing.

- Separate the roles of **reaction-feedback / action-feedback / player-feedback** and review a flow that can be shared across Player-side and Enemy-side execution.

- Define the feedback execution path and minimum validation criteria as a baseline for later combat structure expansion.


---

### Branch
- `feature/combat-feedback`


---

### TODO List

#### 1. Organize Feedback Structure and Responsibilities

- [x] Organize the separation between `reaction-feedback / action-feedback / player-feedback`
      
- [x] Organize the scope of shared feedback and player-local feedback
      
- [x] Organize the connection points between combat-result feedback and notify-based feedback


#### 2. First-Pass Reaction / Player Feedback Integration

- [x] Organize the reaction-feedback path after `TakeDamage`
      
- [x] Review first-pass hit VFX / hit SFX / hit stop application
      
- [x] Review player-local feedback application points


#### 3. First-Pass Action Feedback Integration

- [x] Review Trail and action SFX / VFX application flow
      
- [x] Organize notify-based action-feedback timing
      
- [x] Review whether action-start / action-end feedback hooks are needed


#### 4. Review Shared Player / Enemy Flow

- [x] Verify the Player-side action-feedback execution path
      
- [x] Verify the Enemy-side action-feedback execution path
      
- [x] Review cleanup flow after enemy attack end


#### 5. Organize Minimum Validation Criteria

- [x] Scenario 1: Player attack -> Enemy hit feedback
      
- [x] Scenario 2: Player action -> action-feedback output
      
- [x] Scenario 3: Enemy action -> action-feedback output
      
- [x] Scenario 4: Player-local feedback output


---

### Notes

- This issue focuses on **connecting feedback structure so that combat results and action timing become tangible in play**, rather than adding new combat rules.

- Treat this branch as the feedback baseline for later validation and action-structure expansion work.


---
