# UE5 Portfolio Pull Request

## 제목

**P35: AI Runtime LOD 정책 정리**

## 날짜

**2026.07.02**

## 상태

- [x] 작업 방향 수립
- [x] 측정 / 코드 / 문서 반영
- [x] 검증 완료

## 브랜치

- `refactor/ai-runtime-lod-policy`

## 요약

대규모 Enemy 조건에서 Runtime LOD 후보 축을 분리 측정하고, 실제 적용 가능한 AI update precision 정책을 정리했다.

초기 목표는 BT Service tick / update interval 최적화였지만, 측정 과정에서 render, animation, WeaponActor, perception, movement, BT scheduling, CombatEngage assignment가 서로 영향을 주는 것을 확인했다.
따라서 단순히 interval 값을 키우는 방식이 아니라, `Engage / Alert / Idle` 계층을 먼저 안정화하고 그 계층에 따라 `AIContext / AIIntentState` update precision을 다르게 적용하는 방향으로 정리했다.

## 최종 결론

```text
Mode 1:
combat-capable 조건에서도 보수적으로 사용할 수 있는 Runtime LOD 후보.
AIContext / AIIntentState 호출수를 줄이면서 gameplay smoke가 안정적으로 유지됐다.

Mode 2:
AIContext / AIIntentState 호출수와 BT Tick p95 감소 폭이 가장 크다.
다만 공격적인 후보이므로 far / offscreen / NonCombat / Dormant 계층부터 적용하는 쪽이 적합하다.

EngageContext:
전투 진입과 공격 전환에 직접 관여하므로 기본 interval을 유지한다.
```

정책 판단:

```text
Runtime LOD는 모든 Enemy에 같은 interval을 적용하는 기능이 아니다.
CombatEngage assignment 결과와 runtime relevance에 따라 update precision을 다르게 적용한다.

Engage:
High precision 유지

Alert:
Reduced precision 후보

Idle / NonCombat / Dormant:
Low 또는 Aggressive precision 후보
```

## 주요 변경

```text
1. AI performance profiling 전용 환경 기준 정리
   - 40 / 80 Enemy scale 기준화
   - RenderCoverage / AnimationLOD / MovementNav / BTUpdateInterval 등 측정 목적별 map 분리

2. Runtime 비용 축 분리 측정
   - Enemy mesh visibility
   - animation refresh
   - WeaponActor creation
   - AI Perception / affiliation
   - movement / nav update
   - BT service update interval

3. CombatEngage assignment 안정화
   - Engage / Alert / Idle gate 정리
   - assignment lease / preserve 정책 추가
   - 초기 후보 수집용 assignment warmup 추가

4. BT service interval 제어 개선
   - asset interval 직접 수정 방식에서 runtime scheduling helper 방식으로 전환
   - ScheduleNextTick / SetNextTickTime 기반으로 service tick 예약
   - AIContext / AIIntentState / EngageContext interval 선택 경로 분리

5. 계측 지표 보강
   - UpdateAIContext / UpdateAIIntentState / UpdateEngageContext 직접 호출 counter 추가
   - Default / Reduced / Aggressive interval preset 선택 counter 추가
   - Perception candidate audit / Blackboard engage latency audit 추가
```

## 대표 측정 결과

측정 기준:

```text
Capture Duration: 약 36초
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
GC Event: none
Evidence Manifest: Docs/07_Profiling/AI_Performance/CSV_Evidence_Manifest.md
Pivot Evidence Manifest: Docs/07_Profiling/AI_Performance/CSV_Evidence_Manifest.md
```

40 Enemy 대표 측정:

```text
Mode 0: Profile(20260709_191603).csv
Mode 1: Profile(20260709_191821).csv
Mode 2: Profile(20260709_192202).csv
```

| Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext Count | AIIntent Count | EngageContext Count |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | 11.5721ms | 10.9428ms | 0.2152ms | 11680 | 6040 | 584 |
| 1 | 11.7582ms | 11.0469ms | 0.1881ms | 6278 | 4140 | 576 |
| 2 | 11.6986ms | 11.0404ms | 0.0956ms | 3918 | 3090 | 578 |

80 Enemy 대표 측정:

```text
Mode 0: Profile(20260709_202805).csv
Mode 1: Profile(20260709_202920).csv
Mode 2: Profile(20260709_203937).csv
```

| Mode | Frame p95 | Game p95 | BT Tick p95 | AIContext Count | AIIntent Count | EngageContext Count |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | 16.2377ms | 16.2519ms | 0.4001ms | 23600 | 12000 | 590 |
| 1 | 16.1284ms | 16.1593ms | 0.3787ms | 12216 | 8100 | 592 |
| 2 | 16.2984ms | 16.2689ms | 0.1641ms | 6947 | 5826 | 580 |

해석:

