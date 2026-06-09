# Feature Work Planning Prompt

## 1. 목적

준비된 `Work Brief`를 바탕으로 새 기능 구현 계획을 작성한다.

준비된 `Work Brief`를 기능 구현 단위, 실행 순서, 검증 기준, 문서화 필요 여부로 분해하는 Work Planning Prompt다.

---

## 2. 사용 시점

```yaml
사용 시점
-> Work Brief가 진행 가능 또는 준비 완료 상태일 때
-> Work Brief 이후 새 기능 구현 계획을 구체화해야 할 때
-> 기능 구현 단위와 순서를 나눠야 할 때
-> C++ / Blueprint / Asset / Editor 영향이 함께 있을 때
-> 검증 순서와 Commit 분리 후보를 구현 전에 정리해야 할 때
```

Work Brief가 없거나 준비 상태가 `진행 불가` 또는 `검토 필요`이면 `01_Work_Brief_Intake_Prompt (KR).md`로 돌아간다.

Work Brief Intake에서 작업 유형이 `신규 기능 구현`으로 판정된 경우 이 Prompt를 사용한다.

작은 단일 수정, 순수 문서 작성, 리팩터링 중심 작업에는 기본 사용하지 않는다. 리팩터링이 주된 작업이면 `03_Refactor_Work_Planning_Prompt (KR).md`를 사용한다.

---

## 3. 사용 방법

준비된 Work Brief의 경로, 준비 상태, 작업 개요, 정리된 기능 흐름, 작업 범위, 확정된 결정, 검토필요 항목, 선택필요 항목, 위험 항목, Prompt 라우팅 결과를 제공하고 `복사용 Prompt`를 사용한다.

필요하면 관련 코드 / 문서 / Asset / Blueprint 후보와 참조 문서를 함께 제공한다.

`계획차단 / 선택필요` 항목이 남아 있으면 구현 계획을 확정하지 않고 `01_Work_Brief_Intake_Prompt (KR).md`로 돌아간다.

남은 `비차단 / 검토필요` 항목은 이 Prompt에서 선행 확인 항목 또는 구현 단위 전 탐색 항목으로 변환한다.

계획 결과는 Work Pipeline의 아이디어 계획 / 구조 제안 단계에서 정리한 내용을 적용 및 수정 단계로 넘기기 위한 입력으로 사용한다.

계획 안의 검증 기준과 미검증 항목은 이후 검증 및 안정화 단계의 입력으로 사용한다.

최종 Work Checklist는 이 Prompt의 계획 결과를 바탕으로 `01_Work_Checklist_Writing_Prompt (KR).md`에서 작성한다.

---

## 4. 복사용 Prompt

````text
아래 Work Brief를 바탕으로 기능 구현 계획을 작성해줘.

준비된 Work Brief를 실제 구현 가능한 기능 작업 계획으로 분해하는 데 집중해줘.

Work Brief:
- 경로:
- 준비 상태:
- 작업 개요:
- 정리된 기능 흐름:
- 작업 범위:
- 확정된 결정:
- 검토필요 항목:
- 선택필요 항목:
- 위험 항목:
- Prompt 라우팅 결과:

관련 문서:
- Work Pipeline:
- Operation Guide:
- Project Context:
- Working Rule:
- Working Reference:
- Work Checklist:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:

계획 목표:
- `확정된 결정` 항목은 기본 전제로 유지하고, 코드 / 문서 충돌이 확인될 때만 재검토 항목으로 분리해줘.
- `계획차단 / 선택필요` 또는 `계획차단 / 검토필요` 항목이 있으면 계획을 확정하지 말고 Work Brief Intake로 되돌릴 항목을 먼저 정리해줘.
- 남은 `비차단 / 검토필요` 항목은 Feature Work Planning에서 처리할 선행 확인 항목 또는 구현 단위 전 탐색 항목으로 변환해줘.
- 현재 코드 / 문서 / Asset / Blueprint 연결 지점을 먼저 확인해야 할 항목으로 분리해줘.
- 기능 구현 단위를 작은 순서로 나눠줘.
- 각 구현 단위마다 목표 / 수정 범위 / 비범위 / 선행 조건 / 위험 / 검증 기준 / 문서화 필요 여부를 정리해줘.
- C++ 변경, Blueprint / Asset 변경, Editor 확인 항목을 분리해줘.
- 사용자 결정이 필요한 항목은 구현 전에 선택지로 분리해줘.
- Commit 분리 후보를 변경 위험과 검증 단위 기준으로 제안해줘.
- 적용 및 수정 단계로 넘길 구현 계획 작성용이므로 실제 파일 수정이나 공식 문서 작성은 수행하지 마.

출력 형식:

1. Source of Truth 확인

2. Work Brief 항목 처리 결과

3. 현재 구조 탐색 대상

4. 기능 구현 목표

5. 구현 단위 제안

6. 권장 실행 순서

7. 비범위 / 후속 범위

8. 위험 요소

9. 사용자 결정 필요 항목

10. 검증 계획

11. Commit 분리 후보

12. 문서화 필요 여부

13. 미검증 / 확인 필요 항목

구현 단위는 권장 실행 순서대로 나열해줘.
각 구현 단위는 다음 양식으로 작성해줘.

구현 단위 N
선행 조건
-> [먼저 확인하거나 결정해야 할 항목]

목표
-> [구현할 기능 결과]

수정 범위
-> [직접 수정할 코드 / 문서 / Asset / Blueprint]

