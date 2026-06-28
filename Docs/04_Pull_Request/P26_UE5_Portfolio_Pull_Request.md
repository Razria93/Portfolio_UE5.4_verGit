# UE5 Portfolio Pull Request

## 제목

**P26: Combat Signal TimingCue 연결 v1**

## 날짜

**2026.06.28**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-signal-cue-v1`

---

## 요약

이번 PR에서는 Blink / Repulse 같은 collision 없는 전투 타이밍 신호를 `CombatSignal` 흐름으로 전달할 수 있는 최소 연결 지점을 추가했다.

핵심은 Blink / Repulse 실제 기능을 구현하는 것이 아니라, action montage notify에서 발생한 cue가 Action 정책 확인을 거쳐 source component에서 `FCombatSignal`로 구성되고 target component의 TimingCue receive hook까지 도달하는지 검증하는 것이다.

---

## 변경 배경

P22 / P23 / P24 / P25를 통해 기존 damage hit 흐름은 `CombatSignalSource / CombatSignalTarget` 경계와 damage data 이름 기준으로 정리되었다.

다만 Blink / Repulse는 overlap hit나 UE `TakeDamage()` event로 표현하기 어려운 timing cue 성격을 가진다. 따라서 damage route를 억지로 확장하기보다, `FCombatSignal`의 `TimingCue` 타입을 사용해 collision hit와 cue가 같은 target receive 개념을 공유할 수 있는지 먼저 확인한다.

---

## 변경 범위

### 1. CombatSignal TimingCue Notify 추가

`UCAnimNotify_CombatSignalCue`를 추가했다.

Notify는 `CueTag`만 보유하며, target discovery / signal build / damage 값 계산을 직접 수행하지 않는다.

```text
UCAnimNotify_CombatSignalCue
-> UCActionComponent::HandleActionCombatSignalCue
```

### 2. Action Policy Resolve 추가

TimingCue notify는 기존 action notify 규약에 맞춰 `ActionComponent`를 첫 수신자로 사용한다.

다만 기존 command notify처럼 Action 내부 상태를 바로 변경하지 않고, active Action이 cue policy를 resolve한 뒤 ActionComponent가 source component로 라우팅한다.

```text
ActionComponent
-> active UCAction::ResolveNotifyCombatSignalCue
-> FActionCombatSignalCueRequest
-> ActionComponent route
```

이 구조는 `CombatSignalCue`가 Action 내부 명령이 아니라 외부 target으로 전달되는 combat signal 송신의 시작점이기 때문이다.

### 3. Source-side Cue Build / Send 추가

`UCCombatSignalSourceComponent`에 cue 전용 source entry와 helper를 추가했다.

```text
RequestAICombatSignalCue(CueTag)
-> ResolveCueTargetActor
-> RequestCombatSignalCue
-> BuildCueSignal
-> ValidateCueSignal
-> SendCueSignal
```

현재 target discovery는 AI blackboard의 `CAIKey::Targeting::TargetActor`만 사용한다.

Cue 전용 API에서는 damage 입력을 받지 않는다. TimingCue notify는 cue 종류와 발생 시점만 전달하고, damage / resource commit 후보값은 후속 target-side policy 또는 outcome 단계에서 판단한다.

### 4. Target-side TimingCue Hook 추가

`UCCombatSignalTargetComponent`에서 `ECombatSignalType::TimingCue`를 받을 수 있는 hook을 추가했다.

현재 범위에서는 Blink / Repulse 실제 동작을 수행하지 않고, 다음 cue tag 분기와 수신 여부만 확인한다.

```text
Combat.Cue.Blink
Combat.Cue.Repulse
```

지원하지 않는 cue tag는 다음 reject reason으로 기록한다.

```text
ECombatSignalTargetRejectReason::UnknownCueTag
```

### 5. 문서 갱신

W04 work list, TB W04-07, N06 implementation plan, prompt update note에 이번 결정과 후속 순서를 반영했다.

특히 `ResultOut`은 별도 선행 리팩터링으로 먼저 만들지 않고, Repulse v1에서 필요한 최소 결과 반환을 구현한 뒤 기존 ParryStack / Stagger 흐름과 후속 통합하기로 정리했다.

---

## 검증

### 빌드

```text
PortfolioEditor Win64 Development
```

결과:

```text
성공
```

### 정적 확인

- `git diff --check` 통과
- Notify가 Source / Target을 직접 찾지 않는지 확인
- Action이 SourceComponent를 직접 알지 않는지 확인
- cue 전용 API에서 damage 입력 제거 확인
- 기존 hit damage route 유지 확인

### 런타임 확인

Enemy attack montage의 `UCAnimNotify_CombatSignalCue`에서 `Combat.Cue.Blink`를 발생시켜 Player target까지 전달되는 것을 확인했다.

```text
[CombatSignalTimingCue] Blink cue received
[CombatSignalCueNotify] Sent | Source=BP_CEnemy_C_1 | CueTag=Combat.Cue.Blink
```

확인된 흐름:

```text
Enemy AnimNotify
-> Enemy UCActionComponent
-> active UCAction cue policy resolve
-> Enemy UCActionComponent route
-> Enemy UCCombatSignalSourceComponent
-> FCombatSignal TimingCue 구성
-> Player UCCombatSignalTargetComponent
-> HandleTimingCueSignal
```

---

## 제외 범위

이번 PR에서는 다음 작업을 의도적으로 제외했다.

- Blink movement 구현
- Repulse force / stagger 구현
- Repulse result-out 구현
- ParryStack / Stagger result-out 공통화
- Targeting Service / Subsystem 신규 구현
- UE `TakeDamage()` route 변경
- Guard / Parry 기존 판정 변경
- 기존 damage amount 계산 변경
- Combat Feedback / HitFeedback 명칭 정리

---

## 후속 작업

권장 후속 순서는 다음과 같다.

```text
1. feature/combat-blink-cue-v1
2. feature/combat-repulse-cue-v1
   - 최소 ResultOut 포함
3. refactor/combat-result-out-v1
   - Repulse / ParryStack / Stagger result 흐름 통합
4. refactor/combat-feedback-boundary
5. refactor/combat-signal-reference-validation
```

다음 작업은 `feature/combat-blink-cue-v1`을 추천한다.

Blink는 Repulse보다 상호 action / reaction 정합 부담이 낮고, 이번 PR에서 만든 TimingCue delivery hook을 실제 player defensive movement로 소비하는 첫 사례가 된다.

---

## 관련 문서

- `Docs/01_Work_List/W04_Combat_Signal_Boundary/W04_UE5_Portfolio_Work_List.md`
- `Docs/06_notes/N04_Blink_Repulse_Combat_Packet_Design_Note.md`
- `Docs/06_notes/N05_Combat_Signal_Boundary_Design_Note.md`
- `Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md`
- `Docs/06_notes/task_briefs/W04_Combat_Signal_Boundary/TB_W04_07_Combat_Signal_Cue_v1.md`
- `Docs/06_notes/prompt_updates/PU01_Combat_Signal_Boundary_Prompt_Update_Note.md`
