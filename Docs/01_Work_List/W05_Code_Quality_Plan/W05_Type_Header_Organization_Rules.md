# W05 Type Header Organization Rules

## 제목

**W05: 구조체 나누기 / 헤더 배치 규칙**

## 날짜

**2026.07.23**

## 상태

- [x] Type 헤더 분류 기준 고정
- [x] UHT / BlueprintType 이동 위험 기준 정리
- [x] forward declaration 허용 / 금지 기준 정리
- [x] enum / struct 배치 및 내부 구성 규칙 정리

---

## 1. 목적

이 문서는 `Source/Portfolio/Type` 아래 공용 타입 헤더를 어떤 책임 단위로 나누고, 각 헤더 내부에서 enum과 struct를 어떤 순서로 배치할지 정의한다.

목표는 타입 이름을 새 체계로 바꾸는 것이 아니다. 현재 asset / Blueprint / UHT와 연결된 타입 이름, `UPROPERTY` 이름, enum entry를 유지하면서 헤더 include 전파를 줄이고 책임 단위로 파일을 분리하는 것이다.

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

파일명과 include 구조는 변경할 수 있다. 단, 파일 이동 후에도 serialized asset identity가 깨지지 않도록 타입명과 필드명은 유지한다.

---

## 3. Type 헤더 분류 원칙

Type 헤더는 파일 이름이 아니라 실제 책임으로 나눈다.

```text
Identity
-> 도메인의 정체성을 나타내는 기본 enum / context

Data / Config
-> DataAsset, component property, map entry 등 저장 가능한 설정 데이터

Runtime Context
-> 실행 중 조합되는 transient context

Request / Payload
-> 시스템 경계로 전달되는 요청 또는 입력 payload

Result / Packet
-> 처리 결과 또는 dispatch packet

Feedback / Presentation
-> VFX / SFX / hit stop / camera shake 같은 표현 요청
```

`CWeaponStructure.h`처럼 여러 도메인이 섞인 umbrella 파일은 최종 목표로 두지 않는다. 이행 중 compatibility header를 잠깐 둘 수는 있지만, 최종 상태에서는 사용처가 필요한 Type 헤더를 직접 include해야 한다.

---

## 4. 권장 Type 헤더 분류

최종 권장 분류는 다음과 같다.

```text
CWeaponTypes.h
-> EWeaponType
-> FWeaponContext

CActionTypes.h
-> EActionType
-> EGuardActionPhase
-> EActionNotifyCommand
-> EActionEventType
-> EActionStopSource / EActionStopReason / EActionFinishReason
-> FActionDataKey
-> FActionData
-> FActionExecutionContext
-> FActionContext

CReactionTypes.h
-> EReactionType
-> EReactionNotifyCommand
-> EReactionStopSource / EReactionStopReason / EReactionFinishReason
-> FReactionDataKey
-> FReactionData
-> FReactionExecutionContext

CActionOrchestrationTypes.h
-> action intent / request / candidate / deferred / request result 계열

CReactionOrchestrationTypes.h
-> reaction intent / request / candidate / request result 계열

CExecutionTypes.h
-> execution decision / relationship / apply mode / domain
-> execution participant / query / result / intervention directive
-> FActionExecutionResult
-> FReactionExecutionResult

CObservableOverlayTypes.h
-> observable overlay event / snapshot / query / decision 계열

CCombatHitTypes.h
-> EDamageImpactInfoSource
-> FOverlapContext
-> FDamageImpactInfo
-> FHitContext
-> FCombatSignalHitWindowKey

CCombatDamageTypes.h
-> EDamageDefenseOutcome
-> FDamageSpecKey
-> FDamageSpec
-> FDamageAmount
-> FDefaultDamageEvent

CCombatSignalTypes.h
-> generic combat signal / message bus 타입

CCombatSignalSourceTypes.h
-> ECombatSignalSourceRejectReason
-> FCombatSignalSourcePayload / FCombatSignalSourceContext / FCombatSignalSourceResult

CCombatSignalTargetTypes.h
-> ECombatSignalTargetRejectReason
-> FCombatSignalTargetPayload / FCombatSignalTargetContext / FCombatSignalTargetResult
-> FCombatSignalTargetPacket

CCombatResultTypes.h
-> FCombatResultPacket

CActionFeedbackTypes.h
-> action trail / VFX / SFX feedback 계열

CReactionFeedbackTypes.h
-> reaction VFX / SFX feedback 계열

CCombatFeedbackTypes.h
-> EFeedbackAudience
-> FHitStopRequest
-> FCameraShakeRequest

CAITypes.h
-> perception / context / patrol / engage context 계열

CEngageAssignmentTypes.h
-> ECombatRole
-> EAIUpdatePrecision
-> engage request / assignment / debug state 계열

CHealthTypes.h
-> health / dead state 계열

CMovementTypes.h
-> movement gait / rotation mode 계열

CStateTypes.h
-> execution state / AI intent state 계열

CCharacterComponentReferenceTypes.h
-> FCharacterComponentReferences
```

