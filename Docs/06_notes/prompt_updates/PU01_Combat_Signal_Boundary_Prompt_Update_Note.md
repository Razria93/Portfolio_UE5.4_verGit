# PU01 Combat Signal Boundary Prompt Update Note

## Branch

```text
refactor/combat-signal-boundary
```

## Trigger

W03 이후 전투 처리 구조를 논의하면서 `Request`, `Attack`, `Damage`, `Intent Gateway`, `Coordinator` 같은 이름이 실제 책임보다 먼저 구조를 끌고 가는 문제가 확인되었다.

특히 객체의 상태를 바꾸는 모든 흐름을 하나의 `Request` 파이프라인으로 통일하려는 시도는 입력, damage, timing cue, system event의 성격 차이를 충분히 반영하지 못했다.

또한 브랜치를 너무 작게 나누면 작업 흐름이 불필요하게 복잡해지고, 너무 크게 묶으면 회귀 원인 추적이 어려워지는 문제가 있었다.

## Candidate Update

전투 송수신 구조를 설계할 때 다음 기준을 우선 검토한다.

```text
- Request는 request-response 흐름이 실제로 필요할 때만 핵심 이름으로 쓴다.
- Attack은 공격 행위만이 아니라 source/target 공방 전체를 담아야 하는 경우 핵심 이름으로 쓰지 않는다.
- Damage는 HP commit 또는 damage apply 단계 이름으로 제한한다.
- collision hit와 timing cue, defensive outcome을 함께 다룰 때는 CombatSignal vocabulary를 우선 검토한다.
- 입력 처리 축 / combat 처리 축 / timing cue 처리 축은 먼저 별도 성격으로 검토한다.
- 공용 Gateway / Coordinator는 실제 domain boundary와 반복 패턴이 안정된 뒤 도입한다.
```

브랜치 분할은 기능 이름보다 동작 변화의 성격과 회귀 위험 기준으로 판단한다.

```text
- 문서 / 타입만 추가하는가?
- 기존 target-side 피격 흐름을 바꾸는가?
- 기존 source-side 공격 송출 흐름을 바꾸는가?
- 이름 / 참조를 크게 바꾸는가?
- 새 gameplay behavior를 추가하는가?
```

W04-05 rename 작업 후보:

```text
- 기존 runtime component를 리네임할 때는 컴포넌트명, 내부 API명, 구조체명, 로그 문구를 별도 단계로 나눈다.
- Unreal Engine API 이름과 프로젝트 내부 이름을 먼저 분리해서 검토한다.
- 리네임 브랜치는 기능 변경이 아니라 책임 정렬 결과를 이름에 반영하는 작업으로 제한한다.
```

## Reason

이 기준은 over-abstraction과 God Object 위험을 줄인다.

모든 상태 변경 요청을 하나의 Gateway / Coordinator가 판정하고 분배하게 만들면 해당 객체가 각 domain rule을 과도하게 알게 된다. 반대로 이를 피하기 위해 모든 축을 세밀하게 분리하면 adapter와 계층이 과도하게 늘어난다. 따라서 공통화는 먼저 각 축의 데이터 성격과 반복 패턴이 안정된 뒤 검토한다.

특히 Blink / Repulse처럼 collision 없는 cue 기반 outcome은 `Damage`나 `Attack` 중심 이름으로는 자연스럽게 설명되지 않는다. `CombatSignal`은 target이 아직 해석하지 않은 전투 입력 / 증거 / cue라는 의미를 줄 수 있다.

브랜치 분할 기준을 회귀 위험 축으로 두면 PR diff가 작아지고, 문제가 생겼을 때 target-side / source-side / rename / feature 중 어느 축에서 발생했는지 추적하기 쉽다.

## Apply Target

후보:

```text
Docs/08_AI_Workflow/05_Prompt_Library/01_Prompt_Files/02_Working_Reference/02_Project_Stella_Working_Reference_Prompt (KR).md
Docs/08_AI_Workflow/05_Prompt_Library/00_Prompt_Management/02_Prompt_Pattern_Candidates (KR).md
```

## Status

```text
Candidate
```
