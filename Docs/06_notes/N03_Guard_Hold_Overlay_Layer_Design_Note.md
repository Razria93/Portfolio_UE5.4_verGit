# Guard Hold Overlay Layer Design Note

## 1. 목적

본 문서는 `Guard Hold`를 어떤 실행 모델에 편입할지 논의한 과정을 기록한다.

처음에는 `Guard Hold`를 `Action`, `Defense Domain`, `Stance Domain`, `Movement / Locomotion Layer` 중 어디에 둘지 검토했다. 논의 결과, `ExecutionState`를 계속 늘리기보다 `Idle` 위에 남을 수 있는 관측 가능한 overlay layer를 execution decision에서 추가로 확인하는 방향이 더 적절하다고 판단했다.

이 문서는 구현 지시서가 아니라 설계 판단 기록이다. 현재 브랜치의 v1 구현은 Guard / Parry 동작을 마무리하는 최소 범위로 유지하고, 장기 구조는 후속 작업에서 다시 다룬다.

---

## 2. 문제 인식

현재 상호배제 실행 상태는 다음 축으로 동작한다.

```text
Idle
Action
Reaction
Dead
```

이 축은 캐릭터가 현재 큰 실행 상태에서 무엇을 하고 있는지 판단하는 기준이다. 문제는 `Guard Hold`가 이 축에 깔끔하게 들어가지 않는다는 점이다.

`Guard Hold`는 montage action처럼 시작과 종료가 명확한 실행은 아니지만, 단순 animation pose도 아니다. Guard 상태는 다음 역할을 동시에 가진다.

- ABP에서 Guard pose / locomotion을 유지한다.
- `TakeDamage` 또는 Combat Resolution이 읽을 guard / parry 상태를 제공한다.
- Dodge, Hit Reaction, Guard Out 같은 실행에 의해 종료되어야 한다.

따라서 `ExecutionState == Idle`이라고 해서 캐릭터가 아무 상태도 가지지 않는다고 보면 안 된다. 정확히는 exclusive action / reaction 실행이 없을 뿐이고, 그 위에 Guard 같은 overlay 상태가 남을 수 있다.

---

## 3. 검토한 후보

### 3.1 Action 편입

`Guard Hold`를 `Action`으로 편입하면 기존 orchestration과 intervention 판단을 그대로 사용할 수 있다.

장점:

- Dodge / Hit Reaction / Guard Out이 active action을 끊는 기존 경로를 사용할 수 있다.
- v1 구현 범위가 작다.
- `Block_In` 이후 `Guard Hold`를 active action처럼 세우면 pose cleanup 누락 문제를 빠르게 줄일 수 있다.

단점:

- `Guard Hold`는 montage action이 아니라 유지 상태에 가깝다.
- no-montage action을 허용하기 위한 예외가 필요하다.
- 장기적으로는 다시 overlay layer로 분리될 가능성이 크다.

결론:

`Guard Hold = Action index 3` 방식은 빠른 v1 bridge로는 가능하지만 최종 구조로 보기는 어렵다.

### 3.2 Defense Domain

`Defense Domain`은 guard / parry / block 판정을 표현하기 좋다.

장점:

- guard / parry / block 같은 방어 계열 상태를 한곳에 모을 수 있다.
- Combat Resolution이 읽을 방어 상태의 출처가 명확해진다.

단점:

- `Block_In / Block_Out`까지 Defense Domain으로 보이는 오해가 생길 수 있다.
- `Block_In / Block_Out`은 입력으로 실행되는 전환 action에 가깝다.
- 방어 상태 전체를 새 domain으로 만들면 scope가 커진다.

결론:

`Defense Domain`이라는 이름은 guard / parry 판정에는 맞지만, 전환 action과 지속 상태의 경계를 흐릴 수 있다.

### 3.3 Stance Domain

`Stance Domain`은 `Guard Hold`가 action이 아니라 자세 / 유지 상태라는 점을 잘 설명한다.

장점:

- Guard, LockOn, Aim, Crouch 같은 유지 상태를 같은 계열로 볼 수 있다.
- ABP pose와 gameplay 상태가 함께 유지되는 구조를 설명하기 쉽다.

단점:

- movement / locomotion과 combat 판정이 한 용어 안에 섞일 수 있다.
- 모든 stance가 orchestration 대상은 아니므로 범위 조절이 필요하다.

