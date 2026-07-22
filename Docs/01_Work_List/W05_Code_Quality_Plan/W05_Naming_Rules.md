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
void UpdateContext(FAIContext& InOutAIContext);
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

local variable / injected member / cached member
-> 기존 프로젝트 관례상 Comp 허용
```

`GetMovementComp()`, `MovementComp_Injected` 같은 이름은 이미 광범위하게 사용되고 있으므로 단발 오타 정리 브랜치에서 전면 변경하지 않는다.

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

신규 profiling API는 `Record...ForProfiling()` 형태를 권장한다.

```cpp
static void RecordAnimationRefreshExecutedForProfiling();
```

단, 이미 counter class 이름으로 profiling 성격이 드러나는 기존 combat profiling API의 전면 rename은 별도 판단한다.

---

## 9. 보류 기준

다음 항목은 네이밍 문제로 보여도 단발 오타 정리로 처리하지 않는다.

```text
Comp vs Component 전면 통일
-> public getter, injected/cache member, local variable까지 광범위하게 영향

CWorldSubSystemStructure / SubSystem -> Subsystem
-> 파일명, generated include, UHT, include 경로 영향 가능

combat profiling API suffix 전면 통일
-> 기존 counter class와 호출부가 넓음

책임명 자체가 애매한 public API rename
-> 실제 책임 재분류가 필요할 수 있음
```
