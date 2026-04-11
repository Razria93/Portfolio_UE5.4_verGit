# UE5 Portfolio – Bug Report (EN)

## Title

**M3-B01: Fix delayed state transition and stale Blackboard data caused by early-return in UpdateAIContext**

### Date

- **Day 11**
  
- **2026.03.03**

### Type

- Bug

### Status

- [x] Resolved

### Branch

- `feature/ai-bt-context`


---

## Summary

- In `UCBTService_UpdateAIContext::TickNode()`, context build failures were handled with immediate `return`, which skipped Blackboard cleanup logic.
  
- Introduced explicit result states (`Success / NoData / Error`) and updated the flow to always apply deterministic Blackboard `Set/Clear` operations per state.


---

## Environment

- Engine: Unreal Engine 5.4
  
- Scope: AI BehaviorTree Service (`UpdateAIContext`)
  
- Related keys: `TargetActor`, `bHasLOS`, `LastKnownLocation`, `DistanceToTarget`, `bInRange`, `bReturnHome`


---

## Steps to Reproduce

1. Make AI perceive a target so target/LOS values are written to Blackboard.
   
2. Move the player out of sight to force target loss.
   
3. Create a frame where `BuildPerceptionContext()` returns failure/invalid context.
   
4. Confirm that in the old flow `TickNode()` exits early and skips Blackboard cleanup.
   
5. Observe AI staying in `Investigate` and failing to return to `Idle`.


---

## Expected vs Actual

**Expected**

- On target loss, related Blackboard keys are cleared/updated immediately based on policy.
  
- State transition proceeds reliably from `Investigate -> Idle`.

**Actual**

- Early-return leaves stale values from previous frames in Blackboard.
  
- AI state decision becomes inconsistent, and AI may not return from `Investigate` to `Idle`.


---

## Root Cause

```cpp
void UCBTService_UpdateAIContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// ...
	
	FAIContext aiContext;
	if (!BuildPerceptionContext(ownerPawn, aiContext)) 
		return; // Error Point
	if (!ComputeMetricContext(ownerPawn, blackboardComp, aiContext)) 
		return;
	
	UpdatePerceptionContext(blackboardComp, aiContext);
	UpdateCombatContext(blackboardComp, aiContext);
	UpdateNavigationContext(blackboardComp, aiContext);
}

bool UCBTService_UpdateAIContext::BuildPerceptionContext(APawn* InOwnerPawn, FAIContext& OutAIContext)
{
	// ...
	
	FTargetData topData;
	if (!aiController->BuildPerceptionContext(topData) || !topData.IsValidData())
		return false; // Error Point
	
	// ...
}

bool ACAIController::BuildPerceptionContext(FTargetData& OutTargetData)
{
	UpdateTargetDataMap();
	return SelectTopPriority(OutTargetData); // Error Point
}

bool ACAIController::SelectTopPriority(FTargetData& OutTargetData)
{
	// ...
	
	FTargetData topData;
	for (TPair<AActor*, FTargetData>& pair : TargetDataMap)
	{
		AActor* actorKey = pair.Key;
		FTargetData& data = pair.Value;

		if (!IsValid(actorKey) || !data.IsValidData()) continue;

		if (data.TargetPriority < bestPriority)
		{
			bestPriority = data.TargetPriority;
			topData = data;
			continue;
		}
	}
	
	if (bestPriority == INT_MAX || !topData.IsValidData()) 
		return false; // Error Point
	
	// ...	
}
```

- `ACAIController::SelectTopPriority()` returns `false` when `TargetDataMap` is empty, even for normal "no target" frames.
  
- `TickNode()` treated this as a hard failure and returned early, skipping cleanup/update of Blackboard keys.
  
- This combination caused lost targets not to be forgotten correctly, preventing `Investigate -> Idle` transition.


---

## Fix

1. Introduced context build result type:
   
	- `Success`
     
	- `NoData` (normal no-target condition)
	  
	- `Error` (abnormal failure)

2. Reworked `TickNode()` branching:
	   
	- `Error`:
	 `ClearPerceptionContext`, `ClearCombatContext`, `ClearNavigationContext`
     
	- `NoData`:
	 `ClearPerceptionContext`, `ClearCombatContext`, `UpdateNavigationContext_NoTarget`
     
	- `Success`:
	 `UpdatePerceptionContext`, `UpdateCombatContext`, `UpdateNavigationContext`
	
2. Ensured Blackboard cleanup/update is always executed deterministically, even on failure paths.


---

## Verification

1. Repeated target acquire -> target lost scenarios.
   
2. Checked Blackboard Debug:
	   
	- `TargetActor` cleared
	  
	- `bHasLOS` switched to false
	  
	- `DistanceToTarget` / `bInRange` cleared
	  
3. Confirmed state transition:
	   
	- Enter `Investigate`, then return to `Idle` after conditions expire
	
4. Regression check:
	   
	- Reacquire target and confirm normal re-entry to `Chase/Combat`


---

## Notes

- For AI services, "failure = immediate return" is risky; prefer explicit failure categories with deterministic `Set/Clear` handling.
  
- Blackboard input keys should be updated every service tick interval so stale frame data cannot persist.
  
  
---