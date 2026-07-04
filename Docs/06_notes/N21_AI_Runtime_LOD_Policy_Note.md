# UE5 Portfolio - AI Runtime LOD Policy Note

## 목적

이 문서는 `P35: AI Runtime LOD 정책 정리`의 작업 기준을 정리한다.

P35는 P34에서 만든 AI performance profiling 환경을 사용해 runtime cost를 분리 측정하고, 실제 효과가 있는 축부터 Runtime LOD 정책으로 정리한다.

---

## 기준 Baseline

P35의 비교 기준은 P34 baseline CSV다.

```text
Docs/07_Profiling/AI_Performance/CSV/baseline/case_01_040_enemy_aiperf_engage.csv
```

조건:

```text
Map: MAP_AIPerf_40Enemy
Enemy: 40 placed AIPerf Enemy
State: Engage
Duration: 약 30초
Log State: -noailogging
PIE: F11 fullscreen
```

기준 지표:

```text
FrameTime p95: 12.0703ms
GameThreadTime p95: 11.9513ms
GPUTime p95: 6.9694ms
PortfolioAI_BT_UpdateAIContext p95: 0.1608ms
AIPerception p95: 0.1216ms
```

이 값은 P35의 sanity baseline이다. 40 Enemy는 asset 참조 체인과 gameplay smoke test 기준으로 사용한다.

P35의 runtime cost 비교는 scale을 올려가며 수행한다.

```text
40 Enemy
-> sanity / smoke 기준

80 Enemy
-> 측정 축별 1차 비교 기준

120 Enemy
-> 40 / 80 측정만으로 판단이 부족할 때 사용하는 optional stress extension
```

160 Enemy 이상은 정규 비교가 아니라 stress limit 참고 구간으로 둔다.

---

## 문제 정의

P33에서 BT update interval 비용은 Enemy 수 증가에 따라 커지는 것이 확인됐다.

하지만 대량 Enemy 상황에서는 BT Tick 하나보다 다음 축의 총량이 먼저 문제가 된다.

```text
Actor / Component 수
SkeletalMesh / Animation / Render 비용
Movement / Collision / crowd 비용
WeaponActor / combat collision / overlap 비용
```

따라서 P35는 update interval을 바로 조정하지 않고, Enemy runtime 구성요소 중 어떤 축이 실제로 줄일 가치가 있는지 먼저 확인한다.

---

## Runtime LOD의 범위

P35에서 말하는 Runtime LOD는 Enemy를 완전히 다른 proxy actor로 교체하는 시스템이 아니다.

P35 범위:

```text
현재 Enemy actor를 유지한다.
거리 / 중요도 / 전투 참여도 기준으로 runtime 부담을 줄일 후보를 분류한다.
비활성화 또는 축소해도 gameplay 기준이 깨지지 않는 축을 찾는다.
```

P35 범위 밖:

```text
proxy enemy actor
spawn / despawn manager
object pooling
AI streaming
save/load 상태 이전
```

Proxy / representation LOD는 별도 feature 수준의 작업이다.

---

## 측정 및 작업 방식

P35 이후 Runtime LOD 작업은 바로 LOD 시스템을 구현하지 않는다.
먼저 각 축이 실제로 비용을 줄이는지 격리 측정하고, 효과가 확인된 축만 구현 후보로 올린다.

기본 흐름:

```text
1. Baseline 고정
2. 축별 Off / Reduced 측정
3. 효과 있는 축만 LOD 정책 후보로 승격
4. 구현
5. 동일 조건으로 전후 비교
```

후보 축 전체:

```text
Object Management
-> Enemy actor 수 조율
-> Enemy spawn / despawn / pooling
-> Dormant / proxy actor 전환
-> WeaponActor 생성 여부
-> WeaponActor collision / mesh / trail 생성 여부

Simulation LOD
-> Perception 활성화 여부
-> Behavior Tree 활성화 여부
-> BT service update interval
-> Movement / Nav / PathFollowing 사용 여부
-> Movement update interval
-> Combat-capable 여부

Representation LOD
-> Character mesh visibility
-> Animation update / pose update
-> Locomotion visual detail
-> Static / idle locomotion 대체
-> Weapon mesh visibility
-> Feedback presentation
-> Shadow
-> Material complexity

Update Scheduling
-> Tick interval 조율
-> BT service interval 조율
-> Perception update budget / active cap
-> Distance-based update frequency
-> Time-sliced update
-> Dirty flag 기반 update

Asset / Rendering Policy
-> Skeletal mesh LOD
-> Material 단순화
-> Shadow off
-> Animation Budget Allocator
-> Niagara scalability
-> Low-poly proxy / impostor
```

