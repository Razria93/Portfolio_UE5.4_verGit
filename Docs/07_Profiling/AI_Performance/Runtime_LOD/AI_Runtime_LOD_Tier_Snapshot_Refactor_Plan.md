# AI Runtime LOD Tier Snapshot Refactor Plan

## 목적

State-based Runtime LOD를 실제 정책으로 적용하기 전에, Runtime LOD tier를 어디서 계산하고 어디서 소비할지 책임을 정리한다.

이번 문서는 다음 흐름을 기록한다.

```text
이전 raw 구조
-> 1차 문제 제기
-> 현재 구조
-> 2차 문제 제기
-> 아이디어 제시
-> 후속 리팩토링 계획
-> 현재 적용할 내용
-> 추후 적용할 내용
```

## 이전 Raw 구조

초기 state-based Runtime LOD 구현은 다음 책임이 한 흐름에 섞여 있었다.

```text
CBTServiceIntervalHelper
-> BT service interval CVar 읽기
-> CombatEngageSubsystem에 update precision 질의
-> StateRuntimeLODPolicy에 tier 판정 위임
-> StateRuntimeLODPolicy audit counter 기록
-> interval preset 선택
```

당시 구조의 핵심 흐름:

```text
BTServiceIntervalHelper
-> ResolveAIUpdatePrecision()
-> CombatEngageSubsystem::GetAIUpdatePrecision()
-> EAIUpdatePrecision 반환
-> Mode + Precision으로 BT interval preset 선택
```

여기에 state-based Runtime LOD audit을 붙이는 과정에서 `FAIStateRuntimeLODPolicy`가 다음 책임까지 갖게 됐다.

```text
FAIStateRuntimeLODPolicy
-> StatePolicyMode CVar
-> audit enabled 여부
-> Blackboard에서 가져온 context 기반 tier 판정
-> CSV counter 기록
```

즉, `Policy`라는 이름의 얇은 profiling / CVar 계층 안에 실제 Runtime LOD tier 판정이 들어갔다.

또한 `BTServiceIntervalHelper` 안에도 다음 함수들이 들어가 있었다.

```text
BuildStateRuntimeLODContext(Blackboard)
RecordStateRuntimeLODTier(OwnerComp)
```

이 상태에서는 각 클래스의 책임이 다음처럼 섞인다.

| 위치 | 섞인 책임 |
| --- | --- |
| `CombatEngageSubsystem` | assignment 관리 + update precision 판단 |
| `BTServiceIntervalHelper` | service interval 선택 + Blackboard tier context build + audit 호출 |
| `FAIStateRuntimeLODPolicy` | CVar / audit + tier resolve |

## 1차 문제 제기

첫 번째 문제는 책임 경계가 불명확하다는 점이다.

`CombatEngageSubsystem`은 전투 참여 권한을 관리하는 시스템이다.
따라서 `Engage / Alert assignment`, cap, lease, warmup을 담당하는 것은 자연스럽다.

하지만 Runtime LOD tier는 전투 참여자뿐 아니라 다음 계층까지 포함한다.

```text
CombatCritical
CombatSupport
Awareness
Background
Dormant
```

즉, Runtime LOD tier는 CombatEngage보다 넓은 개념이다.
`Observe / Idle / Dormant`까지 포함하는 판단을 CombatEngageSubsystem 안에 두면 subsystem 책임이 다시 커진다.

두 번째 문제는 `FAIStateRuntimeLODPolicy`의 역할이다.

Movement Runtime LOD policy처럼 policy helper는 다음 정도의 얇은 계층으로 두는 것이 맞다.

```text
CVar 정의
mode clamp
policy active 여부
profiling / audit 여부
```

그런데 tier 판정은 CVar가 없어도 Runtime LOD 정책의 기본 기반이 되는 기능이다.
따라서 `Policy`가 아니라 별도의 tier resolver / classifier 책임으로 분리하는 편이 맞다.

세 번째 문제는 `BTServiceIntervalHelper`의 역할이다.

