# UE5 Portfolio Pull Request

## 제목

**P20: Guard / Parry Action v1 구현**

## 날짜

**2026.06.21**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-guard-parry`

---

## 요약

이번 PR에서는 **Guard / Parry Action v1을 기존 combat 실행 구조에 연결하고, damage 처리 전단에서 Parry / Guard / Hit 결과가 안정적으로 분기되도록 정리했다.**

핵심은 Parry를 완성형 방어 시스템으로 한 번에 확장하는 것이 아니라, Guard In / Hold / Out lifecycle, Parry Window, Guard Hold overlay, damage packet 판정, reaction 분기, attacker 측 result 전달까지 v1 범위에서 검증 가능한 전투 흐름으로 묶는 것이다.

Guard Hold는 별도 active action으로 끌어올리지 않고 observable overlay state로 유지했다. Guard In / Guard Out은 action 전환으로 처리하고, Guard Hold 중 발생하는 BlockHit / Parry / 일반 Hit는 damage 결과와 reaction orchestration을 통해 분기하도록 구성했다.

성격별 핵심 변경은 다음과 같다.

### Gameplay / Combat

- **Guard lifecycle 구성**: Guard 입력을 `Pressed -> Guard In`, `Released -> Guard Out` 흐름으로 연결하고, Guard Hold는 `UCDefenseComponent`의 observable overlay state로 유지했다.

- **Parry / Guard 판정 분리**: Guard In 초반에는 Parry Window를 열고, `SwitchToGuard` notify 이후에는 Guard 판정을 열어 damage packet이 Parry / Guard / Hit으로 분기되도록 만들었다.

- **Defensive Outcome 도입**: `TakeDamage` 결과에 `EDamageDefenseOutcome`을 남겨 Parry, Guard, 일반 Hit가 reaction / feedback / result packet 흐름에서 추적되도록 했다.

- **BlockHit / Parry / Stagger reaction 연결**: Guard 피격은 `BlockHit`, Parry 성공은 `Parry`, Parry stack threshold 도달은 `Stagger` reaction으로 연결했다.

- **CombatResultPacket 전달**: Parry 성공 결과를 attacker 측 receiver로 전달하고, receiver가 ParryStack을 누적해 Stagger request를 생성할 수 있게 했다.

### Refactoring / Architecture

- **Observable Overlay Layer 정리**: Guard Hold를 active execution이 아니라 observable overlay로 보고, action / reaction 시작 전 overlay condition과 handling을 확인하는 구조를 만들었다.

- **Overlay policy registry 안정화**: overlay policy registry를 dirty flag 기반으로 갱신해 stale policy로 인한 handling 실패를 방지했다.

- **Deferred Guard Out 처리**: Guard In 또는 BlockHit 중 release 입력이 들어오면 Guard Out candidate를 deferred로 저장하고, 올바른 consume 시점에 실행하도록 정리했다.

- **Action / Reaction stop 경계 정리**: orchestration intervention으로 발생하는 `Interrupt`와 fallback 성격의 `Stop`을 구분하고, 공통 종료 처리는 내부 stop pipeline으로 모았다.

- **Guard movement mode 전환**: Guard 상태에서는 카메라 추종 회전 없이 현재 facing을 유지하는 Guard locomotion 기준 movement override를 적용하고, Guard 종료 / interrupt / Hit / Parry 이후 기본 movement mode로 복구되도록 했다.

### Documentation

- **W03 Work List 최신화**: Guard / Parry v1 구현 범위, 최종 PIE 검증, 후속 분리 범위를 W03에 반영했다.

- **Bug Report 정리**: Guard release deferred 문제와 Guard Out 중 Hit reaction overlay handling 실패를 Bug Report로 기록했다.

- **Design Note 분리**: Guard Hold overlay layer, Blink / Repulse cue packet, Combat Request / Resolution / Routing 구조를 후속 설계 note로 분리했다.

---

## 핵심 개념

이 섹션은 이번 PR에서 반복해서 사용하는 combat 구조 용어를 먼저 정리한다.

```text
Guard In / Guard Out
-> 입력으로 시작하고 종료되는 Guard 전환 action
-> v1에서는 CAction_Guard가 In / Out phase만 담당한다.
```

```text
Guard Hold
-> Guard In 이후 입력 의도와 방어 상태가 유지되는 구간
-> active action이 아니라 observable overlay state로 관리한다.
```

```text
Parry Window
-> Guard In 초반 damage packet을 Parry로 판정할 수 있는 구간
-> SwitchToGuard notify 이전까지 열리고, 이후 Guard 판정으로 전환된다.
```

```text
Defensive Outcome
-> TakeDamage 처리 결과가 방어 관점에서 어떤 의미를 갖는지 나타내는 결과값
-> v1 값은 None / Guard / Parry이다.
```

```text
Observable Overlay Layer
-> exclusive execution은 비어 있어도 남아 있을 수 있는 관측 가능한 상태 계층
-> Guard Hold, LockOn, Aim, Crouch 같은 상태를 이 계층의 후보로 본다.
```

```text
CombatResultPacket
-> defender 측 damage 판정 결과를 attacker 또는 외부 대상에게 전달하기 위한 결과 packet
-> v1에서는 Parry 성공 결과를 attacker 측 receiver로 전달하는 데 사용한다.
```

---

## 변경 배경

이 섹션은 Guard / Parry v1 구현이 필요했던 이유와, 기존 combat 구조에서 먼저 해결해야 했던 문제를 설명한다.

### Guard Hold를 active action으로 볼지에 대한 판단 필요성

Guard In / Guard Out은 입력으로 시작하고 종료되는 명확한 action이지만, Guard Hold는 입력이 유지되는 동안 남는 자세와 gameplay 판정 상태에 가깝다.

Guard Hold를 active action으로 억지로 편입하면 Idle, Action, Reaction 사이의 exclusive execution 모델이 흐려질 수 있었다. 따라서 Guard Hold는 active execution이 아니라 observable overlay state로 보고, action / reaction 시작 시 해당 overlay가 실행 조건과 충돌하는지 확인하는 구조가 필요했다.

### Parry / Guard 판정을 TakeDamage 이전에 분기해야 하는 필요성

Parry와 Guard는 Hit reaction 이후에 처리할 수 있는 결과가 아니라, damage commit 이전에 packet을 어떻게 해석할지 결정해야 하는 방어 판정이다.

따라서 `TakeDamage` 내부에서 임시 defensive resolution 기준을 만들고, Parry는 damage commit을 막으며, Guard는 mitigated damage를 commit하고, 일반 Hit는 기존 흐름을 유지하도록 분기할 필요가 있었다.

### Reaction과 Feedback 흐름을 기존 구조 안에서 유지해야 하는 필요성

Parry와 Guard를 별도 예외 처리로만 구현하면 기존 Hit / Dead reaction, feedback, orchestration 흐름과 충돌할 수 있었다.

따라서 damage result에 `DefenseOutcome`을 남기고, Reaction Orchestrator가 `Parry`, `BlockHit`, `Hit`, `Dead`를 선택할 수 있도록 결과 구조를 보강했다.

### Attacker 측 후속 처리를 위한 result 전달 경계 필요성

Parry 성공은 defender 쪽 damage commit을 막는 것에서 끝나지 않고, attacker 쪽 feedback, stack, stagger, counter 흐름으로 이어질 수 있다.

이번 PR에서는 counter executor 전체를 구현하지 않고, Parry result를 attacker 측 receiver로 전달하고 ParryStack 기반 Stagger request가 생성되는 경계까지만 v1 범위로 정리했다.

---

## 변경 범위

이 섹션은 이번 PR에서 Guard / Parry v1을 어떻게 구현했고, 그 결과 무엇이 달라졌는지 설명한다.

### 1. Guard Action lifecycle 구성

- **왜**:
  Guard는 Pressed / Released 입력을 기준으로 In / Out이 분리되고, Hold는 입력 유지 상태와 ABP pose가 함께 남는 구조다.
  따라서 하나의 montage lifecycle로 모두 처리하기보다 In / Out action과 Hold overlay를 분리해야 했다.

- **어떻게**:
  `ECombatActionIntent::Guard`를 Guard action candidate로 해석하고, `CAction_Guard`가 Guard In / Guard Out 전환만 담당하도록 구성했다.
  Guard In 시작 시 Guard 입력 의도와 pose를 열고 Parry Window를 활성화했다.
  Guard Out 시작 시 Guard 판정과 Parry 판정을 닫고, Guard pose와 movement mode를 정리하도록 했다.

- **결과**:
  Guard 입력은 `Pressed -> Guard In`, `Released -> Guard Out`으로 흐르고, Hold는 active action이 아니라 Defense overlay state로 안정적으로 유지된다.

### 2. Parry Window와 Guard 판정 전환

- **왜**:
  Guard In 초반은 Parry 판정 구간이고, 이후는 일반 Guard 구간이다.
  이 전환을 notify state window로 이어 붙이면 montage 경계나 release 타이밍에 판정 공백 또는 잔여 판정이 생길 수 있었다.

- **어떻게**:
  Guard In 시작 시 `CanParry=true`, `CanGuard=false`로 시작하고, `SwitchToGuard` notify 시점에 `CanParry=false`, `CanGuard=true`로 전환했다.
  release가 이미 들어온 상태라면 `SwitchToGuard` 이후 Guard 판정을 열지 않고 Guard Out deferred 흐름으로 이어지도록 했다.

- **결과**:
  Parry Window와 Guard Hold 판정이 같은 Guard lifecycle 안에서 명확히 분리되고, damage packet은 현재 방어 상태에 따라 Parry / Guard / Hit으로 분기된다.

### 3. Defensive Outcome과 Reaction 분기

- **왜**:
  Parry는 damage commit을 막지만 후속 reaction / feedback / attacker result 처리가 필요하다.
  단순 reject로 처리하면 결과 의미가 사라지고, Reaction Orchestrator가 Parry reaction을 선택할 수 없다.

- **어떻게**:
  `EDamageDefenseOutcome`을 추가해 `None`, `Guard`, `Parry`를 damage result에 남겼다.
  `ResolveDamageReactionType()`은 `Parry -> Parry`, `Dead transition -> Dead`, `Guard -> BlockHit`, `CommittedDamage > 0 -> Hit` 순서로 reaction type을 선택한다.
  `CReaction_BlockHit`, `CReaction_Parry`, `CReaction_Stagger`를 추가해 방어 결과별 reaction executor를 분리했다.

- **결과**:
  Parry는 damage commit 없이 Parry reaction으로 이어지고, Guard는 mitigated damage와 BlockHit reaction으로 이어지며, 일반 피격은 기존 Hit / Dead reaction 흐름을 유지한다.

### 4. Deferred Guard Out 처리

- **왜**:
  Guard In 또는 BlockHit 중 release 입력이 들어오면 즉시 Guard Out을 실행할 수 없는 구간이 생긴다.
  request를 다시 호출하는 방식은 동일 검증을 반복하고, consume 시점의 책임이 흐려질 수 있었다.

- **어떻게**:
  release request를 Guard Out candidate로 먼저 해석한 뒤, 현재 실행 상태에 따라 deferred candidate로 저장했다.
  Guard In 완료 후에는 `AfterGuardInAction`, BlockHit 완료 후에는 `AfterGuardBlockReaction` consume key로 Guard Out candidate를 소비한다.
  deferred 저장 / 소비는 Orchestrator가 담당하고, defer 여부와 key 판단은 executor hook으로 분리했다.

- **결과**:
  Guard In 또는 BlockHit 중 release가 들어와도 Hold에 갇히지 않고, 올바른 시점에 Guard Out으로 이어진다.

### 5. Observable Overlay Layer와 policy registry 안정화

- **왜**:
  Guard Hold는 active action이 아니지만, Dodge / Hit / Parry / Guard Out 같은 실행과 충돌할 수 있는 상태다.
  따라서 execution decision은 active action / reaction뿐 아니라 observable overlay state도 함께 확인해야 했다.

- **어떻게**:
  `UCObservableOverlayComponent`를 추가해 overlay snapshot 작성, overlay event 적용, overlay handling 적용을 라우팅했다.
  `UCDefenseComponent`는 Guard overlay state owner이자 overlay policy 구현체로 유지했다.
  overlay policy registry는 dirty flag 기반으로 갱신해, 런타임 캐시가 stale 상태로 남아 handling 적용에 실패하지 않도록 했다.

- **결과**:
  Guard Hold는 action / reaction과 독립된 overlay state로 관측되고, execution 시작 전 필요한 경우 overlay clear / restore / lifecycle event가 안정적으로 처리된다.

### 6. Guard movement mode와 asset 연결

- **왜**:
  Guard Hold 중에는 일반 이동 방향 회전이 아니라 Guard 8-way locomotion 기준으로 움직여야 한다.
  또한 Guard In / Out / BlockHit / Parry / Stagger의 asset 연결이 실제 PIE 흐름과 맞아야 했다.

- **어떻게**:
  Guard state 진입 시 Guard movement override를 적용하고, Guard Out / Hit / Parry / interrupt 이후 기본 movement mode로 복구되도록 했다.
  Guard 8-way locomotion은 카메라 방향으로 캐릭터를 회전시키지 않고, 현재 facing을 유지한 채 이동 방향만 BlendSpace에 반영하도록 정리했다.
  `BS_Guard`와 Guard / Block / Parry / Stagger montage, ActionData, ReactionData 연결을 Editor에서 정리했다.
  Action / Reaction data에는 montage start section을 지원해 긴 준비 동작을 데이터 기준으로 조정할 수 있게 했다.

- **결과**:
  Guard Hold 중 8-way locomotion이 적용되고, Guard 종료 또는 reaction 전환 이후 기본 movement mode로 복구된다.
  관련 animation asset 참조와 naming도 현재 프로젝트 규칙에 맞게 정리됐다.

### 7. Parry result packet과 Stagger 연결

- **왜**:
  Parry 성공은 defender 쪽 결과이면서 attacker 쪽 후속 반응의 입력이 된다.
  후속 Counter / Blink / Repulse 구조를 위해서는 defender 결과를 attacker 측에 전달하는 경계가 필요했다.

- **어떻게**:
  Parry 성공 시 `FCombatResultPacket`을 구성해 attacker 측 `ICombatResultReceiver`로 전달했다.
  receiver는 ParryStack을 누적하고, threshold인 3회에 도달하면 Stagger reaction request를 생성한다.
  v1에서는 counter executor 전체를 만들지 않고, Stagger request accepted까지 검증했다.

- **결과**:
  Parry 성공 3회 누적 후 enemy Stagger reaction request가 accepted되는 흐름을 확인했다.
  attacker reaction / counter action / Blink / Repulse 확장은 다음 Branch 범위로 분리됐다.

---

## 주요 처리 흐름

### Guard / Parry 입력과 판정 흐름

```text
Guard Pressed
-> Guard In action
-> Parry Window open
-> SwitchToGuard notify
-> Parry Window close / Guard open
-> Guard Hold overlay 유지
-> Guard Released
-> Guard Out action
-> Guard overlay 정리
```

### Damage packet 분기 흐름

```text
Incoming damage
-> TakeDamage context 구성
-> Defense state 조회
-> CanParry true
   -> DefenseOutcome=Parry
   -> damage commit 없음
   -> Parry reaction
   -> CombatResultPacket attacker 전달