우선 제어 축:

```text
Object Management
-> Enemy actor 수, pooling, proxy, WeaponActor 생성 여부

Simulation LOD
-> Perception, BT, Movement / Nav, Combat-capable gate

Representation LOD
-> Mesh, animation / pose, locomotion detail, weapon presentation, feedback, shadow / material

Update Scheduling
-> tick interval, BT interval, perception budget, dirty flag, time slicing

Asset / Rendering Policy
-> mesh LOD, material, shadow, Niagara scalability, proxy asset
```

전투 action / reaction / combat processing은 gameplay 의미가 큰 축이지만, 일반적으로 world에 동시에 많아도 10개 안팎의 Enemy만 직접 전투 처리에 참여한다.
또한 대부분 tick 단위가 아니라 hit / notify / overlap 같은 event 단위로 실행된다.
따라서 P35 기준에서는 전투 프로세스 자체를 세밀한 성능 제어 축으로 우선 분리하지 않는다.
전투 가능 여부는 `FullCombat`, `ReducedCombat`, `ActionOnly`, `NonCombat`, `Dormant` 단계 전환에서 coarse gate로 다룬다.

측정 scale:

```text
40 Enemy
-> smoke / sanity 확인

80 Enemy
-> 측정 축별 1차 비교

120 Enemy
-> 40 / 80 결과만으로 판단이 부족할 때 optional stress extension으로 사용
```

작업 순서:

```text
1. Object Management Cost Audit
-> Enemy count, WeaponActor, collision, actor/component 수 측정

2. Representation LOD Cost Audit
-> mesh, animation, pose, shadow, material 측정

3. Simulation LOD Cost Audit
-> perception, BT, movement/nav 측정

4. Update Scheduling Audit
-> interval, active cap, dirty flag, time slicing 측정

5. Runtime LOD Implementation v1
-> 효과가 확인된 축만 실제 runtime LOD 정책으로 구현
```

각 비교는 하나의 측정 축만 바꿔 수행한다.
측정 결과가 작거나 gameplay risk가 크면 문서화 후 후순위로 내린다.

EnemyMeshMode 비교 측정 기준:

```text
Capture Duration: 약 36초
Analysis Window: 앞 3초 / 뒤 3초 제외, 중앙 30초 사용
```

`Analysis Window`는 UE 자동 종료 기능이 아니라 CSV 분석 기준이다. CSV는 전체 36초를 기록하고, 비교값을 계산할 때 앞뒤 3초를 제외한다.

EnemyMeshMode 측정은 두 조건으로 분리한다.

```text
Gameplay Stress
-> MAP_AIPerf_40Enemy / 80 Enemy 확장 기준
-> Engage 상태에서 AI / Movement / Combat runtime 비용을 포함한다.

Render Coverage
-> MAP_AIPerf_RenderCoverage_40Enemy 또는 MAP_AIPerf_RenderCoverage_80Enemy 기준
-> 화면에 노출된 skeletal mesh render 비용을 분리한다.
```

### WeaponActor Isolation 측정 계획

EnemyMeshMode 측정으로 render cost와 animation / pose update cost를 분리했다.
다음 축은 gameplay stress 조건에 남아 있는 WeaponActor 비용을 분리한다.

측정 목적:

```text
Enemy WeaponActor 생성 / attach / socket follow 비용 확인
Weapon collision component / overlap delegate 유지 비용 확인
Trail / Niagara / feedback 경로가 포함될 때의 차이 확인
Gameplay Stress에서 남는 GameThread 비용 중 WeaponActor 축의 비중 확인
```

측정 조건:

```text
Map: MAP_AIPerf_40Enemy 또는 80 Enemy 확장 기준
State: Engage
Log State: -noailogging
PIE: F11 fullscreen
Analysis Window: 앞 3초 / 뒤 3초 제외, 중앙 30초 사용
EnemyMeshMode: 0 VisibleDefault
```

