# Hit-Collision System Implementation & AnimNotify-Based Collision Window

## Title

✨ feat: Implement AnimNotify-based Collision Window and route Attachment overlap events to Action (#16)

## Summary

- Added a Dummy Target (`ACEnemy`) to validate the overlap-based hit pipeline immediately in a test level

- Implemented `ACAttachment` to automatically discover/cache `UShapeComponent` children under the Root, and dynamically bind Begin/End Overlap events per ShapeCollision

- Added `UCAnimNotify_Collision` to control collision timing during attacks, calling `CollisionEnabled/CollisionDisabled` via `FlowType(Begin/End)` to drive a montage-timed Collision Window

- Applied self-overlap exclusions (ignore OwnerCharacter/Attacker collisions) to prevent false hits caused by self-collision

- Bound `ACAttachment` events in `UCWeaponComponent` and routed them into `UCAction`, providing an extensible entry point for follow-up work (interface-based queries / damage pipeline)


---

## Done

### 1. Add Dummy Target (ACEnemy)

- Added a new `ACEnemy` class

- Provided a minimal target actor to validate overlap-based hits by simply placing it in the test level


---

### 2. Attachment: Auto-discover/cache ShapeCollision and bind Overlap events

- Collected Root child components in `ACAttachment::BeginPlay()`

- Cached components that can be cast to `UShapeComponent` into `Collisions_Cached`

- Dynamically bound the following overlap events to cached ShapeCollisions

  - `ACAttachment::OnComponentBeginOverlap`  

  - `ACAttachment::OnComponentEndOverlap`

- Applied `CollisionDisabled()` as the default state so no overlaps occur before the window is opened by AnimNotify  

  - Intended to eliminate overlap noise outside the attack timing


---

### 3. AnimNotify-based Collision Window control (UCAnimNotify_Collision)

- Added a new `UCAnimNotify_Collision` class

- Performed Attachment collision control based on AnimNotify `FlowType(Begin/End)`  

  - `Begin`: call `CollisionEnabled(CollisionName)`  

  - `End`: call `CollisionDisabled()`

- Supported enabling only a specific collision via `CollisionName`  

  - If Name is set: enable only the collision that matches the name

  - If Name is not set: enable all cached collisions

- Enabled explicit control of the Collision Window open/close timing on the montage track


---

### 4. Attachment events → Action routing (WeaponComponent binding)

- Connected `ACAttachment` delegates to `UCAction` callbacks in `UCWeaponComponent`

- Routed events as follows  

  - Collision Window events

    - `OnAttachmentCollisionEnabled` → `UCAction::OnAttachmentCollisionEnabled`

    - `OnAttachmentCollisionDisabled` → `UCAction::OnAttachmentCollisionDisabled`

  - Overlap events

    - `OnAttachmentBeginOverlap` → `UCAction::OnAttachmentBeginOverlap`

    - `OnAttachmentEndOverlap` → `UCAction::OnAttachmentEndOverlap`

- After Action receives overlaps, this PR only provides the entry point for follow-ups (data shaping → interface-based query/damage processing)  

  - This PR includes the routing pipeline only

---

### 5. Self-overlap exclusion

- Excluded the following cases in `ACAttachment` overlap handling 

  - `OtherActor == OwnerCharacter` (ignore owner collision)

  - `OtherActor == Attacker` or an equivalent self-reference case (implementation-specific) (ignore self-collision)

- Prevented hit logic from being triggered incorrectly due to overlaps with the Owner/Attacker

---

## Test Plan

1. Launch the project and enter the test level

2. Verify that `ACEnemy` (Dummy) is placed in the level

3. Verify Sword is equipped (Attachment is spawned/attached)

4. Collision Window test  

   - During the attack montage, verify Collision becomes enabled at `UCAnimNotify_Collision(Begin)`  

   - Verify Collision becomes disabled at `UCAnimNotify_Collision(End)`

5. Overlap routing test  

   - Verify Attachment overlap events are invoked when overlapping the target  

   - Verify the events are delivered to Action callbacks via WeaponComponent bindings

6. Exclusion test  

   - Verify overlaps with Owner/Attacker are ignored by the logic

---

## Related Issue / Branch

- Branch: `feature/combat-hit-collision`

- Issue: #16

---

## Notes

- **Collision Window**

  - A time window during which actual hit detection is allowed

  - Opened/closed by AnimNotify to ensure hits occur only on intended frames


- **CollisionName selective activation**

  - When an Attachment has multiple ShapeCollisions, it enables selecting which collision is valid per montage window

- **Responsibility split**

  - `UCAnimNotify_Collision`: controls the montage-timed Collision Window

  - `ACAttachment`: auto-discovery/caching of collisions, collision enable/disable, Begin/End Overlap emission

  - `UCWeaponComponent`: routes Attachment events to Action (binding)
  
  - `UCAction` (derived): extended in follow-ups for hit processing / data shaping / interface-based lookup

---

## In Scope (This PR)

- Added Dummy Target (`ACEnemy`)

- Implemented Attachment ShapeCollision auto-discovery/caching and overlap event bindings

- Implemented AnimNotify-based Collision Window control (`UCAnimNotify_Collision` + Enable/Disable)

- Supported selective activation via `CollisionName`

- Implemented self-overlap exclusions (ignore Owner/Attacker collisions)

- Built the binding pipeline that routes Attachment events to Action

---

## Out of Scope (This PR)

- Implementing DamageComponent / ReceiveDamageComponent and applying actual damage on overlap

- Implementing the interface-based query/dispatch system for external communication

- Completing combat logic: duplicate hit prevention (single-hit guarantee per target), team/state-based filtering, hit reactions, etc.

---

## Follow-ups

- [ ] Define/standardize the hit event payload structure at the Action reception stage (minimal fields)

- [ ] Connect interface-based lookup/calls (e.g., `IDamageable / IHitReceiver`)

- [ ] Add duplicate hit prevention (e.g., manage a HitActor Set during the notify window)

- [ ] Define target filtering (team/state/invulnerability) and hit policies (pierce/multi-hit, etc.)

- [ ] Improve debug logs/visualization (window enable range, overlapped targets, active CollisionName, etc.)


---