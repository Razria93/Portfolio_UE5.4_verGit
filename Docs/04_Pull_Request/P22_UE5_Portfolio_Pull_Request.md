# UE5 Portfolio Pull Request

## 제목

**P22: Combat Signal Target Boundary v1 정리**

## 날짜

**2026.06.23**

## 상태

- [x] **완료**

---

## 브랜치

- `refactor/combat-signal-target-v1`

---

## 요약

이번 PR에서는 `UCTakeDamageComponent`를 rename하거나 `FCombatSignal`에 연결하지 않고, 현재 damage 수신 흐름을 target-side 처리 단계 기준으로 정리했다.

핵심은 기존 Guard / Parry / Hit / Dead 동작을 유지하면서 header와 source의 API 순서를 같은 단계 기준으로 맞추고, `HandleDefaultDamageEvent()` 실행 흐름이 `Receive / Evaluate / Apply / Packet / Notify` 순서로 읽히게 만드는 것이다.

---

## 변경 배경

P21에서 combat 송수신 경계를 `CombatSignal Source / Target` 기준으로 재정의하고 최소 타입 vocabulary를 추가했다.

다음 단계에서 바로 component rename이나 packet 교체를 진행하면 기존 damage flow와 Guard / Parry 회귀 위험이 커질 수 있다. 따라서 이번 PR에서는 `TakeDamageComponent` 내부 책임을 먼저 target-side 단계로 정렬하고, 실제 `CombatSignalTarget` 전환은 후속 브랜치로 남겼다.

---

## 변경 범위

### 1. TakeDamage target-side 단계 정리

`UCTakeDamageComponent` private API를 다음 단계 기준으로 재배치했다.

```text
Entry
-> Receive
-> Evaluate
-> Apply
-> Notify
-> Packet
-> Debug
```

### 2. Header / Source 정의 순서 정렬

`CTakeDamageComponent.h`의 선언 순서와 `CTakeDamageComponent.cpp`의 정의 순서를 맞췄다.

이를 통해 `RequestTakeDamage()` 진입 이후 수신, 평가, 적용, 통지, packet 생성, debug 출력 흐름을 같은 순서로 따라갈 수 있게 했다.

### 3. HandleDefaultDamageEvent 흐름 라벨 정리

`HandleDefaultDamageEvent()` 내부에 다음 단계 라벨을 추가했다.

```text
Receive
Evaluate
Apply
Packet
Notify
```

라벨은 흐름 가독성을 위한 주석이며, 기존 실행 순서와 조건 분기는 변경하지 않았다.

### 4. Result boundary 정리

`FTakeDamageResult`와 `FCombatSignalResult`의 관계를 task brief에 정리했다.

```text
FTakeDamageResult
= TakeDamage flow 내부 authoritative result

FCombatSignalResult
= 외부 송출용 summary result 후보
```

이번 PR에서는 변환 함수를 선언하거나 구현하지 않는다.

---

## 구현 범위

이번 PR의 코드 변경은 `UCTakeDamageComponent` 내부 정렬로 제한했다.

- 기존 public API 유지
- 기존 UE `TakeDamage()` adapter 유지
- 기존 `FTakeDamagePayload`, `FTakeDamageContext`, `FTakeDamageResult`, `FTakeDamagePacket` 유지
- 기존 `FCombatResultPacket` 유지
- `FCombatSignal` 직접 연결 없음

---

## 검증

### 빌드

```text
PortfolioEditor Win64 Development
```

결과:

```text
성공
Target is up to date
```

### 정적 확인

- `CTakeDamageComponent.h` private method group 확인
- `CTakeDamageComponent.cpp` 정의 순서 확인
- `HandleDefaultDamageEvent()` 단계 라벨 확인
- `git diff --check` 통과

---

## 제외 범위

이번 PR에서는 다음 작업을 의도적으로 제외했다.

- `UCTakeDamageComponent` rename
- `UCCombatSignalTargetComponent` 신설
- `FCombatSignal`을 기존 damage flow에 연결
- `FCombatSignalResult` 변환 함수 구현
- `UCApplyDamageComponent` source-side 정리
- Blink / Repulse / GuardBreak cue flow 정리

---

## 후속 작업

권장 후속 브랜치는 다음과 같다.

```text
refactor/combat-signal-source-v1
```

후속 작업 목표:

- `UCApplyDamageComponent` 내부를 Source 관점으로 정리
- hit window / duplicate target / damage spec / target delivery 경계를 명확히 정리
- 기존 weapon overlap damage 동작 유지
- component rename은 source / target 양쪽 책임 정리 후 별도 브랜치에서 진행

---

## 관련 문서

- `Docs/01_Work_List/W04_Combat_Signal_Boundary/W04_UE5_Portfolio_Work_List.md`
- `Docs/06_notes/N05_Combat_Signal_Boundary_Design_Note.md`
- `Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md`
- `Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_03_Combat_Signal_Target_Boundary_v1.md`