비교 방식:

```text
WeaponActor On
-> 현재 gameplay stress 기준값

Enemy WeaponActor Off
-> Enemy 쪽 WeaponActor 생성만 막는다.
-> Player weapon은 유지한다.
-> 목적은 gameplay-safe 적용 후보가 아니라 WeaponActor 비용 분리다.
```

측정 스위치:

```text
Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
-> Enemy WeaponActor 생성
-> 현재 gameplay stress 기준값

Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 1
-> Enemy WeaponActor 생성 생략
-> Equip 시 weapon type state는 유지
-> montage / action flow는 유지하되 WeaponActor / collision / trail 경로를 제거한다.
```

주의:

```text
WeaponActor Off는 sword attack의 시각 표현, socket attach, collision, trail, hit overlap 경로를 깨뜨릴 수 있다.
따라서 정규 전투 LOD 후보가 아니라 비용 분리 측정축이다.
측정 결과가 유의미하면 후속 설계에서 distant / non-combat 계층의 WeaponActor 생성 지연 또는 비활성 정책으로 검토한다.
Combat-capable Enemy는 WeaponActor를 유지한다.
```

기록 지표:

```text
FrameTime p95
GameThreadTime p95
GPUTime p95
Animation p95
TickActors p95
ActorCount/CWeaponActor
RHI/DrawCalls p95
RHI/PrimitivesDrawn p95
gameplay smoke result
```

Render Coverage 통제 조건:

```text
BP_AIPerf_RenderCoverage_Enemy 사용
fixed render coverage camera 사용
Idle animation only
Auto Possess AI off
AIController / BehaviorTree / Perception 미실행
WeaponActor 미생성
Movement / PathFollowing 없음
Combat / Guard / Reaction 진입 없음
Niagara / Trail / Feedback 없음
Player mesh / weapon 화면 미노출
Collision debug draw off
```

측정 과정에서 확인한 문제와 분리:

```text
초기 Gameplay Stress 측정에서는 EnemyMeshMode 1이 GPU / DrawCalls / Primitives를 줄였지만 Frame / GameThread p95는 크게 회복되지 않았다.
이 결과만으로 mesh render 비용이 작다고 판단하기에는 카메라에 잡힌 Enemy 수, 전투 상태, AI / Movement / Combat runtime 비용이 함께 섞여 있었다.
따라서 render 비용을 따로 보기 위해 Render Coverage 조건을 분리했다.
Render Coverage에서는 fixed camera, camera-only pawn, Auto Possess AI off, WeaponActor 미생성, BT / Perception 미실행, Idle animation only 조건을 사용했다.
이 분리 후 40 / 80 Enemy 모두에서 EnemyMeshMode 1이 GPU / DrawCalls / Primitives와 Frame p95를 함께 낮추는 것을 확인했다.
```

현재 측정 스위치:

```text
Portfolio.AI.RuntimeLOD.EnemyMeshMode 0
-> VisibleDefault
-> ACEnemy SkeletalMesh visibility on
-> 기존 pose update 설정 복구

Portfolio.AI.RuntimeLOD.EnemyMeshMode 1
-> HiddenKeepPose
-> ACEnemy SkeletalMesh visibility off
-> pose / bone / socket update 유지

Portfolio.AI.RuntimeLOD.EnemyMeshMode 2
-> HiddenAllowPoseSkip
-> ACEnemy SkeletalMesh visibility off
-> visibility 기반 pose skip 허용

-> PIE 중 변경할 수 있으며 다음 Tick에서 반영된다.
```

Mesh visibility 비교:

```text
Baseline
-> Portfolio.AI.RuntimeLOD.EnemyMeshMode 0

Mesh hidden / pose 유지
-> Portfolio.AI.RuntimeLOD.EnemyMeshMode 1

Mesh hidden / pose skip 허용
-> Portfolio.AI.RuntimeLOD.EnemyMeshMode 2
-> gameplay unsafe 관찰로 정규 성능 측정 제외
```

Mesh visibility off 관찰:

```text
SetHiddenInGame(true) / SetVisibility(false)는 mesh render만 끄는 비교가 아닐 수 있다.
SkeletalMeshComponent가 hidden 상태가 되면 VisibilityBasedAnimTickOption 설정에 따라 pose / bone refresh가 줄거나 멈출 수 있다.
pose 갱신이 멈추면 socket transform도 마지막 pose에 머무를 수 있다.
WeaponActor가 hand / holster socket에 attach된 상태라면 검이 hidden 직전 위치에 고정된 것처럼 보일 수 있다.
```

해석 기준:

```text
Mode 1: HiddenKeepPose
-> mesh는 숨기지만 pose / bone / socket update를 유지한다.
-> render 비용 분리 측정에 가깝다.

Mode 2: HiddenAllowPoseSkip
-> mesh hidden + visibility 기반 pose skip을 허용한다.
-> render cost + pose / socket update cost가 함께 줄어드는 극단 비교일 수 있다.
-> WeaponActor socket follow와 animation-driven 전투 흐름을 깨뜨리는 것으로 관찰되어 정규 성능 측정에서 제외한다.
```

Mode 2 적용 범위:

```text
Mode 2는 전투 중인 Enemy에 바로 적용할 Runtime LOD 후보가 아니다.
Mode 2는 먼 거리 / 비전투 / 비용 분리 조건에서 animation / pose update 비용을 확인하기 위한 측정축이다.
PIE 실행 중 Mode 0 또는 Mode 1에서 Mode 2로 전환하면 전환 시점의 pose가 남아 측정이 오염될 수 있다.
따라서 Mode 2 비용 측정은 PIE 실행 전 CVar를 Mode 2로 고정한 상태에서 수행한다.
```

Runtime LOD 설계 해석:

```text
MoveTo 기반 이동은 actor transform을 Movement / PathFollowing이 갱신하므로 pose update skip과 별개로 계속 움직일 수 있다.
Montage 기반 공격은 SkeletalMesh pose update와 notify / socket timing에 의존하므로 Mode 2 상태에서 멈추거나 깨질 수 있다.
따라서 Mode 2는 combat-capable LOD가 아니라 distant moving 또는 dormant 계층의 후보로만 다룬다.
Combat 진입 전에는 Mode 0 또는 최소 Mode 1로 복귀한 뒤 action / montage를 시작해야 한다.
```

기록 기준:

```text
FrameTime p95
GameThreadTime p95
GPUTime p95
RenderThreadTime p95
AIPerception p95
PortfolioAI scope p95
ActorCount
Tick count
gameplay smoke result
WeaponActor socket follow result
```

---

## Runtime LOD 단계 후보

전투 관련 Runtime LOD는 다음 5단계로 정리한다.

```text
FullCombat
ReducedCombat
ActionOnly
NonCombat
Dormant
```

### FullCombat

```text
WeaponActor 유지
Combat Action 유지
Combat Processing 유지
Feedback 유지
현재 직접 교전 중인 핵심 Enemy 또는 정예 Enemy
```

### ReducedCombat

```text
WeaponActor 유지
Combat Action 유지
Combat Processing 유지
Feedback 축소 또는 제거
전투에는 참여하지만 시각 / 감각 피드백 우선순위가 낮은 Enemy
```

### ActionOnly

```text
Combat Action 유지
Combat Processing 제거
Feedback 제거
WeaponActor는 선택
가까운 거리에서 공격 모션 / 위협 표현은 필요하지만 실제 hit 처리는 제한할 Enemy
```

### NonCombat

```text
Combat Action 제거
Combat Processing 제거
Feedback 제거
WeaponActor 제거
이동 / 경계 / 시선 / 위치 유지 정도만 수행하는 Enemy
```

### Dormant

```text
Combat Action 제거
Combat Processing 제거
Feedback 제거
WeaponActor 제거
BT / Perception / Movement / Anim update 최소화 또는 정지
proxy / pooling / representation 전환 후보
```

측정축과 Runtime LOD 단계의 관계:

```text
WeaponActor Presence
-> LOD 단계 자체가 아니라 ActionOnly 이상에서 WeaponActor를 유지할지 판단하기 위한 보조 측정축

Combat Action
-> ActionOnly 이상에서 필요한 비용

Combat Processing
-> ReducedCombat 이상에서 필요한 비용

Feedback
-> FullCombat에서 필요한 비용
```