결론:

`Stance`는 개념 설명에는 적절하지만, implementation layer 이름으로 확정하기 전에 관측 대상과 중재 대상을 분리해야 한다.

### 3.4 Movement / Locomotion Layer

`Guard Hold`는 animation 관점에서 locomotion / stance 성격이 있다.

장점:

- ABP에서 Guard locomotion을 유지하는 구조와 잘 맞는다.
- walk / run / sprint처럼 입력 유지 중 pose가 계속 바뀌는 상태와 유사하다.

단점:

- 일반 movement는 속도 / 방향 / 가속 / 감속 처리에 가깝다.
- Guard는 damage packet 처리와 interrupt 판단에도 관여한다.
- movement 전체를 execution arbitration 대상으로 끌어오면 scope가 커진다.

결론:

Guard pose는 locomotion layer에서 표현되지만, Guard 상태 자체는 combat 판단에도 쓰이므로 단순 movement로만 볼 수 없다.

### 3.5 Idle Domain

`Idle`도 domain처럼 상태 경쟁에 참여시키자는 관점도 검토했다.

장점:

- `Idle` 위에 남은 상태를 decision 과정에서 놓치지 않도록 문제를 설명할 수 있다.

단점:

- `Idle`은 실행 중인 domain이라기보다 exclusive execution이 비어 있는 baseline에 가깝다.
- `Idle`을 participant로 만들면 모든 request가 `Idle`과 관계 판단을 해야 해서 규칙이 불필요하게 커질 수 있다.

결론:

`Idle`을 domain으로 만들기보다, `Idle` 위에 남은 observable overlay state를 별도로 관측하는 쪽이 더 적절하다.

---

## 4. 최종 설계 판단

최종 판단은 다음 구조를 기준으로 한다.

```text
Exclusive Execution Layer
-> Action
-> Reaction
-> Dead

Observable Overlay Layer
-> Guard Hold
-> LockOn
-> Aim
-> Crouch

Baseline
-> Idle
```

`ExecutionState`는 계속 `Idle / Action / Reaction / Dead` 중심으로 유지한다.

`Idle`은 “아무 상태도 없음”이 아니라 “exclusive execution이 없음”을 뜻한다. 따라서 `ExecutionState == Idle`이어도 `Guard Hold` 같은 overlay state가 남아 있을 수 있다.

Orchestrator는 장기적으로 다음 순서로 판단한다.

```text
Incoming Request
-> active Action / Reaction이 있는지 확인
-> observable overlay snapshot 구성
-> incoming executor가 overlay 상태에서 실행 가능 여부 판단
-> 필요한 overlay handling 요청
-> overlay owner가 handling 적용 가능 여부 확인
-> 둘 다 없으면 baseline Idle로 처리
```

이 구조에서는 `Block_In / Block_Out`은 Action Domain의 전환 action으로 유지한다. `Guard Hold`는 `FExecutionSnapshot`의 observable overlay snapshot에 기록되지만, Orchestrator가 Guard의 세부 조건을 직접 해석하지 않는다.

Guard In / Guard Out / Dodge / Reaction 같은 incoming executor가 snapshot을 읽어 실행 조건과 필요한 overlay handling을 판단하고, `DefenseComponent` 같은 overlay owner는 요청된 handling을 실제로 적용할 수 있는지 확인한 뒤 상태를 변경한다.

---

## 5. v1 적용 기준

현재 브랜치에서는 `Defense Domain` 전체 도입이나 full observable overlay participant 모델을 만들지 않는다.

v1의 우선순위는 다음과 같다.

- Guard / Parry 입력과 animation 흐름을 안정화한다.
- `Block_In` 중 release 문제는 deferred action candidate 구조로 처리한다.
- `Guard Hold`가 cleanup되지 않는 문제는 `ExecutionState` 확장이 아니라 observable overlay policy registry로 관측한다.
- Action / Reaction start 직전에 관련 overlay policy를 조회하고, 필요한 overlay handling을 result에 누적한다.
- Combat Resolution은 이번 단계에서 완성하지 않고, 이후 damage packet interception 단계에서 연결한다.

`Guard Hold = Action index 3` 방식은 빠른 bridge로 가능하지만, 이번 v1에서는 해당 방식을 선택하지 않는다.

