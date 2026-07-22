# AI State-Based Runtime LOD Policy Plan

## 목적

지금까지의 Runtime LOD 작업은 축별 전역 CVar로 비용을 분리하는 방식이었다.

```text
EnemyMovementMode
EnemyAnimationMode
BTUpdateIntervalMode
DisableEnemyWeaponActor
DisableEnemyHitProcessing
DisableEnemyCombatFeedback
```

이 문서는 축별 실험 결과를 바탕으로, 실제 적용 가능한 상태 기반 Runtime LOD 정책 v1을 정의한다.

핵심 전환:

```text
축별 전역 On / Off 실험
-> Enemy 상태와 전투 relevance에 따른 tier 기반 정책
```

## 측정 근거

| 축 | 관찰 결과 | 정책 판단 |
| --- | --- | --- |
| AlertCap / Assignment | Alert 후보 수 제한이 CharacterMovement p95에 직접 영향을 줬다. | 핵심 정책으로 유지 |
| BT Update Interval | service 호출 수 감소는 유효하지만 Frame / Game p95 개선은 제한적이었다. | tier별 update precision으로 사용 |
| Movement / Nav | movement intent block은 비용 감소 가능성이 있으나 전역 적용은 gameplay를 깨뜨린다. | 상태 기반으로만 적용 |
| Animation Refresh | parameter refresh 감소 gate는 동작했지만 단독 frame 개선은 약했다. | Awareness / Background 이하 보조 정책 |
| Enemy Actor Tick | `CEnemy Tick` 제거는 가능하지만 주요 병목은 아니었다. | 낮은 우선순위 |
| WeaponActor | 비용 축으로 유효했다. | Awareness 이하 후보, v1 직접 적용은 보수적 판단 |
| Combat Hit Pipeline | hit window / hit processing 차단 효과는 현재 조건에서 작았다. | v1 핵심 제어에서 제외 |
| Feedback Presentation | Enemy feedback skip은 정상 작동했지만 frame 개선은 작았다. | 최하위 representation 후보 |
| Perception | 비활성화는 입력 자체를 흔들 수 있다. | v1에서는 끄지 않고 후속 active budget으로 분리 |

## 기준

### Tier 기준

Runtime LOD tier는 AIIntentState 이름을 그대로 따르지 않는다.
`Chase`, `Investigate`, `Observe`, `Idle`은 행동 상태이고, Runtime LOD tier는 성능 정책 계층이다.

따라서 tier는 다음 입력을 조합해서 판단한다.

```text
CombatRole
Target awareness
Recent combat interaction
Distance / visibility
Dormant candidate
```

기본 tier:

| Tier | 기준 |
| --- | --- |
| CombatCritical | 직접 전투 결과, 피격 반응, 사망, combat timing 보존이 필요한 객체 |
| CombatSupport | 전투 주변 보조, Alert role, 근거리 전투 후보 |
| Awareness | target awareness는 있지만 combat role이 없는 객체 |
| Background | target awareness가 없고 전투 / 조사 중이 아닌 일반 객체 |
| Dormant | 멀고 보이지 않으며 wake-up 전까지 비활성화할 수 있는 객체 |

해석 기준:

```text
Engage role -> CombatCritical
Alert role -> CombatSupport
Target / LOS 있음 + role 없음 -> Awareness
Target / LOS 없음 -> Background
far + invisible + no recent interaction -> Dormant 후보
```

`Chase`와 `Investigate`는 tier를 직접 결정하지 않는다.
`Engage + Chase`는 CombatCritical이고, `Alert + Chase`는 CombatSupport다.
`Investigate`는 현재 정책상 Engage에서 파생되는 recovery 행동에 가깝기 때문에 CombatRole과 recent combat interaction을 우선 본다.

### Dormant 기준

`Dormant`는 움직이지 않는 비활성 객체로 한정한다.

멀리 있더라도 patrol, return home, scripted idle move처럼 목적지 이동이 필요한 객체는 `Dormant`로 내리지 않는다.
이 경우는 `Idle` tier의 `Idle Movement Low` 정책으로 처리한다.

