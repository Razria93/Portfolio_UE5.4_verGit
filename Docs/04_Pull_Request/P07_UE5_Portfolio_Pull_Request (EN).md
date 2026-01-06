# Implement ApplyDamage Pipeline: ApplyDamage Flow via AnimNotify Push + Overlap → ApplyComp

## Title

✨ feat: implement ApplyDamage pipeline (AnimNotify Push + Overlap → ApplyComp) (#16, #18)

## Summary

- **Previous portfolio (legacy structure)** directly routed the `Attachment Overlap` event into `CAction`, where Action built/customized an `FDamageEvent` and called `OtherActor->TakeDamage()` itself.

- This structure carried the following risks:

  1. **Coupling (cyclic dependency) risk**: results of action execution (overlap/hit) flow back into `CAction`, concentrating cause and effect in a single object.

  2. **UObject validity risk**: it is risky to place core combat flow (damage calculation/application) inside `UObject(CAction)`, whose validity cannot always be guaranteed.

  3. **WeaponComp “Super Object” risk**: if `CWeaponComponent` owns `Attachment/Equipment/Action` and also absorbs “routing + calculation + application,” its responsibilities can become bloated.

- To address this, this portfolio separates responsibilities with the following goals:

  1. Move Action-related features into `CActionComponent`.

  2. Reduce `CAction` to “execution intent/timing.”

  3. Reduce `CAttachment` to “DamageCauser + ContextCarrier.”

  4. Move damage-related features (calculation/application) into `CApplyDamageComponent`.

  5. Establish the flow as:
     `Play Montage`
     → `CAnimNotify_Action(Begin)`
     → `CWeaponComponent::PushContextToAttachment()`
     → `CAttachment::OnComponentBeginOverlap()`
     → `CApplyDamageComponent::RequestApplyDamage()`
     → `CApplyDamageComponent::ProcessApplyDamage(Validate → ResolveSpec → ComputeResult → TakeDamage)`


---

## Completed Items

### 1. ApplyDamage pipeline entry: Attachment → ApplyDamageComponent request path

- Implemented `CAttachment` to build an `FHitContext` on overlap and pass it to `CApplyDamageComponent::RequestApplyDamage(const FHitContext&)`.

- `FHitContext` serves as the “standard request” for ApplyDamage by aggregating related contexts as follows:

  - `FOverlapContext` (facts at the overlap moment)

  - `FAttachmentContext / FEquipmentContext / FActionContext` (state snapshots backed up via AnimNotify Push)


---

### 2. AnimNotify-based Context Push: backup “state snapshots” at Action timing

- Configured `CAnimNotify_Action` to call `CAction::BeginPlayAction / EndPlayAction / NextPlayAction` depending on `FlowType(Begin/End/Next)`.

- On Action Begin/Next timing, Action calls `PushContextToAttachment()` to **pre-backup contexts required for ApplyDamage*- into the Attachment.

  - Current implementation: in `CAction_ComboAttack::BeginPlayAction()` and `CAction_ComboAttack::NextPlayAction()`:

    - Create `FActionContext{ CurrentActionType, ActionIndex }`

    - Call `CWeaponComponent::PushContextToAttachment(const FActionContext&)`

- `CWeaponComponent::PushContextToAttachment()` performs:

  - `CurrentAttachmentType_Cached` → build `FAttachmentContext`

  - `CurrentEquipmentType_Cached` → build `FEquipmentContext`

  - Along with the input `FActionContext`

  - Store into `IHitContextProducer(Attachment)` via `SetLastAttachmentContext / SetLastEquipmentContext / SetLastActionContext`

> The Push stage is strictly “preparation (backup)” only: it does not perform target discovery, damage computation, or apply calls.


---

### 3. OverlapContext standardization: consistent overlap-time context construction in Attachment

- Standardized `FOverlapContext` via `CAttachment::BuildOverlapContext(...)`:

  - `OwnerActor` = attacker (Attachment’s OwnerCharacter)

  - `DamageCauser` = the Attachment itself

  - `OverlappedComponent / OverlapShape` = attack collision component (including `UShapeComponent` cast result)

  - `OtherActor / OtherComponent` = target actor and hit component (including sweep flag / hit result)

- In `CAttachment::OnComponentBeginOverlap()`, self-collision is blocked:

  - Early return if `OwnerCharacter_Cached == OtherActor`


---

### 4. ApplyDamageComponent pipeline: Validate → SpecResolve → Compute → TakeDamage

- Implemented ApplyDamage processing pipeline in `CApplyDamageComponent`:

  - `RequestApplyDamage()` → `ProcessApplyDamage()`

  - Processing stages (current code):

    1. `ValidateRequest(FHitContext)`

    2. `CheckHitRule(FHitContext)` *(currently TODO, always true)*

    3. `ResolveDamageSpec(FHitContext, OutSpec)`

    4. `ComputeDamageResult(FHitContext, Spec, OutResult)`

    5. `ApplyDamageToTarget(FHitContext, Spec, Result)` → `Target->TakeDamage(...)`

- Split spec/result into dedicated structs:

  - `FDamageSpecKey` = (AttachmentType, EquipmentType, ActionType, ActionIndex)

  - `FDamageSpec` = (e.g., BaseDamage)

  - `FDamageResult` = (e.g., FinalDamage)

- Spec lookup uses `DamageSpecMap(TMap<FDamageSpecKey, FDamageSpec>)`:

  - `BuildSpecKey()` combines Attachment/Equipment/Action contexts from `FHitContext` to form the key


---

### 5. TakeDamage event standardization: preserve payload via custom DamageEvent (FDefaultDamageEvent)

- In `ApplyDamageToTarget()`, build `FDefaultDamageEvent` and pass it to `Target->TakeDamage()`:

  - `FDefaultDamageEvent` includes `FDamageSpecKey / FDamageSpec / FDamageResult`

  - `ClassID` is defined as `EDamageEventTypeId::DefaultDamage(0x1001)`

- InstigatorController resolution:

  - Prefer `Attacker->GetInstigatorController()`; if it fails, apply Pawn-cast based fallback

  - If `DamageCauser` lacks instigator data, infer from `Attacker`


---

## Test Plan

1. Verify the character has `CWeaponComponent`, `CActionComponent`, and `CApplyDamageComponent` attached.

2. Play an attack montage and confirm `CAnimNotify_Action(Begin)` is triggered.

3. Confirm `PushContextToAttachment()` is called in `CAction_ComboAttack::BeginPlayAction()`:

   - Verify `LastAttachmentContext / LastEquipmentContext / LastActionContext` are saved on the Attachment via logs.

4. During the Collision Window, when overlapping a target:

   - Verify `CAttachment::OnComponentBeginOverlap()` is called.

   - Verify `FHitContext` is built correctly (OverlapContext + Last*Context) via logs.

5. After `CApplyDamageComponent::RequestApplyDamage()`:

   - Verify the call flow: `ValidateRequest → ResolveDamageSpec → ComputeDamageResult → ApplyDamageToTarget`

   - Verify `Target->TakeDamage()` is called and logs show request/apply damage values.


---

## Related Issues / Branch

- Branch: `feature/combat-apply-damage`

- Issues: #16, #18


---

## Notes

- The primary goal of this PR is not “finishing the final damage rules,” but **making the ApplyDamage pipeline actually operational** as part of building the full combat system flow.

- Push is “preparation (state snapshot backup),” Overlap is “fact (hit occurrence),” and ApplyDamageComponent remains the single responsibility point for “policy/calculation/application.”

- Current implementation pushes ActionContext for `CAction_ComboAttack`; the same pattern can be extended to other actions by pushing context at each action’s Begin/Next timing.


---

## In Scope (Included in this PR)

- Added `CApplyDamageComponent` and implemented ApplyDamage pipeline (`Request → Validate → ResolveSpec → Compute → TakeDamage`)

- Standardized data structures/events: `FHitContext / FOverlapContext / FDamageSpecKey / FDefaultDamageEvent(EDamageEventTypeId)` etc.

- Implemented Attachment context backup via `CWeaponComponent::PushContextToAttachment()` (Attachment/Equipment/Action)

- Implemented: build HitContext on `CAttachment` overlap and request ApplyDamageComponent processing


---

## Out of Scope (Excluded from this PR)

- Duplicate-hit prevention (AlreadyHit Set), team/state/invulnerability-based target filtering

- `CheckHitRule()` policy implementation (currently TODO)

- Sustained effects / DoT / persistent-overlap handling via `RequestStopDamage()`

- Hit reactions (stagger/knockback/hit stop) and combat visuals

- Networking authority / replication policy

- Finalize Apply input/output standard (`FApplyRequest/Result`) and split spec storage into DataAsset/DB


---

## Follow-ups

- [ ] Implement `CheckHitRule()` (duplicate-hit / team / invulnerability / state policies)

- [ ] Split `DamageSpecMap` into DataAsset/DB (reflect TODO)

- [ ] Implement `RequestStopDamage()` (persistent overlap effects / timers / state release policy)

- [ ] Expand to `IDamageable / IHitReceiver` interface-based application calls (receiver responsibility separation)

- [ ] Improve debug output (notify timing / pushed context / SpecKey / final result visualization)


---