```text
Frame / Game p95 개선은 제한적이다.
따라서 이 작업의 성과는 즉각적인 frame gain보다 Runtime LOD policy 검증과 service work reduction으로 해석한다.

Mode 1은 호출수 감소와 gameplay 안정성의 균형이 가장 좋다.
Mode 2는 호출수와 BT Tick p95 감소 폭이 가장 크지만, 공격적인 후보로 분류한다.
EngageContext count는 유지되므로 전투 핵심 판단 주기는 보존됐다.
```

## 디버깅 / 시행착오 요약

최적화 과정에서 측정 방향을 바꾸게 만든 핵심 장애요소:

```text
1. 측정 환경 오염
   - Enemy끼리 충돌 / 피격 / crowding이 BT 비용처럼 보였다.
   - profiling 전용 asset / map을 분리해 오염 변수를 줄였다.

2. CSV 해석 기준 불안정
   - GC 이벤트와 capture 구간 차이로 p95가 흔들렸다.
   - first 3s / last 3s trim, GC 없는 대표값 기준으로 정리했다.

3. Mesh hidden과 pose update 혼선
   - mesh visibility와 animation / socket / montage timing이 섞였다.
   - VisibleDefault / HiddenKeepPose / HiddenAllowPoseSkip으로 분리했다.

4. WeaponActor 비용 축 혼선
   - actor presence, combat processing, feedback 비용이 섞일 수 있었다.
   - WeaponActor 생성 스킵과 후속 상태 보존을 분리했다.

5. Perception 후보 누수
   - Enemy가 서로 perception candidate로 잡혀 target 식별이 지연됐다.
   - Team attitude / affiliation 보정과 candidate audit으로 분리 확인했다.

6. Movement LOD의 representation 손상
   - MovementComponent tick off는 Alert Spread의 locomotion 표현을 깨뜨렸다.
   - 성능 이득 대비 representation 손상이 커서 우선 적용 후보에서 제외했다.

7. BT asset interval 직렬화 문제
   - asset service interval 변경만으로 runtime scheduling 제어가 명확하지 않았다.
   - ScheduleNextTick / SetNextTickTime 기반 helper로 전환했다.

8. active count 오해
   - stat active count를 실제 service 호출수로 보기 어려웠다.
   - 직접 호출 counter와 interval preset counter를 추가했다.

9. assignment bootstrap 문제
   - 80 Enemy request가 6 -> 13 -> 32 -> 62 -> 80처럼 단계적으로 채워졌다.
   - WarmupTime 1.2 기준으로 최초 assignment 후보군 수집을 안정화했다.
```

상세 기록:

```text
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Runtime_LOD_Debugging_Obstacle_Note.md
```

## 검증

```text
1. 40 / 80 Enemy 조건에서 Mode 0 / 1 / 2 측정
2. Engage 2 / Alert 6 / 나머지 Idle 계층 유지 확인
3. Mode 1 / 2에서 AIContext / AIIntentState count 감소 확인
4. EngageContext count 유지 확인
5. Default / Reduced / Aggressive interval preset 분포 확인
6. GC 없는 CSV를 대표 측정값으로 사용
7. 최종 대표 측정과 설계 분기 근거 CSV를 분리해서 보관
```

## 제외 범위

```text
1. Proxy / Dormant Actor 구현
   - 별도 feature로 분리한다.

2. 최종 Runtime LOD component 구현
   - P35는 정책과 후보 축 검증까지 다룬다.

3. Combat action timeline 분리
   - 현재는 montage notify가 combat timing source 역할을 하므로 후속 작업으로 둔다.

4. MovementComponent tick off 적용
   - representation 손상이 커서 현재 Runtime LOD 후보에서 제외한다.
```

## 후속 작업

```text
1. Observe / Aware Intent 분리
   - target은 인식했지만 Engage / Alert 권한이 없는 Enemy를 Idle과 분리한다.

2. AlertCap CVar 비교 측정
   - AlertCap 6 / 40을 같은 코드 상태에서 비교한다.

3. Runtime LOD Implementation v1
   - Mode 1을 combat-capable 보수 후보로 적용한다.
   - Mode 2는 far / offscreen / NonCombat / Dormant 후보로 검토한다.

4. Proxy / Dormant Actor 최적화 검토
   - 실제 Character Actor 수 자체를 줄이는 별도 최적화 후보로 다룬다.
```

## 관련 문서

```text
Docs/06_notes/N21_AI_Runtime_LOD_Policy_Note.md
Docs/07_Profiling/AI_Performance/CSV_Evidence_Manifest.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_BT_Update_Interval_AIContext_Level_Split_Note.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_BT_Update_Interval_AIContext_Level_Split_80Enemy_Correction.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Runtime_LOD_Debugging_Obstacle_Note.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_CombatEngage_Assignment_Bootstrap_Warmup_Plan.md
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_BT_Update_Interval_LOD_Result_Note.md
Docs/07_Profiling/CSV_Analysis_Guide.md
```
