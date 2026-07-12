# CSV Analysis Guide

## 목적

이 문서는 `Docs/07_Profiling` 전체에서 사용하는 CSV 분석 공통 규칙이다.
AI Update Interval, AI Performance, Runtime LOD 측정 모두 이 기준을 우선 적용한다.

도메인별 세부 기준은 별도 문서를 따른다.

```text
Perception Audit:
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Perception_Audit_Analysis_Guide.md
```

## 빠른 분석 순서

분석 요청을 받으면 아래 순서로 처리한다.

```text
1. 첨부된 Log 파일에서 csvprofile start / stop, duration, frame count, GC 이벤트를 확인한다.
2. CSV를 일반 Import-Csv로 바로 읽지 말고 header index 기반으로 파싱한다.
3. FrameTime 누적합으로 전체 capture time을 계산한다.
4. first 3s / last 3s를 제외한 middle window만 분석한다.
5. p95를 기본 비교값으로 사용하고 avg / p99 / max는 보조로 본다.
6. 변경한 축과 직접 연결된 stat을 먼저 본다.
7. ActorCount / Tick count / *_Count로 측정 조건이 실제 반영됐는지 확인한다.
8. 없는 컬럼, 0 값, active count 0을 구분해서 해석한다.
9. CSV 수치만으로 gameplay state를 단정하지 않고 로그 / PIE 관찰을 함께 본다.
10. 답변은 측정 신뢰도 -> 핵심 지표 -> 직접 축 해석 -> 주의점 -> 다음 측정 순서로 작성한다.
```

## 분석이 오래 걸리는 대표 원인

Unreal CSV는 일반 CSV 도구로 바로 읽기 어려운 경우가 있다.

확인된 지연 원인:

```text
CSV 컬럼명이 중복되어 PowerShell Import-Csv가 실패할 수 있다.
로컬 환경에서 python 명령이 연결되어 있지 않을 수 있다.
일부 문서가 mojibake 상태일 수 있어 결과 반영 전 문서 검증이 필요하다.
Event 기반 stat은 호출 빈도가 낮아 p95 비용이 안정적으로 잡히지 않을 수 있다.
```

분석 기본 원칙:

```text
Unreal CSV는 Import-Csv에 바로 의존하지 않는다.
헤더 이름을 index로 매핑한 뒤 row 값을 index 기반으로 읽는다.
동일 이름 컬럼은 첫 번째/마지막 컬럼을 임의로 고르지 말고 목적에 맞는 full header를 확인한다.
문서 출력이 깨지는지 Get-Content 또는 rg로 즉시 확인한다.
```

## 기본 측정 조건

일반적인 비교 측정은 아래 조건을 기본값으로 본다.

```text
Capture Duration: about 36s
Analysis Window: first 3s / last 3s trimmed, middle 30s used
Log State: -noailogging
PIE: F11 fullscreen
```

분석 구간 계산:

```text
FrameTime 누적합으로 전체 capture time을 계산한다.
누적 시간이 3초 이상이고 total - 3초 이하인 frame만 사용한다.
```

지표 사용 기준:

```text
p95: 주요 비교값
avg: 전체 경향 확인
p99: 반복되는 상위 비용 확인
max: outlier 확인
```

## 측정 전 체크리스트

```text
1. CVar가 의도한 값으로 적용됐는지 확인한다.
2. map / camera / PlayerStart / Enemy count가 같은지 확인한다.
3. PIE fullscreen 조건인지 확인한다.
4. -noailogging 조건인지 확인한다.
5. csvprofile start 전에 필요하면 gc를 입력하고 2~3초 대기한다.
6. capture 중 Output Log / Details / Content Browser를 조작하지 않는다.
7. capture log에 CSVEvent "GC"가 있는지 확인한다.
```

주의:

```text
gc 명령은 capture 중 GC 발생을 완전히 막는 보장 수단이 아니다.
다만 csvprofile start 전에 정리를 시도해 capture 중 GC 발생 가능성을 낮추는 용도로 쓴다.
```

## 공통 주요 지표

Frame / thread:

```text
FrameTime
GameThreadTime
RenderThreadTime
GPUTime
RHIThreadTime
```

AI / gameplay:

```text
Exclusive/GameThread/AIPerception
Exclusive/GameThread/BehaviorTreeTick
Exclusive/GameThread/BehaviorTreeSearch
GameThread/PortfolioAI_BT_UpdateAIContext
GameThread/PortfolioAI_BT_UpdateAIIntentState
GameThread/PortfolioAI_BT_UpdateEngageContext
GameThread/PortfolioAI_CombatEngage_Tick
GameThread/PortfolioAI_CombatEngage_RebuildAssignments
```

