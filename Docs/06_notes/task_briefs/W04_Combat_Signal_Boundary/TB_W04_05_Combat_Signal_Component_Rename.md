# TB W04-05 Combat Signal Component Rename

## 작업명

```text
Combat Signal Component Rename
```

## 브랜치

```text
refactor/combat-signal-component-rename
```

## 목표

`ApplyDamage / TakeDamage` 명칭으로 남아 있는 기존 combat 송수신 컴포넌트를 `CombatSignalSource / CombatSignalTarget` 책임 기준에 맞게 리네임한다.

이번 작업은 기능 변경이 아니라, W04-03 / W04-04에서 정리한 책임 경계를 코드 이름에 반영하는 리팩터링이다.

## 배경

W04-03에서는 `UCTakeDamageComponent` 내부 흐름을 target-side 기준으로 정렬했다.

W04-04에서는 `UCApplyDamageComponent` 내부 흐름을 source-side 기준으로 정렬했다.

두 컴포넌트 모두 기존 런타임 동작은 유지하지만, 이름은 아직 `ApplyDamage / TakeDamage`에 머물러 있다. 이 상태가 길어지면 실제 책임과 코드 명칭이 계속 어긋나므로, 책임 정렬이 끝난 시점에서 컴포넌트명, 내부 API명, 구조체명을 Combat Signal 기준으로 맞춘다.

## 핵심 범위

### 컴포넌트 / 파일명

```text
UCApplyDamageComponent
-> UCCombatSignalSourceComponent

UCTakeDamageComponent
-> UCCombatSignalTargetComponent
```

```text
CApplyDamageComponent.h/.cpp
-> CCombatSignalSourceComponent.h/.cpp

CTakeDamageComponent.h/.cpp
-> CCombatSignalTargetComponent.h/.cpp
```

### 내부 API명

source-side API는 `ApplyDamage` 중심 표현을 `CombatSignalSource` 또는 source flow 기준 표현으로 정리한다.

target-side API는 `TakeDamage` 중심 표현을 `CombatSignalTarget` 또는 target flow 기준 표현으로 정리한다.

단, Unreal Engine API 의미가 있는 이름은 유지한다.

```text
UGameplayStatics::ApplyDamage
AActor::TakeDamage
FDamageEvent
FDefaultDamageEvent
```

### 구조체명

기존 source-side runtime 구조체:

```text
FApplyDamagePayload
FApplyDamageContext
FApplyDamageResult
```

target-side runtime 구조체:

```text
FTakeDamagePayload
FTakeDamageContext
FTakeDamageResult
FTakeDamagePacket
```

위 타입들은 참조 범위 확인 후 Combat Signal Source / Target 기준 이름으로 정리한다.

## 제외 범위

- combat 동작 변경
- Guard / Parry / Defensive Outcome 로직 변경
- damage amount 계산 로직 변경
- target `TakeDamage()` 전달 방식 변경
- `FCombatSignal`을 기존 damage flow에 직접 연결
- Blink / Repulse timing cue 구현
- 공용 Gateway / Coordinator 재도입

## 작업 순서

1. 리네임 범위 스캔

```text
ApplyDamage / TakeDamage 명칭 참조 위치를 확인한다.
Unreal Engine API 이름과 프로젝트 내부 이름을 분리한다.
Blueprint / asset reference 영향 여부를 확인한다.
```

2. 컴포넌트 / 파일명 리네임

```text
컴포넌트 클래스명과 파일명을 Source / Target 기준으로 변경한다.
include, forward declaration, member type, cached component 참조를 갱신한다.
```

3. 내부 API명 리네임

```text
public entry, private flow API, helper API 중 책임과 어긋나는 이름을 정리한다.
함수명 변경은 동작 변경 없이 수행한다.
```

4. 구조체명 리네임

```text
payload / context / result / packet 타입명을 source-side / target-side 기준으로 정리한다.
기존 FCombatResultPacket과 의미 충돌이 없는지 확인한다.
```

5. 로그 / 디버그 문구 정리

```text
[@ APPLY DAMAGE], [@ TAKE DAMAGE] 등 예전 책임명을 새 용어로 갱신한다.
```

6. 문서 / PR 반영