```text
이동이 필요함
-> Idle Movement Low

이동이 필요 없음 + 멀리 있음 + 시야 밖
-> Dormant 후보
```

### Combat Hit Pipeline 기준

이 문서의 `Combat Hit Pipeline`은 전투 공격 판정 흐름을 뜻한다.

```text
Combat Hit Pipeline:
-> AnimNotify Collision Begin / End
-> ActionCollisionWindow Begin / End
-> WeaponComponent Open / Close
-> HitWindow Open / Close
-> HitWindow Overlap
-> HitProcessing
-> CombatSignal
-> CombatSignalCue route
```

### Feedback 기준

Feedback Presentation은 combat result와 분리한다.

```text
Action feedback:
-> trail / attack VFX / attack SFX

Reaction feedback:
-> hit / guard / parry reaction presentation

Hit feedback:
-> hit VFX / SFX / decal / camera shake
```

`HitStop`은 단순 presentation이 아니라 timing에 영향을 주므로 Feedback Presentation off 대상에 포함하지 않는다.

## 정책표

v1에서는 축을 과하게 늘리지 않는다.
측정상 의미가 있었던 `Perception`, `Movement`, `BT Update`, `Animation`, `Wake-up`을 핵심 축으로 두고, weapon / hit / reaction 일부는 `Combat Enablement`로 압축한다.

| Tier | CombatCritical | CombatSupport | Awareness | Background | Dormant |
| --- | --- | --- | --- | --- | --- |
| Budget | EngageCap + reactive overflow | AlertCap | Awareness budget 후보 | 나머지 일반 객체 | far / invisible / no recent interaction |
| Perception | High | Reduced | Low | Low or Budgeted | Off + Wake-up |
| Movement | High | Reduced | None | None / Idle Movement Low | None |
| BT Update | High | Reduced | Low | Low | Off / VeryLow |
| Animation | Full | Full or Reduced | Reduced | Reduced | Off |
| Combat Enablement | Full | Limited | Off | Off | Off |
| Representation | Full | Full | Reduced | Reduced | Hidden / Proxy 후보 |
| Feedback Presentation | Action + Reaction + Hit | Reaction + Hit | Minimal Reaction / Hit | Minimal Hit / Off | Off |
| Wake-up | 필요 없음 | 필요 없음 | 유지 | 유지 | range / visibility / damage / script |

`Combat Enablement`는 다음 항목을 하나로 묶은 v1 정책 축이다.

```text
Weapon actor readiness
Outgoing hit authority
Combat Hit Pipeline
Hit receive eligibility
Reaction entry
```

Combat collision / feedback 측정 결과, 해당 축들은 event 기반이며 현재 40 / 80 Enemy 조건에서 단독 최적화 축으로 강하게 밀 정도의 frame gain은 보이지 않았다.
따라서 v1에서는 세부 축으로 쪼개지 않고, 전투 참여 가능 여부를 정하는 coarse gate로 다룬다.

## 적용 예시

### CombatCritical

직접 전투 결과에 관여하는 객체다.
Engage role, HitReact, Dead, 강제 reaction, combat timing 보존이 필요한 상태가 여기에 들어간다.

```text
Movement: High
BT Update: High
Animation: Full
Combat Enablement: Full
Feedback: Action + Reaction + Hit
```

CombatCritical은 montage notify / socket timing / hit window / combat signal route가 모두 유지되어야 한다.
따라서 pose update skip, weapon readiness 제거, outgoing hit authority 제거는 금지한다.

### CombatSupport

전투 주변 보조 객체다.
Alert role, 전투 후보, 근거리 지원 객체가 여기에 들어간다.

```text
Movement: Reduced
BT Update: Reduced
Animation: Full or Reduced
Combat Enablement: Limited
Feedback: Reaction + Hit
```

