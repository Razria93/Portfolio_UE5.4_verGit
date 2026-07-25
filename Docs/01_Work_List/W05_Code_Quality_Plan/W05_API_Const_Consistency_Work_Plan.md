# W05 API Const Consistency Work Plan

## 제목

**W05: ReadOnly API const 정합성 작업 계획**

## 날짜

**2026.07.24**

## 상태

- [x] ReadOnly API const 사용 규칙 정리
- [x] 적용 대상 / 제외 대상 기준 정리
- [x] 1차 후보 영역 분류
- [x] 코드 전수 감사
- [x] ReadOnly / Non-ReadOnly / 보류 후보 분류
- [x] ReadOnly API const 적용 1차
- [x] build 검증
- [ ] PIE 검증

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

## 3. 전수 감사 결과

감사 기준:

```text
검색 대상:
-> Get / Is / Has / Can / Should / Find / Resolve / Build / Make / Calculate / Compute 계열 선언

판정 기준:
-> ReadOnly 후보: owner member 상태를 바꾸지 않고, 결과를 조회 / 계산 / 조립만 하는 API
-> Non-ReadOnly: member mutation, cache cleanup, lazy creation, Blackboard write, subsystem request, audit 기록이 있는 API
-> 보류: UFUNCTION / delegate / override / Blueprint 노출 signature 또는 하위 호출 const 정합성 확인이 필요한 API
```

### 3.1 ReadOnly const 적용 후보

다음 항목은 현재 구현 기준으로 내부 상태를 바꾸지 않는 조회 / 조립 성격이므로 다음 const 적용 pass의 1차 후보로 둔다.

적용 상태:

```text
완료:
-> UCWeaponComponent::GetCurrentWeaponType
-> UCWeaponComponent::GetWeaponActor
-> UCCombatSignalTargetComponent::CanReceiveCombatSignal
-> UCBTService_UpdateAIContext::ComputeHomeMetricContext
-> UCBTService_UpdateAIContext::ComputeAlertRangeContext
-> UCBTService_UpdateAIContext::ComputeReactionContext
-> UCBTService_UpdateAIContext::ComputeDeadContext
-> ACAIController::SelectTopPriority

검증:
-> PortfolioEditor Win64 Development build 통과

남은 검증:
-> PIE smoke
```

### Component

```text
Source/Portfolio/Component/CWeaponComponent.h / .cpp
-> GetCurrentWeaponType
-> GetWeaponActor

판정:
-> CurrentWeaponType 또는 WeaponActor 유효성 확인 후 값을 반환한다.
-> member field 변경이 없으므로 ReadOnly const 후보로 둔다.
-> 1차 적용 완료.
```

### CombatSignal

```text
Source/Portfolio/Component/CCombatSignalTargetComponent.h / .cpp
-> CanReceiveCombatSignal

판정:
-> target context를 판정 / 채우지만 component owner 상태는 바꾸지 않는다.
-> DefenseComp_Injected->CanParry()가 const query로 유지되는지 함께 확인한 뒤 적용한다.
-> 1차 적용 완료.
```

### AI / BehaviorTree service

```text
Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.h / .cpp
-> ComputeHomeMetricContext
-> ComputeAlertRangeContext
-> ComputeReactionContext
-> ComputeDeadContext

판정:
-> service member 상태를 바꾸지 않고, 입력 pawn / blackboard / component에서 값을 읽어 context를 채운다.
-> debug / audit 기록이 없는 함수부터 1차 후보로 둔다.
-> 1차 적용 완료.
```

### AI / Controller

```text
Source/Portfolio/Controller/CAIController.h / .cpp
-> SelectTopPriority

판정:
-> TargetPerceptionStateMap을 조회해 top state를 Out parameter에 채운다.
-> 단독 함수 기준 member mutation은 없으므로 ReadOnly const 후보로 둔다.
-> 적용 시 loop variable을 const reference로 정리하고, BuildPerceptionContext와 분리해서 검증한다.
-> 1차 적용 완료.
```

### 이미 const 정합성이 높은 영역

