# UE5 Portfolio Pull Request

## 제목

**P08: TakeDamage Pipeline 구현**

## 날짜

**2026.01.06**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-take-damage`

---

## 요약

이번 PR에서는 **Enemy가 공격을 받았을 때 Unreal damage event를 수신하고, damage 계산 결과를 HP에 반영하는 흐름을 구성했다.**

이를 통해 Enemy actor가 damage 처리 전체를 직접 들고 있지 않고, damage 수신과 HP 관리를 각각 component 책임으로 나눌 수 있게 됐다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Enemy damage 수신 진입점 연결**: Enemy가 Unreal `TakeDamage()` event를 받고 실제 damage 처리는 별도 component로 위임하도록 구성했다.

- **TakeDamage 처리 단계 구성**: 들어온 damage event를 원본 요청, 처리 중 상태, 최종 결과 단계로 나누어 처리하도록 정리했다.

- **HP 반영 흐름 연결**: accepted damage가 `UCHealthComponent`에 반영되고, HP 감소와 dead flag가 갱신되도록 연결했다.

### Refactoring

- **Actor와 damage 처리 책임 분리**: Enemy actor는 damage event entry와 routing만 담당하고, damage 해석과 계산은 `UCTakeDamageComponent`가 담당하도록 나눴다.

- **Health resource 책임 분리**: HP clamp, damage / heal 적용, dead state 갱신을 `UCHealthComponent` 책임으로 분리했다.

- **Damage 결과 추적 구조 정리**: damage 요청값, 계산 중 context, 최종 result를 구조체로 분리해 로그와 후속 확장 지점에서 추적할 수 있게 했다.

---

## 핵심 개념

이 섹션은 이후 설명에서 반복되는 최소 용어를 먼저 정리한다.

이 PR의 핵심 흐름은 Enemy가 Unreal damage event를 수신한 뒤, `UCTakeDamageComponent`가 damage 처리 단계를 구성하고 `UCHealthComponent`가 HP 반영과 dead state를 관리하는 구조다.

```text
TakeDamage Pipeline(damage 수신 처리 흐름)
-> Unreal TakeDamage event를 받아 damage 요청을 검증하고 HP 반영 결과를 만드는 흐름
-> 이 PR에서는 Enemy entry, UCTakeDamageComponent, UCHealthComponent 책임으로 나눔
```

```text
UCTakeDamageComponent(damage 수신 처리 component)
-> damage event type을 확인하고 payload / context / result를 구성함
-> damage 요청을 검증하고 accepted damage를 health commit으로 넘김
```

```text
UCHealthComponent(HP 관리 component)
-> MaxHP, CurrentHP, PreviousHP, dead flag를 관리함
-> damage / heal 적용과 dead state 갱신을 담당함
```

```text
Payload / Context / Result(요청 값 / 처리 중 상태 / 처리 결과)
-> Payload는 Unreal entry와 apply damage 쪽에서 넘어온 원본 요청 값
-> Context는 처리 중 검증과 계산에 사용하는 resolved 상태
-> Result는 accepted 여부, reject reason, 최종 damage 값을 담는 처리 결과
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 Enemy damage 수신 흐름에서 분리해야 했던 책임을 정리한다.

### Enemy damage 수신 진입점 필요성

ApplyDamage 쪽에서 target에게 damage event를 보내더라도, Enemy가 이를 수신하는 entry가 없으면 실제 HP 감소 흐름으로 이어질 수 없었다.

Enemy는 Unreal `TakeDamage()`를 override해 engine damage event를 받아야 했다.

### Actor 내부 책임 비대화 방지 필요성

Enemy actor 안에서 damage event 해석, instigator resolve, target 상태 검증, HP 반영, dead 판정을 모두 처리하면 actor 책임이 커질 수 있었다.

Actor는 damage event를 받는 entry와 component routing에 집중하고, 실제 damage 처리 절차는 별도 component로 분리할 필요가 있었다.

### HP resource 관리 분리 필요성

Damage 처리 결과가 실제 HP 감소와 dead state로 이어지려면 HP 값을 관리하는 책임이 명확해야 했다.

Damage 계산과 HP resource 관리를 분리해야 이후 reaction, feedback, UI 같은 후속 처리를 같은 결과값 기준으로 확장할 수 있었다.

---

## 변경 범위

이 섹션은 Enemy가 damage event를 수신하고 HP 반영 결과를 만들기 위해 어떤 책임과 흐름을 구성했는지 정리한다.

### 1. Enemy TakeDamage entry 구성

- **왜**:
  ApplyDamage 쪽에서 전달한 engine damage event가 Enemy 내부 damage 처리 흐름으로 들어오는 진입점이 필요했다.

- **어떻게**:
  `ACEnemy::TakeDamage()`를 override하고, 최소 validation 이후 실제 처리는 `UCTakeDamageComponent::RequestTakeDamage()`로 위임했다.
  `UCTakeDamageComponent`가 없을 때는 받은 damage amount를 fallback 값으로 유지했다.