`DamageEventId.h`는 `FDamageEvent::ClassID` 구분용 C++ 내부 ID로 유지한다. Blueprint / `UPROPERTY` / editor 노출이 필요한 타입이 아니므로 `UENUM()`을 사용하지 않는다.

---

## 5. 헤더 내부 전체 배치 규칙

각 Type 헤더의 내부 순서는 큰 영역에서 작은 영역으로 고정한다.

```cpp
#pragma once

#include "CoreMinimal.h"
// 필요한 엔진 / 프로젝트 include

// Forward Declaration

#include "CActionTypes.generated.h"

// Enum

// Key / Identifier

// Data / Config

// Runtime Context

// Request / Payload

// Result / Packet

// Helper API
```

기본 순서는 다음과 같다.

```text
Enum
-> Key / Identifier
-> Data / Config
-> Runtime Context
-> Request / Payload
-> Result / Packet
-> Helper API
```

예외:

```text
- GetTypeHash는 해당 Key struct 바로 아래에 둔다.
- 연산자 overload가 Key의 의미를 완성한다면 Key struct 바로 아래에 둔다.
- UHT 값 타입은 forward declaration으로 대체하지 않고 정의 헤더를 include한다.
```

---

## 6. Enum 간 배치 규칙

여러 enum 선언이 같은 파일에 있을 때는 다음 순서를 따른다.

```text
Identity
-> Subtype / Phase
-> Command / Event
-> Policy / Mode
-> State / Result
-> Reason
```

예:

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
enum class EActionStopSource : uint8
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
-> EExecutionDecision, ECombatSignalOutcome, EDamageDefenseOutcome

Reason
-> EActionStopReason, EActionRequestRejectReason, EReactionRequestRejectReason
```

---

## 7. Struct 간 배치 규칙

여러 struct 선언이 같은 파일에 있을 때는 데이터 생명주기 순서로 배치한다.

```text
Key / Identifier
-> Data / Config
-> Runtime Context
-> Request / Payload
-> Result / Packet
```

예:

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

// Request / Payload

USTRUCT(BlueprintType)
struct FActionCombatSignalCueRequest
{
	GENERATED_BODY()
	// ...
};

// Result / Packet

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

Runtime Context
-> 실행 중 생성 / 갱신되는 context

Request / Payload
-> component / subsystem / orchestrator 경계로 들어가는 입력

Result / Packet
-> 처리 결과, dispatch packet, receiver 전달 값
```

---

## 8. Enum 내부 구성 규칙

enum entry 내부 순서는 이번 작업에서 재정렬하지 않는다. 명시 값이 없는 enum은 순서 변경이 실제 값을 바꿀 수 있기 때문이다.

새 enum을 작성하거나 명시적으로 정리하는 경우에는 다음 규칙을 따른다.

