# W05 Type Header Organization Rules

## 제목

**W05: 구조체 나누기 / Type 헤더 배치 규칙**

## 날짜

**2026.07.23**

## 상태

- [x] Type 헤더 분류 원칙 정리
- [x] UHT / BlueprintType 이동 위험 기준 정리
- [x] forward declaration 허용 / 금지 기준 정리
- [x] enum / struct 배치 및 내부 구성 규칙 정리

---

## 1. 목적

이 문서는 `Source/Portfolio/Type` 아래 공용 Type 헤더를 어떤 책임 단위로 나누고, 각 헤더 안에서 enum과 struct를 어떤 순서로 배치할지 정의한다.

목표는 타입 이름을 한 번에 바꾸는 것이 아니다. 현재 asset / Blueprint / UHT와 연결된 타입 이름, `UPROPERTY` 이름, enum entry를 유지하면서 헤더 include 파급을 줄이고 책임 단위로 파일을 분리하는 것이다.

구체적인 프로젝트 진단, stale 분류, 제거 / 이동 / rename 후보는 Work Plan 문서에 둔다. 이 문서는 반복 적용 가능한 판단 규칙만 유지한다.

---

## 2. 고정 조건

이번 작업에서는 다음 항목을 변경하지 않는다.

```text
- 타입명
- USTRUCT / UENUM 이름
- UPROPERTY 이름
- enum entry 이름
- enum entry 값
- BlueprintType 여부
```

파일명과 include 구조는 변경할 수 있다. 단, 파일 이동 뒤에도 serialized asset identity가 깨지지 않도록 타입명과 필드명은 유지한다.

---

## 3. Type 헤더 분류 원칙

Type 헤더는 파일 이름이 아니라 실제 책임으로 나눈다.

```text
Identity
-> 도메인의 정체성을 나타내는 기본 enum / identifier / lightweight context

Data / Config
-> DataAsset, component config, editor 입력값처럼 저장 가능한 설정 데이터
-> runtime cache나 frame/tick 중 갱신되는 값에는 사용하지 않는다.

Runtime State
-> tick / frame / phase를 거쳐 누적되거나 갱신되는 runtime cache / state

Runtime Context
-> 실행 / 판정 / 요청 흐름 중 계산되어 전달되는 transient 묶음

Request
-> 외부에서 시스템 경계로 넣는 명령 / 의도
-> 해석 결과나 처리 결과에는 사용하지 않는다.

Payload
-> 시스템 경계를 넘는 입력 원본 또는 정규화된 입력 묶음

Resolution / Resolve Result
-> notify / tag / config lookup 같은 해석 결과

Result
-> 처리 결과, 성공 / 실패 / 거부 사유, output state

Packet
-> Payload + Context + Result처럼 dispatch 경계를 넘는 wrapper / aggregate

Key / Runtime Key / Playback Key
-> lookup / match / dedupe identity
-> RuntimeKey / ExecutionKey / PlaybackKey는 runtime dedupe, active instance, effect playback 생명주기를 나타낸다.

Feedback / Presentation
-> VFX / SFX / hit stop / camera shake처럼 표현 계층으로 전달되는 요청 또는 설정

Audit / Debug / Profiling State
-> gameplay shared Type 헤더에 섞지 않고 Core/Debug, Core/Profiling 또는 해당 helper 소유로 분리할 수 있는 진단 전용 상태
```

`Info` 접미사는 가급적 금지한다. 실제 의미가 Context / State / Spec / Result / Snapshot 중 하나라면 해당 이름을 사용한다.

### 3.1 Action / Reaction 타입 헤더 책임

Action / Reaction 계열은 대칭 가능한 부분과 구조적으로 다른 부분을 분리해서 판단한다.

기본 책임은 다음과 같이 고정한다.

