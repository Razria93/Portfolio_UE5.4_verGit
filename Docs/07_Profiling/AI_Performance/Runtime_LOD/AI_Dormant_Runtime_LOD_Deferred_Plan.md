# AI Dormant Runtime LOD Deferred Plan

## 목적

이 문서는 AI Runtime LOD 최적화 탐색 이후, 당장 진행하지 않고 후속 작업으로 보류한 `Dormant Runtime LOD` 작업을 정리한다.

새 세션에서 이 작업을 다시 시작할 때는 이 문서를 먼저 읽고, 기존 측정 축과 현재 보류 판단을 확인한 뒤 진행한다.

## 현재 판단

AI Runtime LOD 최적화 탐색에서는 다음 축을 각각 분리해서 확인했다.

```text
WeaponActor
Perception
BT Update Interval
CombatEngage Assignment / AlertCap
Observe / Investigate lifecycle
Movement / Nav
Animation parameter refresh
Combat Collision / HitProcessing
Combat Feedback Presentation
Enemy Actor Tick
State Runtime LOD tier snapshot
```

결론적으로, “최적화라고 부를 만한 주요 축”은 한 번씩 검토했다.

현재 남아 있는 `Perception Active Budget`, `Wake-up`, `Dormant Manager`, `Representation LOD`, `Proxy`, `120+ stress validation`은 서로 독립된 작은 최적화 축이라기보다, 하나의 큰 작업인 `Dormant Runtime LOD`를 구성하는 하위 요소로 보는 것이 맞다.

## 왜 지금 보류하는가

`Dormant Runtime LOD`는 단순 측정이나 작은 gate 추가가 아니라 실제 gameplay 정책을 반영하는 본격 구현 작업이다.

보류 이유:

```text
1. 구현 범위가 크다.
   - Dormant 진입 조건
   - Dormant 유지 조건
   - Dormant 복귀 조건
   - Perception wake-up
   - Movement / BT / Animation / Representation 복구 순서가 모두 필요하다.

2. 기존 최적화 탐색 목적은 이미 달성했다.
   - 비용 축은 대부분 측정했다.
   - Runtime LOD tier snapshot 구조도 구축했다.
   - v1 정책 기반은 준비됐다.

3. Dormant는 측정 후보가 아니라 실제 시스템 설계에 가깝다.
   - 잘못 적용하면 Enemy가 깨어나지 않거나, combat relevance가 깨질 수 있다.
   - 따라서 별도 feature로 충분한 시간을 두고 다루는 것이 맞다.

4. 현재 우선순위는 이후 남은 기능 / 구조 작업이다.
   - Dormant 구현에 들어가면 장기 작업이 되므로, 지금은 후속 후보로 보류한다.
```

## 현재까지 준비된 기반

P41 기준으로 다음 기반은 이미 들어가 있다.

```text
Runtime LOD tier:
CombatCritical
CombatSupport
Awareness
Background
Dormant

Tier resolver:
CAIRuntimeLODTierResolver

Policy:
CAIStateRuntimeLODPolicy
CAIMovementRuntimeLODPolicy
CAIAnimationRuntimeLODPolicy

Snapshot:
ACAIController::CurrentRuntimeLODTier

Consumers:
BT interval helper
Movement policy
Animation policy
```

즉, Dormant 자체를 바로 구현하지는 않았지만, 이후 Dormant 정책을 얹을 수 있는 tier / snapshot 소비 구조는 준비되어 있다.

## 아직 없는 것

현재 코드 기준으로 `Dormant`는 enum / policy 분기에 존재하지만, 정상 runtime 경로에서 안정적으로 진입하는 완성된 상태는 아니다.

미구현 항목:

```text
1. Dormant 후보 판정 입력
   - distance
   - camera visibility
   - offscreen / invisible
   - recent combat interaction
   - scripted importance

2. Dormant 진입 조건
   - far
   - invisible
   - no target awareness
   - no recent damage / combat interaction
   - no active move requirement

3. Dormant 유지 조건
   - BT / Perception / Movement / Animation을 어느 수준까지 줄일지
   - hidden / proxy / pose skip 적용 여부

4. Dormant 복귀 조건
   - player range
   - camera frustum or forward cone
   - damage
   - noise / script event
   - combat request

5. Wake-up manager
   - dormant actor registry
   - 주기적 sphere query 또는 distance check
   - wake-up 대상 선별

6. Hysteresis / minimum hold time
   - tier가 매 프레임 흔들리지 않도록 최소 유지 시간 필요
```