CombatSupport는 언제든 CombatCritical로 승격될 수 있으므로 perception / movement / BT는 유지하되 빈도를 낮춘다.
다만 직접 공격 권한은 없으므로 outgoing hit authority와 action feedback은 기본적으로 제한한다.

### Awareness

target awareness는 있지만 combat role이 없는 객체다.

```text
Movement: None
BT Update: Low
Animation: Reduced
Combat Enablement: Off
Feedback: Minimal Reaction / Hit
```

Awareness는 전투 권한이 없으므로 Chase / Alert Spread / Attack에 참여하지 않는다.
v1에서는 Awareness에서 movement intent를 제한하는 것이 핵심 적용 후보가 된다.

### Background

target awareness가 없고 전투 / 조사 중이 아닌 상태다.

```text
Movement: None / Idle Movement Low
BT Update: Low
Animation: Reduced
Combat Enablement: Off
Feedback: Minimal Hit / Off
```

`Idle Movement`는 target awareness / combat assignment와 무관한 비전투 이동을 뜻한다.

```text
Patrol
Return Home
Scripted idle move
멀리 있지만 목적지로 이동해야 하는 상태
```

### Dormant

움직이지 않는 비활성 후보 상태다.

```text
Movement: None
BT Update: Off / VeryLow
Animation: Off
Mesh: Hidden / Proxy
Combat Enablement: Off
Feedback: Off
```

Dormant는 자기 자신이 아닌 외부 wake-up 주체가 깨워야 한다.
Dormant에서 BT MoveTo 기반 이동은 허용하지 않는다.
BT MoveTo는 PathFollowing / CharacterMovement / MovementComponent 흐름을 사용하므로 `Movement None` 정책과 충돌한다.

## Dormant Wake-up

Dormant는 Perception / BT / Movement가 꺼지거나 매우 낮은 빈도로 줄어든 상태다.
따라서 Perception Off를 적용하려면 별도 wake-up 정책이 필요하다.

후보:

```text
1. Player-centered sphere query
2. Player-centered sphere component overlap
3. Player distance + camera forward cone
4. damage / noise / scripted event
```

v1 이후 추천 구조:

```text
Dormant registry + 주기적 distance / sphere check
```

이유:

```text
매 프레임 SphereTrace보다 비용을 통제하기 쉽다.
collision channel 설정에 덜 의존한다.
80~200 Enemy 수준에서는 0.25~0.5초 간격 거리 제곱 비교가 충분히 현실적이다.
```

Wake-up manager의 책임:

```text
Dormant 해제
Perception / BT / minimal representation 복구
이후 CombatEngage assignment 흐름에 맡김
```

Wake-up manager는 combat role을 직접 부여하지 않는다.

## v1 계획

v1은 정책 전체를 한 번에 구현하지 않는다.
측정상 가장 의미가 있었고 gameplay risk가 낮은 축부터 적용한다.

### v1 적용 후보

1. Tier resolver 정리
   - `CombatRole`
   - `AIIntentState`
   - target awareness
   - distance / visibility 후보
   - `Chase` / `Investigate`는 상태명만으로 tier를 결정하지 않음

2. Movement
   - Awareness 이하 movement intent 제한
   - Idle은 필요할 때만 `Idle Movement Low`
   - Dormant는 movement 없음

3. BT Update
   - tier별 interval / precision 연결
   - EngageContext는 기본 주기 유지

4. Animation
   - Awareness / Background reduced refresh
   - combat-capable tier는 Full 유지

5. Combat Enablement
   - CombatCritical은 full 유지
   - CombatSupport는 limited
   - Awareness 이하 outgoing hit authority off
   - 세부 collision / feedback 축은 v1 직접 최적화에서 제외

### v1에서 하지 않는 것

```text
Perception Off / Active Budget
Mesh Hidden / Proxy
WeaponActor 생성 제거
Combat Hit Pipeline 제거
Feedback Presentation 제거
Dormant full implementation
```

이 항목들은 정책표에는 남기되, v1 직접 적용 대상에서 제외한다.

## 현재 이후 작업 순서