```text
Source/Portfolio/Component/CActionOrchestratorComponent.h / .cpp
Source/Portfolio/Component/CReactionOrchestratorComponent.h / .cpp
Source/Portfolio/Component/CCombatSignalSourceComponent.h / .cpp
Source/Portfolio/Component/CCombatSignalTargetComponent.h / .cpp
Source/Portfolio/Action/CAction.h / .cpp
Source/Portfolio/Reaction/CReaction.h / .cpp
Source/Portfolio/System/Combat/CWorldSubsystem_CombatEngage.h / .cpp
Source/Portfolio/Type/*.h

판정:
-> Get / Is / Has / Can / Should / Resolve / Build 계열 상당수가 이미 const로 선언되어 있다.
-> 다음 pass에서는 신규 const 적용보다 signature 일치 / 누락 여부 확인 위주로 본다.
```

### 3.2 Non-ReadOnly / const 적용 제외

다음 항목은 이름상 ReadOnly처럼 보일 수 있으나 실제 구현에서 상태 변경이 있으므로 const 적용 대상에서 제외한다.

```text
Source/Portfolio/Component/CActionComponent.h / .cpp
-> ResolveActionExecutor
-> FindActionExecutor
-> BuildActionRuntimeMaps
-> BuildActionDataMap
-> BuildActionExecutorMap
-> AddActionExecutor

판정:
-> ResolveActionExecutor는 executor가 없으면 AddActionExecutor로 생성 / 캐시한다.
-> FindActionExecutor는 invalid cached executor를 ActionExecutorMap에서 Remove한다.
-> BuildAction* 계열은 ActionDataMap / ActionExecutorMap을 Reset / Add / update한다.

Source/Portfolio/Component/CReactionComponent.h / .cpp
-> ResolveReactionExecutor
-> FindReactionExecutor
-> BuildReactionRuntimeMaps
-> BuildReactionDataMap
-> BuildReactionExecutorMap
-> AddReactionExecutor

판정:
-> ResolveReactionExecutor는 executor가 없으면 AddReactionExecutor로 생성 / 캐시한다.
-> FindReactionExecutor는 invalid cached executor를 ReactionExecutorMap에서 Remove한다.
-> BuildReaction* 계열은 ReactionDataMap / ReactionExecutorMap을 Reset / Add / update한다.

Source/Portfolio/Controller/CAIController.h / .cpp
-> BuildPerceptionContext
-> UpdateTargetPerceptionStateMap
-> RefreshRuntimeLODTierFromBlackboard

판정:
-> BuildPerceptionContext는 UpdateTargetPerceptionStateMap을 호출한다.
-> UpdateTargetPerceptionStateMap은 TargetPerceptionStateMap 값을 갱신 / 제거하고 audit을 기록한다.
-> RefreshRuntimeLODTierFromBlackboard는 CurrentRuntimeLODTier를 변경한다.

Source/Portfolio/Character/CAnimInstance.h / .cpp
-> ShouldRefreshAnimationParameters

판정:
-> RuntimeLODAnimationRefreshElapsed를 갱신하고 animation refresh audit을 기록한다.
-> Should prefix지만 runtime state gate 함수이므로 const 대상이 아니다.

Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateAIContext.h / .cpp
-> BuildPerceptionContext
-> ComputeEngageAssignmentContext

판정:
-> BuildPerceptionContext는 ACAIController::BuildPerceptionContext를 통해 perception state map 갱신과 audit 기록을 수행한다.
-> ComputeEngageAssignmentContext는 CombatEngage subsystem에 SubmitRequest를 보내고 AIController audit을 기록한다.

Source/Portfolio/AI/BehaviorTree/Service/CBTService_UpdateEngageContext.h / .cpp
-> BuildEngageContext
-> ComputeEngageContext

판정:
-> BuildEngageContext는 실패 경로에서 FAICombatBTDebug audit 기록을 수행한다.
-> ComputeEngageContext는 context 계산 외에 FAICombatBTDebug audit 기록을 수행한다.
-> audit 기록을 ReadOnly 제외 조건으로 유지하는 한 const 적용 대상에서 제외한다.
```

### 3.3 보류 / 별도 검증 후보

다음 항목은 의미상 조회 / 판정 API일 수 있으나, signature 리스크 또는 하위 호출 정합성 때문에 이번 후보표에서는 보류한다.

