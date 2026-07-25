# W05 Naming Rules

## 제목

**W05: 네이밍 규칙**

## 날짜

**2026.07.23**

## 상태

- [x] Unreal C++ 관례와 프로젝트 기존 관례 기준으로 초안 작성
- [x] 네이밍 / 오타 / API 정리 브랜치에서 우선 적용

---

## 1. 목적

이 문서는 W05 코드 품질 정리에서 반복 적용할 네이밍 기준을 고정한다.

목표는 프로젝트 전체 이름을 새 체계로 한 번에 바꾸는 것이 아니다. Unreal C++ 관례와 현재 프로젝트에 이미 자리 잡은 강한 관례를 기준으로, 단발성 오타 / 표기 흔들림 / 매개변수명 불일치를 판단하기 위한 기준을 둔다.

---

## 2. Public Type / API

class, struct, enum, public function은 Unreal C++ 관례에 맞춰 `PascalCase`를 사용한다.

```cpp
class UCActionComponent;
struct FActionExecutionContext;
enum class EActionRequestResultType;

bool CanResolveAction(...) const;
FActionRequestResult HandleCombatAction(...);
```

---

## 3. 매개변수

입력 매개변수는 `In`, 출력 매개변수는 `Out`, 입출력 매개변수는 `InOut` prefix를 사용한다.

```cpp
void BuildContext(const FCombatSignalTargetPayload& InPayload, FCombatSignalTargetContext& OutContext);
void UpdateContext(FAIBlackboardUpdateContext& InOutAIContext);
```

참조 표기는 `FType& Value`, `const FType& Value` 형태로 통일한다.

```cpp
void SubmitRequest(const FEngageRequestContext& InEngageRequestContext);
EContextBuildResult BuildEngageContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FEngageContext& OutEngageContext);
```

---

## 4. 지역 변수

지역 변수는 `lowerCamelCase`를 사용한다.

```cpp
UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
const float distanceToTarget = FVector::Dist(ownerLocation, targetLocation);
```

`Blackboard`는 하나의 단어로 취급한다.

```cpp
// Good
UBlackboardComponent* blackboardComp;

// Avoid
UBlackboardComponent* blackBoardComp;
```

---

## 5. bool 이름

bool 변수는 Unreal 관례대로 `bPascalCase`를 사용한다.

```cpp
bool bCanMove = true;
bool bHasLOS = false;
```

bool query 함수는 `Is`, `Has`, `Can`, `Should` 계열을 사용한다.

```cpp
bool IsActive() const;
bool HasTarget() const;
bool CanMove() const;
bool ShouldAuditCombatSignal();
```

`Getb...` 형태는 신규 API에서 사용하지 않는다.

```cpp
// Prefer
bool ShouldUsePatrol() const;

// Avoid
bool GetbUsePatrol() const;
```

---

## 6. 내부 참조 suffix

DI로 주입된 참조는 `_Injected`, runtime cache는 `_Cached` suffix를 유지한다.

```cpp
UCActionComponent* ActionComp_Injected = nullptr;
APawn* ControlledPawn_Cached = nullptr;
```

이 suffix는 현재 프로젝트에서 역할을 명확히 드러내는 강한 관례이므로 W05 범위에서 바꾸지 않는다.

---

## 7. Component / Comp 표기

`Component`와 `Comp`는 다음 기준으로 구분한다.

```text
public API / UPROPERTY member
-> 가능하면 Component 사용
-> Unreal engine API / parent class API와 이름 충돌 가능성이 있으면 Comp 허용

local variable / injected member / cached member
-> 기존 프로젝트 관례상 Comp 허용
```

`GetMovementComp()`, `MovementComp_Injected` 같은 이름은 이미 광범위하게 사용되고 있고, 일부 component 계열 public API는 engine / parent API와 충돌을 피하기 위한 회피 네이밍일 수 있다. 따라서 단발 오타 정리 브랜치에서 전면 변경하지 않는다.

---

## 8. Debug / Profiling API

Debug helper는 기존 규칙을 유지한다.

```cpp
static bool ShouldAuditActionComponent();
static bool ShouldPrintActionComponentDebug();
static void RecordActionDecisionRejectedForAudit(...);
static void PrintActionExecutionContextDebug(...);
static void ReportActionNotifyTriggerWarning(...);
```

Profiling helper는 class / namespace가 이미 profiling 책임을 드러내므로 함수명에 `ForProfiling` suffix를 반복하지 않는다.

```cpp
static void RecordAnimationRefreshExecuted();
static void RecordUpdateAIContextTick();
static void RecordResolvedTier(EAIRuntimeLODTier InTier);
```

`Core/Profiling` 밖의 owner class가 profiling event를 별도 wrapper로 분리해야 할 때도 가능하면 suffix 없는 `Record...` 형태를 사용한다. profiling 책임은 호출 위치의 섹션명과 `Core/Profiling` helper class 이름으로 표현한다.

---

## 9. 보류 기준

다음 항목은 네이밍 문제로 보여도 단발 오타 정리로 처리하지 않는다.

```text
Comp vs Component 전면 통일
-> public getter, injected/cache member, local variable까지 광범위하게 영향

책임명 자체가 애매한 public API rename
-> 실제 책임 재분류가 필요할 수 있음
```

---

## 10. ReadOnly API const 사용

`const` member function은 외부에서 내부 상태를 조회하는 ReadOnly API에만 사용한다.

ReadOnly API는 호출해도 다음 상태가 바뀌지 않는 함수다.

```text
- this 객체의 member field
- _Cached / _Injected / runtime state
- owned component / actor / subsystem 상태
- Blackboard 값
- delegate / timer / montage / collision / movement 상태
- debug / audit / profiling counter
- lazy cache / lookup repair 결과
```

