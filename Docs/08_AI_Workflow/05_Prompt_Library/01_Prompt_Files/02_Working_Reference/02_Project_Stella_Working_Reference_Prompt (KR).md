# Project Stella Working Reference Prompt

## 1. 목적

Project Stella의 코드 / 문서 구조를 Unreal Engine 공통 작업 규칙과 대조해, 현재 구조의 정합성, 의도적 차이, 위험한 차이, 후속 보강 항목을 판단하는 Reference 기준을 제공한다.

이 Prompt는 구현 지시가 아니라 Project Stella의 구조, 용어, 책임 경계, 위험 후보를 해석하기 위한 Reference Prompt다.

---

## 2. 사용 시점

```yaml
사용 시점
-> Project Stella 구조가 Unreal Engine 공통 규칙과 맞는지 점검할 때
-> 기능 구현 전 프로젝트 고유 용어와 책임 경계를 확인해야 할 때
-> 현재 구현 문서와 후속 설계 문서를 구분해야 할 때
-> Action / Reaction / Damage / Feedback / CombatResolution 책임이 섞일 위험을 점검할 때
-> DataAsset 전환, Asset Validation, Policy / Gate 분리 같은 후속 구조 후보를 정리할 때
```

---

## 3. 사용 방법

`01_Unreal_Engine_Working_Rule_Prompt (KR).md`와 `01_Unreal_Engine_Working_Reference_Prompt (KR).md`의 공통 Unreal Engine 기준을 먼저 참고한 뒤, Project Stella의 실제 코드 / 문서 구조와 비교한다.

분석 결과는 현재 구조, 프로젝트 고유 기준, 의도적 차이, 위험한 차이, 후속 보강 항목으로 분리한다.

---

## 4. Project Stella 구조 확인 기준

Project Stella의 고유 구조는 Prompt 안에 복사하지 않고, 현재 코드와 관련 기준 문서를 확인해 판단한다.

```yaml
확인 순서
1. 현재 코드
-> 실제 구현 기준
-> Component / Orchestrator / Runtime Object / Executor / Struct / Enum / AnimNotify / Data Flow 확인

2. System Architecture / Engine Technique Document
-> 현재 시스템 구조 / 책임 경계 / 실행 흐름 / 데이터 계약 확인
-> Unreal Engine 기능 / API / 시스템 사용 방식 확인
-> 현재 구조 설명인지 엔진 사용 방식 설명인지 설계 / 구현 기록인지 구분

3. Project Stella Overview
-> 프로젝트 목적 / 포트폴리오 범위 / 핵심 구현 대상 확인

4. System Design Records / Engine Implementation Records
-> Architecture Decision Record / Architecture Issue Report 확인
-> Engine Decision Record / Engine Issue Report 확인
-> 시스템 설계 결정, 구조 문제, 엔진 사용 결정, 엔진 동작 이슈 확인

5. Work List / Verification Log / PR Document
-> 현재 Branch 목표, 변경 상태, 검증 상태, 미검증 항목 확인
```

```yaml
불일치 처리
-> 코드와 문서가 다르면 현재 구현 판단은 코드 기준
-> System Architecture는 현재 구조 설명 기준으로 사용
-> Engine Technique Document는 현재 엔진 기능 / API / 시스템 사용 방식 설명 기준으로 사용
-> System Design Records와 Engine Implementation Records는 의도 / 기록 / 후속 설계 후보로 분리
-> Work List / Verification Log / PR Document는 현재 Branch 상태 확인 기준
-> 기준 충돌은 위험한 차이 또는 문서 정합성 보완 후보로 기록
-> 확인하지 못한 항목은 미검증으로 표시
```

---

## 5. 복사용 Prompt

````text
Project Stella의 현재 코드 / 문서 구조를 분석해줘.

분석 기준은 Unreal Engine 공통 작업 규칙과 Project Stella의 실제 구조를 함께 사용한다.

다음 관점으로 검토한다.

