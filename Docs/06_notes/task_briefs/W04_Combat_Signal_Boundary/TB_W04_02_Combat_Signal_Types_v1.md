# TB W04-02 Combat Signal Types v1

## 작업명

```text
Combat Signal Types v1
```

## 브랜치

```text
refactor/combat-signal-boundary
```

## 목표

`CombatSignalSource`와 `CombatSignalTarget`이 공유할 최소 타입 vocabulary를 추가한다.

이번 작업은 기존 `ApplyDamageComponent`, `TakeDamageComponent`, Action / Reaction / Feedback 흐름에 연결하지 않고, 후속 리팩터링에서 사용할 공용 타입 기반만 만든다.

## 배경

W04-01에서 전투 송수신 경계를 `CombatSignal Source / Target` 기준으로 재정의했다.

이제 다음 리팩터링이 같은 단어를 사용하려면 코드에도 최소 타입이 필요하다. 다만 아직 기존 runtime 흐름에 연결하면 회귀 위험 축이 바뀌므로, 이번 작업은 타입 추가로 제한한다.

## 핵심 범위

- `Source/Portfolio/Type/CCombatSignalStructure.h` 추가
- `Source/Portfolio/Type/CCombatSignalStructure.cpp` 추가
- `ECombatSignalType` 정의
- `ECombatSignalOutcome` 정의
- `ECombatSignalResultType` 정의
- `FCombatSignalHeader` 정의
- `FCombatSignal` 정의
- `FCombatSignalContext` 정의
- `FCombatSignalEvaluation` 정의
- `FCombatSignalApplyResult` 정의
- `FCombatSignalResult` 정의
- 각 struct에 `IsValidMinimal()` 기준 추가

## 제외 범위

- `UCApplyDamageComponent` include / 호출 연결
- `UCTakeDamageComponent` include / 호출 연결
- 기존 `FApplyDamagePayload`, `FTakeDamagePacket`, `FCombatResultPacket` 교체
- component rename
- Blink / Repulse 구현

## 결정 사항

새 타입은 기존 `CWeaponStructure.h`에 의존하지 않는다.

이유:

- 이번 단계는 vocabulary 추가가 목적이다.
- `FApplyDamageSpecKey`, `EReactionType`, `FTakeDamagePacket`에 바로 묶으면 기존 damage pipeline과 결합된다.
- 후속 branch에서 adapter를 만들 때 기존 타입과 CombatSignal 타입의 매핑을 명시적으로 결정하는 편이 안전하다.

따라서 v1 타입은 actor, tag, amount, vector, result state 중심으로 얇게 둔다.

## 완료조건

- 새 타입 파일이 추가되어 있다.
- 기존 gameplay 흐름에 연결되지 않는다.
- 기존 combat/action/reaction 코드 변경이 없다.
- Unreal build가 성공한다.

## 검증

정적 확인:

```text
git diff -- Source/Portfolio/Type/CCombatSignalStructure.h Source/Portfolio/Type/CCombatSignalStructure.cpp
rg -n "CCombatSignalStructure" Source/Portfolio
```

빌드 확인:

```text
PortfolioEditor Win64 Development
```

결과:

```text
성공
```

## 프롬프트 업데이트 확인

추가 프롬프트 업데이트 후보 없음.

W04-01의 PU01에 이미 다음 기준을 기록했다.

```text
collision hit와 timing cue, defensive outcome을 함께 다룰 때는 CombatSignal vocabulary를 우선 검토한다.
```