현재 단계 이후 작업은 다음 순서로 진행한다.

```text
1. Tier resolver 보정
2. ACAIController tier snapshot 추가
3. BT interval 소비 경로를 snapshot 기반으로 변경
4. Movement policy 소비 경로를 snapshot 기반으로 변경
5. Animation policy 소비 경로를 snapshot 기반으로 변경
6. State-based Runtime LOD v1 smoke 측정
7. Perception Active Budget / Wake-up 계획 수립
8. Dormant / Proxy Actor 후속 분리
```

### 1. Tier resolver 보정

`Chase` / `Investigate`를 상태명만으로 `CombatSupport`에 넣지 않는다.
resolver는 role / awareness / recent interaction을 먼저 보고 tier를 결정한다.

권장 판정 순서:

```text
Dormant candidate -> Dormant
Dead / HitReact -> CombatCritical
CombatRole Engage -> CombatCritical
CombatRole Alert -> CombatSupport
Target / LOS 있음 -> Awareness
else -> Background
```

### 2. ACAIController tier snapshot

BT / Movement / Animation이 각자 Blackboard를 다시 조합하지 않게 한다.
`ACAIController`에 현재 Runtime LOD tier를 저장하고 같은 snapshot을 소비하게 만든다.

후보 API:

```cpp
EAIRuntimeLODTier GetCurrentRuntimeLODTier() const;
void RefreshRuntimeLODTierFromBlackboard();
```

### 3. BT interval snapshot 소비

현재 BT interval helper는 Blackboard 기반 resolver를 직접 호출한다.
다음 단계에서는 controller snapshot을 읽도록 바꾼다.

```text
AIContext: fixed interval
AIIntentState: tier snapshot 기반 interval
EngageContext: fixed interval
```

### 4. Movement / Animation snapshot 소비

Movement와 Animation은 같은 tier snapshot을 읽는다.
이 단계부터 실제 Runtime LOD v1 적용이 시작된다.

```text
Movement:
-> CombatCritical High
-> CombatSupport Reduced
-> Awareness None
-> Background None / Idle Movement Low
-> Dormant None

Animation:
-> CombatCritical Full
-> CombatSupport Full or Reduced
-> Awareness Reduced
-> Background Reduced
-> Dormant Off
```

### 5. State-based Runtime LOD v1 측정

최소 측정:

```text
40 Enemy Policy Off / On
80 Enemy Policy Off / On
```

확인 기준:

```text
Engage / Alert cap 유지
Awareness가 불필요하게 이동하지 않음
Attack / HitReact / Investigate 깨짐 없음
CharacterMovement / Animation / BT Tick p95
AIContext / AIIntent / EngageContext count
```

### 6. Perception Active Budget / Wake-up

v1 smoke가 안정화된 뒤 진행한다.
Perception을 단순히 끄는 것이 아니라 active budget과 wake-up 정책으로 분리한다.

### 7. Dormant / Proxy Actor

Dormant는 v1에서 완성하지 않는다.
Wake-up 정책과 representation 전환이 필요하므로 별도 후속 브랜치로 분리한다.

## 측정 계획

1차 측정:

```text
40 Enemy
Policy Off
Policy On
```

2차 측정:

```text
80 Enemy
Policy Off
Policy On
```

고정 조건:

```text
Engage 2 / Alert 6
fixed camera
-noailogging
first 3s / last 3s trim, middle 30s used
GC event 없는 측정 우선 사용
```

확인 항목:

```text
Engage 2 / Alert 6 유지
Awareness가 움직이지 않는지
Idle / Idle Movement 정책이 의도대로 동작하는지
Attack / HitReact / Investigate 깨짐 없는지
CharacterMovement p95
Animation p95
BT Tick p95
AIContext / AIIntent / EngageContext count
Frame / Game p95
```

## 성공 기준

```text
Combat-capable 흐름이 깨지지 않는다.
Awareness / Background 계층에서 불필요한 movement work가 줄어든다.
Animation reduced가 visual break 없이 보조 효과를 낸다.
BT update precision이 기존 Assignment / AlertCap 정책과 충돌하지 않는다.
```