이 helper는 BT service tick interval을 제어하기 위한 소비자다.
Blackboard를 조합해 tier context를 만들고, state audit까지 처리하면 interval helper 이상의 책임을 갖게 된다.

## 현재 구조

1차 리팩토링 후 구조는 다음과 같다.

```text
FAIRuntimeLODTierResolver
-> Blackboard 기반 Runtime LOD tier 판정

FAIStateRuntimeLODPolicy
-> StatePolicyMode CVar
-> audit enabled 여부
-> CSV counter 기록

CBTServiceIntervalHelper
-> BTUpdateIntervalMode CVar
-> Runtime LOD tier 소비
-> AIContext는 기본 interval 유지
-> AIIntentState는 Mode + Tier -> IntervalPreset 선택
-> IntervalPreset -> 실제 interval 반환

CombatEngageSubsystem
-> Engage / Alert assignment 관리
-> CombatRole 제공
```

이 구조에서 `CombatEngageSubsystem::GetAIUpdatePrecision()`은 제거한다.
BT interval 선택은 더 이상 CombatEngageSubsystem의 map을 직접 보지 않고, Blackboard 기반 Runtime LOD tier를 소비한다.

1차 리팩토링의 의미:

```text
CombatEngageSubsystem에서 tier / precision 판단 책임 제거
StateRuntimeLODPolicy에서 tier resolve 책임 제거
BTServiceIntervalHelper에서 context build 책임 제거
Runtime LOD tier 판정을 FAIRuntimeLODTierResolver로 분리
```

이로써 Runtime LOD tier는 BT interval뿐 아니라 Movement / Animation / Perception / Weapon policy에서도 공통으로 사용할 수 있는 기반이 된다.

## 2차 문제 제기

초기 구현은 `BTServiceIntervalHelper`에서 Blackboard 값을 읽어 Runtime LOD tier를 즉석 계산하고, 그 결과로 BT service interval preset을 선택하는 방식이었다.

이 방식은 동작은 가능하지만 구조적으로 다음 문제가 있다.

```text
AIContext service
-> Blackboard context 갱신
-> Blackboard 기반 Runtime LOD tier 판정
-> 다음 service interval 결정
```

`AIContext`는 TargetActor, LOS, distance, CombatRole 같은 판단 입력을 갱신한다.
그런데 `AIContext` 자신의 다음 갱신 주기를 같은 Blackboard 값으로 조정하면 자기참조 구조가 된다.

문제 흐름:

```text
AIContext interval 감소
-> Blackboard 최신성 감소
-> Runtime LOD tier 판정 지연
-> AIIntent / Movement / Animation이 stale tier를 소비
-> Observe / Alert / Idle 튕김 가능성 증가
```

따라서 Runtime LOD tier는 각 소비자가 자기 주기로 즉석 계산하는 값이 아니라, 공통 템포에서 갱신된 snapshot 값으로 다뤄야 한다.

## 아이디어 제시

Runtime LOD tier는 컴퓨터 클럭처럼 여러 AI 시스템이 같은 기준 시점의 값을 읽는 것이 이상적이다.

목표 구조:

```text
Perception / AIContext / CombatEngage / Blackboard
        ↓
Runtime LOD Tier Snapshot
        ↓
BT / Movement / Animation / Weapon / Feedback / Perception budget
```

각 시스템은 Blackboard를 다시 조합해 tier를 계산하지 않고, 이미 갱신된 `CurrentRuntimeLODTier`를 소비한다.

이렇게 하면 다음 장점이 있다.

```text
1. BT / Movement / Animation이 같은 tier를 읽는다.
2. service별 interval 차이로 인한 stale tier 튕김을 줄인다.
3. Runtime LOD tier 판정 책임이 CombatEngageSubsystem이나 BT helper에 묶이지 않는다.
4. 추후 Perception Active Budget, Dormant, Wake-up으로 확장하기 쉽다.
```

## 리팩토링 계획