이번 v1에서는 `Guard Hold`를 action으로 편입하지 않고, snapshot / result handling을 사용하는 구조적 v1을 선택한다.

```text
구조적 v1
-> Guard Hold를 action으로 편입하지 않는다.
-> ExecutionSnapshot에 observable overlay snapshot을 둔다.
-> overlay owner policy는 snapshot 구성 시점에 자기 runtime state를 기록한다.
-> incoming executor는 snapshot을 읽고 실행 가능 여부와 필요한 handling을 결정한다.
-> Action / Reaction start 전에 필요한 경우 overlay handling을 누적한다.
```

이 구현은 full overlay participant 모델이 아니다. v1에서는 `FExecutionParticipant`를 확장하지 않고, `FExecutionSnapshot`의 observable overlay snapshot과 result handling으로 Guard overlay cleanup 문제를 먼저 해결한다.

---

## 6. v1 리팩터링 경과

최초 판단 이후에도 PIE 검증과 코드 구현을 거치며 구조를 여러 차례 조정했다. 이 과정은 단순 구현 변경이 아니라 overlay gate의 책임 경계를 확인하는 과정이었다.

### 6.1 Snapshot 제거 검토에서 observable overlay snapshot 재도입으로 이동

초기에는 `FExecutionSnapshot`에 Guard overlay 상태를 복사해 Orchestrator가 직접 확인하는 방식을 검토했다.

하지만 이 방식은 Orchestrator가 각 overlay의 세부 상태를 알게 만들고, 이후 LockOn / Aim / Crouch 같은 overlay가 추가될 때 snapshot이 계속 비대해질 수 있다.

그래서 중간 단계에서는 `ExecutionSnapshot`에서 overlay 세부 상태를 제거하고, `DefenseComponent` 같은 overlay owner가 `IObservableOverlayPolicy`로 직접 판단하는 방향을 검토했다.

그러나 이 방식은 incoming executor가 실행 가능 여부를 판단하려면 결국 overlay별 상태 조건 구조를 따로 받아야 한다는 문제가 있었다. 별도 requirement 구조가 커질 바에는, decision 시점에 사용할 상태를 `FObservableOverlaySnapshot`으로 명시해 `FExecutionSnapshot` 안에 포함하는 편이 더 단순하다.

따라서 v1은 snapshot을 다시 도입하되, Orchestrator가 Guard 세부 조건을 직접 해석하지 않게 한다. overlay owner는 snapshot에 상태를 기록하고, incoming executor가 그 snapshot을 읽어 실행 가능 여부와 필요한 handling을 결정한다.

### 6.2 단일 Guard 분기에서 snapshot writer registry로 이동

초기 구현은 `ResolveObservableOverlayGate()` 안에서 Guard overlay를 직접 판정하는 형태였다.

하지만 overlay 대상 component가 늘어나는 것은 시간문제이고, Orchestrator가 Guard / LockOn / Aim 같은 세부 정책을 직접 들고 있으면 확장성이 떨어진다.

따라서 Action / Reaction Orchestrator는 snapshot 구성 시점에 `UCObservableOverlayComponent`를 통해 registered overlay policy를 순회한다. 각 policy owner는 자기 runtime state를 `FObservableOverlaySnapshot`에 기록한다.

초기 v1에서는 Action / Reaction Component도 실행 직전 handling 적용을 위해 같은 policy 목록을 직접 보관했다. 이 구조는 snapshot 구성과 handling 적용 책임이 분리되어 있어 동작은 명확하지만, policy 등록 지점이 중복됐다.

이후 `UCObservableOverlayComponent`를 도입해 policy 등록 / snapshot 구성 / handling 적용 위임을 하나의 component로 모았다.

```text
UCObservableOverlayComponent
-> overlay policy registry 소유
-> WriteObservableOverlaySnapshot() 위임
-> ApplyObservableOverlayHandlings() 위임

UCDefenseComponent
-> Guard overlay state 소유
-> IObservableOverlayPolicy 구현
-> Guard snapshot 작성
-> ClearGuardState / ClearGuardOverlay 적용
```

이 변경으로 Orchestrator와 Action / Reaction Component는 overlay policy 목록을 직접 들지 않고, `UCObservableOverlayComponent`를 통해 snapshot 구성과 handling 적용을 요청한다.

### 6.3 축약 query에서 execution decision query 재사용으로 이동

