# UE5 Portfolio - AI Profiling Test Asset Plan Note

## 목적

이 문서는 `P34: AI Profiling Test Asset 분리` 작업의 실행 기준을 정리한다.

P34의 목적은 최적화 구현이 아니라, P35~P37 최적화 작업 전에 측정 환경을 분리하고 재현 가능하게 만드는 것이다.

---

## 문제

P33 측정에서는 빠른 검증을 위해 공유 gameplay asset을 직접 조정했다.

```text
Content/00_UnitTest/TestRoom.umap
Content/01_Character/02_Enemy/BP_CEnemy.uasset
Content/02_Controller/02_Enemy/AI/BehaviorTree/State/BT_Idle.uasset
```

이 방식은 다음 문제가 있다.

```text
일반 gameplay 상태와 profiling 상태가 섞인다.
Enemy 수, 배치, collision, patrol 방식이 공유 asset에 남을 수 있다.
측정 전후 asset 복구 비용이 생긴다.
P35~P37 최적화 효과 비교 기준이 흔들린다.
```

---

## 원칙

```text
공유 gameplay asset은 기본 동작 확인용으로 유지한다.
profiling 전용 asset은 성능 측정 조건을 재현하기 위해 별도로 둔다.
최적화 로직은 P34에서 구현하지 않는다.
P34는 P35~P37의 실험 환경을 준비한다.
```

---

## Asset 분리 기준

### Map

profiling map은 Enemy 수와 배치를 고정하기 위한 전용 map이다.

담당 범위:

```text
Enemy count
Enemy placement
Patrol point placement
Player start position
Engage 시작 조건
측정용 label / marker
```

공유 `TestRoom.umap`은 일반 smoke test용으로 유지한다.

### Enemy Blueprint

profiling enemy는 측정 변수를 줄이기 위한 전용 Enemy Blueprint다.

담당 범위:

```text
profiling용 collision 설정
profiling용 perception 설정
profiling용 combat / patrol tuning override
profiling용 friendly hit 차단 조건
```

공유 `BP_CEnemy.uasset`은 일반 gameplay 기준값을 유지한다.

### BehaviorTree / Blackboard

profiling BT는 측정 조건을 고정하기 위한 전용 AI 흐름이다.

담당 범위:

```text
Idle / Patrol / Engage transition 재현
측정 대상 service 활성화
측정에서 제외할 branch 비활성화
interval 비교 실험 준비
```

공유 `BT_Idle.uasset`과 기존 BT graph는 일반 gameplay 기준으로 유지한다.

---

## Profiling Asset 경로

기본 경로:

```text
Content/00_Profiling/00_AI_Performance/Map/
Content/00_Profiling/00_AI_Performance/01_Character/
Content/00_Profiling/00_AI_Performance/02_Controller/
Content/00_Profiling/00_AI_Performance/03_Animation/
Content/00_Profiling/00_AI_Performance/99_Environment/
```

`Content/00_Profiling/00_AI_Performance`는 AI 성능 측정 전용 asset 루트다.

```text
BT Service update interval
runtime LOD
perception LOD
AnimInstance / WeaponActor / Mesh / Collision 비교
```

위 범위를 같은 profiling context로 묶기 위해 Map / Character / AI Controller / BehaviorTree / Blackboard / Patrol / Animation / Material asset을 이 경로 아래에 둔다.

---

## P34 작업 순서

### 1. 공유 asset 기준 복구 확인

확인 대상:

```text
TestRoom enemy count
BP_CEnemy collision radius
BP_CEnemy health / combat setting
BT_Idle patrol / MoveTo setting
```

목표:

```text
공유 gameplay asset에 P33 profiling 전용 설정이 남지 않았는지 확인한다.
```

### 2. Profiling 전용 폴더 생성

생성 대상:

```text
Content/00_Profiling/00_AI_Performance/
```

하위 구성:

```text
Map
01_Character
02_Controller
03_Animation
99_Environment
```

### 3. Profiling 전용 asset 복제

복제 기준:

```text
TestRoom -> profiling map
BP_CEnemy -> profiling enemy
BT / BB 필요 asset -> profiling AI asset
PatrolPath / PatrolPoint -> profiling patrol asset
AnimBlueprint -> profiling animation asset
Material -> profiling material asset
```

복제 후 redirector 정리와 reference 확인을 수행한다.

### 4. Baseline 측정 조건 구성

기본 baseline:

```text
40 Enemy / Engage / F11 fullscreen / -noailogging
```

후속 scale variant:

```text
60 Enemy
80 Enemy
100 Enemy
120 Enemy
```

P34에서는 40 Enemy 기준 profiling world를 고정한다.

60 / 80 / 100 / 120 Enemy 조건은 P35~P37에서 특정 최적화 축을 검증할 때 파생 map 또는 map variant로 확장한다.

120 초과 구간은 stress limit 확인용으로만 다룬다.

### 5. 극단 비교 테스트 준비

P35~P37에 넘길 비교 축:

```text
AnimInstance off
WeaponActor off
Mesh hidden
Collision off
Perception active cap
BT Service interval comparison
```

P34에서는 비교 테스트를 실행할 수 있는 asset 구조와 절차를 준비한다.

---

## 완료 조건

```text
Profiling 전용 map이 존재한다.
Profiling 전용 enemy 기준이 존재한다.
Profiling 전용 AI / patrol 기준이 존재한다.
공유 gameplay asset에 profiling 전용 설정이 남지 않는다.
40 Enemy engage baseline을 재현할 수 있다.
P35~P37 극단 비교 테스트를 시작할 수 있다.
PR 문서에 실제 asset 경로와 검증 결과가 기록되어 있다.
```

---

## 검증 명령 / 절차

실행 조건:

```text
Unreal Editor 실행 옵션: -noailogging
PIE mode: fullscreen, F11
Stat: stat unit / stat game / stat ai
CSV: csvprofile start / csvprofile stop
Duration: 약 30초
```

Git Bash 실행 예:

```bash
"/c/Program Files/Epic Games/UE_5.4/Engine/Binaries/Win64/UnrealEditor.exe" "C:/UE5_Portfolio/Portfolio_UE5.4_verGit/Portfolio/Portfolio.uproject" -noailogging
```

UE console:

```text
stat unit
stat game
stat ai
csvprofile start
csvprofile stop
```

---

## P35~P37 인계 기준

P34가 끝나면 다음 정보가 후속 PR의 입력값이 된다.

```text
Profiling asset 경로
Baseline enemy count
Baseline placement rule
Friendly hit / crowd blocking 제거 기준
CSV capture 절차
Extreme comparison test list
```

---

## 현재 Baseline 기록

### Case 01: 40 Enemy / AIPerf Engage

Raw CSV:

```text
Docs/07_Profiling/AI_Performance/CSV/baseline/case_01_040_enemy_aiperf_engage.csv
```

검증 결과:

```text
40 Enemy 정상 배치
AIPerf Enemy -> AIPerf AIController 참조 확인
AIPerf AIController -> AIPerf Blackboard / BehaviorTree 참조 확인
Blackboard TargetActor 정상 갱신
Patrol 랜덤 동작
Engage 진입 가능
Enemy끼리 피격 없음
MoveTo 허용반경 100 기준 이동 / 도착 확인
ReturnToHome MoveTo 허용반경 100 기준 확인
Movable Range 5000 기준 홈 복귀 / 전투 이탈 확인
AIPerf PatrolPath / PatrolPoint 참조 확인
AIPerf AnimInstance 참조 확인
```

Crowd / collision 기준:

```text
Enemy끼리 완전한 길막 제거 상태는 아니다.
collision radius를 10으로 줄이고 MoveTo 허용반경을 100으로 늘려 crowd 변수를 줄인다.
군집 해소 알고리즘은 P34 범위가 아니다.
```

대표 지표:

```text
FrameTime p95: 12.0703ms
GameThreadTime p95: 11.9513ms
GPUTime p95: 6.9694ms
PortfolioAI_BT_UpdateAIContext p95: 0.1608ms
AIPerception p95: 0.1216ms
```