### Simulation LOD / Representation LOD 분리

Runtime LOD는 하나의 축으로만 처리하지 않는다.
전투 참여 능력과 화면 표현 비용은 서로 다른 결정을 요구하므로 다음 두 계층으로 분리한다.

```text
Simulation LOD
-> Enemy가 어떤 gameplay 기능을 수행할지 결정한다.
-> FullCombat / ReducedCombat / ActionOnly / NonCombat / Dormant
-> Combat Action, Combat Processing, Movement decision / execution, Perception, BT update가 여기에 속한다.

Representation LOD
-> Enemy를 화면에 어떻게 표현할지 결정한다.
-> Full / Reduced / Minimal / Hidden / Proxy
-> Mesh visibility, animation update, pose update, locomotion visual detail, shadow, material, proxy representation이 여기에 속한다.
```

Enemy mesh 측정에서 사용한 `EnemyMeshMode`는 Representation LOD 측정축이다.
Mode 1은 mesh render 비용을 줄이는 축이고, Mode 2는 hidden 상태에서 animation / pose update 비용을 분리하는 축이다.
반대로 `FullCombat`, `ReducedCombat`, `ActionOnly`, `NonCombat`, `Dormant`는 Simulation LOD 단계다.

LOD 제어 축은 후보 전체와 실제 우선 제어 축을 구분한다.
전투 action / reaction / combat processing은 gameplay 의미가 큰 축이지만, 일반적으로 world에 동시에 많아도 10개 안팎의 Enemy만 직접 전투 처리에 참여한다.
또한 대부분 tick 단위가 아니라 hit / notify / overlap 같은 event 단위로 실행된다.
따라서 P35 기준에서는 전투 프로세스 자체를 세밀한 성능 제어 축으로 우선 분리하지 않는다.
전투 가능 여부는 `FullCombat`, `ReducedCombat`, `ActionOnly`, `NonCombat`, `Dormant` 단계 전환에서 coarse gate로 다룬다.

우선 제어 축:

```text
Simulation LOD
-> movement / nav / update 허용 여부
-> perception / BT update 허용 여부
-> combat-capable 여부는 단계 전환으로만 coarse control

Representation LOD
-> character mesh / animation update / pose update
-> locomotion visual detail
-> weapon presentation
-> feedback presentation
-> shadow / material / proxy
```

Representation LOD는 다음 단계 후보로 정리한다.

```text
Full
-> mesh visible
-> full animation / pose update
-> locomotion visual detail 유지
-> weapon visibility / shadow 유지
-> full material / shadow 유지

Reduced
-> mesh visible
-> animation / pose update 유지
-> locomotion visual detail 일부 축소
-> shadow / material / weapon presentation 축소 후보

Minimal
-> mesh visible 또는 simplified representation
-> animation detail 축소
-> additive / foot IK / turn detail / expensive material 축소 후보

Hidden
-> mesh hidden
-> pose update 유지 여부를 상황별로 판단
-> combat-capable 단계에서는 notify / socket timing 때문에 pose update skip 금지

Proxy
-> 원본 Enemy actor 표현을 proxy representation으로 대체
-> spawn / despawn / pooling / lightweight actor 정책과 함께 후속 feature에서 검토
```

축별 변화는 다음 기준으로 압축한다.

| Axis | Full | Reduced | Minimal | Hidden | Proxy |
| --- | --- | --- | --- | --- | --- |
| Character render | 원본 mesh 표시 | 원본 mesh 표시 | 단순 mesh / LOD 후보 | mesh hidden | proxy / impostor 후보 |
| Animation / pose | full update | pose 유지 | update rate / detail 축소 후보 | combat-capable이면 유지, non-combat이면 skip 후보 | 원본 pose update 없음 |
| Locomotion visual detail | 전체 유지 | 일부 축소 | additive / foot IK / turn detail 축소 | 없음 또는 최소 | proxy 전용 |
| Weapon presentation | weapon 표시 / socket follow / shadow 유지 | shadow / material 축소 후보 | 표시 축소 / 숨김 후보 | 숨김 후보 | 원본 weapon actor 없음 |
| Feedback presentation | Niagara / trail / sound / camera shake 유지 | 일부 축소 | 대부분 제거 후보 | 없음 | 없음 |
| Shadow / material | full | 일부 축소 | simple / off 후보 | off | proxy 기준 |

