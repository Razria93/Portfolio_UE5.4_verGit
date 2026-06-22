# TB W04-01 Combat Signal Boundary 재정의

## 작업명

```text
Combat Signal Boundary Re-scope
```

## 브랜치

```text
refactor/combat-signal-boundary
```

## 목표

기존 `GameplayIntentGateway / GameplayCoordinator` 중심 계획을 보류하고, 전투 송수신 경계를 `CombatSignal Source / Target` 기준으로 재정의한다.

이번 작업은 코드 구현이 아니라 다음 코드 작업이 흔들리지 않도록 이름, 책임, 흐름, 작업 순서를 확정하는 문서 정리 작업이다.

## 배경

W03 이후 초기 후보는 다음 구조였다.

```text
Intent
-> Gateway
-> Coordinator
-> Resolution
-> Domain
```

하지만 검토 결과 현재 코드에서 가장 직접적인 문제는 공용 intent routing 부재가 아니라 `ApplyDamageComponent`와 `TakeDamageComponent`의 책임 및 이름 혼재였다.

특히 `Request`, `Attack`, `Damage`는 각각 다음 한계를 가진다.

- `Request`: request-response 흐름을 암시한다.
- `Attack`: source와 target의 전투 공방 전체를 담기에는 좁다.
- `Damage`: damage commit이 없는 Parry / Blink / Repulse outcome을 담기 어렵다.

따라서 핵심 vocabulary를 `CombatSignal`로 정한다.

## 핵심 범위

- W04 work list 작성
- N05 Combat Signal Boundary 설계 노트 작성
- N06 Combat Signal Branch 구현 계획 작성
- 작업 브리프 작성
- 작업일지 작성
- 프롬프트 업데이트 노트 작성
- 코드 변경 없음

## 제외 범위

- `CCombatSignalStructure` 타입 추가
- `ApplyDamageComponent` 수정
- `TakeDamageComponent` 수정
- component rename
- Blink / Repulse 구현
- Intent Gateway / Coordinator 코드 복구

## 결정 사항

확정한 구조:

```text
UCCombatSignalSourceComponent
-> FCombatSignal
-> UCCombatSignalTargetComponent
-> EvaluateCombatSignal
-> ApplyCombatSignalOutcome
-> NotifyCombatSignalResult
```

보류한 구조:

```text
GameplayIntentGateway
GameplayCoordinator
CombatRequestSource
CombatReceiver
CombatConsequenceCoordinator
```

보류 이유:

- 현재 단계에서 공용 라우팅 계층을 만들면 God Object 위험이 크다.
- 전투 파이프라인의 핵심 문제는 source-side / target-side boundary 정리다.
- 더 작은 두 컴포넌트 축으로 시작하는 편이 기존 코드와 맞다.

## 완료조건

- W04 work list가 CombatSignal 기준으로 작성되어 있다.
- N05 설계 노트가 `CombatSignal Source / Target` 책임을 설명한다.
- N06 구현 계획이 feature/refactor 브랜치를 분리한다.
- 다음 작업으로 같은 브랜치의 `Combat Signal Types v1`을 바로 제안할 수 있다.
- 코드 변경이 없다.

## 검증

문서 추가 작업이므로 빌드는 수행하지 않는다.

검증 기준:

```text
git status --short
rg -n "GameplayIntentGateway|GameplayCoordinator" Docs/01_Work_List/W04_Combat_Signal_Boundary Docs/06_notes/N05_Combat_Signal_Boundary_Design_Note.md Docs/06_notes/N06_Combat_Signal_Branch_Implementation_Plan.md
```

`GameplayIntentGateway / GameplayCoordinator`는 보류 대상으로만 등장해야 한다.

## 프롬프트 업데이트 확인

프롬프트 업데이트 후보 있음.

추가할 내용:

```text
전투 송수신 구조를 설계할 때 Request / Attack / Damage를 기본 이름으로 두지 말고,
collision hit와 timing cue, defensive outcome을 함께 담을 수 있는 CombatSignal vocabulary를 우선 검토한다.
```

기록 위치:

```text
Docs/06_notes/prompt_updates/PU01_Combat_Signal_Boundary_Prompt_Update_Note.md
```