-> CanGuard true
   -> DefenseOutcome=Guard
   -> mitigated damage commit
   -> BlockHit reaction

-> 그 외
   -> DefenseOutcome=None
   -> 기존 damage commit
   -> Hit / Dead reaction
```

### Parry result / Stagger 흐름

```text
Parry 성공
-> defender TakeDamage result 확정
-> CombatResultPacket 생성
-> attacker receiver 전달
-> ParryStack 누적
-> 3/3 도달
-> Stagger reaction request
-> Stagger accepted
```

---

## 구현 결과

- Guard 입력이 Guard In / Guard Out action으로 연결되고, Guard Hold는 observable overlay state로 관리된다.

- Parry Window와 Guard 판정 구간이 notify 기반으로 전환된다.

- damage result에 Defensive Outcome이 남아 Parry / Guard / Hit이 명확히 분기된다.

- Parry는 damage commit 없이 Parry reaction / feedback / attacker result packet으로 이어진다.

- Guard는 v1 기준 mitigated damage와 BlockHit reaction으로 이어진다.

- Guard Out 중 피격은 Guard 판정을 사용하지 않고 일반 Hit damage / Hit reaction 흐름으로 전환된다.

- release 입력이 Guard In 또는 BlockHit 중 들어와도 deferred Guard Out candidate로 보관 후 올바른 시점에 소비된다.

- observable overlay policy registry는 dirty flag 기반으로 갱신되어 stale policy로 인한 handling 실패를 방지한다.

- Parry result packet이 attacker 측 receiver에 전달되고, ParryStack 3회 누적 시 Stagger request가 accepted된다.

- Guard locomotion, Guard / Parry / BlockHit / Stagger reaction asset 연결과 naming을 현재 프로젝트 기준에 맞게 정리했다.

---

## 테스트 방법

### Build

- `PortfolioEditor Win64 Development` 빌드를 실행해 Guard / Parry / Reaction / Overlay 관련 코드가 compile되는지 확인했다.

### PIE

- Guard 입력 시 Guard In montage가 재생되는지 확인했다.

- Guard 유지 시 Guard Hold overlay와 `BS_Guard` 8-way locomotion이 유지되고, 캐릭터 facing이 카메라를 따라 회전하지 않는지 확인했다.

- Guard release 시 Guard Out montage가 재생되고 기본 movement mode로 복구되는지 확인했다.

- Parry Window 중 피격 시 damage commit이 발생하지 않고 Parry reaction / feedback이 실행되는지 확인했다.

- Guard Hold 중 피격 시 mitigated damage와 BlockHit reaction이 실행되는지 확인했다.

- Guard In의 Guard 구간에서 피격 시 `Outcome=Guard`로 처리되는지 확인했다.

- Guard Out 중 피격 시 `Outcome=None`, `Commit=true`로 일반 Hit damage 흐름에 들어가는지 확인했다.

- Parry 3회 성공 시 attacker receiver에 ParryStack이 누적되고 Stagger request가 accepted되는지 확인했다.

### Editor / Asset

- Guard In / Out ActionData와 montage 연결을 확인했다.

- BlockHit / Parry / Stagger ReactionData, executor, montage 연결을 확인했다.

- Parry feedback data와 VFX / SFX 연결을 확인했다.

- Guard / Block / Parry / Stagger animation asset naming을 현재 규칙에 맞춰 정리했다.

---

## 검증 결과

- Guard Hold 3회 피격 시 `Outcome=Guard`, `Commit=true`로 처리되고 HP가 순차 감소하는 것을 확인했다.

- Parry 3회 성공 시 `Outcome=Parry`, `Commit=false`, `Damage=0`, HP 유지가 확인됐다.

- Parry result가 attacker 측 receiver로 전달되고 `ParryStack`이 `1/3 -> 2/3 -> 3/3`으로 누적되는 것을 확인했다.

- `ParryStack` threshold 도달 시 `StaggerRequest Result=Accepted`가 출력되는 것을 확인했다.

- Guard In 중 피격은 `Outcome=Guard`로 처리되는 것을 확인했다.

- Guard Out 중 피격은 `Outcome=None`, `Commit=true`로 일반 Hit damage 흐름에 들어가는 것을 확인했다.

- 최종 회귀 검증 로그는 `TakeDamageOutcome`, `CombatResultDispatch`, `CombatResult` 계열만 유지하도록 정리했다.

---

## 미검증 항목

- Network Replication

- Guard Gauge / stamina / posture / resource 연동

- Perfect Parry / Normal Parry 구분

- Blink / Repulse cue packet 실행 검증

- Combat Resolution component 분리 후 동일 회귀 검증

---

## 비범위

- `UCCombatResolutionComponent` 최종 분리

- `CombatConsequenceCoordinator` 분리

- `ApplyDamageComponent`의 CombatRequester 성격 정리 / 리네임

- Guard ActionData index 기반 phase 선택 제거

- Guard overlay serial / lifecycle id 기반 precise cleanup

- Counter executor / attacker counter action 처리

- Blink / Repulse cue packet 구현

- Perfect Parry / Normal Parry 구분

- Guard Gauge / stamina / posture / resource 처리

- 최종 VFX / SFX / camera shake / hit stop polish

---

## 후속 작업

- Combat Resolution component를 분리해 `TakeDamage` 내부의 defensive outcome 판단 책임을 독립 계층으로 옮긴다.

- CombatConsequenceCoordinator를 분리해 damage apply, reaction, feedback, external result dispatch의 후속 분배 책임을 정리한다.

- feedback 계열은 HitFeedback / ParryFeedback을 성급히 나누기보다 CombatFeedback 관점에서 hit point, timing, cue, reaction notify 기준을 다시 정리한다.

- Guard ActionData phase key를 숫자 index에서 명시적 phase / variant key로 정리한다.

- Guard overlay cleanup은 serial / lifecycle id 기반으로 정밀화한다.

- Blink / Repulse는 cue packet 기반 defensive outcome 후보로 정리하고, target discovery와 packet delivery 책임을 분리한다.

- ParryStack 이후 Counter executor / attacker reaction / Stagger recovery 정책을 별도 Branch에서 확장한다.

---

## 관련 문서

- Work List: `W03_UE5_Portfolio_Work_List.md`

- Work List Index: `00_Work_List_Index.md`

- Pull Request Index: `00_Pull_Request_Index.md`

- Bug Report:
  - `B11_UE5_Portfolio_Bug_Report.md`
  - `B12_UE5_Portfolio_Bug_Report.md`

- Notes:
  - `N02_Guard_Release_Deferred_Request_Note.md`
  - `N03_Guard_Hold_Overlay_Layer_Design_Note.md`
  - `N04_Blink_Repulse_Combat_Packet_Design_Note.md`
  - `N05_Combat_Intent_Request_Resolution_Routing_Design_Note.md`

---

## 정리

P20은 Guard / Parry Action v1을 기존 combat 실행 구조에 연결하고, damage packet이 Parry / Guard / Hit 결과로 안정적으로 분기되는지 검증한 PR이다.

이번 PR에서 Guard Hold는 observable overlay state로 정리됐고, Parry / BlockHit / Stagger는 defensive outcome과 reaction orchestration을 통해 연결됐다. 또한 Parry result packet이 attacker 측 receiver로 전달되어 ParryStack 기반 Stagger request까지 이어지는 v1 경계가 마련됐다.

Combat Resolution 분리, CombatConsequenceCoordinator 분리, feedback 계층 재정리, Counter / Blink / Repulse 확장은 후속 Branch에서 이어간다.
