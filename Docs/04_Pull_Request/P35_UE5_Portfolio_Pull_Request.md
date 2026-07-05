# UE5 Portfolio Pull Request

## 제목

**P35: AI Runtime LOD 정책 정리**

## 날짜

**2026.07.02**

## 상태

- [x] 작업 방향 수립
- [ ] 측정 / 코드 / 문서 반영
- [ ] 검증 완료

---

## 브랜치

- `refactor/ai-runtime-lod-policy`

---

## 주요 커밋 흐름

```text
docs(ai): plan runtime LOD policy
```

---

## 요약

이번 PR은 P34에서 분리한 AI performance profiling 환경을 기준으로, 대량 Enemy 상황에서 runtime cost를 줄일 수 있는 축을 분리 측정하고 Runtime LOD 정책을 정리한다.

P35는 바로 proxy enemy나 spawn / despawn manager를 구현하지 않는다. 먼저 40 / 80 Enemy scale에서 WeaponActor, collision, mesh, animation, movement, component tick 같은 runtime 요소가 실제로 의미 있는 비용 차이를 만드는지 확인한다.

---

## 기준 Baseline

P34에서 고정한 기준 CSV를 P35 비교 기준으로 사용한다.

```text
Docs/07_Profiling/AI_Performance/CSV/baseline/case_01_040_enemy_aiperf_engage.csv
```

측정 조건:

```text
Map: MAP_AIPerf_40Enemy
Enemy: 40 placed AIPerf Enemy
State: Engage
Duration: 약 30초
Log State: -noailogging
PIE: F11 fullscreen
```

기준 지표:

| Metric | Avg | p95 | Max |
| --- | ---: | ---: | ---: |
| FrameTime | 11.1087ms | 12.0703ms | 33.7046ms |
| GameThreadTime | 11.0671ms | 11.9513ms | 250.7286ms |
| GPUTime | 6.0309ms | 6.9694ms | 7.6379ms |
| RenderThreadTime | 0.0573ms | 0.0635ms | 0.6642ms |
| PortfolioAI_BT_UpdateAIContext | 0.1195ms | 0.1608ms | 0.3891ms |
| AIPerception | 0.0897ms | 0.1216ms | 0.2510ms |

이 기준값은 sanity baseline이다.

```text
40 Enemy
-> asset 참조 체인 / gameplay smoke / 측정 절차 정상성 확인

80 Enemy
-> 측정 축별 1차 비교 기준
-> 40 Enemy와 같은 패턴이 반복되면 해당 축의 1차 판단 종료

120 Enemy
-> 40 / 80 측정만으로 판단이 부족할 때 사용하는 optional stress extension
```

---

## 작업 범위

### 1. Runtime 비용 축 분리 측정

P35 이후 Runtime LOD 작업은 다음 흐름으로 진행한다.

```text
1. Baseline 고정
2. 축별 Off / Reduced 측정
3. 효과 있는 축만 LOD 정책 후보로 승격
4. 구현
5. 동일 조건으로 전후 비교
```

측정 / 구현 후보 축:

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

전투 action / reaction / combat processing은 gameplay 의미가 크지만, 동시에 직접 전투 처리에 참여하는 Enemy 수가 제한적이고 대부분 event 단위로 실행된다.
따라서 P35에서는 세부 성능 제어 축으로 우선 분리하지 않고, combat-capable 여부를 Runtime LOD 단계 전환의 coarse gate로 다룬다.

`40 / 80 Enemy`는 측정 축이 아니라 scale 단계다.
`120 Enemy`는 기본 측정 scale이 아니라 필요할 때만 사용하는 stress extension이다.

각 비교는 하나의 측정 축만 바꿔 수행한다.

측정 / 작업 순서:

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

측정 scale:

```text
40 Enemy
-> smoke / sanity 확인

80 Enemy
-> 측정 축별 1차 비교

120 Enemy
-> 40 / 80 결과만으로 판단이 부족할 때만 선택적으로 사용
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

Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 0
-> Enemy WeaponActor 생성 유지

Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor 1
-> Enemy WeaponActor 생성 생략
-> Player WeaponActor는 유지

Portfolio.AI.RuntimeLOD.DisableEnemyPerception 0
-> Enemy Perception 활성

Portfolio.AI.RuntimeLOD.DisableEnemyPerception 1
-> Enemy Perception 비활성
-> Sight sense 비활성화, delegate binding 생략, BuildPerceptionContext NoData 반환
```

