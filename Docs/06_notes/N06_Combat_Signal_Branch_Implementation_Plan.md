# N06 Combat Signal Branch Implementation Plan

## 1. 목적

이 문서는 `CombatSignal` 구조를 실제 코드로 옮기기 위한 작업 분할 계획을 정리한다.

기준은 브랜치를 과도하게 잘게 나누지 않는 것이다. 문서로 확정한 vocabulary를 코드 타입으로 추가하는 작업은 같은 논리 단위이므로 `refactor/combat-signal-boundary` 브랜치 안에서 함께 처리한다. 실제 gameplay 흐름에 연결되는 리팩터링부터 별도 브랜치로 나눈다.

## 2. 전체 순서

```text
1. refactor/combat-signal-boundary
   - CombatSignal 경계 문서화
   - CombatSignal 타입 vocabulary 추가

2. refactor/combat-signal-target-v1
   - TakeDamageComponent target-side 책임 정리

3. refactor/combat-signal-source-v1
   - ApplyDamageComponent source-side 책임 정리

4. refactor/combat-signal-component-rename
   - 책임 정리 이후 component rename

5. feature/combat-signal-cue-v1
   - Blink / Repulse timing cue 연결
```


## 3. 브랜치 분할 기준

브랜치 분할 기준은 기능 이름이 아니라 동작 변화의 성격과 회귀 위험이 달라지는 지점이다.

```text
1. 문서 / 타입만 추가하는가?
2. 기존 target-side 피격 흐름을 바꾸는가?
3. 기존 source-side 공격 송출 흐름을 바꾸는가?
4. 이름 / 참조를 크게 바꾸는가?
5. 새 gameplay behavior를 추가하는가?
```

따라서 분할 기준은 다음과 같다.

| 구분 | 브랜치 | 분리 이유 |
| --- | --- | --- |
| Boundary / Type | `refactor/combat-signal-boundary` | 문서와 타입 vocabulary만 다루며 기존 runtime 흐름에 연결하지 않는다. |
| Target-side refactor | `refactor/combat-signal-target-v1` | `TakeDamageComponent`는 Guard / Parry / Hit / Dead / feedback / attacker result에 직접 영향을 준다. |
| Source-side refactor | `refactor/combat-signal-source-v1` | `ApplyDamageComponent`는 hit window / duplicate target / damage spec / target delivery에 직접 영향을 준다. |
| Component rename | `refactor/combat-signal-component-rename` | 책임 정리 후 이름과 참조를 바꾸며, 동작 변경보다 참조 변경 위험이 크다. |
| Cue feature | `feature/combat-signal-cue-v1` | Blink / Repulse timing cue는 새 gameplay behavior이므로 리팩터링과 분리한다. |

이 기준에 따라 `CombatSignal Types v1`은 별도 브랜치가 아니라 현재 boundary 브랜치의 두 번째 커밋으로 포함한다. 반면 `TakeDamageComponent`나 `ApplyDamageComponent`에 연결되는 순간부터는 회귀 위험 축이 달라지므로 별도 브랜치로 나눈다.

## 4. Branch 1: Combat Signal Boundary

브랜치명:

```text
refactor/combat-signal-boundary
```

목표:

```text
공용 상태 변경 파이프라인 일반화를 보류하고 CombatSignal Source / Target 기준을 문서와 최소 타입으로 확정한다.
```

핵심 범위:

- W04 work list 작성
- N05 design note 작성
- N06 implementation plan 작성
- task brief 작성
- work journal / prompt update note 작성
- `Source/Portfolio/Type/CCombatSignalStructure.h/.cpp` 추가
- CombatSignal 관련 enum / struct vocabulary 추가

제외 범위:

- `UCApplyDamageComponent` 연결
- `UCTakeDamageComponent` 연결
- 기존 `FApplyDamagePayload`, `FTakeDamagePacket`, `FCombatResultPacket` 교체
- component rename
- Blink / Repulse 구현

완료조건:

- `Request`, `Attack`, `Damage` 중심 이름이 핵심 파이프라인에서 제거된다.
- `CombatSignal Source / Target` 책임 경계가 문서화되어 있다.
- 입력 처리 축 / combat 처리 축 / timing cue 처리 축을 하나의 공용 Request 파이프라인으로 묶지 않는 이유가 문서화되어 있다.
- CombatSignal 최소 타입이 추가되어 있다.
- 기존 gameplay 동작 변화가 없다.
- Unreal build 성공.
- TB와 prompt update check가 남는다.

권장 커밋 단위:

```text
docs(combat): define combat signal boundary plan
feat(combat): add combat signal type vocabulary
docs(combat): record combat signal type task brief
```

## 5. Branch 2: Combat Signal Target Boundary v1

브랜치명:

```text
refactor/combat-signal-target-v1
```

목표:

```text
UCTakeDamageComponent 내부 흐름을 CombatSignalTarget 단계로 정리한다.
```

핵심 범위:

- `Receive` 단계 정리
- `Evaluate` 단계 정리
- `Apply` 단계 정리
- `Notify Result` 단계 정리
- 기존 `RequestTakeDamage` public API 유지
- UE `TakeDamage()` adapter 유지

제외 범위:

- 클래스 rename
- source-side component rename
- Blink / Repulse cue 구현

완료조건:

- Guard / Parry / Hit / Dead 기존 동작 유지.
- `TakeDamageComponent` 내부에서 target-side 단계가 명확히 보인다.
- `FTakeDamageResult`와 `FCombatSignalResult` 후보 관계가 문서화된다.
- Unreal build 성공.

## 6. Branch 3: Combat Signal Source Boundary v1

브랜치명:

```text
refactor/combat-signal-source-v1
```

