# UE5 Portfolio – Issue Analysis Report

## Title

**M03-I01: OnTargetPerceptionForgotten event not firing analysis**

### Date

- **2026.01.28**

### Type

- Issue Analysis

### Status

- [x] Analysis completed
- [x] Resolution verified

### Branch

- feature/ai-behaviortree-core


---

## Summary

- Analyzed why the `OnTargetPerceptionForgotten` delegate did not fire and confirmed the fix by enabling `bForgetStaleActors` in project settings.


---

## Reproduction Steps

1. Bind the `OnTargetPerceptionForgotten` delegate in `ACAIController::InitializePerception()`.
2. Transition an AI Perception stimulus into the **Lost** state.
3. Wait until the stimulus **Age** expires.


---

## Expected vs Actual

**Expected**
- When the stimulus expires and is forgotten, `PrintTargetPerceptionForgotten()` should be called and log output should appear.

**Actual**
- `OnTargetPerceptionForgotten` is never called; only the **Gained/Lost** logs from the `Updated` event appear.


---

## Issue Details (Code)

```cpp
bool ACAIController::InitializePerception()
{
	if (!IsValid(AIPerceptionComp)) return false;

	AIPerceptionComp->OnTargetPerceptionForgotten.AddDynamic(
		this,
		&ACAIController::OnTargetPerceptionForgotten);

	return true;
}

void ACAIController::OnTargetPerceptionForgotten(AActor* Actor)
{
	PrintTargetPerceptionForgotten(Actor);
}

void ACAIController::PrintTargetPerceptionForgotten(AActor* Actor) const
{
	FLog::Log(TEXT("== Target Perception Forgotten =="));

	FLog::Log(FString::Printf(TEXT("TargetActor = %s"), *GetNameSafe(Actor)));

	FLog::Log(TEXT("================================="));
}
```


---

## Observed Output (Issue)

```cpp
Custom_FLog: Display: === Target Perception Updated ===
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: Sense = AISense_Sight | Perceived = Gained | Age = 0.00
Custom_FLog: Display: =================================
Custom_FLog: Display: ====== Perception Updated =======
Custom_FLog: Display: -------- Updated Actors ---------
Custom_FLog: Display: UpdateActors        : 1
Custom_FLog: Display: - BP_CPlayer_C_0
Custom_FLog: Display: =================================
Custom_FLog: Display: === Target Perception Updated ===
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: Sense = AISense_Sight | Perceived = Lost | Age = 0.00
Custom_FLog: Display: =================================
Custom_FLog: Display: ====== Perception Updated =======
Custom_FLog: Display: -------- Updated Actors ---------
Custom_FLog: Display: UpdateActors        : 1
Custom_FLog: Display: - BP_CPlayer_C_0
Custom_FLog: Display: =================================
```


---

## Root Cause

- Whether `OnTargetPerceptionForgotten` fires depends on the `bForgetStaleActors` flag inside `UAIPerceptionComponent`.
- `bForgetStaleActors` is a **private** variable with no public setter on the component.
- This behavior must be configured at the project level via `AIModule.AISystem`.


---

## Resolution

Enable `bForgetStaleActors` in the project settings file:

```ini
[/Script/AIModule.AISystem]
bForgetStaleActors=True
```


---

## Verified Output

```cpp
Custom_FLog: Display: === Target Perception Updated ===
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: Sense = AISense_Sight | Perceived = Gained | Age = 0.00
Custom_FLog: Display: =================================
Custom_FLog: Display: ====== Perception Updated =======
Custom_FLog: Display: -------- Updated Actors ---------
Custom_FLog: Display: UpdateActors        : 1
Custom_FLog: Display: - BP_CPlayer_C_0
Custom_FLog: Display: =================================
Custom_FLog: Display: === Target Perception Updated ===
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: Sense = AISense_Sight | Perceived = Lost | Age = 0.00
Custom_FLog: Display: =================================
Custom_FLog: Display: ====== Perception Updated =======
Custom_FLog: Display: -------- Updated Actors ---------
Custom_FLog: Display: UpdateActors        : 1
Custom_FLog: Display: - BP_CPlayer_C_0
Custom_FLog: Display: =================================
Custom_FLog: Display: == Target Perception Forgotten ==
Custom_FLog: Display: TargetActor = BP_CPlayer_C_0
Custom_FLog: Display: =================================
```


---

## Conclusion

- `OnTargetPerceptionForgotten` fires **only when** the stimulus expiration policy (`bForgetStaleActors`) is enabled.
- Because the setting is project-wide, verify the **AIModule.AISystem** configuration when diagnosing similar issues.


---

## References

- Related fix commit: 
	1. `chore(ai-behaviortree-core): turn on bForgetStaleActors in AISystem settings (#26, #27)`
- Related refactor commits: 
	1. `refactor(ai-behaviortree-core): rename BP_CAIController_Melee to BP_CAIController (#26)`
	2. `feat(ai-behaviortree-core): add debug print functions (#26)`
	3. `refactor(ai-behaviortree-core): bind perception delegates without handler implementation (#26)`


---