현재까지 완료된 측정축:

```text
Representation / EnemyMeshMode
-> Gameplay Stress 조건에서 Enemy mesh visibility와 pose update 비용을 확인했다.
-> Render Coverage 조건에서 mesh render cost와 hidden 상태의 animation / pose update cost를 분리했다.
-> EnemyMeshMode 2는 비용 분리에는 유효하지만 combat-capable LOD에서는 unsafe로 분류했다.

WeaponActor Isolation
-> Gameplay Stress 조건에서 Enemy WeaponActor 생성 / attach / socket follow / collision / trail 비용을 분리한다.
-> Enemy WeaponActor만 비활성화하고 Player weapon은 유지한다.
-> 목적은 gameplay-safe LOD 적용이 아니라 비용 분리 측정이다.
-> `Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor` 스위치로 측정한다.
-> 유의미한 차이가 확인되면 distant / non-combat 계층에서 WeaponActor 생성 지연 또는 비활성 정책으로 후속 검토한다.
```

다음 측정축:

```text
Simulation LOD / AI Perception
-> Gameplay Stress 조건에서 Perception 비용과 감지 지연을 분리한다.
-> 160~200 Enemy stress에서 관찰한 perception 지연과 연결해 active perception 수 / 거리 / 중요도 제어 후보를 검토한다.
-> 실제 Perception active cap 구현은 후속 PR로 분리하고, P35에서는 측정축과 적용 가능성만 정리한다.
-> 측정 계획은 Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Perception_Runtime_LOD_Measurements.md에 기록한다.

Simulation LOD / BehaviorTree Update
-> BT 실행 / service update 비용을 분리한다.
-> Update interval / dirty flag / time slicing 후보는 측정 결과를 보고 후속 구현 여부를 결정한다.

Simulation LOD / Movement / Nav
-> Movement decision / PathFollowing / CharacterMovement 비용을 분리한다.
-> 위치와 path를 바꾸는 gameplay 축이므로 Representation LOD와 분리해 판단한다.
```

### 2. Runtime LOD 단계 정의

전투 관련 Runtime LOD 단계:

```text
FullCombat
-> WeaponActor / Combat Action / Combat Processing / Feedback 유지
-> 현재 직접 교전 중인 핵심 Enemy 또는 정예 Enemy

ReducedCombat
-> WeaponActor / Combat Action / Combat Processing 유지
-> Feedback 축소 또는 제거
-> 전투에는 참여하지만 피드백 우선순위가 낮은 Enemy

ActionOnly
-> Combat Action 유지
-> Combat Processing / Feedback 제거
-> WeaponActor는 선택
-> 가까운 거리에서 공격 모션 / 위협 표현은 필요하지만 실제 hit 처리는 제한할 Enemy

NonCombat
-> Combat Action / Combat Processing / Feedback 제거
-> WeaponActor 제거
-> 이동 / 경계 / 시선 / 위치 유지 정도만 수행하는 Enemy

Dormant
-> Combat Action / Combat Processing / Feedback / WeaponActor 제거
-> BT / Perception / Movement / Anim update 최소화 또는 정지
-> proxy / pooling / representation 전환 후보
```

`WeaponActor Presence`는 독립 LOD 단계가 아니라 `ActionOnly` 이상에서 WeaponActor를 유지할지 판단하기 위한 보조 측정축으로 둔다.

Runtime LOD는 두 계층으로 분리한다.

```text
Simulation LOD
-> FullCombat / ReducedCombat / ActionOnly / NonCombat / Dormant
-> Combat Action, Combat Processing, Movement decision / execution, Perception, BT update 결정

Representation LOD
-> Full / Reduced / Minimal / Hidden / Proxy
-> Mesh visibility, animation update, pose update, locomotion visual detail, shadow, material, proxy representation 결정
```

