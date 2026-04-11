# Perception - Input - State Transition - Action Execution Structure

## 1. Purpose

This document is intended to organize the idea that, even though `Player` and `Enemy` operate under different control models,  
they still follow the same flow from the perspective of the execution pipeline.

`Perception -> Input(Intent) -> State Transition -> Action Execution`

The purpose of this document is to clarify the design criteria by separating the responsibilities of each layer, while preventing shared combat components from becoming overly coupled to a specific entity implementation.


---

## 2. Core Concepts

### 2.1 Perception / Awareness

The perception stage is the stage where the character gathers input data required to interpret both external and internal situations.

Examples:

- Whether a nearby target exists
- Distance / direction / sight
- Current weapon state
- Current HP / DeadState
- Current Reaction status
- Player input values
- AI Blackboard / Context data

In other words, perception provides the raw materials used to determine "what can be done."


---

### 2.2 Input(Intent)

Input does not refer only to physical input.  
Here, input refers to the layer that generates behavioral intent.

#### Player Input

The Player generates intent from user input devices.

Examples:

- Movement key input
- Jump input
- Attack input
- Weapon switch input

#### Enemy Input

The Enemy generates intent through the AI decision-making layer instead of direct user input.

Examples:

- Target tracking decision based on Perception results
- AttackIndex selection based on Blackboard
- Chase / Engage / Reaction / Dead transition decisions through BT

In other words, BT effectively replaces the Player's input layer for the Enemy.


---

### 2.3 State Transition

The result of input or decision-making does not immediately lead to action execution.  
The executable conditions of the current character must first be organized through a state transition.

Examples:

- `Idle -> Action`
- `Idle -> Equip`
- `Alive -> Reaction`
- `Alive -> Dying`
- `Dying -> Dead`

The purpose of state transition is as follows.

- Define what the current character can do
- Prevent duplicate execution
- Organize mutually exclusive behaviors
- Synchronize with animation and movement policies


---

### 2.4 Action Execution

Once the state is confirmed, the actual action corresponding to that state gets executed.

Examples:

- Play attack montage
- Equip / unequip gear
- Play HitReact
- Play Dead montage
- Enable weapon collision
- Inject damage context
- Restrict / restore movement

In other words, action execution corresponds to the stage that transforms the decided state into actual behavior.


---

## 3. Player Execution Structure

The Player operates in the following order.

`Perception -> Input -> State Transition -> Action Execution`

### 3.1 Perception

The Player determines current executability based on the following information.

- Current weapon state
- Current State
- Current HP / DeadState
- Whether movement is possible
- Whether input is possible

### 3.2 Input

User input creates intent.

Examples:

- Attack button input
- Movement input
- Weapon switch input

### 3.3 State Transition

Input changes the state after passing current condition checks.

Examples:

- Attack is only possible when currently in `Idle`
- Enter `Reaction` when taking damage
- Enter `Dying` when HP reaches 0

### 3.4 Action Execution

The actual action is executed after the state transition.

Examples:

- `ActionComp` executes the montage
- Inject `ActionContext` into `WeaponComp`
- `ReactionComp` executes HitReact
- `HealthComp` updates DeadState


---

## 4. Enemy Execution Structure

The Enemy also follows the same flow in essence.

`Perception -> Input(Intent Replacement) -> State Transition -> Action Execution`

However, in this case, the input layer is replaced by BT/Blackboard rather than user input.

### 4.1 Perception

The Enemy gathers the following data from the perception layer.

- AI Perception
- Target position / distance
- Blackboard Context
- Whether attacking is possible
- DeadState / Reaction status

### 4.2 Input(Intent Replacement)

Since the Enemy does not have direct player input,  
BT and Blackboard generate behavioral intent instead.

Examples:

- Decide to enter Chase
- Decide to enter Engage
- Select AttackIndex
- Determine HitReact / Dead transition

In other words, BT replaces the Player's input layer for the Enemy.

### 4.3 State Transition

BT selects an appropriate state according to the current context.

Examples:

- `Patrol -> Chase`
- `Chase -> Engage`
- `Engage -> HitReact`
- `Alive -> Dead`

### 4.4 Action Execution

The responsibility for converting the selected state into actual execution belongs to BT Tasks.

Examples:

- Play attack montage
- Stop movement
- Inject AttackContext
- Execute reaction
- Process Dead state

In other words, for the Enemy, `BT + Blackboard + Task` effectively replace the roles of the Player's `StateComp + ActionComp`.


---

## 5. Structural Correspondence Between Player and Enemy

### Player

- Perception: Check local character state / input availability conditions
- Input: User input
- State Transition: Centered on StateComp
- Action Execution: Centered on ActionComp

### Enemy

- Perception: Perception / Blackboard Context
- Input Replacement: BT Decision
- State Transition: Centered on BT / Blackboard
- Action Execution: Centered on BT Tasks

This can be summarized as follows.

- The Player has an `Input-driven execution` structure
- The Enemy has a `Decision-driven execution` structure

However, both cases still share the same final pipeline.

`Intent Generation -> State Transition -> Action Execution`


---

## 6. Problems in the Current Structure

Some shared components are currently written under a Player-centered structure.

Examples:

- `ReactionComponent` directly expects `StateComp`
- The context injection flow of `WeaponComponent` is designed around `CAction`

As a result, the following problems occur.

- The Enemy must keep `StateComp` even when it does not actually use it
- Enemy attacks must partially conform to the Player-style execution path
- Unnecessary coupling occurs between shared components and the Player implementation

In other words, some parts of the current Enemy setup are not essential design requirements,  
but rather an intermediate coupling layer for compatibility with Player-centered shared components.


---

## 7. Design Principles

The following principles should be pursued during future refactoring.

### 7.1 Shared Components Should Not Enforce a Player-Specific State Model

Shared combat components should only need to know things such as the following.

- Whether the entity is dead
- Whether the entity is currently reacting
- Whether movement is possible
- What the current attack context is

Rich Player states such as `Equip`, `Action`, and `Idle` are not shared rules, but are closer to a Player-specific orchestration layer.

### 7.2 Intent Sources May Differ Per Entity

- Player: Input device
- Enemy: BT / Blackboard

However, the shared combat processing flow after intent generation can still be unified as much as possible.

### 7.3 Responsibility for Action Execution and Context Delivery Should Be Separated

- Execution owner
  - Player: `CAction`
  - Enemy: `BT Task`
- Context application owner
  - Shared: `WeaponComponent`

In other words, regardless of who executes the action, final context delivery should be handled in the shared layer.


---

## 8. Conclusion

Although the control methods of the Player and the Enemy are different, their execution structures can still be explained using the same abstract flow.

`Perception -> Input(Intent) -> State Transition -> Action Execution`

The difference lies in the input layer.

- The Player creates intent from actual user input
- The Enemy creates intent through BT and Blackboard

Therefore, in the Enemy, BT can be viewed as an AI-specific orchestration layer that structurally replaces the roles of the Player's `StateComp + ActionComp`.

However, the current implementation still contains parts where shared components are written based on the Player model,  
and because of this, coupling occurs in which the Enemy temporarily retains components that are not fundamentally required.

The future refactoring goals are as follows.

- Remove Player bias from shared combat components
- Separate the orchestration layers of Player / Enemy
- Refactor the shared pipeline so that it only handles combat execution rules


---
