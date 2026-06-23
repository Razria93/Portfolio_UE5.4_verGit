# TB W04-03 Combat Signal Target Boundary v1

## 작업명

```text
Combat Signal Target Boundary v1
```

## 브랜치

```text
refactor/combat-signal-target-v1
```

## 목표

`UCTakeDamageComponent` 내부 흐름을 `CombatSignalTarget` 책임 기준으로 정리한다.

이번 작업은 component rename이나 `FCombatSignal` 연결이 아니라, 기존 damage 수신 흐름을 유지하면서 target-side 처리 단계를 코드에서 명확히 보이게 만드는 준비 리팩터링이다.

## 배경

W04-01 / W04-02에서 combat 송수신 경계를 `CombatSignal Source / Target` 기준으로 재정의하고, 최소 타입 vocabulary를 추가했다.

다음 단계는 실제 runtime에 바로 `FCombatSignal`을 연결하는 것이 아니라, 현재 `UCTakeDamageComponent` 안에 모여 있는 수신, 평가, 적용, 후속 통지 책임을 단계별로 읽히게 정리하는 것이다.

현재 `UCTakeDamageComponent`는 다음 책임을 함께 가진다.

```text
UE damage event 수신
payload / context 구성
context validation
defensive outcome 평가
damage 계산
Health commit
defender reaction / feedback 요청
attacker result packet 전달
debug 출력
```

이번 작업에서는 이 흐름을 다음 단계로 정리한다.

```text
Receive
-> Evaluate
-> Apply
-> Notify
```

## 핵심 범위

- 기존 public API 유지
  - `RequestTakeDamage`
  - `ProcessTakeDamage`
- UE `AActor::TakeDamage()` adapter 흐름 유지
- `HandleDefaultDamageEvent` 내부 단계 주석과 호출 흐름 정리
- `CTakeDamageComponent.h` private method group을 target-side 단계 기준으로 재배치
- 기존 `FTakeDamagePayload`, `FTakeDamageContext`, `FTakeDamageResult`, `FTakeDamagePacket` 유지
- `FTakeDamageResult`와 `FCombatSignalResult`의 관계 후보 문서화
- 기존 Guard / Parry / Hit / Dead 동작 유지

## 제외 범위

- `UCTakeDamageComponent` rename
- `UCCombatSignalTargetComponent` 신설
- `FCombatSignal`을 기존 damage flow에 직접 연결
- `UCApplyDamageComponent` 수정
- `CombatSignalSource` 구현
- Blink / Repulse timing cue 구현
- 기존 `FCombatResultPacket` 교체

## 결정 사항

이번 작업에서는 `FCombatSignal` 타입을 기존 damage flow에 강제로 연결하지 않는다.

이유:

- 현재 목표는 target-side 단계 경계를 명확히 하는 것이다.
- packet 교체까지 동시에 진행하면 회귀 위험 축이 커진다.
- `FTakeDamageResult`와 `FCombatSignalResult`의 대응 관계는 먼저 문서화하고, 실제 교체 여부는 후속 branch에서 판단하는 편이 안전하다.

따라서 v1에서는 기존 타입을 유지한 채 함수 그룹과 호출 흐름만 `Receive / Evaluate / Apply / Notify` 기준으로 정리한다.

## 단계 분류 기준

이번 작업의 기준은 `UCTakeDamageComponent`가 target 입장에서 damage signal을 처리할 때의 시간 순서와 책임 변화다.

```text
Entry
-> Receive
-> Evaluate
-> Apply
-> Notify
-> Packet
-> Debug
```

`Entry`는 외부에서 이 component로 들어오는 진입점이다.

```text
RequestTakeDamage
ProcessTakeDamage
HandleDefaultDamageEvent
```

`Receive`는 외부 입력을 내부에서 처리 가능한 자료로 정규화하는 단계다.

```text
ValidateRequest
BuildPayload
BuildContext
ResolveInstigatorController
```

`Evaluate`는 target의 현재 상태를 기준으로 수신 결과를 판단하는 단계다.

```text
ValidateContext
CanTakeDamage
ComputeTakeDamage
ComputeMitigatedDamage
ComputeFinalTakenDamage
BuildResult
```

`Apply`는 실제 상태 변경을 수행하는 단계다.

```text
CommitTakeDamage
CommitDamageToHealth
```

`Notify`는 처리 결과를 다른 domain 또는 source-side receiver로 전달하는 단계다.

```text
DispatchAcceptedCombatResult
DispatchRejectedCombatResult
DispatchCombatResultToReceiver
ResolveCombatResultReceiverActor
BuildCombatResultPacket
```

`Packet`은 payload / context / result를 후속 단계가 읽을 수 있는 자료로 묶는 단계다.

```text
BuildPacket
```

`Debug`는 runtime decision에 영향을 주지 않는 관찰용 출력이다.

```text
PrintTakeDamageSummaryInfo
PrintTakeDamageContextInfo
PrintTakeDamageOutcomeInfo
PrintObjectInfo
PrintSpecKeyInfo
PrintDamageAmountInfo
```

## 완료조건

- `UCTakeDamageComponent` public API가 유지되어 있다.
- 기존 `ApplyDamageComponent` / weapon overlap flow와 연결 방식이 유지되어 있다.
- 기존 Guard / Parry / Hit / Dead 동작 의도가 바뀌지 않는다.
- `CTakeDamageComponent.h`에서 target-side 단계가 명확히 보인다.
- `HandleDefaultDamageEvent` 흐름이 Receive / Evaluate / Apply / Notify 기준으로 읽힌다.
- `FTakeDamageResult`와 `FCombatSignalResult` 후보 관계가 문서화되어 있다.
- Unreal build가 성공한다.

## 검증

진행 결과:

```text
TakeDamage Header Target Sections v1 완료
TakeDamage Default Event Flow Labels v1 완료
TakeDamage Source Definition Order Alignment 완료
```

확인 내용:

- `CTakeDamageComponent.h` private method group을 `Receive / Evaluate / Apply / Notify / Packet / Debug` 기준으로 재배치했다.
- 기존 public API와 함수명은 유지했다.
- `HandleDefaultDamageEvent` 내부 흐름을 `Receive / Evaluate / Apply / Packet / Notify` 라벨로 정리했다.
- rejected / accepted packet 생성과 dispatch 위치의 의미를 명시했다.
- `CTakeDamageComponent.cpp` 정의 순서를 `CTakeDamageComponent.h` 선언 순서와 일치시켰다.
- 함수 구현 로직은 변경하지 않았다.
- `FCombatSignal`은 기존 damage flow에 연결하지 않았다.

정적 확인:

```text
git diff -- Source/Portfolio/Component/CTakeDamageComponent.h Source/Portfolio/Component/CTakeDamageComponent.cpp
rg -n "Receive|Evaluate|Apply|Notify|FCombatSignalResult|FTakeDamageResult" Source/Portfolio/Component/CTakeDamageComponent.* Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_03_Combat_Signal_Target_Boundary_v1.md
```

빌드 확인:

```text
PortfolioEditor Win64 Development
```

결과:

```text
성공
```

남은 작업:

- `FTakeDamageResult`와 `FCombatSignalResult` 후보 관계를 문서화한다.

## 프롬프트 업데이트 확인

추가 프롬프트 업데이트 후보 없음.

후보 기준:

```text
기존 runtime component를 rename하기 전에 내부 책임 단계를 먼저 정리하고,
새 packet type 연결은 단계 경계가 안정된 뒤 별도 작업으로 분리한다.
```

위 기준은 이번 작업에서 새로 발견된 것이 아니라 W04-01 / PU01의 기존 판단을 따른 것이다.
