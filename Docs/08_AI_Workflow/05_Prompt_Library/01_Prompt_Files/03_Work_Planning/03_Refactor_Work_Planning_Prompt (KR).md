# Refactor Work Planning Prompt

## 1. 목적

큰 리팩터링을 시작하기 전에 작업 목표, 변경 단위, 책임 경계, 제외 범위, 위험, 검증 기준, 문서화 필요 여부를 정리한다.

이 Prompt는 기존 Work Pipeline에 입력할 리팩터링 작업 계획을 만든다.

---

## 2. 사용 시점

```yaml
사용 시점
- 큰 구조 변경 전
- 책임 경계가 여러 계층에 걸쳐 바뀔 때
- C++ 변경과 Blueprint / Asset 영향이 함께 있을 때
- 변경 단위를 나누고 검증 기준을 먼저 정해야 할 때
- Plan Mode에서 목표 / 범위 / 책임 / 검증 기준을 먼저 확정해야 할 때
```

작은 단일 수정, 단순 문서 작성, 이미 변경 단위가 명확한 작업에는 기본 사용하지 않는다.

---

## 3. 사용 방법

리팩터링 대상과 관련 문서를 함께 제공하고 `복사용 Prompt`를 사용한다.

계획 결과는 Work Pipeline의 구조 제안 / 적용 및 수정 / 검증 및 안정화 단계 입력으로 사용한다.

리팩터링 결과를 공식 문서로 남길 때는 성격에 따라 System Architecture, Architecture Decision Record, Architecture Issue Report, Engine Decision Record, Engine Issue Report, Verification Log, PR Document, Portfolio Technical Document로 분리한다.

---

## 4. 복사용 Prompt

````text
Project Stella 리팩터링 계획을 세워줘.

리팩터링 대상:
- [기능 / 시스템 / 책임 경계 / 문서 경로]

관련 문서:
- Work List: [문서 경로 또는 없음]
- Bug Report: [문서 경로 또는 없음]
- System Architecture: [문서 경로 또는 없음]
- System Design Records: [문서 경로 또는 없음]
- Architecture Decision Record: [문서 경로 또는 없음]
- Architecture Issue Report: [문서 경로 또는 없음]
- Engine Technique Document: [문서 경로 또는 없음]
- Engine Implementation Records: [문서 경로 또는 없음]
- Engine Decision Record: [문서 경로 또는 없음]
- Engine Issue Report: [문서 경로 또는 없음]
- Verification Log: [문서 경로 또는 없음]
- PR Document: [문서 경로 또는 없음]
- Portfolio Technical Document: [문서 경로 또는 없음]

계획 목표:
- 관련 코드, 문서, 현재 구조, 목표 구조를 먼저 확인해줘.
- 큰 리팩터링을 한 번에 구현하지 말고 작은 변경 단위로 나눠줘.
- 각 변경 단위마다 목표 / 수정 범위 / 비범위 / 위험 / 검증 기준 / 문서화 필요 여부를 정리해줘.
- 책임 경계는 Orchestrator / Component / Executor / Data / Asset / Blueprint 기준으로 판단해줘.
- 임시 대응, 구조적 해결, 후속 보강을 분리해줘.
- 현재 코드 / System Architecture / Records / Work List 기준이 충돌하면 Source of Truth와 불일치 항목을 분리해줘.
- 사용자 결정이 필요한 항목은 구현 전에 선택지로 분리해줘.
- Work Pipeline에 입력할 계획 작성용이므로 실제 파일 수정이나 공식 문서 작성은 하지 말아줘.

출력 형식:

1. Source of Truth 확인

2. 현재 구조 요약

3. 리팩터링 목표

4. 책임 경계 판단

5. 변경 단위 제안

6. 비범위 / 후속 범위

7. 위험 요소

8. 사용자 결정 필요 항목

9. 권장 실행 순서

10. Commit 분리 후보

11. 검증 기준

12. 문서화 필요 여부

13. 미검증 / 확인 필요 항목

변경 단위는 권장 실행 순서대로 나열해줘.

각 변경 단위는 다음 양식으로 작성해줘.

변경 단위 N
선행 조건
-> [먼저 확인하거나 결정해야 할 항목]

목표
-> [해결할 문제]

수정 범위
-> [직접 수정할 코드 / 문서 / 책임]

비범위
-> [이번 변경에서 건드리지 않을 항목]

위험
-> [회귀 / Asset 영향 / lifecycle 영향 / API 영향]

사용자 결정 필요 여부
-> [필요 / 불필요 / 선택지]

검증 기준
-> Build:
-> Code Flow:
-> PIE:
-> Editor:
-> Asset:

문서화 필요 여부
-> Work List:
-> Bug Report:
-> System Architecture:
-> System Design Records:
-> Architecture Decision Record:
-> Architecture Issue Report:
-> Engine Technique Document:
-> Engine Implementation Records:
-> Engine Decision Record:
-> Engine Issue Report:
-> Verification Log:
-> PR Document:
-> Portfolio Technical Document:

문서화 연결 대상
-> [계획 결과 또는 실행 결과를 연결해야 할 문서]
````

---

## 5. 입력 기준

```yaml
입력 기준
- 리팩터링 대상
- 관련 코드 / 문서 경로
- 현재 문제 또는 목표 구조
- 제외해야 할 범위
- 검증 가능한 환경
- 사용자 결정이 필요한 제약 조건
- 현재 코드와 문서의 기준 시점
```

---

## 6. 출력 기준

```yaml
출력 기준
- Source of Truth 확인
- 현재 구조 요약
- 목표 구조
- 변경 단위
- 비범위
- 위험 요소
- 사용자 결정 필요 항목
- 권장 실행 순서
- Commit 분리 후보
- 검증 기준
- 문서화 필요 여부
- 미검증 / 확인 필요 항목
```

---

## 7. 범위 / 비범위

```yaml
범위
- 리팩터링 계획 수립
- 변경 단위 분리
- 책임 경계와 위험 판단
- Work Pipeline 입력 계획 작성
- 사용자 결정 항목 분리

비범위
- 코드 수정
- 문서 작성
- Git staging / commit
- 공식 문서 카테고리별 최종 문서 작성
```

---

## 8. 제약 조건

```yaml
제약 조건
- 현재 구현 판단은 현재 코드 기준으로 함
- System Architecture와 Records는 구조 의도 / 결정 근거 / 후속 설계 후보로 구분
- 서로 다른 책임 계층은 가능한 한 다른 변경 단위로 분리
- public API / struct / enum 변경은 호출부 정리와 함께 검토
- Blueprint / Asset 영향은 C++ 변경과 검증 항목을 분리
- 한 변경 단위가 너무 많은 검증을 요구하면 더 작게 나눌 수 있는지 검토
- 코드와 문서가 다르면 변경 위험 또는 문서 정합성 보완 후보로 기록
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
- 현재 코드 / 문서 / Records의 Source of Truth를 먼저 확인
- 현재 구조와 목표 구조를 먼저 분리
- 구현할 항목과 보류할 항목을 분리
- 사용자 결정이 필요한 구조 선택은 선택지로 제시
- 확인하지 못한 항목은 미검증으로 표시
```

---

## 10. 검증 기준

```yaml
검증 기준
- 각 변경 단위마다 Build / Code Flow / PIE / Editor / Asset 검증 필요 여부가 분리됨
- 위험 요소와 검증 기준이 연결됨
- 문서화 필요 여부가 산출물별로 분리됨
- 사용자 결정 필요 항목이 구현 전 분리됨
- 권장 실행 순서와 Commit 분리 후보가 변경 위험과 연결됨
- 미검증 / 확인 필요 항목이 완료처럼 표현되지 않음
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
- 변경 단위, 사용자 결정 항목, 검증 기준, 문서화 필요 여부가 구현 전 판단 가능한 수준으로 정리됨

실패
- 현재 구조 확인 불가
- 목표 구조 충돌
- Source of Truth를 판단할 수 없음

미검증
- 코드 / Asset / Blueprint 확인이 필요한 항목
- System Architecture / Records 기준 시점을 확인하지 못한 항목
- 사용자 결정이 필요한 상태로 남은 항목
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Project Stella Working Rule Prompt
- 작업 세션 공통 규칙

Working Reference Prompt
- Unreal / Project Stella 책임 경계 판단 기준

Work Pipeline
- 계획 결과를 구조 제안 / 적용 및 수정 / 검증 및 안정화 단계 입력으로 사용

Refactor Work Planning Prompt
- 리팩터링 실행 전 작업 목표 / 변경 단위 / 위험 / 검증 기준 정리

Document Writing Prompt
- 리팩터링 결과를 공식 문서 카테고리에 맞춰 기록

Verification Log Prompt
- 리팩터링 실행 후 검증 / 미검증 항목 기록

Code Review Prompt
- 리팩터링 변경 후 회귀 / 누락 / 위험 검토
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
- 리팩터링 규모별 출력 형식
- System Architecture 연결 기준
- Engine Technique Document 연결 기준
- PR Document 연결 기준
- Work Planning Prompt 추가 유형
```
