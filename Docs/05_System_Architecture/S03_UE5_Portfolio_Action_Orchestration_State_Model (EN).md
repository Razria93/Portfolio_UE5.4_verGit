# Action Orchestration State Model

## 1. Purpose

This document defines how `State`, `Type`, and `Intent` should be separated  
when designing a shared action execution structure for both Player and AI.

The main goals are as follows.

- Keep the character's top-level execution state simple.
- Separate movement, authored action, weapon state, reaction, and AI decision state into different axes.
- Allow Player input and AI Behavior Tree requests to use the same execution gate.
- Prevent the `State` enum from growing into a list of concrete behaviors.


---

## 2. Problem Recognition

The following are considered structural problems when designing the state model.

- top-level execution state and concrete behavior state can become mixed in one enum
- movement, weapon, attack, reaction, and AI decision can all be pushed into the same `State`
- Player and AI can no longer share the same execution structure cleanly
- values such as Equip / Attack / Reaction / Patrol can pile up on the same axis even though they describe different concerns
- the state enum can grow like a behavior list as the feature set expands

In other words, the problem is not a lack of states.  
The problem is trying to place different kinds of state on one axis.


---

## 3. Core Separation Criteria

The state model should be separated along the following lines.

```text
ExecutionState = top-level execution state of the character body
Movement       = movement input, movement mode, movement physics state
Action         = authored action performed by the character
Weapon         = possession and transition state of equipment / weapon
Reaction       = response caused by external stimulus
AIIntent       = high-level decision state of the AI
```

Each axis explains a different problem.

For example, the following values can coexist at the same time.

```text
AIIntentState  = Engage
ExecutionState = Action
ActionType     = ComboAttack
ActionState    = Playing
WeaponState    = Equipped
AttachmentType = Sword
```

Or a character may be moving while remaining in a free state.

```text
ExecutionState = Idle
MovementState  = Moving
SpeedType      = Run
```

Therefore, these values should not be merged into one `State`.


---

## 4. StateComp

`StateComp` should manage only the top-level execution state of the character.

```cpp
UENUM(BlueprintType)
enum class EExecutionState : uint8
{
	Idle = 0,
	Action,
	Reaction,
	Dead,
	Max,
};
```

### State Meaning

```text
Idle     = not in authored action, reaction, or dead state
Action   = executing a self-initiated action
Reaction = executing a response to external stimulus
Dead     = dead or inactive state
```

`ExecutionState` should not include concrete behaviors or decision states such as `Move`, `Walk`, `Run`, `Jump`, `Equip`, `Unequip`, `Dodge`, `Guard`, `LockOn`, or `Patrol`.

The reason is that `ExecutionState` must remain a top-level mutually exclusive state.


---

## 5. MovementComp

`MovementComp` should be separated as the axis that manages movement input, movement mode, and movement physics state.

Movement is not always treated as an `ActionComp` action.

`Move`, `Walk`, `Run`, `Jump`, and `StopJump` should first enter as `MovementIntent`,  
then the orchestrator should check the current state and forward them to `MovementComp`  
or `CharacterMovementComponent`.

```cpp
UENUM(BlueprintType)
enum class EMovementActionIntent : uint8
{
	None = 0,

	Move,
	Walk,
	Run,
	Jump,
	StopJump,

	Max,
};
```

Movement-related state values should be owned by `MovementComp`.

Example:

```cpp
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle = 0,
	Moving,
	Falling,
	Max,
};
```

If speed mode matters more, `SpeedType` should be kept separately.

```cpp
UENUM(BlueprintType)
enum class ESpeedType : uint8
{
	Walk = 0,
	Run,
	Sprint,
	Max,
};
```


---

### Relationship Between MovementIntent and ActionType

Intent and executable action are not always matched 1:1.

```text
MovementIntent::Walk
-> MovementComp::SetSpeedType(Walk)

MovementIntent::Run
-> MovementComp::SetSpeedType(Run)

MovementIntent::Move
-> MovementComp::AddMovementInput(...)

MovementIntent::Jump
-> Character::Jump()
```

These requests should not be promoted to `EActionType` by default.

However, if movement is designed as a combat action or authored action,  
it can be promoted to `ActionType`.

Example:

```text
CombatIntent::Dodge
-> ActionType::Dodge

MovementIntent::Jump
-> handled by MovementComp when it is a plain jump
-> may be promoted to ActionType when it is a jump attack or evasive jump
```


---

## 6. ActionComp

`ActionComp` should be treated as the axis that manages self-initiated authored actions.

Equip and unequip may also be treated as actions because they are directly initiated by the character.

Example model:

```cpp
UENUM(BlueprintType)
enum class EActionType : uint8
{
	None = 0,

	Equip,
	Unequip,

	LightAttack,
	ComboAttack,
	Guard,
	Dodge,

	All,
	Max,
};
```

The execution state of actions should start from a minimal form.

```cpp
UENUM(BlueprintType)
enum class EActionState : uint8
{
	Idle = 0,
	Playing,
	Max,
};
```

If needed later, states such as `Pending`, `Buffered`, `Recovery`, or `Canceled` can be added.

The rule is not to expand the enum before an actual policy exists.

### Example

During an attack action:

```text
ExecutionState = Action
ActionType     = ComboAttack
ActionState    = Playing
```

During an equip action:

```text
ExecutionState = Action
ActionType     = Equip
ActionState    = Playing
```


---

## 7. WeaponComp

`WeaponComp` should be separated as the axis that manages current attachment form and weapon transition state.

