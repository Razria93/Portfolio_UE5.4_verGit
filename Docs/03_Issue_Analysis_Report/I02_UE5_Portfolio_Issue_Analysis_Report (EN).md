# UE5 Portfolio – Issue Analysis Report (EN)

## Title

**M03-I02: Analysis of mismatch between MoveTo arrival criteria and service distance checks**

### Date

- **2026.03.06**

### Type

- Issue Analysis

### Status

- [x] Analysis completed
- [x] Resolution verified

### Branch

- feature/ai-behaviortree-core


---

## Summary

- `MoveTo` returned success, but the patrol service failed its arrival check, causing `PatrolIndex` updates to stop.
  
- It was already known that updates stopped due to early-return in `CBTService_UpdatePatrolContext` when the measured distance did not pass `ReachThreshold`.
  
- However, it was unclear why the measured distance from the arrived character to the target point still stayed above `100`.
  
- Root cause was identified as follows:
	1. `MoveTo` arrival check includes the character collision capsule radius.
     
	2. `MoveTo` has internal tolerance/acceptance behavior.
     
	3. `CBTService_UpdatePatrolContext` initially used `FVector::Dist` (3D) instead of `FVector::Dist2D`.


---

## Reproduction Steps

1. Configure patrol BT with `MoveTo(Blackboard: PatrolLocation)`.
   
2. Configure `UCBTService_UpdatePatrolContext` and measure distance using `FVector::Dist(ownerLocation, patrolLocation)`.
   
3. Use `Dist <= ReachThreshold` as the service arrival condition.
   
4. Confirm log output where `MoveTo` succeeds but service still keeps `bReached=false`.
   
5. Reproduce delayed/stalled index transitions after the first patrol point.


---

## Expected vs Actual

**Expected**
- When `MoveTo` succeeds, the service should also treat the point as reached and switch to the next patrol point.

**Actual**
- Even after `MoveTo` success, the service still treated it as not reached, so patrol point updates stopped.


---

## Issue Code

```cpp
const float dist = FVector::Dist(currentOwnerLocation, currentPatrolLocation);
bool bReached = dist <= ReachThreshold; // ReachThreshold == 10

if (!bReached) return; // Error Point
```

```txt
MoveTo settings:
- Acceptable Radius: 0
- Blackboard Key: PatrolLocation
```


---

## Observed Output

```cpp
Custom_FLog: Display: [PatrolDist] dist: 109.38 (<= 10.00: false) | Reached: false
```


---

## Root Cause

1. `MoveTo` does not rely on strict center-to-center equality only; completion is affected by collision overlap/reach checks.
   
2. Move completion is influenced by capsule shape, radius/height, task settings, and internal path-following tolerance.
   
3. Therefore, using raw `FVector::Dist` in service logic can easily diverge from `MoveTo` completion behavior.
   
4. Also, Z-offset between character center and patrol point can produce unintended failures with full 3D distance checks.


---

## Resolution

```cpp
const float dist2D = FVector::Dist2D(ownerLocation, patrolLocation);
const float diff_Z = FMath::Abs(ownerLocation.Z - patrolLocation.Z);

bool bReached_XY = dist2D <= ReachThreshold_XY;
bool bReached_Z = diff_Z <= Tolerance_Z;
bool bReached = bReached_XY && bReached_Z;

if (!bReached) return;
```

1. Switched arrival check to `Dist2D`-first logic.
   
2. Split vertical difference into separate `ZTolerance`.
   
3. Split one threshold into two checks: `bReached_XY` and `bReached_Z`.
   
4. Kept `MoveTo` task acceptable radius at default `0.f`.


---

## Verification

1. Ran repeated patrol loop tests (Loop/Reverse/Random).
   
2. Confirmed `bReached=true` transition in service right after `MoveTo` success.
   
3. Verified stable updates of `PatrolIndex` and `PatrolLocation`.
   
4. Verified expected behavior when changing capsule size / acceptable radius.


---

## Conclusion

- `MoveTo` completion uses collision/reach semantics, not simple strict point equality.
  
- Completion is affected by capsule geometry, task settings, and internal tolerance behavior.
  
- For ground AI patrol, `Dist2D + ZTolerance` is more robust and commonly used.
  
- Service success conditions and task success conditions can differ; this must be accounted for explicitly.


---