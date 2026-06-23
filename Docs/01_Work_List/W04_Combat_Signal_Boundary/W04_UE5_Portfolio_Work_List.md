# UE5 Portfolio - Work List

## 제목

**W04: Combat Signal Boundary v1 정리**

## 날짜

**2026.06.22**

## 상태

- [x] **완료**

---

## 브랜치

- `refactor/combat-signal-boundary`

---

## 1. Branch 목표

이번 작업은 W03 Guard / Parry 이후 커진 combat 송수신 책임을 바로 리팩터링하지 않고, 후속 작업의 기준이 될 **Combat Signal Boundary v1**을 문서와 타입으로 먼저 고정한다.

현재 브랜치의 핵심 산출물은 다음과 같다.

```text
1. Combat Signal Boundary 설계 기준 정리
2. 이전 Request / Routing 설계 문서 archive
3. Combat Signal 타입 vocabulary 추가
4. W04 작업 기록 / prompt update 후보 정리
5. P21 PR 문서 작성
```

이번 브랜치에서는 기존 gameplay 흐름을 변경하지 않는다.

```text
변경하지 않는 것
-> ApplyDamageComponent
-> TakeDamageComponent
-> Guard / Parry / Hit / Dead runtime flow
-> 기존 damage packet 연결
```

---

## 2. 완료 기준

이 Branch는 다음 조건을 만족하면 PR 가능한 상태로 본다.

```yaml
완료 기준
- N05 Combat Signal Boundary 설계 노트가 작성되어 있다
- N06 Combat Signal Branch 구현 계획이 작성되어 있다
- 기존 Request / Receiver / Resolution / Coordinator 중심 문서가 archive로 이동되어 있다
- Combat Signal 최소 타입 vocabulary가 추가되어 있다
- 기존 ApplyDamageComponent / TakeDamageComponent / Guard / Parry runtime flow가 변경되지 않는다
- W04 task brief, work journal, prompt update note가 정리되어 있다
- P21 PR 문서가 현재 브랜치 산출물 중심으로 작성되어 있다
- Unreal build가 성공한다
```

---

## 3. 필수 산출물

```yaml
Work List / PR
- Docs/01_Work_List/W04_Combat_Signal_Boundary/W04_UE5_Portfolio_Work_List.md
- Docs/01_Work_List/00_Work_List_Index.md
- Docs/04_Pull_Request/P21_UE5_Portfolio_Pull_Request.md
- Docs/04_Pull_Request/00_Pull_Request_Index.md
```

```yaml
Design Notes
- Docs/06_notes/N05_Combat_Signal_Boundary_Design_Note.md
- Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md
- Docs/06_notes/archive/NA01_Combat_Intent_Request_Resolution_Routing_Design_Note.md
- Docs/06_notes/archive/README.md
```

```yaml
Task Brief / Journal / Prompt Update
- Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/README.md
- Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_01_Combat_Signal_Boundary_Rescope.md
- Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_02_Combat_Signal_Types_v1.md
- Docs/06_notes/work_journal/J01_Combat_Signal_Boundary_Work_Journal.md
- Docs/06_notes/prompt_updates/PU01_Combat_Signal_Boundary_Prompt_Update_Note.md
```

```yaml
Code
- Source/Portfolio/Type/CCombatSignalStructure.h
- Source/Portfolio/Type/CCombatSignalStructure.cpp
```

---

## 4. 완료된 작업 범위

### 4.1 Combat Signal Boundary 설계 기준 정리

**완료**

- `N05_Combat_Signal_Boundary_Design_Note.md` 작성
- `N06_Combat_Signal_Branch_Implementation_Plan.md` 작성
- `CombatSignal Source / Target` 기준 확정
- branch 분할 기준을 feature 이름이 아니라 behavior / risk 축으로 정리

**핵심 결정**

```text
공용 상태 변경 파이프라인을 먼저 만들지 않는다.
입력 데이터 처리 축, combat 데이터 처리 축, timing cue 처리 축을 분리해서 본다.
현재 브랜치에서는 가장 직접적인 문제인 combat source / target 책임 경계를 먼저 안정화한다.
```

이전 후보였던 `GameplayIntentGateway / GameplayCoordinator` 구조는 이번 W04의 주도 구조로 사용하지 않는다.

**보류 이유**

- 입력, damage, timing cue, system event는 발생 원인과 해석 기준이 서로 다르다.
- 이를 하나의 공용 Gateway / Coordinator가 판정하면 God Object 위험이 커진다.
- 반대로 모든 축을 세밀하게 나누면 현재 규모에 비해 계층과 adapter가 과도해진다.

따라서 공용 파이프라인 일반화는 뒤로 미루고, 먼저 입력 처리 축 / combat 처리 축 / timing cue 처리 축을 분리해서 바라보기로 했다.

