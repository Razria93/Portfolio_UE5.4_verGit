# UE5 Portfolio - Project Overview

This document summarizes the goals, scope, technical direction, and documentation structure of `UE5 Action RPG Combat Portfolio`.

Detailed implementation stages are managed in `P01_UE5_Portfolio_Milestones (EN).md` and `P02_UE5 Portfolio_Development Roadmap (EN).md`.

---

## 1. Project Information

```yaml
Project Name: UE5 Action RPG Combat Portfolio
Engine: Unreal Engine 5.4
Development Environment: Visual Studio 2022
Language / Composition: C++ / Blueprint
Version Control: Git / GitHub
Documentation: Markdown / Obsidian
Target Platform: Windows PC / Unreal Editor execution
```

---

## 2. Project Goals

### Genre Goal

Third-person action RPG combat system portfolio.

### Implementation Goals

```yaml
Combat Execution Structure
- Shared Player / Enemy Action execution flow
- Reaction execution flow
- Damage processing flow
- Feedback execution flow
- AI decision source integration with execution layer

Workflow
- Git / PR / Issue Checklist based work management
- Bug Report / System Architecture based records
- Technical Document based portfolio explanation
- AI Workflow / Prompt Library based Codex collaboration
```

---

## 3. Current Implementation Scope

```yaml
Player
- Movement / Camera / Input
- Weapon Equip / Unequip
- Combo Attack
- Dodge-based intervention flow

Combat
- Hit Collision Window
- Hit Context / Damage Context
- ApplyDamage -> FDamageEvent -> TakeDamage based Damage Pipeline
- Hit / Dead Reaction
- Damage Feedback / Reaction Feedback

Enemy AI
- Behavior Tree / Blackboard
- Patrol / Chase / Attack
- Combat priority / waiting behavior
- Action intent dispatch

Documentation
- Issue Checklist
- Pull Request
- Bug Report
- System Architecture
- Technical Documents
- AI Workflow
```

---

## 4. Core Design Direction

### Action / Reaction Execution Structure

Organize Action and Reaction through a shared request / decision / apply / lifecycle flow.

### Damage Pipeline

Keep Unreal Engine's standard `FDamageEvent` and `AActor::TakeDamage()` flow while connecting project-specific hit context, damage result, reaction request, and feedback request.

### Cross-Domain Intervention

Handle interrupt / cancel / block relationships between Action, Reaction, Dodge, and HitReaction explicitly.

### Data-Driven Resolve

Resolve Action, Reaction, Damage, and Feedback data and executors through key-based lookup.

### Shared Player / AI Execution Structure

Player input and AI Behavior Tree can use different decision sources while sharing the same component-driven execution pipeline.

---

## 5. Documentation Structure

```yaml
00_plan
-> Project overview / milestones / development roadmap

01_Work_List
-> Work goals / checklist items / verification criteria / work artifacts by work unit

02_Bug_Report
-> Implementation issues / causes / fixes / verification records

04_Pull_Request
-> PR changes / verification / follow-up records

05_System_Architecture
-> System structure / responsibility boundaries / design decisions / structure change records

07_Technical_Documents
-> Portfolio submission technical documents

08_AI_Workflow
-> Codex-based workflow / Prompt Library / Work Brief / Planning

99_Legacy
-> Legacy Issue Checklists / previous documentation structure archive
```

---

## 6. Portfolio Technical Documents

```yaml
T00
-> Project Overview

T01
-> Project Technical Summary

T02
-> Combat Data Processing Pipeline

T03
-> Action / Reaction Execution Pipeline

T04
-> Enemy AI Combat Behavior Design

T05
-> Data-Driven Design

T06
-> Troubleshooting

T07
-> AI-Assisted Development Workflow
```

---

## 7. Follow-Up Expansion Direction

```yaml
Combat
- Guard / Parry / Counter judgment
- Combat Resolution layer
- Resource / state system improvement

Data
- DataAsset-based authoring structure
- Action / Reaction / Feedback data expansion

AI
- Boss pattern
- Enemy pattern data expansion
- Stronger integration between AI decision source and shared execution pipeline

Documentation
- Documentation Index update
- System Architecture / Engine Technique document role separation
- Final review of portfolio technical documents
- AI Workflow refactor based on real usage
```