```text
*Types.h
-> 도메인 enum 중심
-> 필요하면 enum helper만 포함
-> lookup key, editor data, execution context는 기본적으로 두지 않는다.

*KeyTypes.h
-> lookup / match identity key
-> 해당 key의 GetTypeHash / equality / key helper
-> key만 필요한 사용처가 DataTypes 의존을 끌어오지 않도록 분리한다.

*DataTypes.h
-> DataAsset / component config / editor 입력 데이터
-> resolved execution context

*OrchestrationTypes.h
-> intent
-> candidate
-> request
-> resolution / resolve result
-> request result / execution result

ExecutionTypes.h / ExecutionRuleTypes.h
-> Action / Reaction 공통 실행 판단, participant, intervention, rule 모델
```

Action / Reaction은 다음 영역에서 대칭을 유지한다.

```text
ActionTypes / ReactionTypes
-> enum 중심

ActionKeyTypes / ReactionKeyTypes
-> lookup / match key

ActionDataTypes / ReactionDataTypes
-> editor config data + execution context

ActionOrchestrationTypes / ReactionOrchestrationTypes
-> candidate / request / result / execution result
```

다음 차이는 구조적으로 허용한다.

```text
Action
-> weapon / hit source가 될 수 있다.
-> hit context로 action identity를 전달할 수 있다.

Reaction
-> damage / combat result consumer다.
-> ReactionDataKey가 DamageSpecKey에 의존할 수 있다.
```

단순 대칭만을 위해 대응 타입을 만들지 않는다. 예를 들어 action hit source identity가 있다고 해서 `FReactionContext` 같은 넓은 이름의 타입을 추가하지 않는다. Context가 필요하면 `ExecutionContext`, `RequestContext`, `HitSourceContext`처럼 목적을 이름에 포함한다.

예외:

```text
reserved scaffold
-> 미사용 scaffold는 기본적으로 유지하지 않는다.
-> 단, 코드 품질 정리 직후 바로 구현할 명확한 pipeline / feature 계획이 있으면 reserved scaffold로 둘 수 있다.
-> reserved scaffold는 Work Plan에 보류 사유, 연결될 후속 작업, 재평가 조건을 기록해야 한다.
-> reserved scaffold는 제거 후보나 stale 타입으로 분류하지 않는다.
-> reserved scaffold는 실제 사용처가 생기면 일반 분류 원칙에 맞춰 Context / Result / Packet 등으로 다시 검토한다.
```

---

## 4. 헤더 내부 배치 규칙

각 Type 헤더의 내부 순서는 넓은 영역에서 좁은 영역으로 고정한다.

```cpp
#pragma once

#include "CoreMinimal.h"
// 필요한 Engine / project include

// Forward Declaration

#include "CActionTypes.generated.h"

// Enum

// Key / Identifier

// Data / Config

// Runtime State

// Runtime Context

// Request

// Payload

// Resolution / Resolve Result

// Result

// Packet

// Runtime Key / Playback Key

// Helper API
```

기본 순서:

```text
Enum
-> Key / Identifier
-> Data / Config
-> Runtime State
-> Runtime Context
-> Request
-> Payload
-> Resolution / Resolve Result
-> Result
-> Packet
-> Runtime Key / Playback Key
-> Helper API
```

예외:

```text
- GetTypeHash는 해당 Key struct 바로 아래에 둔다.
- 계산용 operator가 Key의 의미를 완성한다면 Key struct 바로 아래에 둔다.
- UHT가 타입 전체 정의를 요구하면 forward declaration으로 대체하지 않고 정의 헤더를 include한다.
```

---

## 5. Enum 배치 규칙

여러 enum 선언이 같은 파일에 있을 때는 다음 순서를 따른다.

```text
Identity
-> Subtype / Phase
-> Command / Event
-> Policy / Mode
-> State / Result
-> Reason
```

예시:

```cpp
// Enum

UENUM(BlueprintType)
enum class EActionType : uint8
{
	None = 0,
	// ...
};

UENUM(BlueprintType)
enum class EGuardActionPhase : uint8
{
	None = 0,
	// ...
};

UENUM(BlueprintType)
enum class EActionNotifyCommand : uint8
{
	None = 0,
	// ...
};

UENUM(BlueprintType)
enum class EActionEventType : uint8
{
	None = 0,
	// ...
};

UENUM(BlueprintType)
enum class EActionStopReason : uint8
{
	None = 0,
	// ...
};

UENUM(BlueprintType)
enum class EActionFinishReason : uint8
{
	None = 0,
	// ...
};
```