```text
1. None = 0 또는 Invalid 값은 첫 번째에 둔다.
2. 실제 값은 기능 흐름 또는 의미 그룹 순서로 둔다.
3. 의미 그룹 사이에는 빈 줄 1줄을 허용한다.
4. All / Any 같은 wildcard는 실제 값 뒤에 둔다.
5. Max / Count / Num 같은 sentinel은 마지막에 둔다.
```

예:

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

## 9. Struct 내부 구성 규칙

struct 내부는 외부에서 식별에 필요한 값부터 실행/상태 값 순서로 배치한다.

권장 순서:

```text
Identity / Key fields
-> Owner / Source / Target references
-> Config / Static data
-> Runtime state
-> Result / Output state
-> Helper functions
```

예:

```cpp
USTRUCT(BlueprintType)
struct FCombatResultPacket
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	AActor* InstigatorActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	AActor* TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FDamageAmount DamageAmount;

	UPROPERTY(BlueprintReadOnly)
	EDamageDefenseOutcome DefenseOutcome = EDamageDefenseOutcome::None;

	bool IsValid() const;
};
```

규칙:

```text
- UPROPERTY 이름은 변경하지 않는다.
- UPROPERTY metadata는 의미 변경 없이 유지한다.
- helper 함수는 필드 아래에 둔다.
- operator / IsValid / Reset 같은 보조 API는 struct 의미를 완성하는 위치에 둔다.
- 단순 미관 목적의 필드 재정렬은 하지 않는다. serialized asset 리스크가 있으면 기존 순서를 유지한다.
```

---

## 10. Include 배치 규칙

include 배치는 `.cpp`와 `.h`를 구분한다. 공통 원칙은 프로젝트 내부 의존성을 먼저 드러내고, Unreal / Engine 헤더는 그 뒤에 모으는 것이다.

### 10.1 `.cpp` include 순서

`.cpp` 파일은 다음 순서를 따른다.

```text
1. Matching header
2. ProjectGlobal.h
3. Project internal headers
4. Unreal / Engine headers
5. ThirdParty / STL headers
```

예:

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

`Project internal headers` 안에서는 알파벳순보다 도메인 흐름순을 우선한다.

```text
1. Same domain / direct collaborator
2. Gameplay domain
3. AI / RuntimeLOD / System
4. Type
5. Core/Debug
6. Core/Profiling
```

빈 줄은 큰 그룹 사이에만 둔다.

```text
- Matching header
- ProjectGlobal.h
- Project internal headers
- Unreal / Engine headers
- ThirdParty / STL headers
```

Project internal header가 많더라도 기본적으로 한 그룹으로 유지한다. 단, include가 많아 의미가 흐려지는 파일은 도메인 묶음 사이에 빈 줄 1줄을 허용한다.

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

예:

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

이 경우 `FActionData`의 전체 정의가 필요하므로 forward declaration으로 대체할 수 없다.

다음 경우에는 forward declaration을 우선한다.

```text
- pointer
- reference
- const reference
- 함수 선언만 존재
- inline 함수에서 멤버 접근 없음
- 값 생성 / sizeof / template value type 사용 없음
```

전방 선언은 include 아래, `*.generated.h` 위에 한 번만 둔다. 본문에서 `class AActor*`처럼 inline forward declaration을 반복하지 않는다.

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
rg -n "CWeaponStructure.h" Source/Portfolio --glob "*.h" --glob "*.cpp"
git diff --check
```

C++ 변경 검증:

```powershell
& "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" PortfolioEditor Win64 Development -Project="C:\UE5_Portfolio\Portfolio_UE5.4_verGit\Portfolio\Portfolio.uproject" -WaitMutex -FromMsBuild
```

USTRUCT / UENUM 이동이 포함되면 추가로 확인한다.

```text
Editor load
Blueprint compile
PIE smoke
Unknown structure / Struct type mismatch / Failed to load /Script/Portfolio 로그 없음
```