초기 `FObservableOverlayQuery`는 `Snapshot`, `IncomingPart`, `ApplyMode`만 들고 있었다.

하지만 overlay 판단은 active part, incoming part, snapshot, apply mode를 모두 참조할 수 있어야 한다. 따라서 query가 기존 `FExecutionDecisionQuery` 전체와 `ApplyMode`를 함께 들도록 바꿨다.

이 변경으로 incoming executor는 현재 overlay snapshot뿐 아니라 active / incoming execution 관계도 함께 보고 실행 조건을 판단할 수 있다.

### 6.4 단일 handling에서 다중 handling 누적으로 이동

초기 `FObservableOverlayDecision`은 단일 `EObservableOverlayHandling`만 반환했다.

하지만 여러 overlay policy가 동시에 참여하거나, 하나의 policy가 여러 후처리를 요구할 수 있다. 따라서 decision과 result 모두 `TArray<EObservableOverlayHandling>` 기반으로 맞췄다.

Action / Reaction Component는 Orchestrator result에 누적된 handling을 실행 시작 전에 순서대로 적용한다.

### 6.5 owner policy 판단에서 incoming execution condition으로 이동

초기 Defense policy는 `ResolveObservableOverlayDecision()` 안에서 Action / Reaction incoming을 한 번에 판단했다.

하지만 overlay gate는 실행과 상태의 관계를 다룬다. 실행 가능 여부는 incoming executor가 판단하고, 상태 변경은 overlay handling으로 요청하는 편이 책임이 분명하다.

따라서 v1에서는 `WantObservableOverlayRequirement()` / `AllowObservableOverlayRequirement()` 구조를 제거하고, incoming executor의 `ResolveObservableOverlayExecutionCondition()`으로 통합했다. 이 함수는 snapshot을 읽어 실행 가능 여부를 결정하고, 필요하면 `ClearGuardState` 또는 `ClearGuardOverlay` 같은 handling을 함께 요청한다.

### 6.6 PIE 검증 이후 Reaction clear 정책 재검토

초기에는 `Hit` reaction이 들어오면 Guard overlay를 유지하고, `Dead`만 clear하는 임시 분기를 검토했다.

하지만 PIE 검증 결과, overlay gate가 reaction 실행 자체를 막는 계층이 아니기 때문에 `Hit` reaction을 clear하지 않으면 reaction 종료 후에도 Guard pose / state가 남을 수 있다.

따라서 의사처리계층에서 `Block_Hit / Parry / GuardBreak` 같은 결과 타입을 세분화하기 전까지는 reaction 시작 시 Guard overlay를 clear하는 방향이 더 안전하다고 판단했다.

이 판단은 Combat Resolution 또는 damage packet 해석 계층이 생긴 뒤 다시 세분화한다.

---

### 6.7 Guard event routing responsibility update

초기 Guard v1 구현에서는 `UCActionComponent`가 `NotifyGuardInputPressed`, `NotifyGuardInStarted`, `NotifySwitchToGuard`, `NotifyAllowGuardStart`, `NotifyGuardInterrupted` 같은 Guard 전용 API를 가지고 있었다.

이 구조는 빠른 연결에는 유리했지만, Guard 하나만으로도 ActionComponent API가 계속 늘어났다. 또한 내부 구현 대부분이 `UCDefenseComponent`의 Guard overlay 상태를 변경하는 호출이었기 때문에, ActionComponent가 overlay owner의 세부 lifecycle을 알고 있는 형태가 됐다.

따라서 Guard 상태 전환 event는 다음 경로로 이관한다.

```text
Action lifecycle / input side effect
-> UCActionComponent::NotifyObservableOverlayEvent()
-> UCObservableOverlayComponent::NotifyObservableOverlayEvent()
-> IObservableOverlayPolicy::HandleObservableOverlayEvent()
-> UCDefenseComponent
```

이 변경으로 `UCActionComponent`는 Guard 전용 Defense 라우터가 아니라 공통 overlay event 전달 지점으로 축소된다. `UCDefenseComponent`는 Guard overlay state owner로서 `IObservableOverlayPolicy`를 통해 Guard event를 해석하고 실제 상태를 변경한다.