### 1. Tier Resolver 분리

Runtime LOD tier 계산은 별도 resolver가 담당한다.

```text
FAIRuntimeLODTierResolver
-> Blackboard 기반 context build
-> Context 기반 tier resolve
-> LexToString
```

Tier 이름은 AIIntentState 이름을 그대로 따르지 않는다.
AIIntentState는 행동 상태이고, Runtime LOD tier는 성능 정책 계층이기 때문이다.

현재 tier:

| Tier | 의미 |
| --- | --- |
| CombatCritical | 직접 전투 결과, 피격 반응, 사망, combat timing 보존이 필요한 객체 |
| CombatSupport | 전투 주변 보조, Alert role, 근거리 전투 후보 |
| Awareness | target awareness는 있지만 combat assignment가 없는 상태 |
| Background | Idle / Patrol 같은 일반 background 상태 |
| Dormant | offscreen / far / wake-up 대기 후보 |

Tier는 `AIIntentState` 이름을 그대로 따라가지 않는다.
`Chase`와 `Investigate`는 성능 tier가 아니라 행동 상태다.

해석 기준:

```text
Dormant candidate -> Dormant
Dead / HitReact -> CombatCritical
CombatRole Engage -> CombatCritical
CombatRole Alert -> CombatSupport
Target / LOS 있음 + CombatRole None -> Awareness
Target / LOS 없음 -> Background
```

따라서 `Engage + Chase`는 CombatCritical이고, `Alert + Chase`는 CombatSupport다.
`Investigate`는 현재 정책상 Engage에서 파생되는 recovery 행동에 가까우므로 상태명만으로 CombatSupport에 넣지 않는다.

### 2. StateRuntimeLODPolicy 축소

`FAIStateRuntimeLODPolicy`는 tier를 계산하지 않는다.

담당 책임:

```text
StatePolicyMode CVar
audit enabled 여부
CSV counter 기록
```

즉, policy는 profiling / audit용 얇은 막으로 유지한다.

### 3. BTServiceIntervalHelper 정리

`CBTServiceIntervalHelper`는 BT service interval만 결정한다.

담당 책임:

```text
BTUpdateIntervalMode 읽기
Runtime LOD tier 소비
AIContext 기본 interval 유지
AIIntentState Mode + Tier -> IntervalPreset 선택
IntervalPreset -> 실제 interval 반환
```

`BTServiceIntervalHelper` 안에서 Blackboard context build를 직접 하지 않는다.

### 4. CombatEngageSubsystem 책임 축소

`CombatEngageSubsystem`은 전투 참여 권한만 관리한다.

담당 책임:

```text
Engage / Alert request 수집
assignment rebuild
Engage / Alert cap 적용
assignment lease / warmup
CombatRole 제공
```

Runtime LOD tier나 update precision을 직접 판단하지 않는다.

## 현재 적용할 내용

이번 단계에서는 아래까지만 적용한다.

```text
1. FAIRuntimeLODTierResolver 추가
2. FAIStateRuntimeLODPolicy를 CVar / audit 전용으로 축소
3. CBTServiceIntervalHelper에서 CombatEngageSubsystem::GetAIUpdatePrecision 의존 제거
4. AIContext interval은 기본값으로 고정
5. AIIntentState interval 선택을 Runtime LOD tier 기반으로 변경
6. CombatEngageSubsystem에서 GetAIUpdatePrecision 제거
7. ACAIController에 CurrentRuntimeLODTier snapshot 추가
8. UpdateAIContext 이후 Runtime LOD tier snapshot refresh
9. AIIntentState 변경 이후 Runtime LOD tier snapshot refresh
10. BTServiceIntervalHelper가 controller snapshot을 우선 소비
11. Movement Runtime LOD policy가 controller snapshot을 소비
```

단, 아직 완전한 Runtime LOD 적용 구조는 아니다.

이유:

```text
Animation 소비 경로 변경
Dormant / Wake-up manager
Perception budget
```

