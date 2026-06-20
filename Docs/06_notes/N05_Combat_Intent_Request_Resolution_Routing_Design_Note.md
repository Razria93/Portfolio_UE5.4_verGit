# Combat Intent / Request / Resolution / Routing Design Note

## 1. 목적

본 문서는 전투 처리 흐름을 `Intent -> Request -> Resolution -> Routing -> Domain` 계층으로 나누는 장기 구조를 정리한다.

W03 Guard / Parry v1 구현 과정에서 `ApplyDamageComponent`, `TakeDamageComponent`, `CombatResolution`, `CombatConsequenceCoordinator`의 책임 경계가 논의되었다. 이 문서는 해당 논의를 바탕으로 전투 요청이 어떻게 만들어지고, 수신되고, 판정되고, 각 도메인으로 분배되는지 설명한다.

이 문서는 구현 지시서가 아니라 후속 Combat Resolution / Combat Consequence 구조 정리를 위한 설계 판단 기록이다.

---

## 2. 전체 계층

전투 처리는 다음 계층으로 나눈다.

```text
Environment
-> Intent
-> Request
-> Resolution
-> Routing
-> Domain
```

각 계층의 의미는 다음과 같다.

| 계층 | 역할 |
| --- | --- |
| Environment | 입력, AI, 시스템, 오브젝트, 충돌 등 intent가 발생하는 환경 |
| Intent | 사용자의 입력, AI 판단, 시스템 이벤트 등 실행 의도 |
| Request | intent를 실행 또는 판정 가능한 request / packet으로 변환 |
| Resolution | request와 현재 상태를 해석해 결과를 판정 |
| Routing | 판정 결과를 후속 도메인으로 분배 |
| Domain | Action, Reaction, Movement, Weapon, Interaction, Feedback, UI 등 실제 실행 책임을 가진 영역 |

---

## 3. Intent

`Intent`는 아직 실행 결과가 아니라 “무엇을 하고 싶다”는 의도다.

예시는 다음과 같다.

- player input
- AI behavior tree
- system event
- object event
- hit window event
- cue event

모든 intent가 combat pipeline을 타지는 않는다.

```text
Move input
-> Movement domain

Look input
-> Controller / Camera domain

Guard input
-> Action domain

Hit window overlap
-> Combat request

Blink / Repulse cue
-> Combat request
```

기준은 단순하다.

```text
상대 상태와 내 공격 / 방어 정보를 조합해 전투 outcome을 판정해야 하는가?
-> Combat Request로 보낸다.

전투 outcome 판정이 필요 없는 실행 의도인가?
-> 해당 domain으로 직접 보낸다.
```

---

## 4. Request Layer

`Request` 계층은 intent를 실행 또는 판정 가능한 형태로 바꾼다.

그림에서 나뉜 `Requester`, `Builder`, `Sender`, `Receiver`는 모두 Request 계층 안의 역할로 본다.

```text
Intent
-> Requester
-> Builder
-> Sender
-> Receiver
```

### 4.1 Requester

`Requester`는 intent를 request 생성 흐름으로 진입시킨다.

예시는 다음과 같다.

- 공격 hit가 발생했으니 combat request를 만들도록 요청
- lock-on target에게 cue request를 만들도록 요청
- interaction input을 interaction request로 변환
- action input을 action request로 변환

전투 계층에서는 장기적으로 `CombatRequester` 또는 `CombatRequestSource`가 이 역할을 맡을 수 있다.

### 4.2 Builder

`Builder`는 raw data를 request / packet으로 조립한다.

예시는 다음과 같다.

```text
HitContext
-> CombatRequestPacket

CueContext
-> CombatRequestPacket

CombatResolutionResult
-> ReactionRequest / FeedbackRequest / CombatResultPacket
```

다만 request 생성 단계의 builder와 consequence routing 단계의 builder는 역할이 다르다.

| Builder | 역할 |
| --- | --- |
| CombatRequestBuilder | 원본 combat request packet 생성 |
| CombatConsequenceBuilder | resolution result를 후속 도메인 request / packet으로 변환 |

### 4.3 Sender

`Sender`는 만들어진 request / packet을 대상에게 전달한다.

예시는 다음과 같다.

```text
target receiver interface 호출
target actor TakeDamage 호출
interaction target interface 호출
event bus broadcast
```

`Sender`는 결과를 확정하지 않는다. 결과 판정은 receiver 이후의 resolution 계층에서 처리한다.

### 4.4 Receiver

`Receiver`는 request / packet을 받는 경계다.

전투에서는 `CombatReceiver`가 본인, 적, 타인 등 target actor의 combat request 수신 경계가 된다.

```text
CombatRequestPacket
-> CombatReceiver
-> CombatResolution
```