목표:

```text
UCApplyDamageComponent 내부 흐름을 CombatSignalSource 단계로 정리한다.
```

핵심 범위:

- hit window tracking 단계 정리
- duplicate target tracking 단계 정리
- source-side validation 정리
- signal build 후보 경계 정리
- target delivery 경계 정리

제외 범위:

- 클래스 rename
- target-side behavior 변경
- cue-based Blink / Repulse 구현

완료조건:

- 기존 weapon overlap damage 동작 유지.
- source-side 단계가 코드에서 명확히 보인다.
- target delivery가 장기적으로 `ReceiveCombatSignal`로 바뀔 수 있는 형태로 정리된다.
- Unreal build 성공.

## 7. Branch 4: Combat Signal Component Rename

브랜치명:

```text
refactor/combat-signal-component-rename
```

목표:

```text
책임 정리 이후 component 이름을 CombatSignal 기준으로 맞춘다.
```

핵심 범위:

- `UCApplyDamageComponent` -> `UCCombatSignalSourceComponent`
- `UCTakeDamageComponent` -> `UCCombatSignalTargetComponent`
- 파일명 갱신
- include 갱신
- Character / Weapon 참조 갱신
- Blueprint 생성 이름 영향 확인
- renamed native component reference 검증 / 복구
- native component rename reference issue 문서화

제외 범위:

- 새로운 combat behavior 추가
- cue feature 추가
- 전체 character component cache validation 정책 변경

완료조건:

- 이름 변경 전후 동작 차이가 없다.
- 기존 전투 회귀가 통과한다.
- Unreal build 성공.
- redirect / Blueprint 영향이 기록된다.
- reference recovery 범위가 rename 대상 combat signal component로 제한된다.
- B13 / N07에 현상, 원인, 해결 방식이 기록된다.

## 8. Branch 5: Combat Damage Data Types

브랜치명:

```text
refactor/combat-damage-data-types
```

목표:

```text
CombatSignalSource / CombatSignalTarget 리네임 이후 남아 있는 damage data 타입명을 실제 책임 기준으로 정리한다.
```

핵심 범위:

- `FApplyDamageSpecKey` -> `FDamageSpecKey`
- `FApplyDamageSpec` -> `FDamageSpec`
- `FApplyDamageAmount` -> `FDamageAmount`
- `FApplyDamageHitWindowKey` -> `FCombatSignalHitWindowKey`
- `EApplyDamageRejectReason` -> `ECombatSignalSourceRejectReason`
- `ETakeDamageRejectReason` -> `ECombatSignalTargetRejectReason`
- damage spec / amount 사용처 이름 정리
- debug label / commit 주석 정리

제외 범위:

- UE `TakeDamage()` boundary rename
- `UCHealthComponent::TakeDamage` rename
- `FCombatResultPacket` 구조 변경
- combat damage 계산 로직 변경
- Guard / Parry / Defensive Outcome 변경

완료조건:

- damage data 타입명이 source component 이름에 과하게 종속되지 않는다.
- source-side hit window key와 source / target reject reason이 책임 기준 이름을 갖는다.
- 기존 combat runtime 동작이 바뀌지 않는다.
- Unreal build 성공.

## 9. Branch 6: Combat Signal Cue v1

브랜치명:

```text
feature/combat-signal-cue-v1
```

목표:

```text
Blink / Repulse 같은 collision 없는 timing cue가 CombatSignal 흐름으로 전달될 수 있는 최소 연결 지점을 만든다.
```

핵심 범위:

- `ECombatSignalType::TimingCue` 사용
- `UCAnimNotify_CombatSignalCue` 추가
- ActionComponent / active Action policy resolve 경유
- source-side cue build / validate / send
- target-side TimingCue receive hook
- Blink / Repulse cue tag 분기 hook

완료조건:

- collision hit와 timing cue가 같은 target receive 흐름을 공유한다.
- cue 전용 예외 파이프라인을 만들지 않는다.
- Enemy attack notify에서 target으로 TimingCue signal을 전달할 수 있다.
- Blink / Repulse 실제 movement / interaction 구현은 후속 브랜치로 분리한다.

## 10. 후속 순서

```text
1. feature/combat-blink-cue-v1
   - Combat.Cue.Blink를 실제 player defensive movement로 소비한다.
   - enemy 후방 재배치 / 성공 판정 / 기존 hit flow 회귀를 확인한다.
   - ResultOut은 최소화하거나 보류한다.

2. feature/combat-repulse-cue-v1
   - Combat.Cue.Repulse를 player action과 enemy reaction이 맞물리는 상호작용형 방어 행동으로 구현한다.
   - Repulse 성공에 필요한 최소 ResultOut을 이 브랜치 안에 포함한다.
   - attacker-side RepulseSuccess receive / reaction 연결을 검증한다.

3. refactor/combat-result-out-v1
   - Repulse에서 만든 최소 ResultOut 사례와 기존 ParryStack / Stagger 흐름을 비교해 공통화한다.
   - Parry / Repulse / GuardBreak / Counter 계열 결과 반환 기준을 정리한다.

4. refactor/combat-feedback-boundary
   - HitFeedback / CombatFeedback / DamageFeedback 책임과 명칭을 정리한다.

5. refactor/combat-signal-reference-validation
   - component reference validation을 전체 정책으로 일반화할지 판단한다.
```

`ResultOut`은 기능 사례 없이 선행 일반화하지 않는다. Repulse는 성공 결과가 attacker-side reaction으로 되돌아가야 하는 기능이므로, Repulse v1에서 필요한 최소 결과 반환을 먼저 구현한 뒤 기존 ParryStack / Stagger 흐름과 후속 통합한다.
