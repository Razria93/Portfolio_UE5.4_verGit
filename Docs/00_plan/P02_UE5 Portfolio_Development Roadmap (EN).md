# UE5 Portfolio - Development Roadmap

This document organizes the implementation roadmap of `UE5 Action RPG Combat Portfolio` based on the current project state.

Milestones manage the status of major stages, while this roadmap manages the order of upcoming implementation, documentation, and verification work.

---

## 1. Roadmap Criteria

```yaml
Management Criteria
- Organize next work order based on currently implemented features
- Track combat execution structure and documentation work together
- Separate feature / document / verification scope for follow-up branches
```

```yaml
Status Criteria
Completed
-> Closed based on current code and documents

In Progress
-> Structure or documents exist, with implementation / verification / cleanup remaining

Next Work
-> Items for the next branch or near follow-up work

Follow-Up Candidate
-> Expansion candidates outside the current priority
```

---

## 2. Current Roadmap Summary

```yaml
1. Player / Basic Combat Loop and Damage Pipeline
-> Completed

2. Enemy AI Combat Behavior
-> In Progress

3. Combat Feedback
-> In Progress

4. Action Pipeline Improvement
-> In Progress

5. Reaction Pipeline Improvement
-> In Progress

6. Action / Reaction Execution Intervention
-> Next Work

7. Guard / Parry / Counter
-> Next Work

8. Portfolio Technical Documents
-> Next Work

9. System Architecture / Engine Technique Document Structure
-> Next Work

10. AI Workflow Refactor Based on Real Usage
-> Follow-Up Candidate

11. Boss / Pattern / Advanced Combat
-> Follow-Up Candidate
```

---

## 3. Completed Foundation Work

### Player / Weapon / Basic Attack

```yaml
Status
-> Completed

Items
- Player Character / Controller
- SpringArm-based third-person camera
- Basic movement / jump / dodge foundation
- Weapon Equip / Unequip
- Combo Attack
- Montage-based attack execution
```

### Basic Combat Loop and Damage Pipeline

```yaml
Status
-> Completed

Items
- Hit Collision Window
- Hit Context / Damage Context
- ApplyDamage -> FDamageEvent -> TakeDamage flow
- Hit Reaction
- Dead Reaction
- Basic Damage Feedback
```

---

## 4. In-Progress Core Structures

### Combat Feedback

```yaml
Status
-> In Progress

Current Direction
- Action Feedback connection
- Reaction Feedback connection
- Damage Impact Feedback connection
- Player screen / camera / UI feedback candidates
- Animation / VFX / SFX / camera feedback connection candidates

Remaining Items
- Responsibility boundaries for Action / Reaction / Damage / Player Feedback
- Feedback data authoring structure
- VFX / SFX / camera feedback polish
- System Architecture document update
```

### Action Pipeline Improvement

```yaml
Status
-> In Progress

Current Direction
- Separate Action request / decision / apply / lifecycle
- Organize Action relationship / apply mode
- Organize Player / AI Action execution flow
- Action data resolve
- Action execution failure / rollback criteria
- Montage lifecycle criteria
- Action Feedback connection

Remaining Items
- Improve AI action intent integration with Action Pipeline
- Organize Action data authoring structure
- Organize DataAsset-based authoring structure
```

### Reaction Pipeline Improvement

```yaml
Status
-> In Progress

Current Direction
- Separate Reaction request / policy / lifecycle
- Organize Reaction relationship / apply mode
- Damage Result based Reaction Request connection
- Reaction data resolve
- Hit / Dead Reaction
- Reaction Feedback connection
- Montage lifecycle criteria

Remaining Items
- Improve Reaction policy / execution state criteria
- Organize Enemy AI Reaction observation / return flow
- Verify Action / Reaction execution relationship
- Introduce Combat Resolution layer
- Connect Resource / state processing
- Connect Guard / Parry / Counter judgment result
- Update System Architecture documents
```

### Enemy AI Combat Behavior

```yaml
Status
-> In Progress

Current Direction
- Behavior Tree / Blackboard based AI behavior
- Patrol / Chase / Attack
- Combat priority / waiting behavior
- AI action intent dispatch

Remaining Items
- Connect Guard / Parry / Counter with AI reactions
- Expand boss pattern / enemy pattern data
- Improve AI decision source integration with execution pipeline
```

---

## 5. Next Implementation Roadmap

### 5.1. Action / Reaction Execution Intervention

```yaml
Status
-> Next Work

Goal
- Organize Action and Reaction execution relationships
- Organize Execution Relationship Policy
- Organize Execution Intervention Case
- Define interrupt / cancel / block / ignore criteria
- Write Action / Reaction relationship matrix
- Write Execution intervention policy

Verification Criteria
- Existing Combo / Dodge / HitReaction regression check
- Player / AI common applicability check
- Montage lifecycle / delegate cleanup criteria check
```

### 5.2. Parry Implementation

```yaml
Status
-> Next Work

Goal
- Input-buffered Parry Action
- Parry Window
- Combat Resolution based judgment
- Damage nullification on Parry success
- Parry Reaction interrupt
- Damage Feedback / Reaction Feedback connection

Preparation Documents
- D20 Work Brief
- D20 Feature Work Planning
- D20 Work Checklist Draft

Verification Criteria
- Build
- Code Flow
- PIE
- Editor / Asset
```

### 5.3. Guard Implementation

```yaml
Status
-> Next Work

Goal
- Guard input / state
- Guard condition
- Guard success / failure handling
- Guard Break candidate structure
- Damage / Resource / Feedback connection
```

### 5.4. Counter Implementation

```yaml
Status
-> Next Work

Goal
- Counter condition
- Counter Action execution
- Action / Reaction relationship processing
- Feedback connection
```

---

## 6. Documentation Roadmap

### README / Portfolio Technical Documents

```yaml
Status
-> Next Work

Targets
- README
- T00 ~ T07 Technical Documents
- Documentation Index

Goal
- Organize first-entry portfolio document
- Compress portfolio technical explanation
- Organize representative document navigation
```

### System Architecture / Engine Technique

```yaml
Status
-> Next Work

Goal
- Separate pure System Architecture explanation from decision / issue records
- Separate Engine Technique explanation from Engine Decision / Issue records
- Reclassify existing System Architecture documents
- Define Architecture Decision Record / Architecture Issue Report criteria
```

### AI Workflow

```yaml
Status
-> Follow-Up Candidate

Goal
- Re-verify Work Brief / Planning / Checklist flow in actual D20 implementation branch
- Improve Prompt Flow / Routing layer
- Organize Work Checklist update rules
- Clean up Document Writing Prompts
```

---

## 7. Follow-Up Expansion Candidates

```yaml
Advanced Combat
- Perfect Parry / Normal Parry
- Perfect Dodge
- Execution
- Aerial Attack
- Down Attack
- Skill System

Enemy / Boss
- Boss pattern
- Enemy pattern data
- Wave system
- Group combat

Animation / Movement
- Foot IK
- ALS-style locomotion
- Parkour
- Camera direction animation

VFX / UI
- Final hit VFX / SFX polish
- Damage UI
- Resource UI
- Camera shake / hit stop
```

---

## 8. Current Priorities

```yaml
1. Update README / P00 / P01 / P02
2. Update Documentation Index
3. Review T00 ~ T07 Technical Documents
4. Organize System Architecture / Engine Technique document structure
5. Define Action / Reaction execution intervention criteria
6. Implement Parry
7. Implement Guard / Counter
8. Refactor AI Workflow based on real usage
```
