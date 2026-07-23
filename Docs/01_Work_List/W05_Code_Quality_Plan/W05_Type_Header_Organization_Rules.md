# W05 Type Header Organization Rules

## 제목

**W05: 구조체 / 헤더 배치 규칙**

## 날짜

**2026.07.23**

## 상태

- [x] Type 헤더 전수 스캔 기준 초안 작성
- [x] UHT / BlueprintType 이동 위험 기준 정리
- [x] forward declaration 허용 / 금지 기준 정리

---

## 1. 목적

이 문서는 `Source/Portfolio/Type` 아래 공유 타입 헤더를 어떤 기준으로 나누고 배치할지 정리한다.

목표는 타입 이름을 새 체계로 바꾸는 것이 아니다. 현재 asset / Blueprint / UHT에 연결된 타입 이름과 enum entry는 유지하면서, 헤더 include 전파를 줄이고 책임 단위로 파일을 나눌 수 있는 기준을 고정한다.

---

## 2. 기본 원칙

```text
1. 헤더는 실제 정의가 필요한 타입만 include한다.
2. 선언만 필요한 타입은 forward declaration을 우선한다.
3. USTRUCT / UENUM 타입명, UPROPERTY 이름, enum entry는 단순 헤더 정리 작업에서 변경하지 않는다.
4. Blueprint asset에 직렬화된 타입 이동은 작은 단위로만 진행한다.
5. *.generated.h는 해당 헤더의 마지막 include여야 한다.
6. 대형 Type 헤더는 즉시 분해하지 않고 include 의존을 낮춘 뒤 단계적으로 분리한다.
```

---

## 3. Header Include 기준

헤더에서 타입의 실제 크기나 멤버 정의가 필요한 경우에는 include를 유지한다.

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

위 경우는 `FActionData`의 전체 정의가 필요하므로 forward declaration으로 대체할 수 없다.

---

## 4. Forward Declaration 허용 기준

다음 경우에는 헤더에서 forward declaration을 사용할 수 있다.

```cpp
struct FCharacterComponentReferences;
struct FCombatResultPacket;

void InitializeReferences(const FCharacterComponentReferences& InReferences);
void ReceiveCombatResultPacket(const FCombatResultPacket& InPacket);
```

허용 조건:

```text
- pointer
- reference
- const reference
- 함수 선언만 존재
- inline 함수에서 멤버 접근 없음
- 값 생성 / sizeof / template value type 사용 없음
```

실제 멤버 접근은 `.cpp`에서 include 후 수행한다.

```cpp
#include "Type/CCharacterComponentReferenceStructure.h"

void UCActionComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	ActionOrchestratorComp_Injected = InReferences.ActionOrchestratorComp;
}
```

---

## 5. Forward Declaration 금지 기준

다음 경우는 헤더에서 완전한 타입 정의가 필요하다.

```text
- 값 멤버
- UPROPERTY 값 타입
- TArray<FType>, TMap<Key, FType> 같은 template value type
- return-by-value
- inline 함수에서 멤버 접근
- 상속
- sizeof가 필요한 경우
- GENERATED_BODY 대상 타입 정의
```

예:

```cpp
TArray<FActionData> ActionDatas;
TMap<FActionDataKey, FActionData> ActionDataMap;
FActionData ActiveActionData = FActionData();
```

위 형태는 forward declaration 대상이 아니다.

---

## 6. USTRUCT / UENUM 이동 규칙

`USTRUCT`, `UENUM`은 Unreal Header Tool 대상이므로 일반 C++ 타입보다 엄격하게 다룬다.

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
- 단순 헤더 정리 작업에서 타입명 rename
- enum entry rename
- UPROPERTY 이름 rename
- BlueprintType 제거
- asset 재저장이 필요한 대규모 이동을 검증 없이 진행
```

새 UHT 헤더는 다음 순서를 지킨다.

```cpp
#pragma once

#include "CoreMinimal.h"

#include "CActionDataStructure.generated.h"
```

`*.generated.h`는 해당 헤더의 마지막 include여야 한다.

---

## 7. Type Header 배치 기준

장기적으로 `Source/Portfolio/Type`은 책임 단위로 나눈다.

```text
Type/Core
-> enum-only 또는 저수준 key 타입
-> EWeaponType, EActionType, EReactionType, EDeadState, DamageEventId 후보

Type/Action
-> action data, action request, action execution context
-> FActionDataKey, FActionData, FActionExecutionContext

Type/Reaction
-> reaction data, reaction request, reaction feedback
-> FReactionDataKey, FReactionData, FReactionFeedbackRequest

Type/Combat
-> hit, damage, combat signal, combat result
-> FHitContext, FDamageSpec, FDefaultDamageEvent, FCombatSignalTargetPacket, FCombatResultPacket

Type/Execution
-> execution snapshot / participant / decision / intervention
-> FExecutionSnapshot, FExecutionDecisionQuery, FExecutionInterventionDirective

Type/Feedback
-> action / reaction / hit presentation request data
-> FActionFeedbackRequest, FActionVFXFeedbackData, FHitStopRequest, FCameraShakeRequest

Type/AI
-> AI context, patrol, perception, engage context
-> FAIContext, FTargetData, FPatrolPointData, FEngageContext

Type/World
-> world subsystem request / assignment data
-> FEngageRequestContext, FEngageAssignmentContext
```

현재 폴더 구조를 바로 변경하지는 않는다. 이번 작업에서는 이 기준을 문서로 고정하고, 실제 이동은 위험도와 검증 비용에 따라 단계적으로 수행한다.

---

## 8. CWeaponStructure 분리 기준

`CWeaponStructure.h`는 현재 가장 큰 Type 허브다.

```text
Line count: 약 2061
Direct include: 약 45곳
USTRUCT: 약 47개
UENUM: 약 34개
```

장기 분리 후보:

```text
WeaponCore
-> EWeaponType, EActionType, EReactionType, notify command enum

ActionData
-> FActionDataKey, FActionData, FActionExecutionContext

ReactionData
-> FReactionDataKey, FReactionData, FReactionExecutionContext

CombatHitDamage
-> FOverlapContext, FDamageImpactInfo, FHitContext, FDamageSpec, FDefaultDamageEvent

CombatSignalPayload
-> FCombatSignalSourcePayload / Context / Result
-> FCombatSignalTargetPayload / Context / Result
-> FCombatSignalTargetPacket, FCombatResultPacket

ExecutionOverlay
-> FExecutionSnapshot, FExecutionParticipant, FExecutionDecisionQuery
-> FExecutionInterventionQuery, FExecutionInterventionDirective

ActionFeedback
-> FActionFeedbackKey, FActionFeedbackRequest, trail / VFX / SFX feedback data
```

단, 이번 브랜치에서 `CWeaponStructure.h` 전체 분리는 하지 않는다.

---

## 9. 이번 브랜치 우선 적용 기준

이번 브랜치에서는 안전한 순서만 적용한다.

```text
1. 규칙 / 작업계획 문서화
2. non-UHT 타입 include-only 정리
3. forward declaration 후보 검증
4. CWeaponStructure 분리 지도 작성
5. 작은 단위 USTRUCT 이동은 빌드 / PIE 검증 가능할 때만 진행
```

첫 적용 후보:

```text
FCharacterComponentReferences
-> USTRUCT 아님
-> Blueprint / UHT 영향 없음
-> 타입명 / 필드명 변경 없음
-> 대부분 const-ref 함수 선언에만 필요
```

---

## 10. 검증 기준

정적 검증:

```powershell
rg -n "CCharacterComponentReferenceStructure.h|CWeaponStructure.h" ..\Source\Portfolio
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