Movement / animation:

```text
Exclusive/GameThread/CharacterMovement
Exclusive/GameThread/Animation
Ticks/CharacterMovementComponent
Ticks/SkeletalMeshComponent
```

Rendering / actor count:

```text
RHI/DrawCalls
RHI/PrimitivesDrawn
ActorCount/CEnemy
ActorCount/CWeaponActor
ActorCount/CAIController
ActorCount/TotalActorCount
```

Combat / hit window:

```text
PortfolioAI_Combat_CollisionNotifyBegin_Count
PortfolioAI_Combat_CollisionNotifyEnd_Count
PortfolioAI_Combat_HitWindowOpen_Count
PortfolioAI_Combat_HitWindowClose_Count
PortfolioAI_Combat_HitWindowOverlap_Count
PortfolioAI_Combat_HitProcessing_Count
PortfolioAI_CombatSignal_Count
PortfolioAI_CombatSignalCueRoute_Count
```

## 축별 직접 지표

| 측정 축 | 먼저 볼 지표 | 보조 지표 |
| --- | --- | --- |
| WeaponActor | `ActorCount/CWeaponActor`, Frame/Game p95 | DrawCalls, SkeletalMesh tick, gameplay smoke |
| Mesh / RenderCoverage | GPU, DrawCalls, PrimitivesDrawn | Animation, Frame/Game p95 |
| Animation / Pose | Animation p95, animation refresh counter | SkeletalMesh tick, Frame/Game p95 |
| Movement / Nav | CharacterMovement p95, Movement count | PathFollowing, Frame/Game p95, PIE 관찰 |
| Perception | AIPerception p95, audit log | AIContext, BehaviorTreeTick |
| BT interval | `*_Count`, interval preset count | BehaviorTreeTick p95, Frame/Game p95 |
| Combat collision / hit window | HitWindow overlap / HitProcessing count | CombatSignal count, feedback smoke |
| Feedback presentation | Niagara / trail / cue count | GPU, GameThread, visual smoke |

## 컬럼 없음 / 0 / active count 해석

CSV에서 특정 stat이 보이지 않는 경우를 바로 오류로 판단하지 않는다.

```text
컬럼 없음:
해당 capture 동안 그 stat scope가 한 번도 기록되지 않았거나,
해당 코드 경로 / BT branch / service가 실행되지 않았을 가능성이 높다.
정상 여부는 측정 목적과 gameplay smoke로 판단한다.

컬럼은 있으나 p95 == 0:
stat scope는 존재하지만 대부분의 frame에서 비용이 없거나 매우 작다.
active count를 함께 확인한다.

active count 감소:
해당 stat이 기록된 frame 수가 줄었다는 신호다.
호출 빈도 감소의 참고 자료로 볼 수는 있지만, frame 단위 분산 때문에 실제 호출 횟수와 다를 수 있다.

`*_Count` 감소:
CSV custom counter로 누적한 실제 호출 횟수 감소다.
interval / gate / LOD 제어가 호출량을 줄였는지 판단할 때는 active count보다 `*_Count`를 우선한다.
Frame / GameThread p95 개선이 작더라도 `*_Count`가 줄면 작업량 감소로 기록할 수 있다.

active count 0:
해당 경로가 실행되지 않은 것이다.
측정 목적이 "그 경로를 제거하는 것"이면 정상이고,
baseline에서 반드시 실행되어야 하는 경로라면 gameplay smoke를 다시 확인한다.
```

## Event 기반 계측 해석

Event 기반 계측은 Tick 기반 계측보다 p95 비용으로 해석하기 어렵다.

```text
AnimNotify, collision begin/end, hit processing, combat signal 같은 이벤트는 호출 빈도가 낮다.
호출 frame이 적으면 p95 비용이 0이거나 컬럼이 불안정하게 보일 수 있다.
이 경우 비용 자체보다 Count를 먼저 본다.
이벤트 호출이 실제로 발생했는지는 Output Log, visual smoke, gameplay smoke를 함께 확인한다.
```

권장 방식:

```text
Tick / Service / Subsystem처럼 반복 호출되는 API:
-> p95 비용과 Count를 함께 본다.

Notify / Overlap / Hit processing처럼 이벤트성 API:
-> Count와 gameplay smoke를 먼저 본다.
-> p95 비용은 보조 지표로만 본다.
```

## ActorCount와 Tick count 불일치

ActorCount와 Tick count가 다를 때는 둘을 같은 지표로 해석하지 않는다.