## 재개할 때 작업 순서

Dormant 작업을 다시 시작할 때는 아래 순서를 권장한다.

### 1. Dormant Policy 문서 확정

먼저 정책을 코드보다 먼저 확정한다.

정해야 할 것:

```text
Dormant 진입 조건
Dormant 해제 조건
Dormant 중 유지할 최소 기능
Dormant 중 끌 수 있는 기능
Dormant가 절대 적용되면 안 되는 상태
```

초기 권장 기준:

```text
Dormant 진입 후보:
far + invisible + no target awareness + no recent combat interaction + no active movement requirement

Dormant 금지:
CombatCritical
CombatSupport
HitReact
Dead 처리 중
Investigate 진행 중
scripted move / return home / patrol 이동 중
```

### 2. Wake-up 설계

Perception을 끄거나 줄이려면 별도 wake-up 경로가 먼저 있어야 한다.

후보:

```text
Player-centered sphere query
Player-centered sphere component overlap
Distance + camera forward cone
Damage / noise / scripted event
```

권장 시작점:

```text
WorldSubsystem 기반 Dormant registry
주기적 distance / sphere check
Wake-up event 발생 시 tier를 Background 또는 Awareness로 복귀
```

### 3. Perception Active Budget

Perception을 바로 끄는 방식은 profiling gate로는 가능하지만 gameplay 정책으로는 위험하다.

권장 방향:

```text
CombatCritical: Perception High
CombatSupport: Perception Reduced
Awareness: Perception Low
Background: Perception Budgeted
Dormant: Perception Off + Wake-up
```

단, Dormant wake-up 없이 Perception Off를 먼저 적용하지 않는다.

### 4. Representation LOD

Wake-up이 안정화된 뒤 representation을 줄인다.

후보:

```text
mesh hidden
weapon hidden
animation update off
pose skip
proxy actor
shadow off
material simplification
```

주의:

```text
CombatCritical / CombatSupport에는 보수적으로 적용한다.
Background / Dormant부터 적용한다.
Proxy는 별도 큰 작업으로 분리한다.
```

### 5. Stress Validation

정책이 들어간 뒤 측정한다.

권장 순서:

```text
40 Enemy smoke
80 Enemy smoke
120+ Enemy stress
```

측정 시 확인할 것:

```text
Dormant 진입 수
Wake-up 수
깨어난 뒤 perception / BT / movement / animation 정상 복구
Frame / Game p95
CharacterMovement p95
Animation p95
BT Tick p95
AIPerception p95
게임플레이 이상 여부
```

## 권장 브랜치 분리

Dormant는 한 브랜치에서 모두 끝내기보다 다음처럼 나누는 것이 안전하다.

```text
feature/ai-dormant-policy-plan
-> Dormant 정책 / 진입 / 해제 / 금지 조건 문서화

feature/ai-perception-wakeup-budget
-> wake-up 경로와 perception active budget 설계

feature/ai-dormant-manager
-> dormant registry, enter / exit, hysteresis 구현

feature/ai-dormant-representation-lod
-> mesh / animation / weapon / proxy 적용

feature/ai-dormant-stress-validation
-> 40 / 80 / 120+ 측정과 정책 튜닝
```

## 재개 전 참고 문서

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_State_Based_Runtime_LOD_Policy_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Runtime_LOD_Tier_Snapshot_Refactor_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_AlertCap_Comparison_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Perception_Runtime_LOD_Measurements.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Movement_Nav_LOD_Measurement_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/Enemy_Mesh_Runtime_LOD_Measurements.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Runtime_LOD_Debugging_Obstacle_Note.md
```

## 현재 결론

현재 AI Runtime LOD 작업은 `Dormant` 전 단계의 비용 축 탐색과 v1 tier 기반 구조 정리까지 완료한 것으로 본다.

`Dormant Runtime LOD`는 남은 최적화 축이 아니라, 이후 별도 feature로 다룰 본격적인 runtime system 구현 작업이다.

따라서 지금은 보류하고, 이후 시간이 확보되면 이 문서를 기준으로 `Perception Active Budget / Wake-up`부터 재개한다.