비범위
-> [이번 구현 단위에서 건드리지 않을 항목]

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
-> Work Checklist:
-> Bug Report:
-> System Architecture:
-> System Design Records:
-> Engine Technique Document:
-> Engine Implementation Records:
-> Verification Log:
-> PR Document:
-> Portfolio Technical Document:
````

---

## 5. 입력 기준

```yaml
입력 기준
-> Work Brief
-> 작업 개요
-> 정리된 기능 흐름
-> 작업 범위
-> 확정된 결정
-> 검토필요 항목
-> 선택필요 항목
-> 위험 항목
-> Prompt 라우팅 결과
-> 기능 목표
-> 포함 범위
-> 제외 범위
-> 현재 존재하는 코드 / 문서 / Asset / Blueprint
-> Codex 탐색 필요 항목
-> 사용자 결정이 필요한 항목
-> 검증 가능한 환경
```

---

## 6. 출력 기준

```yaml
출력 기준
-> Source of Truth 확인
-> Work Brief 항목 처리 결과
-> 현재 구조 탐색 대상
-> 기능 구현 목표
-> 구현 단위
-> 권장 실행 순서
-> 비범위 / 후속 범위
-> 위험 요소
-> 사용자 결정 필요 항목
-> 검증 계획
-> Commit 분리 후보
-> 문서화 필요 여부
-> 미검증 / 확인 필요 항목
```

---

## 7. 범위 / 비범위

```yaml
범위
-> 기능 구현 계획 수립
-> 구현 단위 분리
-> C++ / Blueprint / Asset / Editor 영향 후보 분리
-> 검증 계획 작성
-> Commit 분리 후보 제안
-> Work Pipeline 입력 계획 작성

비범위
-> 사용자 요청 최초 Intake
-> 최종 Work Checklist 작성
-> 코드 수정
-> 공식 문서 작성
-> Git staging / commit
```

---

## 8. 제약 조건

```yaml
제약 조건
-> Work Brief가 진행 불가 또는 검토 필요 상태이면 계획을 확정하지 않음
-> 계획차단 / 선택필요 항목이 있으면 Work Brief Intake로 되돌림
-> 계획차단 / 검토필요 항목이 있으면 Work Brief Intake로 되돌림
-> 확정된 결정 항목은 코드 / 문서 충돌이 확인되기 전까지 다시 열지 않음
-> 남은 검토필요 항목은 선행 확인 또는 구현 전 탐색 단위로 분리
-> 현재 구현 판단은 현재 코드 기준으로 함
-> 문서와 코드가 다르면 Source of Truth와 불일치 항목을 분리
-> 사용자 결정이 필요한 정책을 Codex가 임의로 확정하지 않음
-> 후속 범위를 현재 기능 구현 단위와 섞지 않음
-> 검증하지 못한 항목은 미검증으로 표시
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> Work Brief로 돌아가 목표 / 범위 / 미결정 항목을 보완
-> 선택필요 항목이 계획차단 / 선택필요인지 비차단 / 선택필요인지 구분
-> 검토필요 항목이 계획차단 / 검토필요이면 Work Brief Intake로 되돌림
-> 관련 코드나 Asset을 모르면 탐색 필요 항목으로 분리
-> 구현 단위가 너무 크면 더 작은 단위로 분해
-> 사용자 결정이 필요한 구조 선택은 선택지로 제시
```

---

## 10. 검증 기준

```yaml
검증 기준
-> Work Brief의 확정된 결정 / 검토필요 항목 / 선택필요 항목이 처리됨
-> 구현 단위마다 Build / Code Flow / PIE / Editor / Asset 검증 필요 여부가 분리됨
-> 위험 요소와 검증 기준이 연결됨
-> 사용자 결정 필요 항목이 구현 전 분리됨
-> 권장 실행 순서와 Commit 분리 후보가 변경 위험과 연결됨
-> 미검증 / 확인 필요 항목이 완료처럼 표현되지 않음
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 기능 구현 단위, 실행 순서, 위험, 검증 기준, 문서화 필요 여부가 구현 전 판단 가능한 수준으로 정리됨

실패
-> Work Brief 준비 상태가 충분하지 않음
-> 현재 구조 확인 불가
-> 기능 목표와 포함 범위가 충돌
-> 사용자 결정 없이는 구현 순서를 정할 수 없음

미검증
-> 코드 / Asset / Blueprint 확인이 필요한 항목
-> Editor / PIE 확인이 필요한 항목
-> 사용자 결정이 필요한 상태로 남은 항목
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Work Brief Intake Prompt
-> 사용자 요청 / Codex 진단 / 준비 상태 판정

Feature Work Planning Prompt
-> 준비된 Work Brief를 기능 구현 단위 / 실행 순서 / 검증 계획으로 분해

Refactor Work Planning Prompt
-> 구조 변경 / 리팩터링 중심 작업의 변경 단위 / 위험 / 검증 기준 정리

Work Checklist Writing Prompt
-> Work Brief와 Planning 결과를 최종 Work Checklist로 변환

Working Rule / Reference Prompt
-> 구현 중 적용할 실행 규칙과 책임 경계 판단 기준
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> 실제 Feature Branch 작업 적용 후 구현 단위 양식 보완
-> Work Brief와 Work Checklist 사이의 입력 / 출력 연결 기준 보완
-> Refactor Work Planning Prompt와 중복되는 항목 점검
```