```text
ActorCount:
월드에 존재하는 actor 수다.
측정용 배치, 비활성 객체, 에디터/테스트용 actor가 포함될 수 있다.

Ticks/*:
해당 frame에서 실제 tick한 객체 수다.
성능 측정 scale은 보통 Tick count를 우선 기준으로 본다.

예:
ActorCount/CEnemy가 85이고 Ticks/CEnemy가 40이면,
성능 측정 scale은 40 Enemy tick 기준으로 해석한다.
다만 actor presence cost를 보는 측정이면 ActorCount도 따로 기록한다.
```

## BT Update Interval 해석 기준

BT interval 측정에서 `BT_UpdateEngageContext`가 보이지 않는다고 즉시 문제로 단정하지 않는다.

```text
BT_UpdateEngageContext는 Engage branch에 붙은 service다.
CombatRole gate 이후 Engage assignment를 받지 못한 Enemy는 Engage branch로 들어가지 않는다.
따라서 AssignmentGate 측정에서는 EngageContext active count가 줄거나 컬럼이 없을 수 있다.
```

확인할 것:

```text
1. Mode 0에서 최소 Engager가 정상적으로 공격하는지
2. Mode 1 / 2에서 AIContext / AIIntent / EngageContext count가 정책대로 변했는지
3. EngageContext가 Mode 0 대비 과도하게 사라져 공격 전환이 깨졌는지
4. Alert cap 밖 Enemy가 Chase / Alert Spread로 몰리지 않는지
```

BT service 호출 횟수 기준:

```text
PortfolioAI_BT_UpdateAIContext_Count:
AIContext TickNode 호출 횟수다.

PortfolioAI_BT_UpdateAIIntentState_Count:
AIIntentState TickNode 호출 횟수다.

PortfolioAI_BT_UpdateEngageContext_Count:
EngageContext TickNode 호출 횟수다.
Engage branch 실행 여부와 연결된다.
```

BT service interval 선택 기준:

```text
PortfolioAI_BT_AIIntentInterval_Default_Count
PortfolioAI_BT_AIIntentInterval_Reduced_Count
PortfolioAI_BT_AIIntentInterval_Aggressive_Count
```

`AIContext`는 현재 Runtime LOD interval 조정 대상이 아니다.
따라서 `PortfolioAI_BT_AIContextInterval_*` counter가 없는 것이 정상이다.
과거 `AIContext interval split` 실험 CSV를 해석할 때만 해당 legacy counter를 참고한다.

AIIntentState interval preset 정상 패턴:

```text
Mode 0:
Default Count만 증가한다.

Mode 1:
Default Count와 Reduced Count가 증가한다.
Aggressive Count는 0 또는 거의 없어야 한다.

Mode 2:
Default Count, Reduced Count, Aggressive Count가 모두 증가한다.
Low precision 대상이 없으면 Aggressive Count가 0일 수 있다.
```

해석:

```text
AIIntentState Count가 줄고 interval preset 분포가 위 패턴을 따르면 BTUpdateIntervalMode는 정상 적용된 것이다.
Frame / Game p95 변화가 작아도 service work reduction으로 기록한다.
```

## 결과 공유 템플릿

```text
Case:
CSV:
Capture Duration:
Analysis Window:
Log State:
PIE:
Map:
CVar:

Pre-capture log:
Cmd: <CVar>
Cmd: gc
Cmd: csvprofile start

Capture log:
LogCsvProfiler: Display: Capture Starting
LogCsvProfiler: Display: Metadata set : starttimestamp="..."
LogCsvProfiler: Display: CSVEvent "GC" [Frame ...]
Cmd: csvprofile stop
LogCsvProfiler: Display: Metadata set : endtimestamp="..."
LogCsvProfiler: Display: Capture Ended. Writing CSV to file : ...
LogCsvProfiler: Display: Frames : ...

Observed:
- 측정 중 특이사항
- 조작 여부
- gameplay smoke 확인 여부
```

## 분석 답변 템플릿

```text
측정 신뢰도:
- GC:
- Capture duration:
- Analysis window:
- CVar / map / scale 확인:

핵심 지표:
| Metric | p95 | 해석 |

직접 축 해석:
- 변경한 축:
- 직접 stat:
- active count / Count:
- Frame / GameThread 영향:

주의점:
- 없는 컬럼:
- ActorCount / Tick count 불일치:
- gameplay smoke로 확인할 항목:

다음 측정:
- 다음 Mode / Enemy count:
- 유지할 조건:
- 반드시 관찰할 항목:
```