단, montage notify는 overlay event를 직접 만들지 않는다. Notify는 기존 `HandleActionNotifyCommand()` 진입점을 통해 active executor에게 timing command만 전달하고, `CAction_Guard`가 `SwitchToGuard` / `AllowGuardStart`의 의미를 해석한 뒤 필요한 overlay event를 dispatch한다.

```text
AnimNotify
-> UCActionComponent::HandleActionNotifyCommand()
-> active UCAction executor
-> CAction_Guard::HandleSpecificNotifyCommand()
-> UCActionComponent::NotifyObservableOverlayEvent()
-> UCObservableOverlayComponent
-> UCDefenseComponent
```

이 규칙은 “notify는 timing만 알리고, 그 timing의 의미는 executor가 해석한다”는 기존 action notify 책임 분리를 유지하기 위한 것이다.

다만 `GuardInCompleted` deferred consume은 overlay 상태 변경이 아니라 action orchestration의 지연 실행 소비이므로 `UCActionOrchestratorComponent` 책임으로 유지한다.

## 7. Incoming Overlay Execution Condition

현재 v1 구조는 incoming executor가 observable overlay snapshot을 기준으로 incoming 실행이 가능한지 판단하고, 시작 전에 필요한 overlay handling을 함께 요청한다.

이유는 일부 Action / Reaction이 특정 overlay 상태를 요구하거나, 반대로 특정 overlay 상태가 있으면 실행되면 안 되기 때문이다. 또한 어떤 실행은 overlay 상태가 있어도 실행 가능하지만, 시작 전에 해당 overlay를 정리해야 한다.

예시는 다음과 같다.

```text
Guard Out
-> Guard overlay가 있어야 실행 의미가 있다.

Block_Hit
-> Guard 상태 또는 guard resolution 결과가 있어야 실행 의미가 있다.

Parry Success
-> Parry window 또는 parry resolution 결과가 있어야 실행 의미가 있다.

Dodge
-> Guard overlay가 있어도 실행 가능하지만 시작 전 clear가 필요하다.

ComboAttack
-> Guard overlay가 남아 있으면 기본적으로 실행되면 안 된다.
```

따라서 v1 구조에서는 overlay gate를 다음 흐름으로 본다.

```text
Overlay owner policy
-> Snapshot 구성 시 자기 runtime state 기록

Incoming executor
-> Snapshot을 읽고 실행 가능 여부 판단
-> 필요한 overlay handling 요청

Orchestrator
-> incoming decision을 result에 반영

Action / Reaction Component
-> requested handling을 실행 직전 ObservableOverlayComponent에 위임

ObservableOverlayComponent
-> policy owner에게 CanApply / Apply 호출
-> owner가 허용하면 상태 변경 적용
-> handling 적용 성공 후 execution 시작
```

Combat Resolution은 damage packet을 해석해 `Hit / Block_Hit / Parry / GuardBreak` 같은 결과 타입을 결정한다. 반면 Orchestration의 overlay gate는 이미 결정된 Action / Reaction을 시작하기 전에, 그 실행이 현재 overlay 상태에서 가능한지와 실행 전에 어떤 overlay cleanup이 필요한지를 판단한다.

v1의 첫 적용 대상은 `Guard Out`이다. `Guard Out`은 Guard overlay가 남아 있을 때만 의미 있는 종료 action이므로, Guard overlay가 이미 clear된 상태에서 들어온 `Guard Out`은 실행하지 않고 ignore한다.

다만 `Guard Out`은 guard 종료 자체를 수행하는 action lifecycle이므로, `ResolveObservableOverlayExecutionCondition()`에서 별도 overlay cleanup handling을 요청하지 않는다. Guard overlay 정리는 `Block_Out` 시작 / 종료 처리에서 담당한다.

`Dodge`는 Guard overlay가 있어도 실행 가능하지만, 시작 전에 `ClearGuardState` handling을 요청한다. `Hit / Dead` reaction도 의사처리계층에서 `Block_Hit / Parry / GuardBreak`가 세분화되기 전까지는 Guard state를 clear하는 기본 정책을 따른다.

현재 v1에서는 이 정책을 reaction base가 아니라 `Hit / Dead` reaction executor가 각각 판단한다. base reaction은 공통 clear 정책을 갖지 않고, 세부 reaction executor가 자기 overlay execution condition을 정의한다.