기존 UE `TakeDamage`는 장기적으로 `CombatReceiver`의 adapter 역할로 볼 수 있다.

---

## 5. Resolution

`Resolution`은 request와 현재 상태를 해석해 결과를 결정한다.

전투에서는 `CombatResolution`이 이 역할을 맡는다.

주요 책임은 다음과 같다.

- request 유효성 판단
- source / target / instigator / damage causer 확인
- defender 상태 확인
- guard / parry / invincible / armor / cue timing 판단
- `Hit / Guard / BlockHit / Parry / Blink / Repulse / Dead / Ignore` outcome 결정
- damage commit 필요 여부 결정
- reaction intent / feedback cue / external result 후보 구성

`Resolution`은 실행하지 않는다.

```text
CombatResolution
-> Parry 성공 판정
-> damage commit 없음
-> defender reaction intent = Parry
-> attacker result = Parried
-> feedback cue = ParrySpark
```

이 단계에서 montage 재생, VFX spawn, HP commit, reaction intervention을 직접 수행하지 않는다.

---

## 6. Routing

`Routing`은 resolution 결과를 각 domain으로 분배한다.

전투에서는 `CombatConsequenceCoordinator`가 이 역할을 맡는 것이 적절하다.

```text
CombatResolutionResult
-> DamageApplyRequest
-> ReactionRequest
-> FeedbackRequest
-> CombatResultPacket
-> Event
```

`CombatConsequenceCoordinator`는 단순 전달자라기보다 후속 처리의 순서와 변환 경계를 조율하는 계층이다.

단, 실제 실행 책임을 가져서는 안 된다.

```text
해야 하는 것
-> 어떤 후속 request가 필요한지 구성
-> 어떤 domain으로 보낼지 결정
-> 처리 순서 조율

하지 않는 것
-> HP 직접 감소
-> montage 직접 재생
-> VFX / SFX 직접 실행
-> action / reaction 충돌 직접 판정
```

---

## 7. Domain

`Domain`은 실제 실행 책임을 가진 영역이다.

예시는 다음과 같다.

- Lifecycle
- Action / Reaction
- Movement
- Weapon
- Interaction
- Feedback
- UX / UI
- Routing / Event

각 domain은 자체 component, object, orchestrator를 가질 수 있다.

---

## 8. Orchestration / Component / Object

그림의 하단에 있는 `Orchestration`, `Component`, `Object`는 domain 내부 구현 역할로 본다.

### Orchestration

`Orchestration`은 실행 충돌, 개입, 예약, 무시, 우선순위 같은 실행 조율을 담당한다.

예시는 다음과 같다.

- `ActionOrchestrator`
- `ReactionOrchestrator`

`Orchestrator`는 intent의 의미를 판정하지 않는다.

```text
CombatResolution
-> Parry reaction intent 생성

ReactionOrchestrator
-> 지금 Parry reaction을 실행할 수 있는가?
-> active action을 끊을 수 있는가?
-> 기존 reaction과 충돌하는가?
-> reserve / defer / ignore가 필요한가?
```

### Component

`Component`는 actor에 붙는 상태 보관 / 실행 API 단위다.

예시는 다음과 같다.

- `HealthComponent`
- `DefenseComponent`
- `MovementComponent`
- `ActionComponent`
- `ReactionComponent`
- `FeedbackComponent`

### Object

`Object`는 데이터, 실행 전략, executor, policy처럼 actor에 직접 붙지 않아도 되는 단위다.

예시는 다음과 같다.

- action executor
- reaction executor
- policy object
- data asset
- request / result struct

---

## 9. 전투 예시

### 공격 입력

```text
Attack input
-> Action request
-> ActionOrchestrator
-> attack montage 실행
-> hit window overlap
-> CombatRequester
-> CombatRequestPacket
-> target CombatReceiver
-> CombatResolution
-> CombatConsequenceCoordinator
-> damage / reaction / feedback / result 분배
```

공격 입력 자체는 combat request가 아니다.  
실제 hit 또는 cue가 발생했을 때 combat request가 만들어진다.

### Guard 입력

```text
Guard input
-> Action request
-> ActionOrchestrator
-> Guard In 실행
-> Defense overlay 상태 유지
```

Guard 입력 자체는 combat resolution을 타지 않는다.

이후 공격을 받으면 다음 흐름이 된다.

```text
Incoming combat packet
-> CombatReceiver
-> CombatResolution
-> Parry / Guard / Hit 판정
-> CombatConsequenceCoordinator
-> reaction / feedback / result 분배
```

### Parry 성공

```text
Incoming combat packet
-> CombatResolution
-> Outcome = Parry
-> DamageCommit = false
-> DefenderReactionIntent = Parry
-> AttackerResult = Parried
-> FeedbackCue = Parry
-> CombatConsequenceCoordinator
-> ReactionOrchestrator
-> Feedback
-> Attacker CombatResultReceiver
```