Representation 단계 요약:

```text
Full
-> 직접 교전 또는 주요 시각 대상
-> 표현 품질 유지

Reduced
-> 전투 또는 근거리 상태는 유지
-> 표현 비용 일부 축소
-> mesh / pose / montage timing 유지

Minimal
-> 화면에는 남김
-> 세부 표현 품질 축소
-> animation detail, material, shadow, feedback presentation 우선 축소

Hidden
-> mesh 숨김
-> combat-capable 단계에서는 notify / socket timing 때문에 pose update skip 금지
-> non-combat / dormant 조건에서만 pose update skip 후보

Proxy
-> 원본 Enemy actor 표현을 lightweight representation으로 대체
-> spawn / despawn / pooling / proxy actor 정책과 함께 후속 feature에서 검토
```

경계 정책:

```text
WeaponActor
-> spawn / existence: Simulation LOD와 Action dependency
-> visibility / shadow: Representation LOD
-> collision / overlap / hit context: Combat Processing
-> trail / Niagara / camera shake / hit feedback: Feedback presentation

Movement / Locomotion
-> movement decision / execution: Simulation LOD
-> locomotion visual detail: Representation LOD

Feedback
-> combat result / gameplay outcome: Simulation LOD 또는 Combat Processing
-> feedback presentation: Representation LOD
-> runtime effect feedback: timing에 영향을 주므로 별도 주의 항목

Action / Montage
-> action decision / action state: Simulation LOD
-> action execution policy: Simulation LOD
-> montage playback / pose output: Representation LOD
-> montage notify timing: 현재 Phase 1에서는 Simulation dependency
```

따라서 `WeaponActor Presence`는 독립 LOD 단계가 아니라, `ActionOnly` 이상에서 WeaponActor를 유지해야 하는지 판단하기 위한 보조 측정축으로 둔다.
전투 처리와 피드백이 제거되는 단계에서도 무기 실루엣이 필요한지, 또는 무기 actor 자체를 지연 생성 / 제거할 수 있는지 분리해 확인한다.

다만 현재 Phase 1에서는 montage notify가 combat timing source 역할을 한다.
따라서 combat-capable LOD에서는 montage playback / notify route를 유지한다.

```text
FullCombat
-> montage playback 유지
-> notify route 유지

ReducedCombat
-> montage playback 유지
-> notify route 유지
-> feedback presentation만 축소 가능

ActionOnly
-> action state 유지
-> montage playback 유지 후보
-> notify route는 유지하되 Combat Processing으로 연결하지 않는다.

NonCombat
-> combat action 제거
-> combat montage playback 제거 가능

Dormant
-> combat action 제거
-> montage 제거 또는 pose update 최소화 가능
```

Montage 제거 또는 pose update skip은 `NonCombat` / `Dormant` 단계에서만 허용한다.
Combat timing을 montage notify에서 분리하는 작업은 후속 `Action Timeline` 개선 후보로 둔다.

Combat-capable 비용은 action / reaction / combat processing을 각각 끄는 방식으로 측정하지 않는다.
필요하면 coarse measurement로만 확인한다.

```text
Baseline
-> Perception / BT / Movement 유지
-> attack action / weapon collision / hit processing / feedback 허용

Combat-capable Off
-> Perception / BT / Movement 유지
-> attack action 진입 거부
-> weapon collision window 차단
-> hit processing 차단
-> feedback presentation 차단
```

후속 `Action Timeline` 개선 방향:

```text
현재:
Montage playback
-> AnimNotify
-> hit window / cue / command timing

개선 후보:
Action Timeline
-> simulation timing source
-> montage는 representation / sync 대상
-> AnimNotify는 optional sync marker 또는 authoring helper
```

부하가 높아졌을 때의 저하 순서는 다음 기준을 따른다.

```text
1. 중요도가 낮은 Enemy부터 선택한다.
2. 같은 중요도라면 낮은 Simulation LOD 단계의 Enemy부터 선택한다.
3. 먼저 Representation LOD를 낮춘다.
4. 그래도 예산을 넘으면 Simulation LOD를 낮춘다.
5. 현재 직접 교전 중인 FullCombat Enemy는 마지막까지 유지한다.
```

