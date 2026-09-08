# 문서상 p.16 AI Intent 프로젝트 감사 및 구성 계획

> 파일명은 초기 22페이지 체계의 감사 기록을 보존한다. 현행 25페이지 체계에서는 p.16에 적용한다.

> 상태: **코드·설계 문서 감사 완료 / 사용자 제작 Intent Flow 다이어그램 HTML 반영 완료**

## 확인 근거와 주장 범위

- `UCBTService_UpdateAIIntentState`는 Blackboard의 사망·활성 Reaction·Balance 차단 상태를 먼저 확인하고, 이후 Combat Target·Investigate·Alert·Participation 문맥으로 `EAIIntentState`를 결정한다.
- `UCBTTask_StartCombatAction`은 Blackboard의 `bCanCombatAction`, `bIsCombatAction`을 확인한 뒤 `ACEnemy::HandleAICombatAction(CombatActionIntent)`을 요청하고, 다음 행동 가능 시각을 기록한다.
- `UCBTTask_WaitEndCombatAction`은 Action을 직접 종료하지 않으며 `bIsCombatAction`이 해제될 때까지 대기한다.
- 이 페이지는 Combat Target의 Source of Truth와 Participation의 역할 할당을 설명하지 않는다. 두 내용은 각각 p.17과 p.18의 책임이다.

## 페이지 구성

### 구조 문제 — BT가 실행·종료까지 소유하면 판단과 실행 수명주기가 결합됨

BT가 행동의 시작·중단·종료를 직접 소유하면 AI 전용 실행 규칙이 생겨 Player와 Enemy의 정책이 갈라질 수 있다. Dead·HitReact·Incapacitated 같은 절대 상태는 Intent 판단에 반영하되, Action의 수락·전환·종료는 공통 실행 계약에 둔다.

### 중앙 다이어그램 — Intent Flow

사용자 제작 `Diagram/PP/AI Intent.png`를 `Assets/Diagrams/p16_ai_intent.png`로 복사해 반영했다.

```text
Blackboard Context
  → Priority / Intent Service
  → BT Task Request
  → Shared Action Execution
  → Wait (bIsCombatAction 해제)
  → 다음 판단
```

`Dead → HitReact → Incapacitated` 우선순위와 일반 Intent를 구분하되, BT가 Action을 직접 실행하거나 종료한다고 표현하지 않는다. Combat Target·Participation은 Blackboard로 들어오는 문맥으로만 표기한다.

### 구조 해결 — BT는 Intent를 기록하고 Task는 요청·대기만 맡김

| 책임 경계 | 확정 범위 |
| --- | --- |
| Blackboard Context | Target·Participation·거리·Reaction·Dead의 투영 문맥을 읽는다. 상태 권위는 소유하지 않는다. |
| Intent Service | Dead·HitReact·Incapacitated를 우선한 뒤 일반 문맥으로 `AIIntentState`를 결정·기록한다. |
| BT Task / Execution | Task는 `CombatActionIntent`를 요청하고 `bIsCombatAction` 종료를 대기한다. 수락·실행·전환·종료는 공통 Action 계층이 소유한다. |

HTML에는 런타임 검증 카드나 별도 문제 해결 경험을 두지 않는다. 실제 Runtime Capture 전에는 특정 Intent 전환을 성공 사례나 성능 결과로 단정하지 않는다.

## 표현 제한

- BT가 전투 Action을 직접 구현하거나 Action 실행 결과를 소유한다고 표현하지 않는다.
- Combat Target과 Participation을 AI 의도 내부의 상태 권위로 표현하지 않는다.
- 실제 Runtime Capture가 없는 상태 전환을 성공 사례나 성능 결과처럼 표현하지 않는다.
