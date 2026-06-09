# UE5 Portfolio - Milestones

This document organizes the major implementation stages and current progress of `UE5 Action RPG Combat Portfolio`.

It is maintained as a milestone document for tracking completed, in-progress, and follow-up expansion scope based on the current project state.

---

## 1. Milestone Criteria

```yaml
Management Criteria
- Feature-level implementation status
- Combat execution structure status
- Documentation / verification status
- Follow-up expansion candidates
```

```yaml
Status Criteria
Completed
-> Main goals are implemented / organized based on current code and documents

In Progress
-> Main structure exists, with implementation, verification, or documentation work remaining

Follow-Up
-> Scope to be handled in a later branch
```

---

## 2. Milestone Summary

```yaml
M0. Project Environment / Documentation Foundation
-> Completed

M1. Player Basic Controls / Weapon / Basic Attack
-> Completed

M2. Basic Combat Loop and Damage Pipeline
-> Completed

M3. Enemy AI Combat Behavior
-> In Progress

M4. Combat Feedback
-> In Progress

M5. Action Pipeline Improvement
-> In Progress

M6. Reaction Pipeline Improvement
-> In Progress

M7. Action / Reaction Execution Intervention
-> Follow-Up

M8. Guard / Parry / Counter Combat Judgment
-> Follow-Up

M9. Portfolio Technical Documents
-> In Progress

M10. AI Workflow / Prompt Library
-> In Progress
```

---

## 3. M0 - Project Environment / Documentation Foundation

### Status

Completed

### Goal

Set up the Unreal Engine project, Git / GitHub, and Markdown documentation structure.

### Completed Items

```yaml
Implementation / Environment
- Unreal Engine 5.4 project setup
- Visual Studio 2022 development environment
- Git / GitHub repository setup
- `.gitignore` setup

Documentation
- Issue Checklist
- Pull Request
- Bug Report
- System Architecture
- Technical Documents
- AI Workflow
```

---

## 4. M1 - Player Basic Controls / Weapon / Basic Attack

### Status

Completed

### Goal

Build the Player character's basic movement, camera, weapon equip, and basic attack flow.

### Completed Items

```yaml
Player
- Character / Controller
- SpringArm-based third-person camera
- Basic movement / jump / dodge foundation

Weapon
- Weapon equip / unequip
- WeaponActor / Attachment based weapon connection

Combat
- Basic Attack
- Combo Attack
- Montage-based attack execution
```

---

## 5. M2 - Basic Combat Loop and Damage Pipeline

### Status

Completed

### Goal

Build the basic combat loop from hit collision to damage application, hit reaction, and death handling.

### Completed Items

```yaml
Combat
- Hit Collision Window
- Hit Context / Damage Context
- ApplyDamage -> FDamageEvent -> TakeDamage flow
- Damage Result processing

Reaction
- Hit Reaction
- Dead Reaction

Feedback
- Damage Feedback
- Reaction Feedback
```

---

## 6. M3 - Enemy AI Combat Behavior

### Status

In Progress

### Goal

Allow Enemy AI to patrol, chase, attack, wait, react, and connect to the shared combat execution structure.

### Current Items

```yaml
AI
- Behavior Tree
- Blackboard
- Patrol / Chase
- Attack intent dispatch
- Combat priority / waiting behavior
- Look at Player
```

### Remaining Items

```yaml
Follow-Up Implementation / Verification
- Connect Guard / Parry / Counter with Enemy AI reactions
- Expand boss pattern / enemy pattern data
- Improve AI decision source integration with common execution pipeline
```

---

## 7. M4 - Combat Feedback

### Status

In Progress

### Goal

Connect Action / Reaction / Damage results to combat feedback that the player can perceive.

### Current Items

```yaml
Action Feedback
- Action start / end feedback
- Attack timing feedback
- Montage event based feedback connection

Reaction Feedback
- Hit / Dead reaction feedback
- Reaction result to feedback connection
- Animation / VFX / SFX connection candidates

Damage Feedback
- Hit location / direction based feedback
- Damage impact feedback
- Feedback request / execution structure

Player Feedback
- Screen / camera / UI feedback candidates for player readability
- Visual / audio feedback candidates for combat result readability
```

### Remaining Items

```yaml
- Responsibility boundaries for Action / Reaction / Damage / Player Feedback
- Feedback data authoring structure
- VFX / SFX / camera feedback polish
- System Architecture document structure update
```

---

## 8. M5 - Action Pipeline Improvement

### Status

In Progress

### Goal

Improve the Player / AI Action execution flow through request / decision / apply / lifecycle structure.

### Current Items

```yaml
Action
- Action request
- Action execution decision
- Action relationship / apply mode
- Action executor lifecycle
- Action data resolve
- Montage lifecycle criteria
- Action Feedback connection
```

### Remaining Items

```yaml
- Improve AI action intent integration with Action Pipeline
- Organize Action data authoring structure
- Define Action execution failure / rollback criteria
- Organize DataAsset-based authoring structure
```

---

## 9. M6 - Reaction Pipeline Improvement

### Status

In Progress

### Goal