```text
UFUNCTION / Blueprint 노출 API
DECLARE_DYNAMIC... delegate signature
engine override
AnimNotify / BehaviorTree override
TakeDamage override

판정:
-> 1차 const 적용 pass에서 제외한다.
-> 필요한 경우 Blueprint compile / Editor load / PIE smoke를 포함한 별도 pass에서 처리한다.

Source/Portfolio/Component/CActionComponent.h / .cpp
-> ResolveActionData

Source/Portfolio/Component/CReactionComponent.h / .cpp
-> ResolveReactionData

판정:
-> map 조회와 result 채움 자체는 ReadOnly 성격이다.
-> 실패 / 성공 경로에서 debug audit 기록을 수행하므로, audit 기록을 ReadOnly 제외 조건으로 유지할지 먼저 결정한 뒤 적용한다.

Source/Portfolio/Component/CActionComponent.h / .cpp
-> BuildActionExecutorReferences

Source/Portfolio/Component/CReactionComponent.h / .cpp
-> BuildReactionExecutorReferences

Source/Portfolio/Character/Player/CPlayer.h / .cpp
-> BuildReferences

Source/Portfolio/Character/Enemy/CEnemy.h / .cpp
-> BuildReferences

판정:
-> 함수 본문은 component pointer graph를 조립하는 읽기성 함수다.
-> 다만 OutReferences에 non-const owner / component pointer를 담는 계약이므로 const 적용 시 this pointer 변환과 참조 구조체 계약 전파를 별도로 검토한다.

Source/Portfolio/AI/RuntimeLOD/*
Source/Portfolio/AI/Blackboard/*
Source/Portfolio/Core/Debug/*
Source/Portfolio/Core/Profiling/*

판정:
-> static policy / helper 함수가 많아 member const 적용 범위와 다르다.
-> ShouldAudit / Get CVar 계열은 유지하고, Record / Print / Report 계열은 const 후보에서 제외한다.
```

### 3.4 1차 적용 후 잔여 재스캔

```text
재스캔 기준:
-> Get / Is / Has / Can / Should / Find / Resolve / Build / Make / Calculate / Compute 계열 header 선언

결과:
-> 명확한 member ReadOnly API const 후보는 1차 적용 완료.
-> 남은 member 함수는 상태 변경, audit / profiling 기록, cache mutation, lazy 생성, Blueprint / override signature, 또는 현재 비-const로 남긴 pointer graph 보류 항목이다.
-> static policy / helper 함수는 member const 대상이 아니므로 이번 pass에서 제외한다.
-> CMovementComponent::CalculateSpeed / CalculateDirection은 movement runtime state 계산 / 갱신 흐름으로 보아 ReadOnly API가 아니다.

다음 조건 전까지 추가 const 적용은 보류:
-> audit 기록 함수를 ReadOnly 예외로 허용할지 정책 변경
-> 현재 비-const FCharacterComponentReferences builder의 pointer graph 계약 변경
-> Blueprint / override signature를 별도 pass에서 검증하기로 결정
```

### 3.5 ReadOnly Param const 적용 1차 재스캔

ReadOnly Param const 적용 1차는 다음 기준으로 재스캔했다.

```text
검색 기준:
-> Component / Action / Reaction / Weapon / CombatSignal / AI / System / Core
-> FString / FText / TArray / TMap / project FStruct by-value input
-> UFUNCTION / delegate / override / Blueprint 노출 signature 제외
-> FName / enum / scalar 값 전달은 이번 pass에서 const& 강제 대상에서 제외

결과:
-> 1차 범위에서 새로 const&로 바꿀 대형 by-value 입력 후보는 발견되지 않았다.
-> TArray / TMap / FString / FText 계열 입력은 이미 const&이거나 Out / InOut 참조로 분류되어 있다.
-> project FStruct 입력은 대부분 이미 const&이며, FDamageEvent는 UE TakeDamage 계열 signature와 CombatSignal damage forwarding 경로라 현 상태를 유지한다.
-> FAIStimulus는 perception UFUNCTION callback signature이므로 제외한다.
-> FName 입력은 작은 값 타입이고 현재 delegate / notify / trigger / collision key 경로에 넓게 사용되므로 이번 pass에서 변경하지 않는다.

판정:
-> 이번 pass는 코드 signature 변경 없이 감사 완료로 처리한다.
-> ReadOnly Param 정책 위반으로 볼 명확한 내부/private helper 후보는 남기지 않았다.
-> 다음 pass는 parameter const가 아니라 ReadOnly member function const 누락 / 보류 후보 재검증 중심으로 진행한다.
```

### 3.6 local const / Others 전수 검토

`const` 사용 현황을 다음 기준으로 재분류했다.

```text
ReadOnly API
-> 함수 뒤 const: Foo(...) const

ReadOnly Param
-> parameter 내부 const: const FType& / const UObject* / const TCHAR*

Others
-> 위 둘이 아닌 const: local const, static / namespace const, pointer const form 등
```

전체 현황:

