# Cross Domain Interrupt Work Prompt

## 1. 목적

Project Stella의 Action / Reaction execution 사이 cross-domain interrupt / intervention 작업을 재개하기 위한 특정 기능 작업 Prompt다.

이 Prompt는 범용 작업 규칙이 아니라 특정 기능 문맥을 이어받기 위한 Prompt다.

---

## 2. 사용 시점

```yaml
사용 시점
-> Cross-domain interrupt / intervention 작업을 재개할 때
-> Action -> Reaction, Reaction -> Action, Action -> Action, Reaction -> Reaction 흐름을 같은 모델로 정리할 때
-> InterventionDirective / Want / Allow / Notify Window / Policy / Gate 구조를 다시 검토할 때
```

---

## 3. 사용 방법

관련 System Architecture, System Design Records, Engine Technique Document, Engine Implementation Records와 현재 코드를 먼저 확인한 뒤 `복사용 Prompt`를 사용한다.

검토 전 상태이므로 현재 코드 / 문서와 맞지 않는 항목은 후속 보정 대상으로 분리한다.

---

## 4. 복사용 Prompt

````text
Project Stella의 cross-domain interrupt / intervention 작업을 이어서 진행해줘.

작업 목표:
1. Action / Reaction execution 사이의 cross-domain intervention을 완성한다.
2. Action -> Reaction, Reaction -> Action, Action -> Action, Reaction -> Reaction 흐름을 같은 모델로 처리한다.
3. Intervention은 replace API가 아니라 directive로 다룬다.
4. WantIntervention과 AllowIntervention은 다른 질문으로 유지한다.
5. Notify Window는 runtime gate로 보고, 기본 policy와 runtime window policy를 분리할 수 있는 방향을 검토한다.

먼저 확인할 문서:
- Docs/05_System_Architecture/S19_UE5_Portfolio_Action_Reaction_Execution_Symmetry_Implementation_Plan (KR).md
- Docs/05_System_Architecture/S23_UE5_Portfolio_Execution_Intervention_Directive_Decision (KR).md
- Docs/05_System_Architecture/S24_UE5_Portfolio_Execution_Intervention_Policy_Decision (KR).md
- Docs/05_System_Architecture/S27_UE5_Portfolio_Execution_Intervention_Key_Window_Model (KR).md
- Docs/05_System_Architecture/S28_UE5_Portfolio_Execution_Intervention_Policy_Gate_Refactor (KR).md

먼저 확인할 코드:
- Source/Portfolio/Type/CWeaponStructure.h
- Source/Portfolio/Type/CActionOrchestrationStructure.h
- Source/Portfolio/Type/CReactionOrchestrationStructure.h
- Source/Portfolio/Component/CActionOrchestratorComponent.*
- Source/Portfolio/Component/CReactionOrchestratorComponent.*
- Source/Portfolio/Component/CActionComponent.*
- Source/Portfolio/Component/CReactionComponent.*
- Source/Portfolio/Action/CAction.*
- Source/Portfolio/Reaction/CReaction.*
- Source/Portfolio/Notify/CAnimNotifyState_ExecutionInterventionWindow.*

작업 방향:
1. 현재 stop reason / stop source가 문서 방향과 얼마나 다른지 확인한다.
2. 즉시 enum을 갈아엎기보다 기존 코드와 Asset 영향 범위를 확인한다.
3. Action / Reaction runtime window container 중복을 공통 helper로 분리할 수 있는지 검토한다.
4. 전체 executor base class 통합은 서두르지 않는다.
5. stop directive 적용 실패 시 incoming execution을 시작하지 않는 기준을 유지한다.
6. 변경 전 계획을 공유하고, C++ / Blueprint / Asset 검증 필요 항목을 분리한다.
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 현재 Branch 또는 작업 범위
-> 관련 System Architecture 문서
-> 관련 System Design Records
-> 관련 Engine Technique Document
-> 관련 Engine Implementation Records
-> 현재 코드 상태
-> 사용자가 이어가려는 세부 목표
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 현재 구조 확인
-> 문서와 코드 차이
-> 변경 계획
-> 위험 요소
-> 검증 기준
-> 후속 작업 범위
```

---

## 7. 범위 / 비범위

```yaml
범위
-> Cross-domain interrupt / intervention 작업 재개

비범위
-> 범용 Prompt 규칙
-> 무조건적인 enum / 구조 전체 교체
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 현재 코드와 문서가 다르면 현재 코드 기준으로 판단
-> Asset / Blueprint 영향이 있는 이름 변경은 신중히 검토
-> 특정 기능 Prompt이므로 범용 규칙처럼 사용하지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 현재 구현 / 후속 설계 / 폐기 후보를 분리
-> 사용자 결정이 필요한 구조 선택은 선택지로 제시
```

---

## 10. 검증 기준

```yaml
검증 기준
-> Action / Reaction intervention 흐름이 같은 모델로 설명되는가
-> Want / Allow / Directive / Component Apply 책임이 분리되는가
-> Build / PIE / Editor / Asset 검증 필요 항목이 분리되는가
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 현재 구조와 다음 변경 계획이 분리됨

실패
-> 관련 문서 / 코드 확인 불가

미검증
-> Asset / Blueprint / PIE 확인 필요
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Cross Domain Interrupt Work Prompt
-> 특정 기능 작업 재개 Prompt

Project Stella Working Rule Prompt
-> Project Stella 작업 공통 규칙
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> 현재 코드 기준 재검토
-> Archive / 유지 여부 판단
```