Guard Out 중 재입력은 duration window가 아니라 단발 notify 이후 상태 전환으로 처리한다. `Block_Out` 시작 시에는 `CanStartGuard`를 잠근 상태로 두고, `AllowGuardStart` notify가 호출된 시점부터 `CanStartGuard=true`로 전환한다.

이후 Guard In request가 들어오면 Guard In이 active Guard Out을 intervention으로 끊고 새 Guard In lifecycle을 시작한다. 이 경우 기존 Guard Out stop에서는 Guard state를 clear하지 않고, 새 Guard In start가 pose / parry state를 덮어쓴다. 반대로 Dodge / Hit / Dead처럼 Guard 외부 실행이 Guard Out을 끊는 경우에는 incoming executor가 `ClearGuardState` handling을 요청해 Guard runtime 전체를 정리한다.

정리하면 v1의 책임 분리는 다음과 같다.

```text
Overlay owner
-> WriteObservableOverlaySnapshot()
-> 현재 overlay state를 snapshot에 기록

ObservableOverlayComponent
-> registered overlay policy를 순회
-> snapshot 작성과 handling 적용을 policy owner에게 위임

Incoming executor
-> ResolveObservableOverlayExecutionCondition()
-> 실행 가능 여부와 필요한 overlay handling 결정

Orchestrator
-> decision과 handling을 execution result에 반영

Action / Reaction Component
-> requested handling 적용을 ObservableOverlayComponent에 요청

ObservableOverlayComponent
-> CanApplyObservableOverlayHandling()
-> ApplyObservableOverlayHandling()
-> owner authorization 이후 상태 변경 적용
```

---

## 8. Guard Input Intent와 SwitchToGuard 기준

`bWantsGuarding`은 Guard action lifecycle 상태가 아니라 input intent 상태로 둔다.

따라서 Guard In / Guard Out action이 실제로 시작됐는지와 별개로, Guard input press / release request가 들어온 시점에 먼저 갱신한다.

```text
Guard Pressed request
-> bWantsGuarding=true 즉시 반영
-> Guard In candidate 처리

Guard Released request
-> bWantsGuarding=false 즉시 반영
-> Guard Out candidate 처리
-> Block_In 중이면 GuardOut candidate만 deferred
```

이 분리로 `Block_In` 중 release가 들어와도 `SwitchToGuard` 시점에서 현재 입력 유지 여부를 정확히 판단할 수 있다.

`SwitchToGuard`는 Strict Switch Rule을 따른다.

```text
SwitchToGuard 전에 release
-> Parry Window만 인정
-> Guard 판정은 열지 않음
-> Block_In complete 이후 deferred GuardOut consume

SwitchToGuard 이후 release
-> SwitchToGuard부터 release 전까지 Guard 판정 인정
-> release 이후 GuardOut 실행 또는 consume
```

즉, tap은 Parry 시도로 인정하고, hold가 유지된 경우에만 Parry 이후 Guard로 승격한다.

---

## 9. Block_Hit 복귀 정책

`Block_Hit`을 별도 Reaction으로 사용할 경우, Guard Hold 상태를 그대로 유지한 채 맞는 것이 아니라 `Guard Hold`를 일시적으로 대체하는 피격 반응으로 본다.

따라서 `Block_Hit` 시작 시에는 Guard pose overlay와 guard / parry 판정을 정리하고, `Block_Hit` 종료 시에는 Guard 입력 의도에 따라 복귀 여부를 판단해야 한다.

권장 흐름은 다음과 같다.

```text
Guard Hold 중 피격
-> Damage / Defense 판정에서 Block_Hit 선택
-> Block_Hit Reaction 시작
   -> Guard pose overlay clear
   -> guard 판정 false
   -> parry 판정 false
   -> guard 입력 의도는 유지
-> Block_Hit Reaction 종료
   -> guard 입력 의도가 유지 중이면 Guard Hold 복구
   -> guard 입력이 해제되어 있으면 Guard Out 또는 Idle 복귀
```

`Complete`와 `Stop / Interrupt`는 다르게 취급한다.

- `Complete`: 정상적인 `Block_Hit` 종료이므로 `bWantsGuarding`을 기준으로 Guard Hold 복구 또는 Guard Out / Idle 복귀를 판단할 수 있다.
- `Stop / Interrupt`: death, stronger reaction, forced cancel, cinematic 같은 외부 중단일 수 있으므로 기본적으로 보수적인 정리를 우선한다.

