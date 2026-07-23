# W05 Type Header Organization Work Plan

## 제목

**W05: 구조체 나누기 / 헤더 배치 작업 계획**

## 날짜

**2026.07.23**

## 상태

- [x] 브랜치 생성
- [x] Type 헤더 라인 수 / UHT 대상 전수 스캔
- [x] include 의존성 전수 스캔
- [x] UHT / Blueprint / asset reference 위험 기준 정리
- [x] 구조체 / 헤더 배치 규칙 문서 작성
- [ ] include-only 첫 적용
- [ ] CWeaponStructure 분리 지도 작성
- [ ] 최종 검증
- [ ] PR 문서 작성

---

## 브랜치

```text
refactor/type-header-organization
```

---

## 1. 목표

이번 브랜치는 `Source/Portfolio/Type` 아래 공유 타입 헤더의 책임 경계와 include 전파 문제를 정리하기 위한 준비 작업이다.

주 목표:

```text
1. Type 헤더 배치 규칙을 고정한다.
2. UHT / BlueprintType 이동 위험 기준을 문서화한다.
3. include-only 정리로 안전한 첫 적용 사례를 만든다.
4. CWeaponStructure.h 대형 허브를 어떤 단위로 나눌지 지도화한다.
```

이번 브랜치는 대규모 타입 rename이나 asset 재저장이 필요한 이동을 목표로 하지 않는다.

---

## 2. 전수 스캔 요약

`Source/Portfolio/Type` 헤더 라인 수:

```text
2061 CWeaponStructure.h
 298 CAIStructure.h
 279 CCombatSignalStructure.h
 252 CActionOrchestrationStructure.h
 236 CReactionFeedbackStructure.h
 140 CWorldSubsystemStructure.h
  81 CReactionOrchestrationStructure.h
  30 CStateStructure.h
  28 CMovementStructure.h
  28 CCharacterComponentReferenceStructure.h
  20 CHealthStructure.h
   9 DamageEventId.h
```

직접 include 수 기준 주요 후보:

```text
CWeaponStructure.h
-> 약 45곳
-> Action / Reaction / Component / Notify / Interface / Core Debug 전반

CCharacterComponentReferenceStructure.h
-> 약 21곳
-> reference injection 경로

CActionOrchestrationStructure.h
-> 약 18곳
-> action request / orchestrator / AI task / debug

CWorldSubsystemStructure.h
-> 약 15곳
-> AI context / combat world subsystem / feedback / debug

CAIStructure.h
-> 약 12곳
```

---

## 3. P0 후보

### CWeaponStructure.h

가장 큰 Type 허브이며 책임이 과하게 섞여 있다.

포함 책임:

```text
Weapon / Action / Reaction enum
Action data
Reaction data
Hit / Overlap / Damage context
CombatSignal source / target payload / result
CombatResult packet
Execution snapshot / decision / intervention
Observable overlay
Action feedback VFX / SFX
```

위험:

```text
- USTRUCT / UENUM 대량 포함
- BlueprintType 타입 다수 포함
- FActionData / FReactionData / FDamageSpec / feedback data가 asset에 직렬화될 가능성 높음
- enum 이름 / entry 변경 금지
- 타입명 변경 금지
```

판단:

```text
이번 브랜치에서 전체 분리는 하지 않는다.
분리 지도 작성 후 작은 단위 후속 브랜치로 진행한다.
```

---

## 4. P1 후보

### FCharacterComponentReferences include-only 정리

`CCharacterComponentReferenceStructure.h`는 UHT 대상이 아닌 plain C++ struct다.

첫 적용 대상으로 적합한 이유:

```text
- USTRUCT 아님
- Blueprint / asset 영향 없음
- 타입명 / 필드명 변경 없음
- 많은 헤더가 const-ref 함수 선언에만 사용
- 헤더 include를 forward declaration으로 대체 가능
```

작업 방식:

```text
헤더:
-> struct FCharacterComponentReferences;

cpp:
-> #include "Type/CCharacterComponentReferenceStructure.h"
```

### CombatResult / HitContext / CombatSignal 계열 forward declaration 후보

후속 검토 후보:

```text
FCombatResultPacket
FHitContext
FCombatSignal
FCombatSignalTargetPacket
FActionExecutionResult
FReactionExecutionResult
```

주의:

```text
이 타입들은 USTRUCT / BlueprintType 경로와 연결되어 있다.
헤더에서 const-ref 선언만 쓰는지, 값 멤버 / UPROPERTY / TArray / return-by-value가 있는지 파일별로 확인해야 한다.
```

---

## 5. P2 후보

다음 파일들은 책임 경계가 비교적 선명하거나 크기가 작아 당장 분리 필요성이 낮다.

```text
CAIStructure.h
CCombatSignalStructure.h
CHealthStructure.h
CMovementStructure.h
CStateStructure.h
DamageEventId.h
```

단, `DamageEventId.h`는 `UENUM()`이 있지만 자체 generated include가 없는 특이 케이스다. 지금은 `CWeaponStructure.h`에서 include되어 `FDefaultDamageEvent::ClassID`에 사용된다. 구조 변경은 별도 검토한다.

---

## 6. 이번 브랜치 처리 범위

포함:

```text
1. Type Header Organization Rules 작성
2. Type Header Organization Work Plan 작성
3. FCharacterComponentReferences include-only 정리
4. CWeaponStructure.h 내부 타입 분리 지도 작성
5. 필요하면 CombatResult / HitContext / CombatSignal forward declaration 후보 목록화
```

제외:

```text
1. CWeaponStructure.h 전체 분리
2. BlueprintType struct / enum 타입명 변경
3. enum entry 변경
4. UPROPERTY 이름 변경
5. asset 재저장 / redirect가 필요한 변경
6. DamageEventId.h 구조 변경
```

---

## 7. 작업 순서

```text
1. 규칙 / 작업계획 문서 작성
2. FCharacterComponentReferences 사용처 재스캔
3. 헤더에서 선언만 필요한 곳을 forward declaration으로 변경
4. 실제 필드 접근 cpp에 include 유지 / 추가
5. git diff --check
6. PortfolioEditor Development 빌드
7. CWeaponStructure.h 분리 지도 작성
8. 후속 작업 범위 판단
```

---

## 8. 검증 기준

정적 확인:

```powershell
rg -n "CCharacterComponentReferenceStructure.h" ..\Source\Portfolio --glob "*.h" --glob "*.cpp"
git diff --check
```

빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat" PortfolioEditor Win64 Development -Project="C:\UE5_Portfolio\Portfolio_UE5.4_verGit\Portfolio\Portfolio.uproject" -WaitMutex -FromMsBuild
```

USTRUCT / UENUM 이동이 포함될 경우:

```text
Editor load
Blueprint compile
PIE smoke
Unknown structure / Struct type mismatch / Failed to load /Script/Portfolio 로그 없음
```

---

## 9. PR 가능 조건

```text
- 타입명 / enum entry / UPROPERTY 이름 변경이 없다.
- UHT 대상 이동이 있다면 generated include 규칙을 지켰다.
- include-only 정리는 빌드로 검증됐다.
- CWeaponStructure.h 분리 지도와 후속 범위가 문서화됐다.
- 보류 항목이 삭제되지 않고 후속 작업으로 남아 있다.
```