---

## 10. 현재 컴포넌트와의 관계

현재 구조에서는 여러 책임이 `ApplyDamageComponent`와 `TakeDamageComponent`에 압축되어 있다.

```text
ApplyDamageComponent
-> hit context 수집
-> spec 조회
-> target에게 damage request 전달

TakeDamageComponent
-> request 수신
-> defensive outcome 판단
-> damage commit
-> reaction dispatch
-> feedback dispatch
-> attacker result dispatch
```

장기 리팩터링 후보는 다음과 같다.

| 현재 책임 | 장기 후보 |
| --- | --- |
| ApplyDamage hit / cue 요청 생성 | CombatRequester / CombatRequestSource |
| ApplyDamage target 전달 | CombatRequestSender |
| TakeDamage 수신 | CombatReceiver / TakeDamage adapter |
| TakeDamage 방어 판정 | CombatResolution |
| TakeDamage 후속 분배 | CombatConsequenceCoordinator |
| Health 감소 | Health / DamageApply 계층 |
| Reaction 실행 판단 | ReactionOrchestrator |
| Feedback 실행 | Feedback 계층 |
| Attacker 결과 전달 | CombatResultPacket / Event |

---

## 11. 설계 기준

이 구조의 핵심 기준은 다음과 같다.

- 전투 outcome 판정은 `CombatResolution`에서만 한다.
- 후속 처리 분배는 `CombatConsequenceCoordinator`에서 한다.
- 실행 가능성 / 충돌 / 개입 판단은 각 `Orchestrator`가 한다.
- 상태 / 자원 변경은 해당 domain component가 한다.
- feedback은 이미 확정된 outcome을 재생할 뿐 outcome을 다시 판정하지 않는다.
- 외부로 전달된 결과가 새로운 실행을 만들면, 그것은 새로운 intent 또는 event로 본다.

---

## 12. 현재 구조 분석 기준 작업 목록

현재 코드 기준으로 `ApplyDamageComponent`와 `TakeDamageComponent`는 다음 책임을 가진다.

### 12.1 ApplyDamageComponent

현재 `ApplyDamageComponent`는 이름과 달리 target의 HP를 직접 변경하지 않는다.
주요 역할은 공격자 쪽 hit 정보를 검증하고, damage request를 구성해 target에게 전달하는 것이다.

현재 책임은 다음과 같다.

```text
HitWindow opened / closed 관리
-> 동일 hit window 안의 duplicate target 기록
-> HitContext validate
-> source / causer / target validate
-> friendly fire / self target / duplicate hit 필터
-> ApplyDamageSpec 조회
-> RequestDamage 계산
-> FDefaultDamageEvent 구성
-> TargetActor->TakeDamage 호출
```

장기 위치는 다음과 같다.

| 현재 함수 / 책임 | 장기 후보 |
| --- | --- |
| `NotifyHitWindowOpened/Closed` | CombatRequester의 hit window tracking 또는 별도 HitWindowTracker |
| `ValidateRequest`, `ValidateContext`, `CanApplyDamage` | CombatRequestSource의 source-side request validation |
| `ResolveApplyDamageSpec` | CombatRequestBuilder 또는 DamageSpecResolver |
| `ComputeApplyDamage` | CombatRequestBuilder의 requested damage 구성 |
| `ApplyDamageToTarget` | CombatRequestSender 또는 CombatReceiver 호출 경계 |
| `FApplyDamagePayload/Context/Result` | `FCombatRequestPayload/Context/Result` 후보 |

따라서 `ApplyDamageComponent`는 후속 리팩터링에서 `CombatRequester` 또는 `CombatRequestSource` 성격으로 축소한다.
다만 현재 branch에서는 `TakeDamageComponent` 쪽 책임 분리보다 우선순위가 낮다.

### 12.2 TakeDamageComponent

현재 `TakeDamageComponent`는 수신, 판정, 수치 적용, 후속 분배를 모두 가진 압축형 컴포넌트다.

현재 책임은 다음과 같다.

```text
UE TakeDamage 수신
-> FDefaultDamageEvent validate
-> FTakeDamagePayload / Context 구성
-> dead / parry / guard / zero damage 판단
-> mitigated / final damage 계산
-> HealthComponent에 HP commit
-> ReactionOrchestrator에 damage reaction request 전달
-> DamageFeedbackComponent에 damage feedback 전달
-> attacker에게 CombatResultPacket 전달
```

장기 위치는 다음과 같다.

