# AIStateComp vs BB-BT Comparison

## Purpose

- Compare the `Component approach` and the `BB-BT (Blackboard–BehaviorTree) approach` from the same perspective: **state** and **behavior design**

- As a result, this project adopts a **hybrid structure** with **BB-BT as the behavior core** and **components as supporting systems**


---

## Terms

### 1. Component approach

- `Blackboard` **stores data**, `Component` such as `AIStateComponent` **changes data**, and `BehaviorTree` **decides behavior using the updated data**

- State transitions and event handling (hit, pattern change, etc) are **concentrated inside component logic**

- Can be designed to keep **structural symmetry** with Player `CStateComponent`

#### Core definition

- Responsibilities for data storage / change / use are separated

- Structural symmetry with `Player` is preserved


---

### 2. BB-BT approach (Blackboard–BehaviorTree)

- `Blackboard` **stores data** and `BehaviorTree` **changes and uses data** at the same time

- `BehaviorTree` changes state and decides behavior through conditions (`Decorator`), branches (`Selector`), and execution (`Task`)

- State changes also happen **through BT nodes (Task/Service)**

- Behavior design is **visualized and managed at the BT asset level**

#### Core definition

- `BehaviorTree` owns both data change and data use

- AI behavior design is completed inside the `BehaviorTree` asset


---

## Key comparison summary

| **Category** | AIStateComponent approach | BB-BT approach |
| --- | --- | --- |
| **State ownership** | Component | Blackboard |
| **Behavior decision owner** | Component Logics | BehaviorTree |
| **State change location** | C++ code | BT nodes (Task/Service) |
| **Blackboard role** | Secondary store | Single source of data |
| **Extension method** | Component modification | Node/Decorator composition |
| **Visualization/Debug** | Code-centric | BT graph-centric |
| **Engine alignment** | Custom structure | UE AI infrastructure-centric |


---

## AIStateComponent evaluation

### Pros

- Responsibilities are clearly separated, keeping a **consistent flow**

- Player–Enemy **state symmetry** can be maintained

### Cons

- Component responsibility can grow as AI types/patterns increase

- If separated from `BehaviorTree`, **UE AI infrastructure usage decreases**

- Behavior flow changes require component code updates, reducing scalability


---

## BB-BT evaluation

### Pros

- Directly aligned with Unreal Engine **AI design philosophy**

- Behavior logic is **modularized** by Node / Decorator / Service, making extension and edits easier

- `Blackboard` and `BehaviorTree` assets define and use state, making decision flow clear

- Graph-based structure provides **strong visualization and debugging**

### Cons

- Too many nodes/services can **increase tree complexity**

- If data collection/calculation moves into BT, Task/Service responsibilities can bloat


---

## Structure Decision

- **Decision / Priority / Branching**
  → `BehaviorTree`

- **State data storage**
  → `Blackboard`

- **Data collection / Calculation / External requests**
  → `Component`


#### Core definition

- `BlackBoard` stores state and data

- `Component` collects and calculates data needed for decisions

- `BehaviorTree` updates state values from collected data and executes appropriate behavior


---

## Conclusion

- The AIStateComponent approach is structurally valid and provides clear state control, but scalability and edit convenience can be lower

- In UE AI infrastructure, scalability, and debugging, a **BB-BT centered structure is more reasonable**

- Therefore this project adopts **BB-BT as the behavior core**, with **components as supporting systems** in a hybrid structure


---