Frame / Game p95 개선이 작더라도, tier별 work reduction이 명확하고 gameplay smoke가 안정적이면 Runtime LOD v1 정책 기반으로 유지한다.

## 40 Enemy Smoke 결과

이번 smoke 측정은 `StatePolicyMode 1`이 직접적인 Frame / Game p95 개선을 만드는지 확인하기보다, tier snapshot 통합 구조가 gameplay를 깨지 않고 BT / Movement / Animation 소비 경로에 연결되는지 확인하는 목적이다.

측정 조건:

```text
Case: 40 Enemy / StatePolicyMode 0
ID: 20260712_211525

Case: 40 Enemy / StatePolicyMode 1
ID: 20260712_211733

Capture Duration: 약 37초
Analysis Window: first 3s / last 3s trimmed
Log State: -noailogging
PIE: F11 fullscreen
Fixed camera
GC Event: none
```

공통 CVar:

```text
Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime 1.2
Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap 2
Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap 6
Portfolio.AI.RuntimeLOD.EngageAssignmentAudit 0
Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit 0

Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode 2
Portfolio.AI.RuntimeLOD.EnemyMovementMode 0
Portfolio.AI.RuntimeLOD.EnemyAnimationMode 0
Portfolio.AI.RuntimeLOD.AnimationRefreshAudit 1

Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit 0
Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit 0
Portfolio.AI.RuntimeLOD.CanMoveDecoratorAudit 0

Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
Portfolio.AI.RuntimeLOD.DisableEnemyCombatFeedback 0
Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing 0
```

Gameplay smoke:

```text
Engage 2 유지
Alert 6 유지
나머지 Observe 또는 Idle 유지
attack montage 정상
movement 이상 없음
animation 이상 없음
GC 이벤트 없음
```

Frame / subsystem 결과:

| Metric | StatePolicyMode 0 | StatePolicyMode 1 | 해석 |
| --- | ---: | ---: | --- |
| Frame p95 | 11.9403ms | 12.0101ms | 거의 동일 |
| Game p95 | 11.3695ms | 11.3800ms | 거의 동일 |
| BT Tick p95 | 0.2187ms | 0.2123ms | 거의 동일 |
| CharacterMovement p95 | 0.5314ms | 0.5492ms | 거의 동일 |
| Animation p95 | 2.1760ms | 2.1841ms | 거의 동일 |
| AnimParallel p95 | 3.3673ms | 3.3678ms | 거의 동일 |

호출 / refresh counter:

| Counter | StatePolicyMode 0 | StatePolicyMode 1 |
| --- | ---: | ---: |
| AIContext Count | 11,840 | 11,920 |
| AIIntent Count | 2,860 | 2,900 |
| EngageContext Count | 588 | 596 |
| AnimRefresh Attempt | 110,240 | 110,520 |
| AnimRefresh Executed | 110,240 | 31,640 |
| AnimRefresh Skipped | 0 | 78,880 |

Tier / interval counter:

| Counter | StatePolicyMode 0 | StatePolicyMode 1 |
| --- | ---: | ---: |
| CombatCritical Tier | 0 | 304 |
| CombatSupport Tier | 0 | 612 |
| Awareness Tier | 0 | 1,984 |
| AIIntent Default Interval | 302 | 304 |
| AIIntent Reduced Interval | 606 | 612 |
| AIIntent Aggressive Interval | 1,952 | 1,984 |

해석:

```text
BT 호출 수가 비슷한 것은 정상이다.
AIContext는 tier 판정 입력을 만드는 producer이므로 고정 interval을 유지한다.
EngageContext는 combat timing 계층이므로 고정 interval을 유지한다.
AIIntentState는 두 케이스 모두 BTUpdateIntervalMode 2 조건이므로 interval 분포가 비슷하다.
```

Mode 1에서 의미 있는 변화는 다음이다.