### 4.2 이전 Request / Routing 설계 archive

**완료**

- 기존 `Combat Intent / Request / Resolution / Routing` 설계 노트를 archive로 이동
- active note와 충돌하지 않도록 참조 갱신
- archive index 추가

**Archive 문서**

```text
Docs/06_notes/archive/NA01_Combat_Intent_Request_Resolution_Routing_Design_Note.md
```

### 4.3 Combat Signal 타입 vocabulary 추가

**완료**

- `Source/Portfolio/Type/CCombatSignalStructure.h` 추가
- `Source/Portfolio/Type/CCombatSignalStructure.cpp` 추가
- 각 struct에 `IsValidMinimal()` 추가
- `AActor` 전방 선언 추가
- Unreal build 성공

**추가 타입**

```text
ECombatSignalType
ECombatSignalOutcome
ECombatSignalResultType
FCombatSignalHeader
FCombatSignal
FCombatSignalContext
FCombatSignalEvaluation
FCombatSignalApplyResult
FCombatSignalResult
```

**제한**

```text
CombatSignal 타입은 아직 기존 damage flow에 연결하지 않는다.
actor 유효성이나 combat rule 검증은 이후 target-side refactor에서 다룬다.
```

### 4.4 작업 기록 문서 정리

**완료**

- `TB_W04_01_Combat_Signal_Boundary_Rescope.md`
- `TB_W04_02_Combat_Signal_Types_v1.md`
- `J01_Combat_Signal_Boundary_Work_Journal.md`
- `PU01_Combat_Signal_Boundary_Prompt_Update_Note.md`

**프롬프트 업데이트 후보**

```text
Request / Attack / Damage 명명 기준
CombatSignal vocabulary 우선 검토 기준
branch split을 behavior / risk 축으로 나누는 기준
```

### 4.5 PR 문서 정리

- [x] `P21_UE5_Portfolio_Pull_Request.md`를 작성했다.
- [x] Pull Request Index에 P21 항목을 추가했다.
- [x] PR 문서는 현재 Branch 산출물 중심으로 정리했다.
- [x] 이전 `Intent / Request / Receiver / Resolution / Coordinator` 구조는 배경 수준으로만 언급했다.

### 4.6 확정 vocabulary

#### 컴포넌트

```text
UCCombatSignalSourceComponent
UCCombatSignalTargetComponent
```

`Source`는 전투 신호를 만드는 쪽이다. 무기 overlap, montage notify, timing cue, lock-on cue 등에서 target에게 보낼 전투 신호를 구성하고 전달한다.

`Target`은 전투 신호를 받는 쪽이다. 자신의 guard / parry / dead / invincible / cue timing 상태를 바탕으로 outcome을 평가하고 결과 적용과 후속 통지를 수행한다.

#### 구조체

```text
FCombatSignalHeader
FCombatSignal
FCombatSignalContext
FCombatSignalEvaluation
FCombatSignalApplyResult
FCombatSignalResult
```

#### 열거형

```text
ECombatSignalType
- None
- HitEvidence
- TimingCue
- DirectDamage
- System

ECombatSignalOutcome
- None
- Hit
- Blocked
- Parried
- Blink
- Repulse
- Staggered
- Dead

ECombatSignalResultType
- None
- Handled
- Ignored
- Rejected
```

### 4.7 책임 경계

#### CombatSignalSource

책임:

- hit window open / close 상태를 추적한다.
- 동일 hit window 안에서 duplicate target을 거른다.
- overlap / cue / timing signal을 `FCombatSignal`로 구성한다.
- target actor의 `CombatSignalTarget` 경계를 찾아 signal을 전달한다.
- 필요 시 target에서 돌아온 result를 받아 ParryStack, debug, result out에 연결한다.

하지 않는 일:

- defender의 Guard / Parry / Blink / Repulse 성공 여부를 결정하지 않는다.
- HP를 직접 감소시키지 않는다.
- defender reaction / feedback을 직접 실행하지 않는다.

#### CombatSignalTarget

책임:

- `FCombatSignal`을 수신한다.
- signal 유효성과 target-side context를 검증한다.
- Guard / Parry / Hit / Blink / Repulse / Dead outcome을 평가한다.
- Health / Reaction / Feedback / ResultOut으로 넘길 apply/result 자료를 만든다.
- 필요한 후속 domain 호출 순서를 관리한다.
- UE `TakeDamage()`는 장기적으로 이쪽으로 들어오는 legacy adapter로 축소한다.

하지 않는 일:

- source-side hit window를 관리하지 않는다.
- target discovery를 수행하지 않는다.
- source actor의 공격 판정을 대신 결정하지 않는다.

### 4.8 장기 내부 흐름

