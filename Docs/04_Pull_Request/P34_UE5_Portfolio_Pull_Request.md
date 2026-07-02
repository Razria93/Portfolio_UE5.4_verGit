# UE5 Portfolio Pull Request

## 제목

**P34: AI Profiling Test Asset 분리**

## 날짜

**2026.07.02**

## 상태

- [x] 작업 방향 수립
- [ ] Asset / 문서 반영
- [ ] 검증 완료

---

## 브랜치

- `chore/ai-profiling-test-assets`

---

## 주요 커밋 흐름

```text
docs(ai): plan profiling test asset split
chore(ai): add profiling test assets
docs(ai): record profiling asset setup
```

---

## 요약

이번 PR은 P35~P37 AI 최적화 작업에 들어가기 전에, 공유 gameplay asset과 profiling 전용 asset을 분리한다.

목표는 `TestRoom`, `BP_CEnemy`, `BT_Idle` 같은 일반 gameplay asset에 측정 전용 설정을 남기지 않고, Enemy 수 / 배치 / collision / perception / BT 조건을 재현 가능한 전용 환경에서 관리하는 것이다.

이번 PR에서는 runtime LOD, perception LOD, BT service interval LOD 같은 최적화 로직을 구현하지 않는다. 최적화 구현은 P35~P37에서 진행한다.

---

## 작업 배경

P33 측정 중 대량 Enemy 환경을 만들기 위해 공유 asset을 직접 조정했다.

```text
Content/00_UnitTest/TestRoom.umap
Content/01_Character/02_Enemy/BP_CEnemy.uasset
Content/02_Controller/02_Enemy/AI/BehaviorTree/State/BT_Idle.uasset
```

이 방식은 빠르게 측정하기에는 좋지만, 다음 문제가 있다.

```text
일반 gameplay 조건과 profiling 조건이 섞인다.
측정용 Enemy 수 / 배치 / collision 값이 공유 asset에 남을 수 있다.
P35~P37 최적화 전후 비교 기준이 흔들릴 수 있다.
측정 환경을 다시 만들 때 수동 작업이 많아진다.
```

따라서 P34에서는 profiling 전용 asset을 분리하고, 후속 최적화 작업에서 사용할 측정 조건을 고정한다.

---

## 작업 범위

### 1. Profiling 전용 asset 구성

계획 대상:

```text
Profiling 전용 Map
Profiling 전용 Enemy Blueprint
Profiling 전용 BehaviorTree / Blackboard 또는 override 설정
Profiling 전용 Patrol / spawn / placement 기준
```

Profiling asset 경로:

```text
Content/00_Profiling/AI_Performance/Maps/
Content/00_Profiling/AI_Performance/Character/
Content/00_Profiling/AI_Performance/AI/
```

`AI_Performance`는 update interval, runtime LOD, perception LOD, mesh / collision / weapon actor 비교를 포함하는 AI 성능 측정 범위다.

### 2. 측정 조건 고정

고정할 조건:

```text
Enemy count: 40 / 60 / 80 / 100 / 120 기준 구성
Enemy spacing: Enemy끼리 길막지 않는 분산 배치
Friendly hit: Enemy끼리 피격이 발생하지 않는 조건
Patrol / Engage transition: 플레이어와의 engage 상황 재현 가능
PIE fullscreen: F11 기준
Log state: -noailogging 기준
CSV capture: csvprofile start / stop 기준
```

### 3. 극단 비교 테스트 준비

P35~P37에서 사용할 비교 조건:

```text
AnimInstance off
WeaponActor off
Mesh hidden
Collision off
Perception active cap
BT Service interval comparison
```

이번 PR에서 모든 비교를 완료하지 않는다. 다만 비교 테스트를 수행할 수 있는 asset 기준과 절차를 준비한다.

---

## 제외 범위

```text
runtime LOD 구현
perception LOD 구현
BT Service interval LOD 구현
AI 행동 로직 변경
공유 gameplay asset에 profiling 전용 설정 유지
CSV profiling 계측 코드 추가
Enhanced Input migration
```

---

## 검증 계획

정적 확인:

```text
git status --short
git diff --check
```

Asset 확인:

```text
profiling 전용 asset 경로 확인
공유 TestRoom / BP_CEnemy / BT_Idle에 profiling 설정이 남지 않았는지 확인
profiling 전용 map에서 Enemy count와 배치 재현 확인
```

PIE smoke test:

```text
Unreal Editor를 -noailogging 옵션으로 실행
profiling 전용 map에서 PIE 실행
F11 fullscreen 전환
stat unit / stat game / stat ai 활성화
csvprofile start / stop 동작 확인
40 또는 60 Enemy 기준 engage 상태 재현 확인
Enemy끼리 길막 / friendly hit가 측정 변수가 되지 않는지 확인
```

---

## 관련 문서

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/06_notes/N18_AI_Performance_Bottleneck_And_LOD_Plan_Note.md
Docs/06_notes/N20_AI_Profiling_Test_Asset_Plan_Note.md
Docs/04_Pull_Request/P33_UE5_Portfolio_Pull_Request.md
```
