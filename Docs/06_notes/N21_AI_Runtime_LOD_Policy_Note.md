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
-> 80 Enemy에서 효과가 확인된 측정 축의 primary comparison 기준
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

## 극단 비교 테스트

구현 전에 runtime cost 측정 축을 먼저 분리한다.

측정 축:

```text
WeaponActor off
Combat collision off / reduced
Mesh hidden
AnimInstance off 또는 animation update 최소화
CharacterMovement 제한
Non-essential component tick off
Shadow off
Material 단순화
Low-poly / proxy mesh 비교
Perception off
BT Tick off 또는 update interval 증가
```

위 항목이 P35의 runtime cost 측정 축이다.

`40 / 80 / 120 Enemy`는 측정 축이 아니라 scale 단계다.

각 비교는 하나의 측정 축만 바꿔 수행한다.

측정 우선순위:

```text
1차 측정
-> 코드 / 에디터 설정으로 바로 끌 수 있는 축
-> Mesh visibility, WeaponActor, AnimInstance, Movement, Collision, Tick, Shadow, Perception, BT Tick

2차 측정
-> 실제 Runtime LOD로 적용 가능한 주기 / 거리 기반 조정 축
-> component tick interval, BT service interval, perception activation range / cap

3차 측정
-> 에셋 제작 또는 대체가 필요한 축
-> material 단순화, low-poly mesh, proxy representation
```

P35의 우선순위는 1차와 2차 측정이다. 3차 측정은 render 비용이 큰 축으로 확인될 때 후속 PR 후보로 분리한다.

측정 흐름:

```text
1. 40 Enemy에서 기능이 깨지지 않는지 확인한다.
2. 80 Enemy에서 측정 축별 1차 비교를 수행한다.
3. 80 Enemy에서 차이가 보인 측정 축만 120 Enemy에서 primary comparison을 수행한다.
```

EnemyMeshMode 비교 측정 기준:

```text
Capture Duration: 약 36초
Analysis Window: 앞 3초 / 뒤 3초 제외, 중앙 30초 사용
```

`Analysis Window`는 UE 자동 종료 기능이 아니라 CSV 분석 기준이다. CSV는 전체 36초를 기록하고, 비교값을 계산할 때 앞뒤 3초를 제외한다.

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

### Near

```text
플레이어와 가깝거나 전투에 직접 참여 중인 Enemy
기존 동작 유지
combat / animation / collision / weapon actor 유지
```

### Mid

```text
근처에 있지만 직접 전투 중은 아닌 Enemy
cosmetic update 또는 일부 collision cost 축소 후보
```

### Far

```text
멀리 있거나 현재 전투 영향도가 낮은 Enemy
mesh visibility / shadow / weapon actor / movement update 제한 후보
```

### Dormant

```text
매우 멀거나 현재 gameplay 영향도가 낮은 Enemy
proxy 또는 representation 전환 후보
P35에서는 구현하지 않고 후속 feature로 분리
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