판단 기준:

```text
Identity
-> EActionType, EReactionType, EWeaponType, ECombatSignalType

Subtype / Phase
-> EGuardActionPhase, EActionFeedbackTiming, ECombatRole

Command / Event
-> EActionNotifyCommand, EReactionNotifyCommand, EActionEventType, EObservableOverlayEventType

Policy / Mode
-> EExecutionRelationship, EExecutionApplyMode, EObservableOverlayHandling

State / Result
-> EExecutionDecision, EDamageDefenseOutcome

Reason
-> EActionStopReason, EActionRequestRejectReason, EReactionRequestRejectReason
```

---

## 6. Struct 배치 규칙

여러 struct 선언이 같은 파일에 있을 때는 데이터 생명주기 순서로 배치한다.

```text
Key / Identifier
-> Data / Config
-> Runtime State
-> Runtime Context
-> Request
-> Payload
-> Resolution / Resolve Result
-> Result
-> Packet
-> Runtime Key / Playback Key
```

예시:

```cpp
// Key / Identifier

USTRUCT(BlueprintType)
struct FActionDataKey
{
	GENERATED_BODY()
	// ...
};

FORCEINLINE uint32 GetTypeHash(const FActionDataKey& InKey)
{
	// ...
}

// Data / Config

USTRUCT(BlueprintType)
struct FActionData
{
	GENERATED_BODY()
	// ...
};

// Runtime Context

USTRUCT(BlueprintType)
struct FActionExecutionContext
{
	GENERATED_BODY()
	// ...
};

// Request

USTRUCT(BlueprintType)
struct FCombatActionRequest
{
	GENERATED_BODY()
	// ...
};

// Result

USTRUCT(BlueprintType)
struct FActionRequestResult
{
	GENERATED_BODY()
	// ...
};
```

판단 기준:

```text
Key / Identifier
-> map key, identity key, hash key

Data / Config
-> EditAnywhere data, DataAsset-style config, static definition

Runtime State
-> runtime cache, frame/tick accumulated state, phase state

Runtime Context
-> 실행 중 생성 / 계산 / 전달되는 transient context

Request
-> component / subsystem / orchestrator 경계로 들어가는 명령 또는 의도

Payload
-> boundary를 넘는 입력 원본 또는 정규화 입력

Resolution / Resolve Result
-> lookup, notify parse, tag resolve, config resolve 결과

Result
-> 처리 결과, dispatch 결과, receiver 전달 결과

Packet
-> 둘 이상의 의미 묶음을 경계 밖으로 전달하는 wrapper
```

---

## 7. Struct 내부 구성 규칙

struct 내부는 외부에서 식별에 필요한 값부터 실행 / 상태 값 순서로 배치한다.

권장 순서:

```text
Identity / Key fields
-> Owner / Source / Target references
-> Config / Static data
-> Runtime state
-> Result / Output state
-> Helper functions
```

규칙:

```text
- UPROPERTY 이름은 변경하지 않는다.
- UPROPERTY metadata는 의미 변경 없이 유지한다.
- helper 함수는 필드 아래에 둔다.
- operator / IsValid / Reset 같은 보조 API는 struct 의미를 완성하는 위치에 둔다.
- 단순 미관 목적의 필드 재정렬은 하지 않는다. serialized asset 위험이 있으면 기존 순서를 유지한다.
- A가 B의 멤버로 포함되면 A를 먼저 선언하고 B를 뒤에 둔다.
- wrapper / aggregate / packet은 포함 대상보다 뒤에 둔다.
- 기존 Key와 필드 / 비교 의미가 같으면 중복 Key를 만들지 않고 기존 Key를 재사용한다.
```

---

## 8. Naming 판단 규칙

새 타입을 만들거나 rename 후보를 판단할 때는 다음 기준을 사용한다.

