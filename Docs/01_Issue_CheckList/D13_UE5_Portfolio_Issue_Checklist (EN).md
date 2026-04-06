# UE5 Portfolio Issue Checklist

## Title

**M03-03: Organize Combat Core Shared Rules**

### Date

- **Day 13**

- **Date : 2026.04.01**


---

### Goals

- Organize the combat core shared by attackers and receivers, and finalize the common damage processing rules.

- Organize the responsibilities and minimum policies across the `HitContext -> ApplyDamage -> DefaultDamageEvent -> TakeDamage -> Health -> Reaction` flow.

- Structure the system so damage is processed under the same rules regardless of whether the target is the Player or an Enemy.


---

### Branch

- `feature/combat-core-shared`


---

### TODO List

#### 1. Organize ApplyDamage Responsibilities

- [x] Organize input/output responsibilities of the `ApplyDamage` stage

- [x] Finalize spec lookup policy based on `FApplyDamageSpecKey`

- [x] Organize spec miss handling policy

- [x] Organize minimal debug log flow


#### 2. Organize Hit Window and Duplicate Hit Rules

- [x] Add hit window based attack window identification rules

- [x] Add duplicate hit prevention policy within the same attack window

- [x] Preserve self-hit prohibition rule

- [x] Organize duplicate target overlap handling policy

- [x] Organize hit window open / close follow-up policy


#### 3. Organize TakeDamage Responsibilities

- [x] Fix the meaning of `Requested / Mitigated / FinalTaken / Committed`

- [x] Organize request reject conditions

- [x] Organize accepted / rejected follow-up direction

- [x] Organize dead target handling policy


#### 4. Organize Reaction Connection Rules

- [x] Organize minimum conditions for reaction entry

- [x] Organize connection rules from damage result to reaction request

- [x] Organize priority between death transition and reaction


#### 5. Integrated Validation

- [x] Scenario 1: confirm Player / Enemy damage processing under the same rules

- [x] Scenario 2: confirm duplicate hit prevention within the same attack flow

- [x] Scenario 3: confirm normal damage processing from attack start to first hit

- [x] Scenario 4: confirm follow-up flow including reaction / death transition after damage

- [x] Scenario 5: confirm additional damage handling policy on dead targets


---

### Current Outcome

- Both `ApplyDamage` and `TakeDamage` have been organized into a shared `Payload / Context / Result` flow.

- Hit window based duplicate-hit prevention, invalid request rejection, and dead target protection have been reflected in the current branch.

- `Reaction` has been connected using `CommittedDamage` and dead-state before/after conditions.


---

### Notes

- The goal of this issue is **to organize shared combat rules and minimum policies**, rather than advanced numeric design.

- Later extensions such as Guard, Armor, and Resistance should be designed to build on top of the current shared combat core.


---

### Follow-Up TODO

  - Decide whether to introduce a team identification structure

  - Decide Friendly Fire allowance and resolution policy

  - Connect extended receiver-side policies such as Guard / Armor / Resistance


---