```text
StateLOD tier counter가 기록된다.
BT interval helper가 controller snapshot을 소비한다.
Movement / Animation policy가 같은 controller snapshot을 소비한다.
Animation parameter refresh가 tier에 따라 줄어든다.
```

단, 줄어든 것은 animation 실행 전체가 아니다.

```text
줄어든 것:
RefreshMovementParameters()
RefreshStateParameters()

줄어들지 않은 것:
NativeUpdateAnimation 호출
AnimGraph evaluation
pose update
SkeletalMeshComponent tick
AnimationParallelEvaluation
```

따라서 `AnimRefresh Executed`는 크게 줄었지만 `Animation p95`와 `AnimParallel p95`는 거의 변하지 않았다. 이번 결과는 성능 수치 개선보다, Runtime LOD v1 정책을 같은 tier snapshot 기반으로 안전하게 확장할 수 있음을 확인한 smoke 결과로 본다.

## 최적화 이슈 마감 기준

이 브랜치가 완료되면 현재 AI Runtime LOD 최적화 이슈는 1차 결론으로 마감한다.

마감 기준:

```text
State-based Runtime LOD Policy v1 적용
40 / 80 Enemy 측정
gameplay smoke 확인
측정 결과와 정책 결론 문서화
```

이 시점의 결론은 다음과 같이 정리한다.

```text
대량 Enemy를 모두 같은 비용으로 업데이트하지 않는다.
CombatCritical / CombatSupport / Awareness / Background / Dormant tier로 나눈다.
Combat-capable 객체는 보수적으로 유지한다.
전투 권한이 없는 객체부터 movement / BT / animation 비용을 줄인다.
Perception / proxy / mesh hidden / wake-up manager는 후속 고도화로 남긴다.
```

따라서 이 브랜치의 완료는 `AI Runtime LOD v1 마감`으로 본다.
이 시점까지 “최적화라고 부를 만한 주요 비용 축”은 한 번씩 분리해서 검토했다.

남아 있는 `Perception Active Budget`, `Wake-up`, `Dormant Manager`, `Representation LOD`, `Proxy`, `120+ stress validation`은 서로 독립된 작은 최적화 축이라기보다 하나의 큰 작업인 `Dormant Runtime LOD`를 구성하는 하위 요소다.

`Dormant Runtime LOD`는 실제 gameplay 정책과 wake-up 시스템을 동반하는 본격 구현 작업이므로 현재 최적화 탐색 흐름에서는 보류한다.
재개 시에는 다음 문서를 기준으로 진행한다.

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Dormant_Runtime_LOD_Deferred_Plan.md
```

현재 브랜치에서 완료된 항목:

```text
1. Tier resolver 보정
   - Chase / Investigate를 상태명으로 분류하지 않고 role / awareness 기준으로 판정

2. ACAIController Tier Snapshot
   - CurrentRuntimeLODTier 저장
   - BT / Movement / Animation이 같은 tier 값을 소비

3. Movement / Animation 소비 경로 연결
   - Movement policy는 controller snapshot 소비
   - Animation policy는 controller snapshot 소비

4. State-based Runtime LOD v1 측정
   - 40 Enemy Policy Off / On
```

보류된 Dormant Runtime LOD 후보:

```text
1. 80 Enemy StatePolicyMode 0 / 1 확인
   - 40 Enemy smoke 통과 후 확장 확인

2. Perception Active Budget / Wake-up
   - Perception을 바로 끄지 않고 active budget / wake-up 정책으로 분리

3. Dormant / Proxy Actor
   - Dormant registry
   - range / visibility / damage / script wake-up
   - hidden / proxy representation 검토

4. Animation Budget / Pose Skip
   - combat-capable tier 제외
   - Background / Dormant 후보부터 검토

5. Distance / camera visibility 기반 LOD tuning
   - camera frustum / forward cone / distance 조합
   - tier hysteresis / minimum hold time 추가

6. 120+ Enemy stress scene
   - v1 정책이 40 / 80 이후에도 유지되는지 확인
```