```cpp
UENUM(BlueprintType)
enum class EAttachmentType : uint8
{
	Unarmed = 0,
	Sword,
	All,
	Max,
};
```

`EquipmentType` can be used as a broad category for equipment execution objects or equipment data.

If actual equipment behavior later expands through `UObject`-based executors,  
the enum should not be expanded excessively.

```cpp
UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	None = 0,
	Default,
	All,
	Max,
};
```

Because actual attach / detach timing may happen in the middle of an equip montage,  
`WeaponState` should be separated from action state.

```cpp
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Unequipped = 0,
	Equipping,
	Equipped,
	Unequipping,
	Max,
};
```


---

### Why ActionState and WeaponState Should Be Separated

`ActionState::Playing` only means that the action is being played.

`WeaponState` describes whether the actual weapon is usable yet or still transitioning.

Example:

```text
ExecutionState = Action
ActionType     = Equip
ActionState    = Playing
WeaponState    = Equipping
AttachmentType = Unarmed
```

When the actual attachment happens at a montage notify:

```text
ExecutionState = Action
ActionType     = Equip
ActionState    = Playing
WeaponState    = Equipped
AttachmentType = Sword
```

In other words, the Action axis and Weapon axis should remain separate.


---

## 8. ReactionComp

`ReactionComp` should be separated as the axis that manages responses caused by external stimulus.

```cpp
UENUM(BlueprintType)
enum class EReactionType : uint8
{
	None = 0,
	Hit,
	Knockback,
	Dead,
	Max,
};
```

During a hit reaction:

```text
ExecutionState = Reaction
ReactionType   = Hit
```

Dead reaction is intended to converge to `ExecutionState::Dead`.


---

## 9. AI Intent

The AI's high-level decision state should not be stored in `StateComp`.

AI decision state should be managed by the Behavior Tree, Blackboard, and AIController axis.

```cpp
UENUM(BlueprintType)
enum class EAIIntentState : uint8
{
	Idle = 0,
	Patrol,
	Investigate,
	Chase,
	Alert,
	Engage,
	Dead,
	Max,
};
```

`EAIIntentState` represents what the AI is trying to do.

`EExecutionState` represents the actual execution state of the character body.

Example:

```text
AIIntentState  = Engage
ExecutionState = Idle
```

Attack request should be allowed.

```text
AIIntentState  = Engage
ExecutionState = Reaction
```

Attack request should be rejected because the character is in reaction.


---

## 10. Orchestrator Decision Criteria

`ActionOrchestratorComponent` is intended to receive requests created from Player input or AI tasks,  
then apply shared execution rules.

### Movement Request

```text
RequestMovementAction
- check ExecutionState
- check MovementIntent
- check whether movement is allowed
- execute through MovementComp or CharacterMovementComponent
```

Example:

```text
MovementIntent = Run
-> SpeedType = Run
```

### Combat Request

```text
RequestCombatAction
- check ExecutionState
- check WeaponState
- check ActionState
- convert CombatIntent into ActionType
- execute through ActionComp
```

Example:

```text
CombatIntent = ComboAttack
-> ActionType = ComboAttack
```

### Equipment Request

```text
RequestEquipmentAction
- check ExecutionState
- check current WeaponState / AttachmentType
- convert EquipmentIntent into ActionType::Equip or ActionType::Unequip
- execute through ActionComp and WeaponComp
```

Example:

```text
EquipmentIntent = Toggle
WeaponState     = Unequipped
-> ActionType   = Equip
```


---

## 11. Final State-Model Direction

```text
[StateComp]
EExecutionState
- Idle
- Action
- Reaction
- Dead

[MovementComp]
EMovementActionIntent
- None
- Move
- Walk
- Run
- Jump
- StopJump
- Max

EMovementState
- Idle
- Moving
- Falling
- Max

ESpeedType
- Walk
- Run
- Sprint
- Max

[ActionComp]
EActionType
- None
- Equip
- Unequip
- LightAttack
- ComboAttack
- Guard
- Dodge
- All
- Max

EActionState
- Idle
- Playing
- Max

[WeaponComp]
EAttachmentType
- Unarmed
- Sword
- All
- Max

EEquipmentType
- None
- Default
- All
- Max

EWeaponState
- Unequipped
- Equipping
- Equipped
- Unequipping
- Max

[ReactionComp]
EReactionType
- None
- Hit
- Knockback
- Dead
- Max

[AI]
EAIIntentState
- Idle
- Patrol
- Investigate
- Chase
- Alert
- Engage
- Dead
- Max
```


---

## 12. Design Principles

```text
ExecutionState = top-level mutually exclusive execution state
MovementState  = movement physics or movement progress state
SpeedType      = movement speed mode
ActionType     = type of action initiated by the character
ActionState    = action execution state
WeaponState    = weapon transition and actual usability state
AttachmentType = current attachment form
ReactionType   = type of external response
AIIntentState  = AI decision state
```

Everything should not be merged into a single `State`.

The responsibility of `State`, `Type`, `Intent`, and `Result` should remain separated.

```text
State  = current state being occupied
Type   = category or identifier
Intent = request intention before execution
Result = processing result
```

Intent and execution are not always matched 1:1.

```text
EquipmentIntent::Toggle
-> ActionType::Equip or ActionType::Unequip

MovementIntent::Run
-> SpeedType::Run

CombatIntent::ComboAttack
-> ActionType::ComboAttack
```

Under this structure, Player and AI may follow different decision paths,  
while still sharing the same execution states and action execution rules.