`EnemyMeshMode`는 Representation LOD 측정축이고, 전투 단계 5종은 Simulation LOD 단계다.
부하가 높아지면 낮은 중요도 / 낮은 Simulation LOD Enemy부터 Representation LOD를 먼저 낮추고, 그래도 예산을 넘으면 Simulation LOD를 낮춘다.

P35 기준 우선 제어 축:

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

전투 action / reaction / combat processing은 gameplay 의미가 크지만, 동시에 직접 전투 처리에 참여하는 Enemy 수가 제한적이고 대부분 event 단위로 실행된다.
따라서 P35에서는 세부 성능 제어 축으로 우선 분리하지 않고, `FullCombat` / `ReducedCombat` / `ActionOnly` / `NonCombat` / `Dormant` 단계 전환에서 coarse gate로 다룬다.

Representation LOD 단계 후보:

| Axis                  | Full                  | Reduced                 | Minimal                             | Hidden              | Proxy              |
| --------------------- | --------------------- | ----------------------- | ----------------------------------- | ------------------- | ------------------ |
| Character render      | 원본 표시                 | 원본 표시                   | 단순 mesh / LOD 후보                    | 숨김                  | proxy 표현           |
| Animation / Pose      | full update           | pose 유지                 | detail / rate 축소 후보                 | combat-capable이면 유지 | 원본 update 없음       |
| Locomotion detail     | 전체 유지                 | 일부 축소                   | additive / foot IK / turn detail 축소 | 없음 또는 최소            | proxy 전용           |
| Weapon presentation   | 표시 / socket follow 유지 | shadow / material 축소 후보 | 표시 축소 / 숨김 후보                       | 숨김 후보               | 원본 weapon actor 없음 |
| Feedback presentation | full                  | 일부 축소                   | 대부분 제거 후보                           | 없음                  | 없음                 |
| Shadow / Material     | full                  | 일부 축소                   | simple / off 후보                     | off                 | proxy 기준           |

Representation 단계 요약:

```text
Full
-> 표현 품질 유지

Reduced
-> 근거리 표현 일부 축소

Minimal
-> 세부 표현 축소

Hidden
-> mesh 숨김
-> combat-capable 단계에서는 pose update skip 금지

Proxy
-> 원본 표현을 lightweight representation으로 대체
```

경계 정책:

```text
WeaponActor
-> spawn / existence: Simulation LOD와 Action dependency
-> visibility / shadow: Representation LOD
-> collision / hit context: Combat Processing
-> trail / Niagara / camera shake: Feedback presentation

Movement / Locomotion
-> movement decision / execution: Simulation LOD
-> locomotion visual detail: Representation LOD

Feedback
-> hit / guard / parry / stagger result: Simulation 또는 Combat Processing
-> Niagara / trail / sound / camera shake / decal: Representation LOD
-> hit stop: timing에 영향을 주므로 별도 주의 항목

Action / Montage
-> action state / execution policy: Simulation LOD
-> montage pose output: Representation LOD
-> montage notify timing: 현재 Phase 1에서는 Simulation dependency
```

현재 Phase 1에서는 `AnimNotify`가 combat timing source 역할을 한다.
따라서 combat-capable LOD에서는 montage playback / notify route를 유지한다.
Montage 제거 또는 pose update skip은 `NonCombat` / `Dormant` 단계에서만 허용한다.
Combat timing을 montage에서 분리하는 작업은 후속 `Action Timeline` 개선 후보로 둔다.

Combat-capable 비용은 action / reaction / combat processing을 각각 끄는 방식으로 측정하지 않는다.
필요하면 Perception / BT / Movement는 유지한 상태에서 attack action, weapon collision, hit processing, feedback 진입을 막는 on/off coarse measurement로만 확인한다.

### 3. 적용 후보 분류

분류 기준:

```text
효과 큼
-> P35 구현 후보

효과 작음
-> 문서화 후 후순위

부작용 큼
-> 적용 제외 또는 후속 feature 후보
```

---

## 제외 범위