```text
1. Source 준비
Weapon overlap / montage notify / cue window 발생

2. Signal 생성
UCCombatSignalSourceComponent
-> FCombatSignal 구성

3. Signal 전달
SourceComponent
-> TargetActor의 UCCombatSignalTargetComponent 탐색
-> ReceiveCombatSignal 호출

4. Target 평가
TargetComponent
-> EvaluateCombatSignal
-> Guard / Parry / Hit / Blink / Repulse / Dead 판단

5. 결과 적용
TargetComponent
-> ApplyCombatSignalOutcome
-> Health / Reaction / Feedback / ResultOut으로 분배

6. 결과 통지
TargetComponent
-> 필요 시 SourceComponent 또는 attacker-side result receiver에 CombatSignalResult 전달
```

### 4.9 API 후보

#### Source

```cpp
void OpenSignalWindow(...);
void CloseSignalWindow(...);
FCombatSignal BuildHitSignal(...);
FCombatSignal BuildCueSignal(...);
FCombatSignalResult SendCombatSignal(const FCombatSignal& InSignal);
void NotifyCombatSignalResult(const FCombatSignalResult& InResult);
```

#### Target

```cpp
FCombatSignalResult ReceiveCombatSignal(const FCombatSignal& InSignal);
FCombatSignalEvaluation EvaluateCombatSignal(const FCombatSignal& InSignal);
FCombatSignalApplyResult ApplyCombatSignalOutcome(const FCombatSignalEvaluation& InEvaluation);
void NotifyCombatSignalResult(const FCombatSignalResult& InResult);
```

#### Legacy Adapter

```cpp
FCombatSignal BuildSignalFromDamageEvent(...);
FCombatSignalResult ReceiveEngineDamage(...);
```

### 4.10 인터페이스 구성

v1에서는 interface를 최소화한다.

우선 후보:

```cpp
class ICCombatSignalTarget
{
    ReceiveCombatSignal(...);
};
```

후속 후보:

```cpp
class ICCombatSignalSource
{
    NotifyCombatSignalResult(...);
};
```

Target interface가 먼저 필요한 이유는 수신 경계가 반드시 필요하기 때문이다. 반면 source result notify는 ParryStack / Stagger / debug / network 요구가 정리된 후 추가해도 된다.

### 4.11 현재 컴포넌트와의 관계

#### ApplyDamageComponent

현재 `UCApplyDamageComponent`는 이름과 달리 HP를 직접 적용하지 않는다.

현재 책임:

```text
hit window 관리
duplicate target 관리
hit context validate
damage spec 조회
requested damage 계산
target TakeDamage 호출
```

장기 위치:

```text
UCApplyDamageComponent
-> UCCombatSignalSourceComponent
```

#### TakeDamageComponent

현재 `UCTakeDamageComponent`는 수신, 평가, 적용, 후속 분배를 모두 가진다.

현재 책임:

```text
UE TakeDamage 수신
payload / context 구성
defensive outcome 판단
damage 계산
Health commit
reaction / feedback dispatch
attacker result packet dispatch
```

장기 위치:

```text
UCTakeDamageComponent
-> UCCombatSignalTargetComponent
```

---

## 5. 비범위

```yaml
비범위
- TakeDamageComponent 내부 책임 분리
- ApplyDamageComponent 내부 책임 분리
- UCCombatSignalSourceComponent / UCCombatSignalTargetComponent rename
- CombatSignal 타입을 기존 damage flow에 연결
- 기존 FApplyDamagePayload / FTakeDamagePacket / FCombatResultPacket 교체
- Blink / Repulse timing cue 구현
- 공용 Gateway / Coordinator 구현
- gameplay behavior 변경
```

---

## 6. 검증 기준

### 문서 존재 / 연결 확인

- [x] Work List Index에서 W04 항목 확인
- [x] Pull Request Index에서 P21 항목 확인
- [x] N05 / N06 문서 존재 확인
- [x] NA01 archive 문서와 archive README 존재 확인
- [x] TB_W04_01 / TB_W04_02 존재 확인
- [x] Work Journal / Prompt Update Note 존재 확인

### 구조 참조 확인

- [x] `GameplayIntentGateway / GameplayCoordinator`는 active 구현 대상으로 등장하지 않는다.
- [x] 이전 Request / Routing 문서는 archive 문서로 추적된다.
- [x] W04 문서에서 공용 상태 변경 파이프라인은 보류 / 재검토 대상으로 설명된다.
- [x] `CombatSignal Source / Target`이 현재 Branch의 기준 구조로 설명된다.

### 코드 연결 확인

- [x] `CCombatSignalStructure.h/.cpp` 외 기존 combat runtime code 변경 없음
- [x] `FCombatSignal` 계열 타입이 기존 gameplay flow에 연결되지 않음
- [x] 신규 타입 header는 `CoreMinimal.h`, generated header, `AActor` 전방 선언만 사용