- **결과**:
  Enemy는 Unreal damage event를 수신하고, actor 내부에서 직접 처리하지 않고 component pipeline으로 넘긴다.

### 2. DamageEvent type routing 구성

- **왜**:
  Unreal `FDamageEvent`는 여러 type으로 확장될 수 있으므로, 현재 branch에서 지원하는 damage event type을 명확히 구분해야 했다.

- **어떻게**:
  `FDamageEvent::IsOfType()`으로 `FDefaultDamageEvent` 여부를 확인하고, default damage event는 `HandleDefaultDamageEvent()`에서 처리하도록 routing했다.

- **결과**:
  지원하는 damage event type은 명시적인 처리 경로로 들어가고, 지원하지 않는 type은 원본 damage amount를 반환하는 fallback 흐름으로 남는다.

### 3. Payload / Context / Result 단계 구성

- **왜**:
  Damage 처리에는 원본 입력, resolved 객체와 HP snapshot, 최종 처리 결과가 함께 필요했다.
  이 값들이 한 함수 안에서 섞이면 검증, 계산, 로그, 후속 확장 지점을 추적하기 어렵다.

- **어떻게**:
  `FDefaultDamageEvent`와 engine entry 값을 `FTakeDamagePayload`로 모으고, 처리에 필요한 resolved 객체와 HP snapshot을 `FTakeDamageContext`로 구성했다.
  최종 accepted 여부와 damage amount는 `FTakeDamageResult`에 기록했다.

- **결과**:
  Damage 수신 흐름은 요청 값, 처리 중 상태, 처리 결과를 구분해 추적할 수 있게 됐다.

### 4. Instigator fallback resolve 구성

- **왜**:
  Engine entry로 들어온 instigator가 비어 있어도, damage causer 또는 owner 관계에서 공격 주체를 추적해야 할 수 있었다.

- **어떻게**:
  `EventInstigator`가 있으면 우선 사용하고, 없으면 `DamageCauser`, causer pawn, causer owner, owner pawn 순서로 controller를 탐색했다.

- **결과**:
  Damage source 추적이 engine entry 값 하나에만 의존하지 않고, causer / owner 관계를 따라 보정된다.

### 5. TakeDamage evaluate / commit 구성

- **왜**:
  Damage 요청은 target, causer, instigator, dead state 같은 조건을 통과한 경우에만 HP에 반영되어야 했다.

- **어떻게**:
  `ValidateRequest -> BuildPayload -> BuildContext -> EvaluateTakeDamage -> CommitTakeDamage` 순서로 처리 흐름을 나눴다.
  Evaluate 단계에서 invalid target / causer / instigator / already dead 조건을 reject reason으로 기록하고, accepted damage만 health commit으로 넘겼다.

- **결과**:
  Damage 적용 여부와 reject 이유를 분리해서 확인할 수 있고, accepted damage만 HP에 반영된다.

### 6. UCHealthComponent 구성

- **왜**:
  HP 값, damage / heal 적용, dead flag 갱신은 damage event 해석과 별도 책임으로 관리되어야 했다.

- **어떻게**:
  `UCHealthComponent`를 만들고 `MaxHP`, `PreviousHP`, `CurrentHP`, `bIsDead`를 관리하도록 했다.
  Damage / heal 적용 시 HP를 clamp하고, HP 변화 이후 dead state를 갱신하도록 구성했다.

- **결과**:
  Enemy의 HP resource와 dead state는 `UCHealthComponent`에서 일관되게 관리된다.

### 7. Debug output 구성

- **왜**:
  TakeDamage pipeline은 여러 단계로 나뉘므로, 요청 객체와 damage amount, HP 변화, dead state를 확인할 수 있어야 했다.

- **어떻게**:
  `UCTakeDamageComponent`는 object / spec key / damage amount 정보를 출력하고, `UCHealthComponent`는 HP 변화와 dead state를 출력하도록 로그 함수를 구성했다.

- **결과**:
  단일 damage event가 어떤 객체와 값으로 처리됐는지 로그에서 추적할 수 있다.

---

## 주요 처리 흐름

이 섹션은 Enemy가 damage event를 수신한 뒤 HP 반영 결과를 만들기까지의 대표 흐름을 정리한다.

### TakeDamage Entry 흐름

```text
ApplyDamage dispatch
-> Enemy TakeDamage 진입
-> damage amount 최소 검증
-> UCTakeDamageComponent 존재 확인
-> RequestTakeDamage 호출
-> 처리 결과 damage 반환
```

이 흐름은 engine damage event가 Enemy actor entry를 거쳐 component pipeline으로 넘어가는 과정을 의미한다.

### TakeDamage Processing 흐름

```text
RequestTakeDamage
-> DamageEvent type 확인
-> FDefaultDamageEvent 처리 경로 진입
-> request validation
-> payload 구성
-> context 구성
-> evaluate
-> commit
-> result 반환
```