몽타주와 notify는 상태 전환 타이밍을 알려주는 역할에 가깝고, 복구 여부 판단은 `DefenseComponent` 또는 Reaction 종료 처리 쪽에서 담당하는 것이 적절하다.

v1에서는 먼저 `Block_Hit`이 별도 Reaction으로 실행될 수 있는 구조를 확인하고, 입력 유지 시 Guard Hold 복귀와 입력 해제 시 Guard Out 연결은 단계적으로 확정한다.

---

## 10. Guard Runtime 정리 기준

Guard runtime 정리는 두 단계로 나눈다.

```text
HandleGuardLifecycleCompleted
-> 정상 종료
-> 입력 의도 / pose / guard / parry / 재시작 lock까지 정리

HandleGuardLifecycleInterrupted
-> 간섭 종료
-> 입력 의도 / pose / guard / parry / 재시작 lock까지 정리

ClearGuardOverlay
-> Guard 상태는 유지하되 pose / guard / parry 판정만 잠깐 내림
-> guard 입력 의도와 재시작 lock은 유지

RestoreGuardOverlay
-> 입력 의도가 유지 중이면 Guard Hold 상태 복구
```

`ClearGuardState`는 Dodge, Hit, Dead처럼 다른 실행이 Guard lifecycle을 끝내야 하는 경우에 사용한다. `ClearGuardOverlay`는 `Block_Hit`처럼 입력 의도는 유지하되 pose / guard / parry 판정만 잠깐 내릴 때 사용하고, 이후 `RestoreGuardOverlay`에서 입력 의도를 기준으로 Guard Hold 복구 여부를 판단한다.

Guard snapshot은 v1에서 다음 값을 가진다.

```text
bWantsGuarding
bIsGuardingPose
bCanGuard
bCanParry
bCanStartGuard
```

이 중 `bCanStartGuard`는 Guard In 재진입 허용 여부를 나타낸다. Guard In은 `bIsGuardingPose == false`이고 `bCanStartGuard == true`일 때만 실행 의미가 있다.

---

## 11. Combat Resolution과의 관계

Observable Overlay Layer는 Combat Resolution을 대체하지 않는다. 다만 두 구조는 같은 설계 패턴을 공유할 수 있다.

공통 패턴은 다음과 같다.

```text
공통 진입점
-> 현재 판단에 필요한 snapshot 구성
-> incoming 또는 packet 해석자가 snapshot을 기준으로 결과 결정
-> 필요한 후처리 handling 또는 mutation 요청
-> 공통 흐름이 decision과 요청된 처리를 적용
```

Overlay gate는 Action / Reaction 시작 직전에 현재 overlay 상태와 새 실행이 공존 가능한지 판단한다.

Combat Resolution은 이후 damage packet 처리 시점에서 parry / guard / invincible / armor / buff 같은 정책이 damage, reaction, feedback 결과에 어떻게 개입할지 판단한다.

따라서 Combat Resolution도 장기적으로는 다음과 유사한 구조가 될 수 있다.

```text
TakeDamage 또는 damage packet 진입
-> Combat Resolution snapshot 구성
-> Defense / Parry / Guard / Invincible / Buff 상태를 기준으로 packet 해석
-> damage block / reduce / continue / parry 결과 병합
-> Damage / Reaction / Feedback 흐름으로 전달
```

차이는 결과 복잡도다. Overlay gate는 실행 가능 여부와 overlay handling 누적이 핵심이지만, Combat Resolution은 damage amount 변경, reaction 억제, feedback 요청, attacker reaction, hit stop 같은 결과를 함께 병합해야 한다. 따라서 Combat Resolution에는 overlay gate보다 더 명확한 priority / terminal decision / mutation rule이 필요할 수 있다.

---

## 12. 관련 문서

- `Docs/01_Work_List/W03_Parry/W03_UE5_Portfolio_Work_List.md`
- `Docs/02_Bug_Report/B11_UE5_Portfolio_Bug_Report.md`
- `Docs/06_notes/N02_Guard_Release_Deferred_Request_Note.md`

`B11`은 Guard release 문제와 v1 처리 기록을 담당한다.

`N02`는 deferred action candidate 구조의 필요성과 consume 흐름을 담당한다.

`N03`은 `Guard Hold`를 둘러싼 execution state / overlay layer 설계 판단을 담당한다.