```text
ReadOnly API   : 731
ReadOnly Param : 1073
Others         : 415
const 포함 라인 : 1902
```

`Others` 세부 분류:

```text
132  Local const struct/value snapshot
77   Local const bool gate
52   Local const scalar
47   Static/namespace constant
42   Local const enum
32   Local pointer-to-const UObject/Actor
18   Local const UE value
6    Pointer const form
4    Container of pointer-to-const
2    Local const template value
2    Static local container cache
1    Unclassified
```

검토 결론:

```text
허용 / 권장:
-> Static / namespace constant
-> Static local lookup table
-> 의미 있는 struct / value snapshot
-> UE value snapshot
-> Container of pointer-to-const
-> const TCHAR* reason / literal

선별 허용:
-> local bool gate
-> local enum
-> local scalar
-> snapshot / 판단 기준 / 이전 상태 / 복잡한 조건식 이름 부여일 때만 유지

지양:
-> 모든 local bool / int / enum에 기계적으로 const 붙이기
-> UObject / AActor pointer-to-const 대량 적용
-> AActor* const 같은 포인터 자체 const 남용
```

현재 프로젝트 판정:

```text
-> local const는 전면 제거 대상이 아니다.
-> 기존 Others 대부분은 snapshot / branch gate / static key / debug reason / lookup table 성격이라 유지 가능하다.
-> local bool / scalar / enum 중 의미 없는 단순 임시값만 선별 정리한다.
-> UObject / AActor pointer-to-const는 새로 대량 적용하지 않는다.
```

선별 cleanup 결과:

```text
정리 완료:
-> 단순 bool result 임시값: bStarted / bReserved / bRequested / bApplied
-> Notify trigger 비교용 단순 active type / index 임시값
-> Investigate index 증가용 단순 int32 임시값

유지:
-> previous state / threshold / cooldown / latency / match tier
-> guard phase / reaction type / execution state snapshot
-> 복잡한 조건식에 이름을 붙인 branch gate
-> static / namespace constant
-> UObject / AActor pointer-to-const 기존 사용

판정:
-> local const 정책 검증용 선별 정리만 수행했다.
-> 의미가 있는 snapshot / gate까지 기계적으로 제거하지 않는다.
```

### 3.7 ReadOnly member function const 보류 후보 재검증

ReadOnly member function const 잔여 후보를 다음 기준으로 재스캔했다.

```text
검색 기준:
-> Get / Is / Has / Can / Should / Find / Resolve / Build / Make / Calculate / Compute 계열 header 선언
-> const가 붙지 않은 member function 후보
-> static function / free function / override / UFUNCTION / delegate signature 제외

재스캔 결과:
-> 명확한 신규 ReadOnly member function const 적용 후보는 발견되지 않았다.
-> static policy / debug / profiling helper는 member const 적용 대상이 아니다.
-> free function helper는 member const 적용 대상이 아니다.
-> CAnimInstance::ShouldRefreshAnimationParameters는 RuntimeLODAnimationRefreshElapsed를 갱신하고 profiling / audit 조건에 걸리므로 Non-ReadOnly로 유지한다.
-> UCActionComponent::ResolveActionData / UCReactionComponent::ResolveReactionData는 data map 조회 자체는 ReadOnly 성격이지만 실패 경로에서 audit 기록을 수행하므로 현재 정책에서는 보류한다.
-> UCActionComponent::ResolveActionExecutor / UCReactionComponent::ResolveReactionExecutor는 executor lazy 생성 / cache mutation 경로가 있으므로 Non-ReadOnly로 유지한다.
-> UCActionComponent::FindActionExecutor / UCReactionComponent::FindReactionExecutor는 invalid cached executor 제거 경로가 있으므로 Non-ReadOnly로 유지한다.
-> UCActionComponent::BuildActionExecutorReferences / UCReactionComponent::BuildReactionExecutorReferences는 non-const FCharacterComponentReferences pointer graph를 조립하므로 현재 계약에서는 보류한다.
-> ACPlayer::BuildReferences / ACEnemy::BuildReferences는 non-const owner / component pointer graph를 OutReferences에 채우므로 현재 계약에서는 보류한다.

판정:
-> 이번 pass는 코드 signature 변경 없이 감사 완료로 처리한다.
-> 추가 const 적용은 audit 기록 정책 변경, FCharacterComponentReferences pointer graph 계약 변경, Blueprint / override signature 별도 검증 결정 전까지 보류한다.
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
