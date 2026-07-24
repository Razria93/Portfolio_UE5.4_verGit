# W05 API Const Consistency Work Plan

## 제목

**W05: ReadOnly API const 정합성 작업 계획**

## 날짜

**2026.07.24**

## 상태

- [x] ReadOnly API const 사용 규칙 정리
- [x] 적용 대상 / 제외 대상 기준 정리
- [x] 1차 후보 영역 분류
- [ ] 코드 전수 감사
- [ ] ReadOnly API const 적용
- [ ] build / PIE 검증

---

## 1. 목적

이 문서는 `refactor/api-const-consistency` 작업에서 처리할 실제 후보와 보류 기준을 기록한다.

규칙 본문은 `W05_Naming_Rules.md`의 `ReadOnly API const 사용` 섹션을 따른다. 이 문서는 현재 프로젝트 코드 기준의 후보 목록과 적용 순서를 관리한다.

목표는 모든 함수에 기계적으로 `const`를 붙이는 것이 아니다. 외부에서 내부 상태를 조회하는 ReadOnly API에만 `const`를 붙여, 호출자가 해당 API를 상태 변경 없는 조회로 믿을 수 있게 만드는 것이다.

---

## 2. 적용 원칙

### 2.1 1차 적용 범위

```text
대상:
-> public / protected / private member API 중 조회 / 판정 / 계산 성격이 명확한 함수
-> Get / Is / Has / Can / Should / Find / Resolve / Build / Calculate / Compute 계열
-> owner 상태를 바꾸지 않는 Out / InOut result 채움 함수

제외:
-> local variable const cleanup
-> parameter const 전면 정리
-> return type const 전면 정리
-> UFUNCTION / Blueprint 노출 API signature
-> delegate signature
-> engine override signature
-> UPROPERTY / serialized USTRUCT field
```

### 2.2 ReadOnly 탈락 조건

다음 중 하나라도 있으면 1차 const 적용 대상에서 제외한다.

```text
-> member field 변경
-> member TArray / TMap add / remove / update
-> _Cached 값 갱신
-> Blackboard Set / Clear
-> owned component / actor / subsystem 상태 변경
-> delegate broadcast / bind / unbind
-> timer start / stop
-> montage play / stop
-> collision / movement 변경
-> gameplay request 전송
-> debug / audit / profiling 기록
-> lazy initialization
-> component lookup 결과 cache
-> const_cast 필요
```

---

## 3. 1차 감사 후보

### Component

```text
Source/Portfolio/Component/CActionComponent.h / .cpp
-> ResolveActionData
-> ResolveActionExecutor
-> Find / Get / Can / Has 계열 조회 API

Source/Portfolio/Component/CReactionComponent.h / .cpp
-> ResolveReactionData
-> ResolveReactionExecutor
-> Find / Get / Can / Has 계열 조회 API

Source/Portfolio/Component/CActionOrchestratorComponent.h / .cpp
-> BuildDecisionQuery
-> BuildDecisionResult
-> BuildActionExecutionResult
-> Can / Should / Resolve 계열 중 상태 변경 없는 함수

Source/Portfolio/Component/CReactionOrchestratorComponent.h / .cpp
-> BuildDecisionQuery
-> BuildDecisionResult
-> BuildReactionExecutionResult
-> Can / Should / Resolve 계열 중 상태 변경 없는 함수
```

### Action / Reaction

```text
Source/Portfolio/Action/CAction.h / .cpp
-> Is / Has / Can 계열 조회 API
-> BuildFeedbackRequest
-> ResolveExecutionDecision 계열 override는 base signature 기준 확인 후 처리

Source/Portfolio/Reaction/CReaction.h / .cpp
-> Is / Has / Can 계열 조회 API
-> BuildFeedbackRequest
-> ResolveExecutionDecision 계열 override는 base signature 기준 확인 후 처리
```

### CombatSignal

```text
Source/Portfolio/Component/CCombatSignalSourceComponent.h / .cpp
-> Validate / Build / Resolve / IsDuplicateHit / IsFriendlyTarget 계열
-> Request / Send / Cache / Commit 계열은 제외

Source/Portfolio/Component/CCombatSignalTargetComponent.h / .cpp
-> Validate / Build / CanReceiveCombatSignal / Compute 계열
-> Handle / Commit / Dispatch / Request 계열은 제외
```

### AI / Controller

