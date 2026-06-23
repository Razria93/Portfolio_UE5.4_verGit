# TB W04-04 Combat Signal Source Boundary v1

## 작업명

```text
Combat Signal Source Boundary v1
```

## 브랜치

```text
refactor/combat-signal-source-v1
```

## 목표

`UCApplyDamageComponent` 내부 흐름을 `CombatSignalSource` 책임 기준으로 정리한다.

이번 작업은 component rename이나 `FCombatSignal` 연결이 아니라, 기존 weapon overlap damage 송신 흐름을 유지하면서 source-side 처리 단계를 코드에서 명확히 보이게 만드는 준비 리팩터링이다.

## 배경

W04-03에서는 `UCTakeDamageComponent`를 target-side 기준으로 정리했다.

다음 단계는 `UCApplyDamageComponent` 내부에 모여 있는 hit window 관리, overlap 입력 수신, damage spec 해석, target 전달, duplicate hit cache 책임을 source-side 단계로 읽히게 정리하는 것이다.

현재 `UCApplyDamageComponent`는 이름과 달리 target HP를 직접 적용하지 않는다. 실제로는 source 쪽에서 hit evidence를 받아 damage payload / context를 만들고, target의 `TakeDamage()` entry로 전달하는 역할에 가깝다.

현재 `UCApplyDamageComponent`는 다음 책임을 함께 가진다.

```text
hit window open / close 수신
overlap hit context 수신
hit request validation
payload / context 구성
duplicate target 검증
damage spec 조회
damage amount 계산
target TakeDamage 호출
damaged target cache
debug 출력
```

이번 작업에서는 이 흐름을 다음 단계로 정리한다.

```text
HitWindow
-> Entry
-> Receive
-> Resolve
-> Send
-> Cache
-> Helper
-> Debug
```

## 핵심 범위

- 기존 public API 유지
  - `NotifyHitWindowOpened`
  - `NotifyHitWindowClosed`
  - `RequestApplyDamage`
- 기존 weapon overlap damage 흐름 유지
- `ProcessApplyDamage` 내부 단계 주석과 호출 흐름 정리
- `CApplyDamageComponent.h` private method group을 source-side 단계 기준으로 재배치
- `CApplyDamageComponent.cpp` 정의 순서를 header 선언 순서와 일치
- 기존 `FHitContext`, `FApplyDamagePayload`, `FApplyDamageContext`, `FApplyDamageResult` 유지
- 기존 target `TakeDamage()` 전달 방식 유지

## 제외 범위

- `UCApplyDamageComponent` rename
- `UCCombatSignalSourceComponent` 신설
- `FCombatSignal`을 기존 damage flow에 직접 연결
- `RequestApplyDamage` API rename
- `UCTakeDamageComponent` 수정
- `FCombatSignalResult` 변환 함수 구현
- Blink / Repulse timing cue 구현

## 결정 사항

이번 작업에서는 `FCombatSignal` 타입을 기존 source damage flow에 강제로 연결하지 않는다.

이유:

- 현재 목표는 source-side 단계 경계를 명확히 하는 것이다.
- target-side 정리와 동일하게 runtime packet 교체는 후속 브랜치에서 판단한다.
- component rename은 source / target 양쪽 내부 책임 정리가 끝난 뒤 진행하는 편이 안전하다.

따라서 v1에서는 기존 타입과 public API를 유지한 채 함수 그룹과 내부 책임 흐름만 source-side 기준으로 정리한다.

## 단계 분류 기준

이번 작업의 기준은 `UCApplyDamageComponent`가 source 입장에서 hit evidence를 target damage 전달로 바꿀 때의 시간 순서와 책임 변화다.

```text
HitWindow
-> Entry
-> Receive
-> Resolve
-> Send
-> Cache
-> Helper
-> Debug
```

`HitWindow`는 montage timing 기반 hit 가능 구간과 window별 hit cache lifecycle을 관리하는 단계다.

```text
NotifyHitWindowOpened
NotifyHitWindowClosed
```

`Entry`는 overlap hit를 damage 송신 처리로 넘기는 진입점이다.

```text
RequestApplyDamage
ProcessApplyDamage
```

`Receive`는 overlap에서 들어온 hit context를 내부에서 처리 가능한 자료로 정규화하고 검증하는 단계다.

```text
ValidateRequest
BuildPayload
BuildContext
```

`Resolve`는 source-side damage 전달에 필요한 spec, controller, damage amount를 해석하는 단계다.

```text
ValidateContext
CanApplyDamage
ResolveApplyDamageSpec
ComputeApplyDamage
BuildResult
```

`Send`는 target 쪽 `TakeDamage()` entry로 damage event를 전달하는 단계다.

```text
CommitApplyDamage
ApplyDamageToTarget
```

`Cache`는 같은 hit window 안에서 동일 target 중복 적용을 막기 위한 기록 단계다.

```text
CacheDamagedTargetInWindow
```

`Helper`는 주요 source-side 흐름에서 호출되는 세부 계산 / 조회 함수다.

```text
BuildHitWindowKey
BuildSpecKey
ResolveInstigatorController
IsDuplicateHit
IsFriendlyTarget
```

`Debug`는 runtime decision에 영향을 주지 않는 관찰용 출력이다.

```text
PrintApplyDamageSummaryInfo
PrintApplyDamageContextInfo
PrintApplyDamageRejectedSummaryInfo
PrintApplyDamageRejectedContextInfo
PrintOverlapContextInfo
PrintHitContextInfo
PrintDamageSpecInfo
PrintDamageResultInfo
PrintRejectReasonInfo
```

## 완료조건

- `UCApplyDamageComponent` public API가 유지되어 있다.
- 기존 weapon overlap damage 흐름과 target `TakeDamage()` 전달 방식이 유지되어 있다.
- 기존 Guard / Parry / Hit / Dead 동작 의도가 바뀌지 않는다.
- `CApplyDamageComponent.h`에서 source-side 단계가 명확히 보인다.
- `ProcessApplyDamage` 흐름이 source-side 단계 기준으로 읽힌다.
- `FCombatSignal`은 기존 damage flow에 연결하지 않는다.
- Unreal build가 성공한다.

## 검증

진행 전 기준:

```text
TB_W04_04 생성
W04 Work List 상태 진행 중 반영
코드 수정 전 source-side 단계 분류 기준 확정
```

정적 확인:

```text
git diff -- Source/Portfolio/Component/CApplyDamageComponent.h Source/Portfolio/Component/CApplyDamageComponent.cpp
rg -n "Entry|HitWindow|Receive|Resolve|Send|Cache|Debug" Source/Portfolio/Component/CApplyDamageComponent.h Source/Portfolio/Component/CApplyDamageComponent.cpp Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_04_Combat_Signal_Source_Boundary_v1.md
```

빌드 확인:

```text
PortfolioEditor Win64 Development
```

## 프롬프트 업데이트 확인

추가 프롬프트 업데이트 후보는 작업 후 확인한다.

후보 기준:

```text
기존 runtime component를 rename하기 전에 내부 책임 단계를 먼저 정리하고,
새 packet type 연결은 단계 경계가 안정된 뒤 별도 작업으로 분리한다.
```