```text
proxy enemy system
spawn / despawn manager
AI Perception active cap
BT Service interval LOD
BehaviorTree 구조 변경
전투 로직 변경
```

P35에서는 위 항목을 실제 gameplay 정책으로 구현하지 않는다.
다만 Perception / BT / Movement는 Simulation LOD 측정축으로 남겨두고, 효과가 확인된 항목만 후속 PR에서 구현한다.

---

## 검증 계획

정적 확인:

```text
git status --short
git diff --check
```

측정 조건:

```text
Unreal Editor 실행 옵션: -noailogging
Map: MAP_AIPerf_40Enemy 또는 P35에서 파생한 scale variant
PIE: F11 fullscreen
Stats: stat unit / stat game / stat ai
CSV: csvprofile start / csvprofile stop
Capture Duration: 약 36초
Analysis Window: 앞 3초 / 뒤 3초 제외, 중앙 30초 사용
```

`Analysis Window`는 UE 자동 종료 기능이 아니라 CSV 분석 기준이다. CSV는 전체 36초를 기록하고, 비교값을 계산할 때 앞뒤 3초를 제외한다.

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

따라서 `EnemyMeshMode 1`은 render 비용 분리 측정에 가깝고, `EnemyMeshMode 2`는 `skeletal mesh visibility / pose update 영향 포함` 극단 비교로 해석한다.
Mode 2는 WeaponActor socket follow와 animation-driven 전투 흐름을 깨뜨리는 것으로 관찰되어 정규 성능 측정에서 제외한다.

Mode 2 추가 해석:

```text
Mode 2는 전투 중 Enemy에 바로 적용할 gameplay-safe Runtime LOD 후보가 아니다.
Mode 2는 Render Coverage 조건에서 animation / pose update 비용을 분리하기 위한 측정축으로만 사용한다.
PIE 실행 중 Mode 0 또는 Mode 1에서 Mode 2로 전환하면 전환 시점의 pose / socket state가 남아 결과가 오염될 수 있다.
따라서 Mode 2 측정은 PIE 실행 전 CVar를 Mode 2로 고정한 뒤 수행한다.
```

측정 조건 분리:

```text
Gameplay Stress
-> MAP_AIPerf_40Enemy / 80 Enemy 확장 기준
-> Engage 상태에서 AI / Movement / Combat runtime 비용을 포함한다.

Render Coverage
-> MAP_AIPerf_RenderCoverage_40Enemy / MAP_AIPerf_RenderCoverage_80Enemy 기준
-> 화면에 노출된 skeletal mesh render 비용을 분리한다.
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

비교 기록:

```text
40 / 80 / 120 scale별 FrameTime / GameThreadTime / GPUTime / AIPerception / PortfolioAI scope p95 비교
actor count / component tick 변화 확인
gameplay smoke test로 attack / hit / guard / parry 흐름 유지 확인
WeaponActor socket follow 유지 여부 확인
```

현재 측정 진행:

```text
40 Enemy / EnemyMeshMode 0 측정 완료
40 Enemy / EnemyMeshMode 1 측정 완료
40 Enemy / EnemyMeshMode 2 gameplay unsafe 관찰로 정규 측정 제외
80 Enemy / EnemyMeshMode 0 측정 완료
80 Enemy / EnemyMeshMode 1 측정 완료
40 Enemy / RenderCoverage / EnemyMeshMode 0 측정 완료
40 Enemy / RenderCoverage / EnemyMeshMode 1 측정 완료
40 Enemy / RenderCoverage / EnemyMeshMode 2 측정 완료
80 Enemy / RenderCoverage / EnemyMeshMode 0 측정 완료
80 Enemy / RenderCoverage / EnemyMeshMode 1 측정 완료
80 Enemy / RenderCoverage / EnemyMeshMode 2 측정 완료
40 Enemy / WeaponActor Isolation / DisableEnemyWeaponActor 0 측정 완료
40 Enemy / WeaponActor Isolation / DisableEnemyWeaponActor 1 측정 완료
80 Enemy / WeaponActor Isolation / DisableEnemyWeaponActor 0 측정 완료
80 Enemy / WeaponActor Isolation / DisableEnemyWeaponActor 1 측정 완료
```

측정 결과:

### Enemy Mesh Runtime LOD

| Case | Enemy | Mode                  | 시간     | Frame p95 |  Game p95 |  GPU p95 | Animation p95 | BT Tick p95 | AIPerception p95 | DrawCalls p95 | Primitives p95 | 판정    | 메모                                                                          |
| ---- | ----: | --------------------- | ------ | --------: | --------: | -------: | ----------: | ---------------: | ------------: | -------------: | ----- | --------------------------------------------------------------------------- |
| M00  |    40 | 0 VisibleDefault      | 37.72s | 12.6880ms | 12.7354ms | 8.4217ms |      2.0121ms |    0.3067ms |         0.1252ms |           572 |      4,181,001 | 기준    | 40 Enemy mesh visible 기준이다.                                                 |
| M01  |    40 | 1 HiddenKeepPose      | 37.98s | 12.3922ms | 12.4150ms | 7.3382ms |      1.9191ms |    0.3057ms |         0.1233ms |         377.7 |        377,250 | 제한 효과 | render 비용은 줄었지만 Frame / GameThread 개선은 작다. WeaponActor socket follow는 유지됐다. |
| M02  |    40 | 2 HiddenAllowPoseSkip | -      |         - |         - |        - |             - |           - |                - |             - |              - | 제외    | WeaponActor socket follow와 animation-driven 전투 흐름을 깨뜨려 정규 측정에서 제외한다.        |
| M03  |    80 | 0 VisibleDefault      | 38.52s | 21.2578ms | 21.2928ms | 9.3746ms |      3.7646ms |    0.5091ms |         0.2852ms |           583 |      3,771,918 | 기준    | 80 Enemy부터 Frame / GameThread p95가 60fps 기준을 넘는다.                           |
| M04  |    80 | 1 HiddenKeepPose      | 37.32s | 21.7991ms | 21.7849ms | 8.5164ms |      3.7889ms |    0.5141ms |         0.2845ms |           389 |        380,366 | 제한 효과 | render 비용은 줄었지만 Frame / GameThread p95는 회복되지 않았다.                           |

### Render Coverage

| Case | Enemy | Mode                  | 시간     | Frame p95 |  Game p95 |  GPU p95 | Animation p95 | BT Tick p95 | AIPerception p95 | DrawCalls p95 | Primitives p95 | 판정    | 메모                                                                                       |
| ---- | ----: | --------------------- | ------ | --------: | --------: | -------: | ------------: | ----------: | ---------------: | ------------: | -------------- | ----- | ---------------------------------------------------------------------------------------- |
| R00  |    40 | 0 VisibleDefault      | 37.43s |  9.9527ms |  9.3059ms | 7.1942ms |      1.7608ms |           - |         0.0022ms |           555 | 3,275,424      | 기준    | AI / BT / WeaponActor 제거 상태의 render coverage 기준값이다.                                      |
| R01  |    40 | 1 HiddenKeepPose      | 37.39s |  9.3213ms |  8.7718ms | 5.9211ms |      1.6742ms |           - |         0.0018ms |           194 | 34,960         | 효과 확인 | visible mesh render 비용 제거로 GPU / DrawCalls / Primitives와 Frame p95가 함께 감소했다.             |
| R04  |    40 | 2 HiddenAllowPoseSkip | 37.60s |  8.8687ms |  6.0611ms | 5.8075ms |      0.0577ms |           - |         0.0018ms |           193 | 34,976         | 효과 확인 | PIE 실행 전 Mode 2 고정. Mode 1 대비 Animation p95가 1.6742ms에서 0.0577ms로 감소했다.                  |
| R02  |    80 | 0 VisibleDefault      | 37.06s | 13.6320ms | 13.6657ms | 7.9463ms |      3.7604ms |           - |         0.0019ms |           916 | 5,400,982      | 기준    | AI / BT / WeaponActor 제거 상태에서도 skeletal mesh 수 증가로 Frame / DrawCalls / Primitives가 증가했다. |
| R03  |    80 | 1 HiddenKeepPose      | 37.20s | 12.0643ms | 12.1393ms | 5.8511ms |      2.8188ms |           - |         0.0018ms |           194 | 35,062         | 효과 확인 | 80 Enemy에서도 visible mesh render 비용 제거 효과가 유지됐다.                                          |
| R05  |    80 | 2 HiddenAllowPoseSkip | 37.43s |  8.9149ms |  6.9307ms | 5.9291ms |      0.0933ms |           - |         0.0018ms |           194 | 34,980         | 효과 확인 | PIE 실행 전 Mode 2 고정. Mode 1 대비 Animation p95가 2.8188ms에서 0.0933ms로 감소했다.                  |

### WeaponActor Isolation

| Case | Enemy | DisableEnemyWeaponActor | 시간     | Frame p95 |  Game p95 |  GPU p95 | Animation p95 | BT Tick p95 | AIPerception p95 | DrawCalls p95 | CWeaponActor p95 | TotalActor p95 | SkeletalMesh Tick | 판정    | 메모                                                                                |
| ---- | ----: | ----------------------: | ------ | --------: | --------: | -------: | ------------: | ----------: | ---------------: | ------------: | ---------------: | -------------: | ----------------: | ----- | --------------------------------------------------------------------------------- |
| W00  |    40 |                       0 | 37.25s | 10.8848ms |  9.8752ms | 7.1903ms |      1.8961ms |    0.1308ms |         0.1306ms |           733 |               41 |            339 |                80 | 기준    | Enemy WeaponActor 생성 기준이다.                                                        |
| W01  |    40 |                       1 | 37.21s |  9.8752ms |  9.2505ms | 7.0476ms |      1.5954ms |    0.1265ms |         0.1187ms |           573 |                0 |            299 |                40 | 효과 확인 | Enemy WeaponActor 제거로 Actor / SkeletalMesh tick / DrawCalls / Frame p95가 함께 감소했다. |
| W02  |    80 |                       0 | 37.22s | 16.8035ms | 16.7116ms | 8.0555ms |      4.0316ms |    0.2720ms |         0.4109ms |         1,258 |               81 |            499 |               160 | 기준    | 80 Enemy WeaponActor 생성 기준이다.                                                     |
| W03  |    80 |                       1 | 37.18s | 14.8267ms | 14.8160ms | 7.8513ms |      3.5294ms |    0.2493ms |         0.2526ms |           936 |                0 |            419 |                80 | 효과 확인 | Actor / SkeletalMesh tick / DrawCalls / Frame p95가 함께 감소했다.                       |

측정 해석:

```text
Gameplay Stress 조건에서는 EnemyMeshMode 1이 GPU / DrawCalls / Primitives를 줄이지만 Frame / GameThread 회복 효과는 제한적이다.
Render Coverage 조건에서는 EnemyMeshMode 1이 GPU / DrawCalls / Primitives와 Frame p95를 함께 낮춘다.
따라서 mesh visibility hidden은 render cost 축에서는 실제 효과가 있지만, 전투 상황의 frame budget 문제는 AI / Movement / Combat runtime 비용과 함께 분리해서 봐야 한다.
80 Enemy Render Coverage Mode 0에서는 AI / BT / WeaponActor가 제거된 상태에서도 Frame p95와 DrawCalls / Primitives가 증가했다.
80 Enemy Render Coverage Mode 1에서는 DrawCalls p95가 916에서 194로, Primitives p95가 5,400,982에서 35,062로 감소했고 Frame p95도 13.6320ms에서 12.0643ms로 낮아졌다.
40 Enemy Render Coverage Mode 2에서는 Mode 1 대비 GPU / DrawCalls / Primitives는 거의 유지됐지만 Animation p95가 크게 감소했다.
80 Enemy Render Coverage Mode 2에서도 같은 패턴이 반복되어, Mode 2는 render cost 추가 절감보다 hidden 상태의 animation / pose update 비용 절감축으로 해석한다.
WeaponActor Isolation에서는 DisableEnemyWeaponActor 1 적용 시 CWeaponActor가 41에서 0으로 떨어지고 TotalActorCount와 SkeletalMeshComponent tick도 함께 감소했다.
따라서 WeaponActor는 Object Management와 Representation 양쪽에 비용이 있는 축으로 본다.
80 Enemy에서도 같은 actor / component / draw call 감소가 반복됐지만 Frame / GameThread p95는 거의 회복되지 않았다.
재측정 기준에서는 Frame / GameThread / Animation p95도 함께 감소했다.
따라서 WeaponActor 제거는 Object Management와 animation update 비용 축에서 유효하지만, 실제 적용은 combat-capable 단계와 weapon dependency를 함께 고려해야 한다.
```

측정 과정에서 확인한 문제와 분리:

```text
초기 Gameplay Stress 측정에서는 EnemyMeshMode 1이 GPU / DrawCalls / Primitives를 줄였지만 Frame / GameThread p95는 크게 회복되지 않았다.
이 결과만으로 mesh render 비용이 작다고 판단하기에는 카메라에 실제로 잡힌 Enemy 수, 전투 상태, AI / Movement / Combat runtime 비용이 함께 섞여 있었다.
따라서 render 비용만 따로 보기 위해 Render Coverage 조건을 분리했다.
Render Coverage에서는 camera-only pawn, fixed camera, Auto Possess AI off, BT / Perception 미실행, WeaponActor 미생성, Idle animation only 조건을 사용했다.
이 분리 후 40 / 80 Enemy 모두에서 EnemyMeshMode 1이 GPU / DrawCalls / Primitives와 Frame p95를 함께 낮추는 것을 확인했다.
```

현재 결론:

```text
Enemy mesh render cost는 실제로 존재하며, 화면에 노출된 skeletal mesh 수가 늘면 Frame / DrawCalls / Primitives가 함께 증가한다.
Mesh hidden은 render cost 축에서는 효과가 있다.
Gameplay Stress에서 효과가 제한적이었던 이유는 render 비용이 없어서가 아니라 AI / Movement / Combat runtime 비용이 함께 섞였기 때문이다.
따라서 P35 이후 Runtime LOD는 render 축과 gameplay runtime 축을 분리해 검토한다.
Mesh render 축은 40 / 80 측정에서 패턴이 반복됐으므로 120 정규 측정 없이 1차 판단을 종료한다.
```

80 Enemy / Mode 0 기준:

```text
FrameTime p95: 21.2578ms
GameThreadTime p95: 21.2928ms
GPUTime p95: 9.3746ms
BehaviorTreeTick p95: 0.5091ms
AIPerception p95: 0.2852ms
```

80 Enemy부터는 GameThread / FrameTime p95가 60fps 기준을 넘는다. 따라서 같은 80 Enemy 조건에서 Mode 1 HiddenKeepPose를 측정해 mesh visibility 제거가 frame / GPU / draw call에 미치는 영향을 먼저 비교한다.

80 Enemy / Mode 1 비교:

```text
FrameTime p95: 21.2578ms -> 21.7991ms
GameThreadTime p95: 21.2928ms -> 21.7849ms
GPUTime p95: 9.3746ms -> 8.5164ms
DrawCalls p95: 583 -> 389
Primitives p95: 3,771,918 -> 380,366
```

Mode 1은 render 비용을 줄이지만 Frame / GameThread p95는 회복하지 못했다. 따라서 mesh visibility hidden은 render cost 분리 측정으로 유효하지만, 현재 80 Enemy 조건의 frame budget 문제는 GameThread runtime cost 축을 우선 검토하는 쪽이 더 타당하다.

---

## 관련 문서

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/06_notes/N18_AI_Performance_Bottleneck_And_LOD_Plan_Note.md
Docs/06_notes/N20_AI_Profiling_Test_Asset_Plan_Note.md
Docs/06_notes/N21_AI_Runtime_LOD_Policy_Note.md
Docs/07_Profiling/AI_Performance/CSV_Analysis_Guide.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Perception_Runtime_LOD_Measurements.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/Enemy_Mesh_Runtime_LOD_Measurements.md
Docs/07_Profiling/AI_Performance/CSV/MANIFEST.md
```