1. Project Stella 기준을 먼저 확인한다.
   - Project Stella는 Stella Blade 액션 시스템 분석 / 구현 포트폴리오다.
   - 원작 전체 재현이 아니라, 핵심 전투 구조를 설명 가능한 방식으로 구현하는 프로젝트다.
   - 전투 공방, 입력 반응, 상태 전환, 카메라 / 연출 흐름을 포트폴리오 범위에서 구현하는지 확인한다.
   - 관련 System Architecture를 확인하고 현재 구조 설명인지 확인한다.
   - 관련 Engine Technique Document를 확인하고 Unreal Engine 기능 / API / 시스템 사용 방식 설명인지 확인한다.
   - 시스템 설계 결정과 구조 문제는 System Design Records 기준으로 확인한다.
   - 엔진 사용 결정과 엔진 동작 이슈는 Engine Implementation Records 기준으로 확인한다.
   - System Architecture, Engine Technique Document, 각 Record의 내용은 Prompt가 직접 소유하지 않고, 확인 대상 문서로 사용한다.

2. 공통 Unreal Engine 기준과 맞는 부분
   - Actor / Component / Subsystem / Runtime Object / Executor / DataAsset / AnimNotify 책임 경계와 맞는지 확인한다.
   - C++ / Blueprint / Asset / Editor 검증 가능성이 분리되어 있는지 확인한다.
   - AnimNotify / Montage가 timing event 역할에 머무는지 확인한다.

3. Project Stella 고유 구조
   - Action / Reaction execution 구조를 확인한다.
   - ApplyDamage / TakeDamage / Feedback / CombatResolution 책임을 구분한다.
   - Request / Candidate / ExecutionContext / Decision / Relationship / ApplyMode / InterventionDirective / Payload / Context / Result / Packet 같은 프로젝트 고유 실행 언어가 일관되게 쓰이는지 확인한다.
   - Runtime Object / Executor가 개별 실행 정책과 runtime 상태를 담당하는지 확인한다.

4. 현재 구현과 후속 설계 문서 구분
   - 현재 코드에 이미 구현된 구조인지 확인한다.
   - 문서상 후속 설계 또는 후보 단계인지 확인한다.
   - 구현 완료처럼 보이지만 검증되지 않은 내용은 미검증 또는 후속 후보로 분리한다.

5. 의도적 차이와 위험한 차이 분리
   - 포트폴리오 목적상 의도적으로 문서화 밀도가 높은 부분은 유지 기준으로 본다.
   - DataAsset 전환 전 component-owned data container처럼 단계적 구현 상태는 후속 후보로 분리한다.
   - CombatResolution 부재, Policy / Gate 혼재, Asset validation 부족, Blueprint rename 영향처럼 구조 위험이 있는 항목은 위험으로 기록한다.

6. 후속 보강 항목 정리
   - DataAsset 전환 기준
   - Asset Validation 기준
   - Policy / Gate 분리 기준
   - CombatResolution 도입 기준
   - Rename / Blueprint 영향 관리 기준
   - 검증 로그 양식 보강 후보

출력은 다음 형식으로 작성한다.

핵심 결론
-> 현재 구조가 어떤 기준에서 안정적인지 요약
-> 가장 주의해야 할 위험을 요약

정합한 부분
-> Unreal Engine 공통 기준과 잘 맞는 부분

프로젝트 고유 기준
-> Project Stella 목적과 실행 언어에 맞는 부분

의도적 차이
-> Project Stella 목적상 유지 가능한 차이

위험한 차이
-> 후속 작업에서 관리해야 할 구조 위험

후속 보강 항목
-> 우선순위가 높은 보강 후보

미검증 항목
-> 코드 / 문서 / Asset / Blueprint 확인이 더 필요한 항목
````

---

## 6. 입력 기준

```yaml
입력 기준
-> 분석 대상 기능 또는 문서 범위
-> 관련 코드 경로
-> 관련 System Architecture 경로
-> 관련 Engine Technique Document 경로
-> 관련 System Design Records 경로
-> 관련 Engine Implementation Records 경로
-> Project Stella Overview
-> 현재 Branch의 Work List / Verification Log / PR Document
-> 관련 Portfolio Technical Document
-> 현재 구현인지 후속 설계인지 불명확한 항목
-> 사용자가 우려하는 책임 경계 또는 검증 위험
```

---

## 7. 출력 기준

```yaml
출력 기준
-> 핵심 결론
-> Project Stella 목적 / 범위 기준
-> 공통 Unreal 기준과 정합한 부분
-> Project Stella 목적상 의도적인 차이
-> 후속 관리가 필요한 위험한 차이
-> 프로젝트 고유 용어 / 실행 흐름 정합성
-> 후속 보강 항목
-> 미검증 항목
```