위 항목은 작업 범위가 커지므로 후속 단계로 분리한다.

현재 적용 후 구조:

```text
BTServiceIntervalHelper
-> AIContext는 Default interval 반환
-> AIIntentState는 ACAIController::GetCurrentRuntimeLODTier() 우선 소비
-> controller snapshot이 없을 때만 FAIRuntimeLODTierResolver::ResolveTier(Blackboard) fallback
-> Mode + Tier로 interval preset 선택

UpdateAIContext
-> Blackboard context 갱신
-> ACAIController::RefreshRuntimeLODTierFromBlackboard()
-> CurrentRuntimeLODTier 저장

UpdateAIIntentState
-> AIIntentState 변경
-> ACAIController::RefreshRuntimeLODTierFromBlackboard()
-> Dead / HitReact 같은 absolute state tier 반영

Movement Runtime LOD
-> FAIMovementRuntimeLODPolicy::GetEnemyMovementMode(Owner)
-> StatePolicyMode가 꺼져 있으면 기존 EnemyMovementMode CVar 소비
-> StatePolicyMode가 켜져 있으면 OwnerController.GetCurrentRuntimeLODTier() 소비
-> Awareness / Dormant는 movement intent block
```

이는 최종 구조는 아니지만, CombatEngageSubsystem과 BT helper에 섞여 있던 책임을 먼저 분리하는 중간 단계다.

## 추후 적용할 내용

### 1. AIContext interval 고정

`AIContext`는 target / LOS / distance / CombatRole 같은 판단 입력을 갱신하는 producer다.

따라서 Runtime LOD 대상에서 제외하고 기본 interval을 유지하는 것이 원칙적으로 안전하다.

권장 정책:

```text
AIContext: 고정 interval
AIIntentState: Runtime LOD tier 기반 interval 조정
EngageContext: combat timing 계층이므로 고정 interval
```

### 2. Animation 소비 경로 변경

Movement는 controller snapshot 소비 경로로 연결했다.
다음 단계에서는 Animation도 Blackboard를 다시 조합하지 않고 controller snapshot을 읽는다.

예상 흐름:

```text
UCAnimInstance
-> OwnerController.GetCurrentRuntimeLODTier()
-> tier별 parameter refresh / animation detail policy 적용
```

### 3. AIRuntimeLODSubsystem 승격

Dormant / Wake-up / Perception Active Budget까지 들어오면 `ACAIController` snapshot만으로는 부족해질 수 있다.

그 시점에는 별도 subsystem으로 승격한다.

후보:

```text
UCWorldSubsystem_AIRuntimeLOD
```

담당 책임:

```text
Controller별 CurrentRuntimeLODTier 관리
distance / visibility 기반 Dormant 후보 관리
wake-up query
Perception Active Budget
tier별 policy 값 제공
```

단, 현재 단계에서 바로 subsystem으로 가지는 않는다.
아직 Dormant / Wake-up / Perception budget이 구현 전이므로 작업 범위 대비 이득이 작다.

## 결론

최종 방향은 다음과 같다.

```text
Runtime LOD tier 판정은 공통 resolver로 분리한다.
StateRuntimeLODPolicy는 CVar / audit 전용으로 유지한다.
BTServiceIntervalHelper는 tier를 소비해 interval만 선택한다.
CombatEngageSubsystem은 CombatRole assignment만 담당한다.
ACAIController는 CurrentRuntimeLODTier snapshot을 저장한다.
Movement Runtime LOD policy는 controller snapshot을 소비한다.
다음 단계에서는 Animation이 controller snapshot을 소비한다.
장기적으로는 AIRuntimeLODSubsystem으로 승격한다.
```

이 리팩토링은 성능 수치 개선 자체보다 Runtime LOD 정책을 안전하게 확장하기 위한 구조 정리다.
특히 service별 갱신 주기 차이로 인해 발생할 수 있는 stale tier / 상태 튕김 문제를 줄이는 것이 핵심이다.