```text
Data
-> 저장 가능한 설정 데이터. DataAsset, component config, editor 입력값에 사용한다.

Context
-> 실행 / 판정 / 요청 흐름에서 계산되어 전달되는 transient 묶음에 사용한다.

State
-> tick / frame / phase를 거쳐 누적되거나 유지되는 runtime state에 사용한다.

Request
-> 외부에서 시스템에 넣는 명령 / 의도에 사용한다. 해석 결과에는 사용하지 않는다.

Payload
-> boundary를 넘는 입력 원본 또는 정규화된 입력 묶음에 사용한다.

Resolution / Resolve Result
-> notify / tag / config lookup 해석 결과에 사용한다.

Result
-> 처리 결과에 사용한다.

Packet
-> Payload + Context + Result 같은 dispatch wrapper에 사용한다.

Key
-> lookup / match / dedupe identity에 사용한다.

RuntimeKey / ExecutionKey / PlaybackKey
-> runtime dedupe / active instance / effect playback 생명주기를 명확히 구분할 때 사용한다.

FeedbackMatchKey
-> 어떤 feedback data를 선택할지 결정하는 matching identity에 사용한다.
-> request / data matching 단계에서 사용한다.
-> ActionType / ActionIndex / ReactionType / DamageSpecKey / Timing / TriggerKey / fallback tier 같은 값은 matching 단계의 입력이다.
-> 기존 DataKey와 필드가 같더라도 목적이 feedback matching이면 FeedbackMatchKey로 명명한다.

FeedbackPlaybackKey
-> 선택된 feedback data의 실제 playback identity를 기준으로 runtime 중복 실행 여부를 결정할 때 사용한다.
-> playback dedupe는 feedback request identity가 아니라 effect asset + playback condition 기준으로 통일한다.
-> Timing / TriggerKey / ActionType / ReactionType은 matching 단계의 입력이며 playback dedupe key에는 포함하지 않는다.
-> VFX playback condition은 VFX asset, play type, socket, relative transform 기준으로 본다.
-> SFX playback condition은 SFX asset, play type 기준으로 본다.
-> play type은 현재 Once / Loop 같은 재생 생명주기 구분이다.

Info
-> 가급적 금지한다. 실제 의미가 더 좁으면 Context / State / Spec / Result / Snapshot 등을 사용한다.
```

---

## 9. Enum 내부 구성 규칙

enum entry 내부 순서는 이번 작업에서 재정렬하지 않는다. 명시 값이 없는 enum은 순서 변경이 실제 값을 바꿀 수 있기 때문이다.

새 enum을 작성하거나 명시적으로 정리하는 경우에는 다음 규칙을 따른다.

```text
1. None = 0 또는 Invalid 값을 첫 번째에 둔다.
2. 실제 값은 기능 흐름 또는 의미 그룹 순서로 둔다.
3. 의미 그룹 사이에는 빈 줄 1줄을 허용한다.
4. All / Any 같은 wildcard는 실제 값 뒤에 둔다.
5. Max / Count / Num 같은 sentinel은 마지막에 둔다.
```

예시:

```cpp
UENUM(BlueprintType)
enum class EActionType : uint8
{
	None = 0,

	Idle,

	Equip,
	Unequip,

	ComboAttack,

	Guard,
	Dodge,

	All,

	Max,
};
```

---

## 10. Include 배치 규칙

include 배치는 `.cpp`와 `.h`를 구분한다. 공통 원칙은 프로젝트 내부 의존성을 먼저 드러내고, Unreal / Engine 헤더를 그 뒤에 모으는 것이다.

### 10.1 `.cpp` include 순서

`.cpp` 파일은 다음 순서를 따른다.

```text
1. Matching header
2. ProjectGlobal.h
3. Project internal headers
4. Unreal / Engine headers
5. ThirdParty / STL headers
```

예시:

```cpp
#include "Component/CMovementComponent.h"

#include "ProjectGlobal.h"

#include "Component/CStateComponent.h"
#include "AI/RuntimeLOD/CAIMovementRuntimeLODPolicy.h"
#include "Type/CMovementTypes.h"
#include "Type/CStateTypes.h"
#include "Core/Debug/FMovementDebug.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
```