즉 `const`는 단순히 C++ 문법상 붙일 수 있다는 표시가 아니라, 호출자가 이 API를 상태 변경 없는 조회로 믿어도 된다는 계약이다.

다음 이름 계열은 우선 ReadOnly 후보로 본다.

```text
Get...
Is...
Has...
Can...
Should...
Find...
Resolve...
Build...
Make...
Calculate...
Compute...
```

단, 이름만으로 확정하지 않는다. 본문을 확인해서 내부 상태 변경이 없을 때만 `const`를 붙인다.

```cpp
bool IsActive() const;
bool CanMove() const;
EActionType GetActiveActionType() const;
FExecutionDecisionResult ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const;
FActionFeedbackRequest BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey = NAME_None) const;
```

출력 매개변수를 채우는 함수도 owner 상태를 바꾸지 않는다면 `const` 대상이다.

```cpp
bool ResolveActionData(const FActionDataKey& InKey, FActionData& OutData) const;
```

`InOut` 매개변수를 수정하더라도 caller-owned 임시 계산값을 보정하는 함수이고 owner 상태를 바꾸지 않으면 `const`를 허용한다.

```cpp
float ComputeMitigatedDamage(FCombatSignalTargetContext& InOutContext) const;
```

다음 동작 중 하나라도 수행하면 ReadOnly API가 아니므로 `const`를 붙이지 않는다.

```text
- member field 대입
- TArray / TMap member add / remove / update
- _Cached 값 갱신
- Blackboard Set / Clear
- component / actor / subsystem 상태 변경
- delegate broadcast / bind / unbind
- timer start / stop
- montage play / stop
- collision / movement 변경
- gameplay request 전송
- debug / audit / profiling 기록
- lazy initialization
- component lookup 결과 cache
- const_cast 사용 필요
```

이름 계열별 판단 기준은 다음과 같다.

```text
Get
-> 읽고 반환하면 const
-> cache init / repair / lazy load가 있으면 non-const

Is / Has
-> predicate only면 const
-> 상태 갱신 후 판단하면 non-const

Can / Should
-> 정책 판단만 하면 const
-> cooldown 소비, attempt 기록, audit counter 기록이면 non-const

Build / Make / Calculate / Compute
-> transient value 생성이면 const
-> object 등록, spawn, cache 저장, 입력 object mutation이면 non-const

Resolve / Find
-> lookup / select / derive면 const
-> reserve / consume / commit / find-or-add / repair면 non-const
```

Unreal / UHT 경계는 별도 주의한다.

```text
UFUNCTION / Blueprint 노출 API
-> 일괄 변경하지 않는다. Blueprint pin / generated binding 영향 확인 후 별도 처리한다.

delegate signature
-> 일괄 변경하지 않는다. 바인딩 함수와 정확히 맞아야 한다.

override
-> 부모 signature 기준을 따른다. 임의로 const를 추가하지 않는다.

UPROPERTY / serialized USTRUCT field
-> const 정리 대상이 아니다.

UObject pointer
-> const AActor* 같은 pointer-to-const 대량 적용을 하지 않는다.
-> ReadOnly API const 정리와 local pointer const 정리는 별도 pass로 분리한다.
```

### 10.1 내부 local const 사용

local variable의 `const`는 기본값으로 강제하지 않는다.

프로젝트에서 `const`의 1차 용도는 다음 두 가지다.

```text
ReadOnly API
-> owner 상태를 바꾸지 않는 member function 계약

ReadOnly Param
-> caller-owned 입력값을 수정하지 않는 parameter 계약
```

다만 함수 내부 local value에서도 값의 의미를 고정하는 경우에는 `const`를 허용한다. 이때 기준은 “값이 바뀌지 않으므로 const를 붙인다”가 아니라, “이 값은 이 시점의 snapshot / 판단 기준 / 내부 상수라서 이후 코드에서 바뀌면 오해나 버그가 된다”이다.

허용 / 권장:

```text
파일 내부 상수
-> namespace 내부 const FName / static constexpr / static const
-> tag, key, collision profile name, magic number 대체값

static local lookup table
-> 함수 내부에서 한 번 구성되고 읽기 전용으로 쓰이는 static const TArray / TMap

의미 있는 snapshot
-> 이전 상태
-> trace / overlap 시점의 위치, 회전, hit result
-> 이후 로직의 기준점이 되는 context / payload / result 복사본

문자열 literal reason
-> const TCHAR* reason = TEXT("Cooldown");
```

선별 허용:

```text
local bool
-> 복잡한 조건식에 이름을 붙이는 gate면 허용
-> 단순 null check / 한 줄 임시값이면 생략 가능

local enum / scalar
-> 이전 상태, 시작 시점 값, 계산 기준이면 허용
-> 단순 중간 계산값이면 생략 가능

UE value type
-> FVector / FRotator / FTransform / FName / FString 등은 snapshot 의미가 있으면 허용
-> 계속 보정 / 누적 / 수정할 값이면 non-const
```

지양:

```text
모든 local bool / int / enum에 기계적으로 const 붙이기
-> 코드가 시끄러워지고 의미 신호가 약해진다.

UObject / AActor pointer-to-const 대량 적용
-> Unreal API 호환성과 reflection 관례를 우선한다.
-> 읽기 전용 계약은 우선 API const와 const parameter에서 표현한다.

포인터 자체 const 남용
-> AActor* const Target 같은 형태는 로컬 재대입만 막는다.
-> 객체 불변성이 아니므로 일반 정책으로 강제하지 않는다.
```
