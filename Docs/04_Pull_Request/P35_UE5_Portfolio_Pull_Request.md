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

P35는 바로 proxy enemy나 spawn / despawn manager를 구현하지 않는다. 먼저 `MAP_AIPerf_40Enemy` baseline으로 asset / gameplay 정상성을 확인하고, 80 / 120 Enemy scale variant에서 WeaponActor, collision, mesh, animation, movement, component tick 같은 runtime 요소가 실제로 의미 있는 비용 차이를 만드는지 확인한다.

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
-> 효과가 보이지 않는 측정 축은 120 Enemy 정규 측정에서 제외 가능

120 Enemy
-> 80 Enemy에서 효과가 확인된 후보의 primary comparison 기준
```

---

## 작업 범위

### 1. Runtime 비용 축 분리 측정

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

측정 scale:

```text
40 Enemy
-> smoke / sanity 확인

80 Enemy
-> 측정 축별 1차 비교

120 Enemy
-> 효과가 확인된 측정 축의 최종 비교
```

현재 측정 스위치:

```text
Portfolio.AI.RuntimeLOD.EnemyMeshHidden 1
-> ACEnemy SkeletalMesh visibility off
-> PIE 중 변경할 수 있으며 다음 Tick에서 반영된다.
```

### 2. Runtime LOD 단계 정의

초기 정책 후보:

```text
Near
-> 기존 동작 유지

Mid
-> 일부 collision / animation / cosmetic update 축소

Far
-> mesh visibility / shadow / weapon actor / movement update 제한 후보

Dormant
-> proxy 또는 representation 전환 후보
-> P35 구현 범위에서는 제외하고 후속 feature 후보로 기록
```

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

Perception LOD는 P36, Update LOD는 P37에서 다룬다.

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
Duration: 약 30초
```

Mesh visibility 비교:

```text
Baseline
-> Portfolio.AI.RuntimeLOD.EnemyMeshHidden 0

Mesh off
-> Portfolio.AI.RuntimeLOD.EnemyMeshHidden 1
```

Mesh visibility off 관찰:

```text
SetHiddenInGame(true) / SetVisibility(false)는 mesh render만 끄는 비교가 아닐 수 있다.
SkeletalMeshComponent가 hidden 상태가 되면 VisibilityBasedAnimTickOption 설정에 따라 pose / bone refresh가 줄거나 멈출 수 있다.
pose 갱신이 멈추면 socket transform도 마지막 pose에 머무를 수 있다.
WeaponActor가 hand / holster socket에 attach된 상태라면 검이 hidden 직전 위치에 고정된 것처럼 보일 수 있다.
```

따라서 이 측정은 단순 render 비용만 분리한 측정이 아니라 `skeletal mesh visibility / pose update 영향 포함` 극단 비교로 해석한다.

비교 기록:

```text
40 / 80 / 120 scale별 FrameTime / GameThreadTime / GPUTime / AIPerception / PortfolioAI scope p95 비교
actor count / component tick 변화 확인
gameplay smoke test로 attack / hit / guard / parry 흐름 유지 확인
WeaponActor socket follow 유지 여부 확인
```

---

## 관련 문서

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/06_notes/N18_AI_Performance_Bottleneck_And_LOD_Plan_Note.md
Docs/06_notes/N20_AI_Profiling_Test_Asset_Plan_Note.md
Docs/06_notes/N21_AI_Runtime_LOD_Policy_Note.md
Docs/07_Profiling/AI_Performance/CSV/MANIFEST.md
```
