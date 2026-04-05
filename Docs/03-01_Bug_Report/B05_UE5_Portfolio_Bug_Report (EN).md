# UE5 Portfolio Bug Report (EN)

## Title

**M3-B05: Fix first-hit `InvalidRequest` reject caused by `CurrentHitWindowId` increment timing issue**

### Date

- **Day 18**

- **2026.04.06**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/combat-core-shared`


---

## Summary

- In `ACAttachment::CollisionEnabled()`, `UShapeComponent` could call `SetCollisionEnabled` and trigger an overlap before `CurrentHitWindowId` was incremented.

- As a result, `BuildOverlapContext()` created an `FHitContext` with `HitWindowId = -1`, and `UCApplyDamageComponent::ValidateRequest()` immediately rejected it as `InvalidRequest`.

- This bug was fixed by reorganizing `ACAttachment::CollisionEnabled()` so that a valid `HitWindowId` is prepared before `UShapeComponent` calls `SetCollisionEnabled`.


---

## Environment

- Engine: Unreal Engine 5.4

- Target: `ACAttachment` / `UCApplyDamageComponent`

- Related Flow:

  - Player or Enemy enables attachment collision through an attack animation notify

  - The first overlap occurs immediately after activation

  - `HitWindowId` is not yet set to a valid value at the overlap timing

  - The first hit is rejected as `InvalidRequest` in the `ApplyDamage` stage


---

## Reproduction Steps

1. Call `CollisionEnabled()` through an attack animation notify on a melee weapon attachment used by either the Player or an Enemy.

2. Perform the attack at close range so that an overlap occurs immediately after collision is enabled.

3. Check the `FOverlapContext.HitWindowId` value at the first overlap timing through logs.

4. Verify that `UCApplyDamageComponent::ValidateRequest()` rejects the request because `HitWindowId == INDEX_NONE`.

5. Confirm that the following log pattern is reproduced.

```text
RejectReason = EApplyDamageRejectReason::InvalidRequest
HitWindowId = -1
```


---

## Expected Result vs Actual Result

**Expected Result**

- A valid hit window id should be prepared before attachment collision can generate an overlap.

- The first overlap should also be delivered as an `FHitContext` with a valid `HitWindowId`.

- The `ApplyDamage -> TakeDamage` flow should proceed normally starting from the first hit.

**Actual Result**

- At the first overlap timing, `HitWindowId` was passed as `INDEX_NONE(-1)`.

- `UCApplyDamageComponent::ValidateRequest()` rejected the request as `InvalidRequest`.

- As a result, the first hit was not applied and the flow terminated early on the sender side.


---

## Cause

```cpp
bool bCollisionEnabled = false;

if (!InName.IsNone()) 
{
	for (UShapeComponent* collision : Collisions_Cached)
	{
		if (collision->GetFName() == InName)
		{
			collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			bCollisionEnabled = true;
			break;
		}
	}
}
else
{
	for (UShapeComponent* collision : Collisions_Cached)
	{
		collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		bCollisionEnabled = true;
	}
}

if (!bCollisionEnabled) return;

if (!bHitWindowOpened)
{
	++CurrentHitWindowId; // Error Point

	bHitWindowOpened = true;

	if (IsValid(ApplyDamageComp_Cached))
	{
		ApplyDamageComp_Cached->NotifyHitWindowOpened(this, CurrentHitWindowId);
	}
}
```

- The core issue was the processing order inside `ACAttachment::CollisionEnabled()`.

- In the previous structure, attachment collision activation and hit window initialization were separated, so the first overlap could arrive before the hit window id was ready.

- The resulting flow was as follows.
	
	1. An attack notify calls `CollisionEnabled()`.
	
	2. The first overlap callback occurs immediately after attachment collision is enabled.
	
	3. `BuildOverlapContext()` reads `CurrentHitWindowId` before it has been updated.
	
	4. `HitWindowId = INDEX_NONE(-1)` is stored in `FHitContext`.
	
	5. `UCApplyDamageComponent::ValidateRequest()` interprets it as an invalid request.

- The essence of this bug was that **a valid hit window id needed to be prepared before the first overlap occurred, but the overlap timing and hit window initialization timing were out of sync**.


---

## Fix

```cpp
TArray<UShapeComponent*> collisionsToEnable;

if (!InName.IsNone())
{
	for (UShapeComponent* collision : Collisions_Cached)
	{
		if (collision->GetFName() == InName)
		{
			collisionsToEnable.Add(collision);
			break;
		}
	}
}
else
{
	for (UShapeComponent* collision : Collisions_Cached)
	{
		collisionsToEnable.Add(collision);
	}
}

// Early-Return
if (collisionsToEnable.IsEmpty()) return;

if (!bHitWindowOpened)
{
	++CurrentHitWindowId;
	bHitWindowOpened = true;

	if (IsValid(ApplyDamageComp_Cached))
	{
		ApplyDamageComp_Cached->NotifyHitWindowOpened(this, CurrentHitWindowId);
	}
}

for (UShapeComponent* collision : collisionsToEnable)
{
	collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
```

- The flow of `ACAttachment::CollisionEnabled()` was corrected as follows.
	
	1. First, collect the `UShapeComponent` list that will actually be activated.
	
	2. If there is no collision to activate, return immediately.
	
	3. Open the hit window and increment `CurrentHitWindowId` only when there is a valid activation target.
	
	4. Perform the actual collision enable after that.

- The intent of this fix was as follows.
	
	1. Prevent an invalid state where only the hit window opens without any valid collision.
	
	2. Guarantee that a valid `HitWindowId` is always ready before the first overlap.
	
	3. Stabilize sender-side input conditions so that `ApplyDamage` does not reject the first hit as `InvalidRequest`.


---

## Verification

1. Repeatedly checked first-overlap logs immediately after attack start for both Player attacks and Enemy attacks.

2. Verified that `HitWindowId` is no longer printed as `-1` at the first overlap timing.

3. Verified that `UCApplyDamageComponent::ValidateRequest()` no longer rejects the first hit as `InvalidRequest`.

4. Verified that `ApplyDamage` summary logs are printed normally starting from the first hit.

5. Verified that a bad state where only the hit window opens does not occur when there is no collision to activate.


---

## Result

```cpp
===== Apply Damage Summary ======
[@ APPLY DAMAGE]
| DamageCauser = BP_CAttachment_Sword_C_1 
| Target = BP_CEnemy_C_1 
| HitWindowId = 0
| Base = 10.000 
| Request = 10.000 
| Committed = 10.000
=================================
```


---

## Notes

- Changing the initial value of `HitWindowId` to `0` was considered, but it was not applied because it would only hide the structural issue as a temporary workaround.

- Keeping `INDEX_NONE(-1)` as the meaning of "no valid hit window has been opened yet" is structurally safer.

- The key point of this fix was not changing the default value, but clearly reordering the flow as **check collision enable availability -> open hit window -> perform actual collision enable**.


---