```text
W04 Work List, Task Brief, PR 문서를 현재 브랜치 결과 기준으로 갱신한다.
prompt update 필요 여부를 확인한다.
```

## 완료조건

- `UCApplyDamageComponent` / `UCTakeDamageComponent` 명칭이 코드에서 제거되거나 legacy adapter 의미로만 남아 있다.
- source-side / target-side 컴포넌트명이 책임과 일치한다.
- 내부 API명과 구조체명이 새 컴포넌트 책임을 따른다.
- Unreal Engine API 이름은 불필요하게 변경하지 않는다.
- 기존 combat runtime 동작이 바뀌지 않는다.
- `git diff --check`가 통과한다.
- `PortfolioEditor Win64 Development` 빌드가 성공한다.

## 진행 결과

```text
컴포넌트 / 파일 / 클래스명 리네임 완료
내부 flow API 리네임 완료
Payload / Context / Result / Packet 타입명 리네임 완료
필드 / local variable 리네임 완료
디버그 라벨 리네임 완료
Unreal class / property redirect 추가 완료
renamed component reference 검증 / 복구 API 추가 완료
Unreal build 성공
```

## 결정 사항

- `UCApplyDamageComponent` / `UCTakeDamageComponent`는 각각 `UCCombatSignalSourceComponent` / `UCCombatSignalTargetComponent`로 변경했다.
- `CApplyDamageComponent.h/.cpp` / `CTakeDamageComponent.h/.cpp`는 각각 `CCombatSignalSourceComponent.h/.cpp` / `CCombatSignalTargetComponent.h/.cpp`로 변경했다.
- source-side / target-side flow API, 캐싱 필드, local variable, subobject display name은 새 책임명 기준으로 정리했다.
- `FApplyDamagePayload / Context / Result`와 `FTakeDamagePayload / Context / Result / Packet`은 Combat Signal Source / Target 타입명으로 변경했다.
- `FApplyDamageSpec`, `FApplyDamageSpecKey`, `FApplyDamageAmount`, `EApplyDamageRejectReason`, `ETakeDamageRejectReason`은 damage data 계층 의미가 남아 있어 후속 브랜치로 분리했다.
- UE `AActor::TakeDamage`, `Super::TakeDamage`, target `TakeDamage()` 호출, `UCHealthComponent::TakeDamage`는 engine / resource 경계이므로 유지했다.
- runtime asset reference 보호를 위해 class redirect와 property redirect를 추가했다.
- native component rename 이후 Actor에는 컴포넌트가 존재하지만 C++ 멤버 포인터가 유효하지 않은 경우가 확인되어, `ACPlayer` / `ACEnemy`의 `BeginPlay()`에서 `ResolveComponentReferences()`로 rename 대상 컴포넌트 참조를 한 번 검증 / 복구한다.
- 이번 브랜치의 참조 복구 범위는 `CombatSignalSourceComponent` / `CombatSignalTargetComponent`로 제한하고, 전체 character component cache validation은 후속 브랜치 후보로 분리한다.
- debug label은 Combat Signal Source / Target 기준으로 정리하되, 실제 damage amount 출력과 Health commit 단계 표현은 유지했다.

## 검증 결과

```text
rg old component/API/type/debug label scan 통과
git diff --check 통과
PortfolioEditor Win64 Development 빌드 성공
```

## 커밋 분할 후보

```text
refactor(combat): rename combat signal components
refactor(combat): rename combat signal flow APIs
refactor(combat): rename combat signal payload types
refactor(combat): rename combat signal fields
refactor(combat): update combat signal debug labels
docs(combat): document combat signal rename
```

## 프롬프트 업데이트 확인

이번 작업에서 확인한 후보:

```text
기존 runtime component를 리네임할 때는 컴포넌트명, 내부 API명, 구조체명, 로그 문구를 별도 단계로 나누고,
Unreal Engine API 이름과 프로젝트 내부 이름을 먼저 분리해서 검토한다.
damage data, engine API, resource commit API처럼 기존 domain 의미가 남아 있는 이름은 별도 후속 작업으로 분리한다.
serialized reference가 있는 Unreal component rename은 class redirect와 property redirect를 함께 검토한다.
```
