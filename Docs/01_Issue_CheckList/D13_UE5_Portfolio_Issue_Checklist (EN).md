# UE5 Portfolio – Issue Checklist

## Title

**M03-03: Organize Combat Core Shared Rules**

### Date

- **Day 13**

- **Date : 2026.04.01**


---

### Goals

- Organize the combat core shared by attackers and receivers, and finalize the common damage processing rules.

- Organize the responsibilities and minimum policies across the `HitContext -> ApplyDamage -> DefaultDamageEvent -> TakeDamage -> Health -> Reaction` flow.

- Structure the system so damage is processed under the same rules regardless of whether the target is the player or an Enemy.


---

### Branch

- `feature/combat-core-shared`


---

### TODO List

#### 1. Organize ApplyDamage Responsibilities

- [ ] Organize input/output responsibilities of the `ApplyDamage` stage

- [ ] Finalize spec lookup policy based on `FApplyDamageSpecKey`

- [ ] Organize spec miss handling policy

- [ ] Organize minimal debug log flow


#### 2. Organize Duplicate Hit and Attack Rules

- [ ] Add duplicate hit prevention policy within the same attack window

- [ ] Preserve self-hit prohibition rule

- [ ] Organize duplicate target overlap handling policy

- [ ] Organize follow-up policy for stop damage / overlap end


#### 3. Organize Team / Friendly Fire Policy

- [ ] Decide whether to introduce a team identification structure

- [ ] Decide whether friendly fire is allowed

- [ ] Apply stub policy if not implemented yet


#### 4. Organize TakeDamage Responsibilities

- [ ] Fix the meaning of `Requested / Mitigated / FinalTaken / FinalApplied`

- [ ] Organize request reject conditions

- [ ] Organize accepted / rejected follow-up direction

- [ ] Organize dead target handling policy


#### 5. Organize Reaction Connection Rules

- [ ] Organize minimum conditions for reaction entry

- [ ] Organize connection rules from damage result to reaction request

- [ ] Organize priority between death transition and reaction


#### 6. Integrated Validation

- [ ] Scenario 1: confirm Player/Enemy damage processing under the same rules

- [ ] Scenario 2: confirm duplicate hit prevention within the same attack window

- [ ] Scenario 3: confirm invalid request rejection

- [ ] Scenario 4: confirm additional damage handling policy on dead targets


---

### Notes

- The goal of this issue is **to organize shared combat rules and minimum policies**, rather than advanced numeric design.

- Later extensions such as Guard, Armor, Resistance, Team, and Friendly Fire should be designed to build on top of these rules.


---