### 빌드 확인

```text
PortfolioEditor Win64 Development
```

- [x] UnrealHeaderTool 통과
- [x] `CCombatSignalStructure.cpp` 컴파일 통과
- [x] Editor target 빌드 성공

---

## 7. 후속 작업

### 7.1 W04-03 Combat Signal Target Boundary v1

**상태**

```text
완료
```

**브랜치**

```text
refactor/combat-signal-target-v1
```

**목표**

```text
UCTakeDamageComponent 내부 흐름을 CombatSignalTarget 책임 기준으로 정리한다.
```

**핵심 범위**

- `UCTakeDamageComponent` private API를 Entry / Receive / Evaluate / Apply / Notify / Packet / Debug 기준으로 재배치
- `CTakeDamageComponent.cpp` 정의 순서를 header 선언 순서와 일치
- `HandleDefaultDamageEvent` 내부 흐름을 Receive / Evaluate / Apply / Packet / Notify 라벨로 정리
- `FTakeDamageResult`와 `FCombatSignalResult` 관계를 내부 authoritative result / 외부 summary result 후보로 정리
- 기존 `RequestTakeDamage` 흐름 유지
- UE `TakeDamage()` adapter 유지
- `FCombatSignal` 직접 연결 없음
- 클래스명 rename은 아직 하지 않음

**완료조건**

- 기존 Guard / Parry / Hit / Dead 동작 유지
- target-side 책임 단계가 코드에서 명확히 보임
- `FTakeDamageResult` / `FCombatSignalResult` 관계가 현재 브랜치 범위 안에서 정리됨
- Unreal build 성공

### 7.2 W04-04 Combat Signal Source Boundary v1

**상태**

```text
완료
```

**브랜치**

```text
refactor/combat-signal-source-v1
```

**목표**

```text
UCApplyDamageComponent 내부 흐름을 CombatSignalSource 책임 기준으로 정리한다.
```

**핵심 범위**

- `UCApplyDamageComponent` private API를 HitWindow / Entry / Receive / Resolve / Send / Cache / Helper / Debug 기준으로 재배치
- `CApplyDamageComponent.cpp` 정의 순서를 header 선언 순서와 일치
- `ProcessApplyDamage` 내부 흐름을 Receive / Resolve / Send / Debug 라벨로 정리
- 기존 `RequestApplyDamage` 흐름 유지
- 기존 weapon overlap damage 흐름 유지
- target `TakeDamage()` 전달 방식 유지
- `FCombatSignal` 직접 연결 없음
- 클래스명 rename은 아직 하지 않음

**완료조건**

- 기존 weapon overlap damage 동작 유지
- source-side 책임 단계가 코드에서 명확히 보임
- `ProcessApplyDamage` 흐름이 source-side 단계 기준으로 읽힘
- Unreal build 성공

### 7.3 W04-05 Combat Signal Component Rename

**상태**

```text
다음 작업
```

**브랜치**

```text
refactor/combat-signal-component-rename
```

**목표**

```text
책임 정리가 끝난 뒤 ApplyDamage / TakeDamage 명칭을 CombatSignalSource / Target으로 교체한다.
```

**핵심 범위**

- `UCApplyDamageComponent` -> `UCCombatSignalSourceComponent`
- `UCTakeDamageComponent` -> `UCCombatSignalTargetComponent`
- Character / Weapon 참조 갱신
- Blueprint 영향 확인

**완료조건**

- 이름만 바꾼 것이 아니라 이전 브랜치의 책임 정리가 선행되어 있다.
- 기존 전투 회귀 통과
- Unreal build 성공

### 7.4 W04-06 Combat Signal Cue v1

**상태**

```text
예정
```

**브랜치**

```text
feature/combat-signal-cue-v1
```

**목표**

```text
Blink / Repulse 같은 collision 없는 timing cue를 CombatSignal 흐름에 연결한다.
```

**핵심 범위**

- `ECombatSignalType::TimingCue` 사용
- target discovery 방식 정리
- Blink / Repulse evaluation hook 추가

**완료조건**

- collision hit와 timing cue가 같은 target receive 흐름을 공유한다.
- cue 전용 예외 파이프라인을 만들지 않는다.

## 8. 관련 문서

- `Docs/06_notes/N03_Guard_Hold_Overlay_Layer_Design_Note.md`
- `Docs/06_notes/N04_Blink_Repulse_Combat_Packet_Design_Note.md`
- `Docs/06_notes/archive/NA01_Combat_Intent_Request_Resolution_Routing_Design_Note.md`
- `Docs/06_notes/N05_Combat_Signal_Boundary_Design_Note.md`
- `Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md`