Connect and improve the Reaction execution flow from Damage result, state changes, hit reactions, and feedback.

### Current Items

```yaml
Reaction
- Reaction request
- Reaction execution policy
- Reaction relationship / apply mode
- Reaction executor lifecycle
- Reaction data resolve
- Hit Reaction
- Dead Reaction
- Montage lifecycle criteria
- Reaction Feedback connection
```

### Remaining Items

```yaml
Follow-Up Implementation / Verification
- Define Damage Result to Reaction Request criteria
- Improve Reaction policy / execution state criteria
- Organize Enemy AI Reaction observation / return flow
- Verify Action / Reaction execution relationships
- Introduce Combat Resolution layer
- Connect Resource / state system
- Connect Guard / Parry / Counter judgment results
- Update System Architecture document structure
```

---

## 10. M7 - Action / Reaction Execution Intervention

### Status

Follow-Up

### Goal

Define interrupt / cancel / block / ignore criteria when Action and Reaction run simultaneously or intervene with each other.

### Follow-Up Scope

```yaml
Execution Relationship Policy
- Relationship between active Action and new Action
- Relationship between active Action and Reaction
- Relationship between active Reaction and new Reaction
- Action / Reaction priority judgment
- interrupt / cancel / block / ignore criteria

Execution Intervention Case
- Dodge-based intervention
- HitReaction-based Action interrupt
- Parry Reaction based Action transition candidate
- ExecutionState transition criteria

Verification
- Existing Combo / Dodge / HitReaction regression check
- Player / AI common applicability check
- Montage lifecycle / delegate cleanup criteria check
```

### Remaining Items

```yaml
- Action / Reaction relationship matrix
- Execution intervention policy
- Action / Reaction apply mode criteria
- System Architecture document update
```

---

## 11. M8 - Guard / Parry / Counter Combat Judgment

### Status

Follow-Up

### Goal

Extend Stella Blade-style Guard / Parry / Counter judgment on top of the current combat execution structure.

### Follow-Up Scope

```yaml
Guard
- Guard input / state / judgment
- Guard Break

Parry
- Input buffering
- Parry Window
- Combat Resolution based judgment
- Parry Reaction interrupt
- Damage / Reaction Feedback connection

Counter
- Counter condition
- Counter execution flow
- Action / Reaction relationship processing
```

### Preparation Documents

```yaml
D20
-> Parry Work Brief
-> Parry Feature Work Planning
-> Parry Work Checklist Draft
```

---

## 12. M9 - Portfolio Technical Documents

### Status

In Progress

### Goal

Organize portfolio technical documents and README so evaluators can understand the project structure and implementation intent.

### Current Items

```yaml
Technical Documents
- T00 Project Overview
- T01 Project Technical Summary
- T02 Combat Data Processing Pipeline
- T03 Action / Reaction Execution Pipeline
- T04 Enemy AI Combat Behavior Design
- T05 Data-Driven Design
- T06 Troubleshooting
- T07 AI-Assisted Development Workflow

README
- Project overview
- Implementation scope
- Core design points
- Documentation navigation
```

### Remaining Items

```yaml
Follow-Up Cleanup
- Project Stella naming decision
- Documentation Index update
- System Architecture / Engine Technique role separation
- Final review of portfolio technical documents
```

---

## 13. M10 - AI Workflow / Prompt Library

### Status

In Progress

### Goal

Build an AI-based workflow and Prompt Library for working with Codex.

### Current Items

```yaml
AI Workflow
- Index
- Overview
- Project Context
- Operation Guide
- Work Pipeline
- Backlog

Prompt Library
- Prompt Blueprint
- Working Rule
- Working Reference
- Work Planning
- Document Writing
- Review / Verification
- Git Operation

D20 Verification
- Work Brief
- Feature Work Planning
- Work Checklist Draft
```

### Remaining Items

```yaml
- Prompt Flow / Routing layer cleanup
- Work Brief / Planning / Checklist field contract cleanup
- Document Writing Prompt cleanup
- Prompt sentence quality review
- Re-verify Workflow in actual D20 implementation branch
```

---

## 14. Tag Candidates

```yaml
v0.1-player-combat-core
-> Player / Weapon / Basic Attack

v0.2-basic-combat-damage-pipeline
-> Basic Combat Loop and Damage Pipeline

v0.3-enemy-ai-combat
-> Enemy AI Combat Behavior

v0.4-combat-feedback
-> Combat Feedback

v0.5-action-pipeline
-> Action Pipeline Improvement

v0.6-reaction-pipeline
-> Reaction Pipeline Improvement

v0.7-action-reaction-intervention
-> Action / Reaction Execution Intervention

v0.8-guard-parry-counter
-> Guard / Parry / Counter expansion

v0.9-portfolio-docs
-> Portfolio technical documents / README

v0.10-ai-workflow
-> AI Workflow / Prompt Library
```

---

## 15. Current Priorities

```yaml
1. Update README / Technical Documents
2. Update Documentation Index
3. Organize existing System Architecture document structure
4. Define Action / Reaction execution intervention criteria
5. Implement Parry
6. Implement Guard / Counter
7. Write Verification Log / PR Document based on implementation results
```