```text
Source/Portfolio/Controller/CAIController.h / .cpp
-> Get / Has / Build / Select 계열 조회 API
-> Blackboard write, perception state map update, audit record 함수는 제외

Source/Portfolio/AI/RuntimeLOD/*
-> policy / resolver의 read-only query 함수 우선 확인

Source/Portfolio/AI/Blackboard/*
-> key registry / value helper 중 write helper와 read helper 분리 확인
```

### Weapon / Character

```text
Source/Portfolio/Component/CWeaponComponent.h / .cpp
-> GetCurrentWeaponType
-> GetWeaponActor
-> BuildWeaponContext

Source/Portfolio/Weapon/CWeaponActor.h / .cpp
-> GetLastOverlapContext
-> GetLastWeaponContext
-> GetLastActionDataKey
-> BuildHitContext / BuildOverlapContext 계열

Source/Portfolio/Character/Player/CPlayer.h / .cpp
Source/Portfolio/Character/Enemy/CEnemy.h / .cpp
-> Get component accessor 계열
-> Handle / TakeDamage / delegate callback 계열은 제외
```

### Core Debug / Profiling

```text
Source/Portfolio/Core/Debug/*
Source/Portfolio/Core/Profiling/*
-> ShouldAudit / ShouldPrint / Get CVar 계열만 후보
-> Record / Print / Report 계열은 audit / debug / profiling 기록이므로 제외
```

---

## 4. 보류 / 제외 후보

### UHT / Blueprint / delegate / override

```text
UFUNCTION
BlueprintCallable
BlueprintPure
BlueprintNativeEvent
BlueprintImplementableEvent
DECLARE_DYNAMIC...
engine override
AnimNotify / BehaviorTree override
TakeDamage override
```

판정:
-> signature 변경 위험이 있으므로 1차 const pass에서 제외한다.
-> 필요한 경우 별도 검증 pass에서 Blueprint compile / Editor load / PIE smoke와 함께 처리한다.

### 상태 변경 이름 계열

```text
Set...
Update...
Reset...
Clear...
Initialize...
Inject...
Recover...
Cache...
Refresh...
Record...
Report...
Request...
Submit...
Handle...
Apply...
Execute...
Start...
Stop...
Tick...
Commit...
Dispatch...
Broadcast...
```

판정:
-> 기본적으로 ReadOnly API가 아니므로 const 후보에서 제외한다.
-> 이름과 실제 역할이 충돌하면 const 적용이 아니라 이름 / 책임 재검토 후보로 분류한다.

### local const cleanup

```text
Type* const local
const float localValue
const FActionRequestResult result
for (const FType& item : Items)
```

판정:
-> 유효한 code hygiene 후보지만 이번 ReadOnly API const pass와 분리한다.
-> 필요하면 `refactor/local-const-cleanup` 별도 pass에서 처리한다.

---

## 5. 감사 명령

### ReadOnly 후보 선언 조회

```powershell
rg -n "\b(Get|Is|Has|Can|Should|Find|Resolve|Build|Make|Calculate|Compute)[A-Z]\w*\s*\([^;{]*\)\s*;" Source/Portfolio --glob "*.h"
```

### 이미 const가 붙은 후보 기준선 확인

```powershell
rg -n "\b(Get|Is|Has|Can|Should|Find|Resolve|Build|Calculate|Compute)[A-Z]\w*\s*\([^;{]*\)\s*const" Source/Portfolio --glob "*.h" --glob "*.cpp"
```

### const 제외 후보 조회

```powershell
rg -n "\b(Set|Update|Reset|Clear|Initialize|Inject|Recover|Cache|Refresh|Record|Report|Request|Submit|Handle|Apply|Execute|Start|Stop|Tick|Commit|Dispatch|Broadcast)\w*\s*\(" Source/Portfolio --glob "*.h" --glob "*.cpp"
```

### 위험 신호 조회

```powershell
rg -n "\bmutable\b|const_cast<" Source/Portfolio --glob "*.h" --glob "*.cpp"
```

---

## 6. 검증 기준

```text
1. 후보 함수 선언 / 정의 const 일치 확인
2. const 적용 함수 내부에서 member mutation 없음 확인
3. const 적용 함수가 호출하는 하위 함수 const 호출 가능 여부 확인
4. UFUNCTION / delegate / override signature 변경 없음 확인
5. git diff --check
6. PortfolioEditor Win64 Development build
7. PIE smoke
```

Blueprint-facing API 또는 UHT signature를 건드린 경우에는 다음을 추가한다.

```text
Editor load
Blueprint compile
PIE log에서 Error / Fatal / Ensure / Blueprint compile 실패 여부 확인
```