---

## 8. 범위 / 비범위

```yaml
범위
-> Project Stella 구조 분석
-> Unreal Engine 공통 기준과 프로젝트 구조 대조
-> Project Stella 고유 용어와 실행 언어 정합성 판단
-> Action / Reaction / Damage / Feedback / CombatResolution 책임 경계 검토
-> 현재 구현 / 후속 설계 / 미검증 항목 분리

비범위
-> 코드 직접 수정
-> 문서 직접 갱신
-> Asset / Blueprint 실제 검증
-> 일반 Unreal Engine 작업 규칙 자체의 재작성
```

---

## 9. 제약 조건

```yaml
제약 조건
-> Project Stella를 원작 재현 프로젝트처럼 해석하지 않음
-> 포트폴리오 범위에서 설명 가능한 핵심 전투 구조 구현 기준을 유지
-> 현재 구현과 후속 설계를 섞어 완료 상태로 표현하지 않음
-> CombatResolution 후보를 TakeDamage 또는 ReactionOrchestrator에 임시로 밀어 넣는 방향을 기본값으로 보지 않음
-> Blueprint / Asset 검증이 필요한 항목은 확인 완료로 표현하지 않음
```

---

## 10. 모호성 처리 기준

```yaml
모호한 경우
-> 현재 코드 기준인지 문서상 후속 방향인지 먼저 구분
-> 확인 가능한 구조와 추정이 필요한 구조를 분리
-> 프로젝트 고유 용어의 의미가 불명확하면 정의 후보로 기록
-> 사용자 결정이 필요한 구조 선택은 질문 또는 다음 선택지로 분리
```

---

## 11. 검증 기준

```yaml
검증 기준
-> 관련 코드와 문서가 같은 구조를 설명하는지 확인
-> 현재 코드와 System Architecture의 기준 시점이 일치하는지 확인
-> 현재 코드와 Engine Technique Document의 엔진 기능 / API 사용 설명이 일치하는지 확인
-> System Design Records와 Engine Implementation Records의 판단 기록이 현재 구조와 충돌하지 않는지 확인
-> Project Stella 목적과 포트폴리오 범위에 맞는 구조인지 확인
-> 프로젝트 고유 용어가 같은 의미로 쓰이는지 확인
-> 책임 경계가 Action / Reaction / Damage / Feedback / CombatResolution 사이에서 충돌하지 않는지 확인
-> C++로 확인 가능한 내용과 Editor / Asset 확인이 필요한 내용을 분리
```

---

## 12. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 분석 대상의 정합성, 차이, 위험, 후속 보강 항목이 분리됨

실패
-> 분석 대상 파일 또는 기준 문서를 확인할 수 없음
-> 구조 판단에 필요한 코드 또는 System Architecture 기준을 확인할 수 없음
-> 엔진 기능 / API 사용 판단에 필요한 Engine Technique Document 또는 코드 기준을 확인할 수 없음
-> 필요한 설계 결정 / 구조 이슈 / 엔진 사용 결정 / 엔진 이슈 기록을 확인할 수 없음
-> 현재 구현과 후속 설계를 구분할 근거가 부족함

미검증
-> Blueprint / Asset / Editor 상태 확인이 필요한 항목
-> 현재 코드 탐색 없이 문서만으로 판단한 항목
```

---

## 13. 기존 Prompt와 역할 경계

```yaml
Unreal Engine Working Rule Prompt
-> Unreal Engine 작업 공통 실행 규칙 제공

Unreal Engine Working Reference Prompt
-> Unreal Engine 작업의 책임 경계, 구현 원칙, Blueprint / Asset 경계 기준 제공

Project Stella Working Reference Prompt
-> Project Stella 구조, 용어, 책임 경계, 위험 후보 판단 기준 제공
```

---

## 14. 계속 수정할 항목

```yaml
후속 보완 후보
-> CombatResolution 도입 기준
-> DataAsset 전환 기준
-> Asset Validation 기준
-> Policy / Gate 분리 기준
-> Rename / Blueprint 영향 관리 기준
-> Runtime Object / Executor 용어 사용 기준
-> Project Stella 검증 로그 양식
```
