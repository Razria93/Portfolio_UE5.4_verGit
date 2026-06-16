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
-> 필요한 경우 observable overlay policy registry 확인
-> 각 overlay owner가 종료 / 유지 / 무시 여부 판단
-> 둘 다 없으면 baseline Idle로 처리
```

이 구조에서는 `Block_In / Block_Out`은 Action Domain의 전환 action으로 유지한다. `Guard Hold`는 `ExecutionSnapshot`에 세부 상태를 복사하지 않고, `DefenseComponent` 같은 overlay owner가 policy로 참여해 직접 판단하는 방향을 기준으로 한다.

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
-> ExecutionSnapshot에는 overlay 세부 상태를 넣지 않는다.
-> observable overlay policy registry를 순회한다.
-> 첫 policy owner는 DefenseComponent / Guard overlay로 둔다.
-> Action / Reaction start 전에 필요한 경우 overlay handling을 누적한다.
```

이 구현은 full overlay participant 모델이 아니다. v1에서는 `FExecutionParticipant`를 확장하지 않고, policy decision과 result handling으로 Guard overlay cleanup 문제를 먼저 해결한다.

---

## 6. Combat Resolution과의 관계

Observable Overlay Layer는 Combat Resolution을 대체하지 않는다. 다만 두 구조는 같은 설계 패턴을 공유할 수 있다.

공통 패턴은 다음과 같다.

```text
공통 진입점
-> 등록된 policy provider 순회
-> 각 owner가 자기 runtime state와 query를 기준으로 relevant / allowed / handling 판단
-> 공통 흐름이 decision을 병합
```

Overlay gate는 Action / Reaction 시작 직전에 현재 overlay 상태와 새 실행이 공존 가능한지 판단한다.

Combat Resolution은 이후 damage packet 처리 시점에서 parry / guard / invincible / armor / buff 같은 정책이 damage, reaction, feedback 결과에 어떻게 개입할지 판단한다.

따라서 Combat Resolution도 장기적으로는 다음과 유사한 구조가 될 수 있다.

```text
TakeDamage 또는 damage packet 진입
-> Combat Resolution policy registry 순회
-> Defense / Parry / Guard / Invincible / Buff policy 판단
-> damage block / reduce / continue / parry 결과 병합
-> Damage / Reaction / Feedback 흐름으로 전달
```

차이는 결과 복잡도다. Overlay gate는 `allowed`와 overlay handling 누적이 핵심이지만, Combat Resolution은 damage amount 변경, reaction 억제, feedback 요청, attacker reaction, hit stop 같은 결과를 함께 병합해야 한다. 따라서 Combat Resolution에는 overlay gate보다 더 명확한 priority / terminal decision / mutation rule이 필요할 수 있다.

---

## 7. 관련 문서

- `Docs/01_Work_List/W03_Parry/W03_UE5_Portfolio_Work_List.md`
- `Docs/02_Bug_Report/B11_UE5_Portfolio_Bug_Report.md`
- `Docs/06_notes/N02_Guard_Release_Deferred_Request_Note.md`

`B11`은 Guard release 문제와 v1 처리 기록을 담당한다.

`N02`는 deferred action candidate 구조의 필요성과 consume 흐름을 담당한다.

`N03`은 `Guard Hold`를 둘러싼 execution state / overlay layer 설계 판단을 담당한다.