---

## 현재 측정 상태

### 완료된 측정축

```text
Representation / EnemyMeshMode
-> Gameplay Stress 조건에서 mesh visibility 비용을 확인했다.
-> Render Coverage 조건에서 mesh render cost와 hidden 상태의 animation / pose update cost를 분리했다.
-> EnemyMeshMode 2는 비용 분리에는 유효하지만 combat-capable 단계에서는 unsafe로 분류한다.

Object Management / WeaponActor Presence
-> 40 / 80 Enemy 조건에서 Enemy WeaponActor 생성 여부를 분리했다.
-> WeaponActor 제거는 ActorCount, SkeletalMeshComponent tick, DrawCalls, Frame / GameThread p95를 낮추는 유효 축으로 확인됐다.
-> 실제 적용은 combat-capable 단계와 weapon dependency를 함께 고려해야 한다.
```

### 다음 측정축

```text
Simulation LOD / AI Perception
-> Perception 비용과 감지 지연을 분리한다.
-> 대량 Enemy stress에서 관찰한 perception 지연을 active perception 수 / 거리 / 중요도 제어 후보와 연결해 검토한다.
-> 측정 계획은 Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Perception_Runtime_LOD_Measurements.md에 기록한다.

Simulation LOD / BehaviorTree Update
-> BT 실행 / service update 비용을 분리한다.
-> Update interval / dirty flag / time slicing 후보는 측정 결과를 보고 후속 구현 여부를 결정한다.

Simulation LOD / Movement / Nav
-> Movement decision / PathFollowing / CharacterMovement 비용을 분리한다.
-> 위치와 path를 바꾸는 gameplay 축이므로 Representation LOD와 분리해 판단한다.
```

P35에서는 측정축과 적용 가능성을 정리한다.
실제 Perception active cap, BT interval LOD, Movement LOD, proxy / pooling 구현은 후속 PR에서 다룬다.

---

예상 구조:

```cpp
enum class EEnemySimulationLOD
{
	FullCombat,
	ReducedCombat,
	ActionOnly,
	NonCombat,
	Dormant,
};

enum class EEnemyRepresentationLOD
{
	Full,
	Reduced,
	Minimal,
	Hidden,
	Proxy,
};

struct FEnemyRuntimeLODState
{
	EEnemySimulationLOD SimulationLOD = EEnemySimulationLOD::FullCombat;
	EEnemyRepresentationLOD RepresentationLOD = EEnemyRepresentationLOD::Full;
};
```

Capability로 풀면 다음 형태가 된다.

```cpp
struct FEnemyRuntimeLODCapability
{
	bool bAllowCombatAction = true;
	bool bAllowCombatProcessing = true;
	bool bAllowFeedback = true;
	bool bAllowMovement = true;
	bool bAllowPerception = true;

	bool bShowMesh = true;
	bool bAllowFullAnimation = true;
	bool bAllowPoseUpdate = true;
	bool bCastShadow = true;
	bool bShowWeapon = true;
	bool bUseProxy = false;
};
```

---

## 구현 후보 판정 기준

### P35 구현 후보

```text
baseline 대비 p95 차이가 명확하다.
gameplay smoke test가 통과한다.
전투 로직 변경 없이 적용할 수 있다.
asset 오염 없이 profiling 환경에서 재현된다.
```

### 후순위 후보

```text
수치 차이가 작다.
특정 대량 Enemy 조건에서만 의미가 있다.
구현 대비 효과가 낮다.
```

### 제외 후보

```text
전투 로직을 바꾼다.
hit / guard / parry 흐름을 깨뜨린다.
asset 또는 상태 전환 리스크가 크다.
proxy / streaming / pooling 수준의 구조가 필요하다.
```

---

## 완료 조건

```text
P34 baseline 기준이 문서화되어 있다.
Runtime cost 분리 측정 항목이 정리되어 있다.
각 항목의 효과 / 부작용 / 구현 여부가 분류되어 있다.
Runtime LOD 단계 기준이 정리되어 있다.
P36 perception LOD / P37 update LOD와 범위가 분리되어 있다.
```

측정 기록:

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/Enemy_Mesh_Runtime_LOD_Measurements.md
```
