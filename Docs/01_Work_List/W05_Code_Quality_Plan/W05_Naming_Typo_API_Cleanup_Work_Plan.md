# W05 Naming / Typo / API Cleanup Work Plan

## 제목

**W05: 네이밍 / 오타 / API 정리 작업 계획**

## 날짜

**2026.07.23**

## 상태

- [x] 네이밍 패턴 스캔
- [x] 네이밍 규칙 문서 분리
- [x] 이번 브랜치 처리 범위 / 보류 범위 분류
- [x] P0 단발 네이밍 불일치 수정
- [x] P1 public API rename 적용
- [x] Profiling helper API suffix 정리
- [x] World subsystem structure file rename 적용
- [x] 최종 검증
- [x] PR 문서 작성

---

## 브랜치

```text
refactor/naming-typo-api-cleanup
```

---

## 1. 목표

이번 작업은 기능 동작을 바꾸지 않고, 프로젝트 내부 네이밍의 신뢰도를 정리한다.

목표는 새 네이밍 체계를 크게 도입하는 것이 아니다. 이미 프로젝트에 자리 잡은 강한 관례를 문서로 고정하고, 단발성 오타 / 표기 흔들림 / 매개변수명 불일치만 정리한다.

```text
1. 명백한 오타와 단발 네이밍 불일치 제거
2. Unreal C++ 관례와 현재 프로젝트 관례의 경계 고정
3. public API / Blueprint / UHT 영향이 있는 rename은 별도 판단
4. 이후 네이밍 리뷰에서 사용할 기준 문서 연결
```

---

## 2. 적용 규칙

네이밍 규칙은 별도 문서에서 관리한다.

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_Naming_Rules.md
```

이번 브랜치는 위 규칙을 기준으로 단발 오타 / 표기 흔들림 / 매개변수명 불일치만 정리한다.

---

## 3. 이번 브랜치 처리 범위

### P0: 바로 수정

다음 항목은 local-only 또는 형식성 변경이다. Blueprint / asset reference 영향이 없으므로 이번 브랜치에서 처리한다.

```text
CPlayerController.cpp
-> inAxisValue -> InAxisValue

CActionComponent.cpp
-> executorkey -> executorKey

CReactionComponent.cpp
-> executorkey -> executorKey

CBTService_UpdateAIContext.cpp
-> dist_target -> distanceToTarget
-> dist_home -> distanceToHome

CBTService_UpdateEngageContext.cpp
-> blackBoardComp -> blackboardComp
-> dist_target -> distanceToTarget

CWorldSubsystem_CombatEngage.cpp
-> FEngageRequestContext & -> FEngageRequestContext&

CBTService_UpdateEngageContext.h
-> FEngageContext & -> FEngageContext&
```

### P1: public API rename 적용

다음 항목은 public C++ API rename이지만, 기존 이름이 bool getter 규칙과 맞지 않으므로 이번 브랜치에서 적용한다.

```text
CEnemy.h
-> GetbUsePatrol() -> ShouldUsePatrol()
-> GetbUseInvestigate() -> ShouldUseInvestigate()
-> GetbUseAlertStep() -> ShouldUseAlertStep()

호출부:
-> CAIController.cpp
```

`UFUNCTION`이 아니므로 Blueprint 직접 영향은 낮지만 public inline API이므로 P0 단발 수정과 별도 commit으로 처리한다.

### P1: Profiling helper API suffix 정리

`Core/Profiling` helper class는 class 이름이 profiling 책임을 이미 드러내므로 `ForProfiling` suffix를 제거하는 방향으로 통일한다.

```text
CAIAnimationProfiling
-> RecordAnimationRefreshAttemptForProfiling() -> RecordAnimationRefreshAttempt()
-> RecordAnimationRefreshExecutedForProfiling() -> RecordAnimationRefreshExecuted()
-> RecordAnimationRefreshSkippedForProfiling() -> RecordAnimationRefreshSkipped()

