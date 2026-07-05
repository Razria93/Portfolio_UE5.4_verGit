# AI Performance CSV Analysis Guide

## 목적

AI Performance / Runtime LOD CSV를 반복 분석할 때 사용하는 공통 기준을 정리한다.

도메인별 세부 기준은 별도 문서를 따른다.

```text
Perception Audit:
Docs/07_Profiling/AI_Performance/Runtime_LOD/AI_Perception_Audit_Analysis_Guide.md
```

---

## 분석이 오래 걸리는 대표 원인

Unreal CSV는 일반적인 CSV 도구로 바로 읽기 어려운 경우가 있다.

이번 분석에서 확인한 지연 원인:

```text
CSV 컬럼명이 중복되어 PowerShell Import-Csv가 실패했다.
로컬 환경에서 python 명령이 연결되어 있지 않았다.
기존 문서 일부가 mojibake 상태라 결과 반영 전에 문서 검증이 필요했다.
```

이후 분석 기본 원칙:

```text
Unreal CSV는 Import-Csv에 바로 의존하지 않는다.
헤더 이름을 index로 매핑한 뒤 row 값을 index 기반으로 읽는다.
문서 출력이 깨지는지 Get-Content로 즉시 확인한다.
```

---

## 기본 측정 조건

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

---

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

---

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

---

## 해석 순서

```text
1. 측정 조건이 유효한지 확인한다.
2. FrameTime / GameThreadTime / GPUTime p95를 먼저 본다.
3. 변경한 축과 직접 연결된 지표를 본다.
4. ActorCount / Tick count로 조건이 실제 반영됐는지 확인한다.
5. avg보다 p95를 우선 비교하고, p99 / max는 outlier로 따로 본다.
6. CSV 수치만으로 부족한 경우 로그 기반 audit 결과를 함께 본다.
```

예시:

```text
WeaponActor 제거 측정
-> ActorCount/CWeaponActor 확인
-> FrameTime / GameThreadTime p95 확인
-> Animation / CharacterMovement / DrawCalls는 보조 지표

Perception Audit 측정
-> PerceptionCandidateAudit 로그 확인
-> AIPerception p95 확인
-> BT_UpdateAIContext / BehaviorTreeTick p95 확인
```

---

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

---

## 분석 응답 기준

분석 응답은 다음 순서로 작성한다.

```text
1. 측정 신뢰도 이슈가 있으면 먼저 말한다.
2. 문제가 없다면 "측정 조건상 큰 이상 없음"을 먼저 말한다.
3. 핵심 p95 지표를 요약한다.
4. 변경 축과 직접 연결된 지표를 해석한다.
5. 아직 확정하면 안 되는 항목을 분리한다.
6. 다음 측정 조건을 제안한다.
```