| 현재 함수 / 책임 | 장기 후보 |
| --- | --- |
| `RequestTakeDamage`, `ProcessTakeDamage`, `HandleDefaultDamageEvent` 앞단 | CombatReceiver / TakeDamage adapter |
| `BuildPayload`, `BuildContext` | CombatReceiver adapter 또는 CombatRequest 변환 |
| `ValidateContext` | CombatResolution 입력 검증 후보 |
| `CanTakeDamage` | CombatResolution policy 후보 |
| `ComputeMitigatedDamage`, `ComputeFinalTakenDamage` | CombatResolution damage candidate 계산 후보 |
| `CommitTakeDamage`, `CommitDamageToHealth` | Health / DamageApply 계층 |
| `BuildResult` | CombatResolutionResult 후보 |
| `BuildPacket` | 임시 TakeDamage packet 또는 CombatResolutionResult adapter |
| `DispatchCombatResultToDefender` | CombatConsequenceCoordinator의 reaction / feedback 분배 후보 |
| `BuildCombatResultPacket`, `DispatchCombatResultToAttacker` | CombatConsequenceCoordinator의 external result 분배 후보 |
| `ResolveCombatResultReceiverActor` | CombatResult sender / target resolution 후보 |

### 12.3 우선 작업 순서

현재 코드에서 바로 컴포넌트를 여러 개로 나누기보다, 먼저 `TakeDamageComponent` 내부 경계를 함수 / 구조체 단위로 안정화한다.

권장 작업 순서는 다음과 같다.

1. **TakeDamageComponent 내부 단계를 명시적으로 분리**
   - `Receive`
   - `Resolve`
   - `Apply`
   - `Coordinate`

2. **CombatResolutionResult 후보 구조 정의**
   - 기존 `FTakeDamageResult`를 바로 폐기하지 않는다.
   - `DefenseOutcome`, `bShouldCommitDamage`, `FinalTakenDamage`, `CommittedDamage`, `ReactionType`, `FeedbackCue`, `CombatResultPacket` 후보 정보를 어떤 구조가 소유할지 먼저 정한다.

3. **Resolution 후보 함수 묶기**
   - `ValidateContext`
   - `CanTakeDamage`
   - `ComputeMitigatedDamage`
   - `ComputeFinalTakenDamage`
   - `ResolveDamageReactionType` 계열
   - 이 함수들은 이후 `CombatResolutionComponent`로 이동하기 쉬운 형태로 입력 / 출력 경계를 맞춘다.

4. **Damage apply 책임 분리**
   - `CommitDamageToHealth`는 `CombatResolution`에 남기지 않는다.
   - `HealthComponent` 또는 별도 damage apply 계층이 실제 상태 변경을 맡도록 경계를 둔다.

5. **Consequence coordination 후보 함수 묶기**
   - defender reaction request 생성
   - defender feedback request 생성
   - attacker combat result packet 생성 / 전달
   - rejected result 처리
   - 이 함수들은 이후 `CombatConsequenceCoordinator`로 이동하기 쉬운 형태로 정리한다.

6. **ApplyDamageComponent는 후순위로 리네임 / 축소**
   - `ApplyDamage`라는 이름은 장기적으로 부정확하다.
   - 다만 현재는 hit window, spec, UE TakeDamage 호출 경계와 강하게 묶여 있으므로 `TakeDamageComponent` 경계 정리 이후에 `CombatRequestSource` 후보로 다룬다.

### 12.4 이번 branch에서의 현실적 범위

이번 W03 branch에서 바로 수행할 수 있는 범위는 다음과 같다.

- `TakeDamageComponent` 내부의 책임 구분을 문서화한다.
- 새로 추가한 `FCombatResultPacket`은 external result packet 후보로 유지한다.
- `CombatResultReceiver` interface는 attacker result 수신 경계 후보로 유지한다.
- W03 v1에서는 Parry 성공 시 `CombatResultDispatch` 로그로 전달 시도 / 완료를 확인하고, receiver 쪽 `CombatResult` 로그로 수신 여부와 packet 내용을 분리해 확인한다.
  - `Receiver`는 result packet을 받은 actor다.
  - `Requester`는 result packet을 만들어 돌려보낸 쪽이며, 현재 Parry 검증에서는 원본 damage packet을 받은 player다.
  - `Source`와 `DamageCauser`는 원본 combat / damage packet에서 온 공격 주체와 실제 damage causer다.
- `CombatResolutionComponent`, `CombatConsequenceCoordinatorComponent`를 즉시 완성하지 않는다.
- 다음 branch에서 `TakeDamageComponent`의 `Resolve / Coordinate` 경계를 먼저 코드로 분리한다.

---

## 13. 관련 문서

- `Docs/01_Work_List/W03_Parry/W03_UE5_Portfolio_Work_List.md`
- `Docs/06_notes/N03_Guard_Hold_Overlay_Layer_Design_Note.md`
- `Docs/06_notes/N04_Blink_Repulse_Combat_Packet_Design_Note.md`