이 흐름은 damage 요청이 원본 입력, 처리 중 상태, 최종 결과 단계로 정리되는 과정을 의미한다.

### Health Commit 흐름

```text
accepted damage
-> UCHealthComponent TakeDamage 호출
-> CurrentHP clamp
-> 실제 적용 damage 계산
-> dead state 갱신
-> FinalAppliedDamage 기록
```

이 흐름은 accepted damage가 실제 HP 감소와 dead state 갱신으로 이어지는 과정을 의미한다.

### Instigator Resolve 흐름

```text
EventInstigator 확인
-> DamageCauser instigator 확인
-> DamageCauser pawn controller 확인
-> DamageCauser owner instigator 확인
-> owner pawn controller 확인
-> 모두 없으면 nullptr
```

이 흐름은 engine entry에 instigator가 없을 때 damage source를 causer / owner 관계에서 보정하는 과정을 의미한다.

---

## 구현 결과

- Enemy는 Unreal `TakeDamage()` event를 수신하고 `UCTakeDamageComponent`로 damage 처리를 위임할 수 있다.

- `UCTakeDamageComponent`는 default damage event를 payload / context / result 단계로 처리할 수 있다.

- Invalid target / causer / instigator / already dead 조건은 reject reason으로 기록된다.

- Accepted damage는 `UCHealthComponent`에 반영되고, HP clamp와 dead state 갱신이 수행된다.

- Reaction / feedback은 직접 실행하지 않고, commit 이후 확장 지점으로 남는다.

---

## 테스트 방법

### TakeDamage Entry

- Enemy에 `UCTakeDamageComponent`와 `UCHealthComponent`가 부착되어 있는지 확인한다.

- ApplyDamage 흐름에서 target Enemy의 `TakeDamage()`가 호출되는지 확인한다.

- `ACEnemy::TakeDamage()`가 직접 damage 계산을 하지 않고 `UCTakeDamageComponent::RequestTakeDamage()`로 위임하는지 확인한다.

### DamageEvent Routing

- `FDefaultDamageEvent`가 `FDamageEvent`를 통해 전달되는지 확인한다.

- `DamageEvent.IsOfType(FDefaultDamageEvent::ClassID)` 분기가 default damage event를 구분하는지 확인한다.

- 지원하지 않는 damage event type이 들어왔을 때 default 처리와 구분되는지 확인한다.

### Payload / Context / Result

- `FTakeDamagePayload`에 damaged actor, instigator, damage causer, apply damage metadata가 기록되는지 확인한다.

- `FTakeDamageContext`에 resolved instigator와 HP snapshot이 기록되는지 확인한다.

- Invalid target / causer / instigator / already dead 상황에서 reject reason이 설정되는지 확인한다.

### Health Commit

- Accepted damage가 `UCHealthComponent::TakeDamage()`로 전달되는지 확인한다.

- HP가 0 아래로 내려가지 않고 clamp되는지 확인한다.

- HP가 0이 되었을 때 dead flag가 true로 전환되는지 확인한다.

### Debug Output

- `PrintTakeDamageSummaryInfo()`에서 object / damage amount 정보가 출력되는지 확인한다.

- `UCHealthComponent` 로그에서 HP 변화량과 dead state가 확인되는지 확인한다.

---

## 검증 결과

- `ACEnemy::TakeDamage -> UCTakeDamageComponent::RequestTakeDamage` routing을 확인했다.

- `FDefaultDamageEvent` type routing을 확인했다.

- Payload / Context / Result 기반 TakeDamage 처리 흐름을 확인했다.

- `ResolveInstigatorController()` fallback 흐름을 확인했다.

- `UCHealthComponent` HP clamp, damage 적용, dead flag update를 확인했다.

- TakeDamage / Health debug output을 확인했다.

---

## 비범위

- Reaction 실행과 feedback 실행은 이번 PR에서 직접 구현하지 않고 commit 이후 확장 지점으로 남긴다.

- Shield / invulnerable / friendly fire / damage cooldown / zero damage 같은 세부 gate는 후속 확장 범위로 남긴다.

- Player와 Enemy가 함께 사용하는 공통 damage 구조는 이후 PR에서 확장한다.

---

## 관련 문서

- Issue Checklist: `D09_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

P08은 Enemy가 engine damage event를 수신하고 HP 반영 결과를 만들 수 있도록 receiver-side TakeDamage pipeline을 구성한 PR이다.

Enemy actor는 damage entry와 routing만 담당하고, damage event 해석과 결과 생성은 `UCTakeDamageComponent`, HP resource 관리는 `UCHealthComponent`가 담당하도록 책임을 분리했다.

이 구조를 통해 이후 reaction 실행, feedback 실행, Player / Enemy 공통 damage 구조로 확장할 수 있는 receiver-side damage 처리 흐름을 확보했다.