CAIBehaviorTreeProfiling
-> RecordUpdateAIContextTickForProfiling() -> RecordUpdateAIContextTick()
-> RecordUpdateAIIntentStateTickForProfiling() -> RecordUpdateAIIntentStateTick()
-> RecordUpdateEngageContextTickForProfiling() -> RecordUpdateEngageContextTick()
-> RecordAIIntentIntervalPresetForProfiling() -> RecordAIIntentIntervalPreset()

CAIStateRuntimeLODProfiling
-> RecordResolvedTierForProfiling() -> RecordResolvedTier()
```

`CCombatFeedbackProfiling`, `FCombatCollisionProfilingCounters`는 이미 suffix 없는 형태이므로 유지한다.

### P1: World subsystem structure file rename

`CWorldSubSystemStructure`는 파일명과 include 경로만 `SubSystem` 표기가 남아 있고, 내부 타입명에는 영향이 없다.
따라서 이번 브랜치에서 `CWorldSubsystemStructure`로 정리한다.

```text
Source/Portfolio/Type/CWorldSubSystemStructure.h
-> Source/Portfolio/Type/CWorldSubsystemStructure.h

Source/Portfolio/Type/CWorldSubSystemStructure.cpp
-> Source/Portfolio/Type/CWorldSubsystemStructure.cpp
```

---

## 4. 이번 브랜치 보류 범위

다음 항목은 네이밍 이슈로 보이더라도 변경 범위나 구조 영향이 크므로 이번 브랜치에서 처리하지 않는다.

```text
Comp vs Component 전면 통일
-> public getter, injected/cache member, local variable까지 광범위하게 영향
-> API 스타일 변경 작업으로 분리

Core/Profiling 밖 owner-side profiling wrapper 전면 통일
-> helper API와 owner-side wrapper 모두 이번 브랜치에서 suffix를 제거
-> profiling 책임은 `Core/Profiling` helper class 이름과 호출 섹션으로 표현

RequestAICombatSignalCue 같은 책임명 불일치 후보
-> 실제 책임 재분류가 필요할 수 있음
-> 동작 경계 검토 후 별도 commit 또는 후속 브랜치로 분리
```

---

## 5. 작업 순서

```text
1. P0 단발 네이밍 불일치 수정
2. rg 재검색으로 잔존 후보 확인
3. P1 GetbUse 계열을 ShouldUse 계열로 별도 commit 처리
4. P1 Core/Profiling helper API suffix를 별도 commit 처리
5. 추가 local snake_case 후보 정리
6. CWorldSubsystemStructure 파일명 / include 경로 정리
7. W05 문서 업데이트
8. git diff --check
9. C++ header/API 변경이 있으므로 PortfolioEditor Development 빌드
10. PR 문서 업데이트
```

---

## 6. 검증 기준

정적 확인:

```powershell
rg -n "inAxisValue|executorkey|dist_target|dist_home|blackBoardComp|FEngageContext &|FEngageRequestContext &" ..\Source\Portfolio --glob "*.h" --glob "*.cpp"
rg -n "GetbUse|Record[A-Za-z0-9]+ForProfiling|CWorldSubSystemStructure" ..\Source\Portfolio --glob "*.h" --glob "*.cpp"
git diff --check
```

C++ API 변경 시 빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" PortfolioEditor Win64 Development -Project="C:\UE5_Portfolio\Portfolio_UE5.4_verGit\Portfolio\Portfolio.uproject" -WaitMutex -FromMsBuild
```

---

## 7. PR 가능 조건

```yaml
PR 가능:
- 기능 동작 변경이 없다
- P0 네이밍 불일치가 제거됐다
- P1 public API rename과 profiling helper API suffix 정리가 문서 기준과 일치한다
- 보류 항목은 구조 영향이 큰 범위만 후속 범위로 남아 있다
- git diff --check가 통과했다
- Development 빌드를 확인했다

PR 보류:
- Blueprint / asset reference 위험이 있는 rename이 섞였다
- 파일명 / generated include / UHT 영향 rename이 빌드로 검증되지 않았다
- 네이밍 정리를 넘어 책임 재설계가 필요해졌다
```