`Project internal headers` 안에서는 알파벳순보다 도메인 흐름을 우선한다.

```text
1. Same domain / direct collaborator
2. Gameplay domain
3. AI / RuntimeLOD / System
4. Type
5. Core/Debug
6. Core/Profiling
```

빈 줄은 큰 그룹 사이에만 둔다.

### 10.2 `.h` include 순서

`.h` 파일은 UHT 규칙을 우선한다.

```text
1. #pragma once
2. CoreMinimal.h
3. Required Engine headers
4. Required project Type / Interface headers
5. Forward declarations
6. *.generated.h
```

예시:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CActionTypes.h"

class ACharacter;
class UCWeaponComponent;

#include "CActionComponent.generated.h"
```

`*.generated.h`는 항상 해당 헤더의 마지막 include여야 한다.

---

## 11. Forward Declaration 기준

헤더에서 타입의 실제 크기나 멤버 정의가 필요하면 include한다.

```cpp
UPROPERTY()
FActionData ActiveActionData;

TArray<FActionData> ActionDatas;

FActionData GetActiveData() const;

FORCEINLINE bool HasActiveData() const
{
	return ActiveActionData.IsValid();
}
```

위 경우 `FActionData`의 전체 정의가 필요하므로 forward declaration으로 대체할 수 없다.

다음 경우에는 forward declaration을 우선한다.

```text
- pointer
- reference
- const reference
- 함수 선언만 존재
- inline 함수에서 멤버 접근 없음
- 값 생성 / sizeof / template value type 사용 없음
```

전방 선언은 include 아래, `*.generated.h` 앞에 한 번만 둔다. 본문에서 `class AActor*`처럼 inline forward declaration을 반복하지 않는다.

```cpp
#pragma once

#include "CoreMinimal.h"

class AActor;
struct FCombatResultPacket;

#include "CCombatResultTypes.generated.h"
```

---

## 12. UHT / Blueprint 이동 규칙

`USTRUCT`, `UENUM`은 Unreal Header Tool 대상이므로 일반 C++ 타입보다 보수적으로 다룬다.

허용:

```text
- 타입명 유지
- enum entry 유지
- UPROPERTY 이름 유지
- BlueprintType 유지
- 파일만 책임 단위로 분리
```

금지:

```text
- 타입명 rename
- enum entry rename
- UPROPERTY 이름 rename
- BlueprintType 제거
- asset 재저장이 필요한 대규모 이동을 검증 없이 진행
```

USTRUCT / UENUM rename 또는 Blueprint serialization에 영향을 줄 수 있는 변경은 별도 rename pass에서 처리한다. 해당 pass는 redirect, build, Editor load, Blueprint compile, PIE smoke 검증을 포함해야 한다.

UHT 헤더는 다음 순서를 지킨다.

```cpp
#pragma once

#include "CoreMinimal.h"

#include "CActionTypes.generated.h"
```

`*.generated.h`는 해당 헤더의 마지막 include여야 한다.

---

## 13. 검증 기준

정적 검증:

```powershell
rg -n "<umbrella-or-stale-header>" Source/Portfolio --glob "*.h" --glob "*.cpp"
git diff --check
```

문서 검증:

```text
- Rules 문서에는 안정적인 판단 규칙만 둔다.
- 프로젝트별 스캔 결과, stale 분류, 제거 / 이동 / rename 후보는 Work Plan 문서에 둔다.
- 사용자가 지정한 후보 항목을 의도적으로 제외했다면 제외 사유를 결과에 명시한다.
```

C++ 변경 검증:

```powershell
& "<UE_ROOT>\Engine\Build\BatchFiles\Build.bat" PortfolioEditor Win64 Development -Project="<PROJECT_ROOT>\Portfolio.uproject" -WaitMutex -FromMsBuild
```

USTRUCT / UENUM 이동을 포함하면 추가로 확인한다.

```text
Editor load
Blueprint compile
PIE smoke
Unknown structure / Struct type mismatch / Failed to load /Script/Portfolio 로그 없음
```
