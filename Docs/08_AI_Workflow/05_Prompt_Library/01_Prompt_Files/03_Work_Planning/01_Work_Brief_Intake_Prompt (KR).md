# Work Brief Intake Prompt

## 1. 목적

작업 시작 전 자연어 요청을 조율된 `Work Brief` 값으로 정리하고, 검토 / 선택 / 확정 항목을 표시해 `Feature Work Planning`으로 넘어갈 수 있는 상태인지 판정한다.

이 Prompt는 사용자와 Codex가 작업 개요를 조율한 결과를 Work Brief로 갱신하기 위한 Intake Prompt다.

---

## 2. 사용 시점

```yaml
사용 시점
-> 새 Branch 또는 주요 작업 단위를 시작하기 전
-> 사용자의 자연어 요청을 작업 가능한 범위로 정리해야 할 때
-> 목표 / 범위 / 위험 / 미결정 항목을 사용자와 Codex가 함께 조율해야 할 때
-> 바로 Work Checklist를 만들기에는 입력이 부족한지 판단해야 할 때
-> 기존 Work Brief를 보완하거나 준비 상태를 다시 판정해야 할 때
```

---

## 3. 사용 방법

사용자가 자연어로 작업 요청을 제시하면 Codex가 요청을 해석해 `Work Brief` 문서를 작성하거나 갱신한다.

검토가 필요한 항목과 사용자의 선택이 필요한 항목은 Work Brief 안에서 상태별 섹션으로 분리한다.

사용자는 Work Brief 문서의 표시 항목을 보고 채팅에서 답변하고, Codex는 답변을 반영해 Work Brief를 다시 갱신한다.

`진행 가능` 또는 `준비 완료` 상태가 되면 작업 유형에 맞는 Planning Prompt 또는 실행 Prompt 후보를 제시한다.

Branch 단위 관리 문서가 필요하면 이후 `Work Checklist Writing Prompt`를 사용해 최종 Work Checklist를 작성한다.

---

## 4. 복사용 Prompt

````text
아래 사용자 요청을 바탕으로 Work Brief를 작성하거나 보완해줘.

목적은 작업 시작 전에 사용자와 Codex가 조율한 목표 / 범위 / 위험 / 미결정 항목을 Work Brief 값으로 정리하고, Feature Work Planning으로 넘어갈 수 있는 작업 개요를 만드는 것이다.

사용자가 자연어로 설명한 내용을 Work Brief 문서 형식으로 정리해줘.

1. 작업 목표, 작업 유형, 준비 상태, Planning Prompt 후보를 정리해줘.
2. 조율된 기능 흐름과 후속 흐름을 분리해줘.
3. 작업 범위와 후속 범위를 정리하고, 영구 제외 항목이 있으면 제외 범위를 별도로 분리해줘.
4. Feature Work Planning 진입 여부를 판단하는 데 필요한 수준으로 구조 타당성, 구현 비용, 변경 위험, 검증 위험, 문서화 영향을 진단해줘.
5. 코드 / 문서 / Asset 탐색이 필요한 항목은 `계획차단 / 검토필요` 또는 `비차단 / 검토필요` 섹션으로 분리해줘.
6. 사용자 결정이 필요한 항목은 `계획차단 / 선택필요` 또는 `비차단 / 선택필요` 섹션으로 분리하고 선택지를 `-` 목록으로 제시해줘.
7. 이미 결정된 항목은 `확정된 결정`, 이번 Branch 이후로 넘길 항목은 `후속 후보` 섹션으로 분리해줘.
8. 작업 유형을 `신규 기능 구현 / 기존 기능 보완 / 구조 변경 / 검토 / 검증 / 문서 작성 / Commit PR 준비` 중에서 판정해줘.
9. 판정 결과를 `Planning Prompt 후보`, `실행 계층 후보`, `Work Checklist Writing 후보 / 판단 위치`로 나눠줘.
10. Work Brief 준비 상태를 `진행 불가 / 검토 필요 / 진행 가능 / 준비 완료` 중 하나로 판정해줘.
11. `계획차단 / 검토필요` 항목은 Work Brief 단계에서 닫아야 하고, `비차단 / 검토필요` 항목은 Feature Work Planning의 선행 확인 항목으로 넘길 수 있는지 판단해줘.
12. `계획차단 / 선택필요`와 `비차단 / 선택필요` 항목은 사용자가 고를 수 있는 선택지와 권장안을 제시하고, 사용자가 선택하면 `확정`으로 이동해줘.
13. 비어 있는 `검토필요 항목` / `선택필요 항목` 섹션은 `현재 없음`으로 표시해줘.
14. `진행 불가` 또는 `검토 필요`이면 Work Brief에 남은 계획차단 항목과 권장 다음 단계를 남겨줘.

진단 기준:
- 구조 타당성: 현재 코드 / 문서 / Unreal 책임 경계 / Project Stella 구조 기준에서 흐름이 성립하는지 확인
- 구현 비용: 새 클래스 / 컴포넌트 / Asset / Blueprint / 기존 흐름 수정 범위가 Branch 단위로 감당 가능한지 확인
- 변경 위험: 기존 기능, 호출 흐름, 데이터 계약, Blueprint / Asset 참조를 깨뜨릴 가능성 확인
- 검증 위험: Build / Code Flow / PIE / Editor / Asset 검증으로 완료 여부를 확인할 수 있는지 확인
- 문서화 영향: Work Checklist / System Architecture / Design Records / Engine Records / Verification Log / PR Document 반영 필요 여부 확인
- Intake 단계의 진단은 Planning 진입 가능성 판정에 필요한 수준으로 제한하고, 구현 단위 / 실행 순서 / 상세 위험 분해는 Feature Work Planning으로 넘김
- 작업 유형이 섞이면 범위와 우선순위를 분리

대상 작업:
- Branch / 작업명:
- Work Brief 경로:
- 최종 Work Checklist 경로:

입력:
- [사용자 자연어 요청 또는 기존 Work Brief 내용]

출력 형식:
- Markdown
- 대화 로그가 아니라 현재 합의된 Work Brief 문서 형태
- Work Brief 관련 출력은 상태별 섹션을 사용하고 태그형 표기는 사용하지 않음
- 선택지는 `-` 목록으로 작성하고 짧은 설명과 권장 정도를 함께 표시
````

---

## 5. 진단 기준

Intake 단계의 진단은 `Feature Work Planning`으로 넘어갈 수 있는지 판단하기 위한 최소 진단이다.

구현 단위, 실행 순서, 상세 위험, 검증 순서, Commit 분리 후보는 `Feature Work Planning Prompt`에서 작성한다.

```yaml
구조 타당성
-> 현재 코드 / 문서 / Unreal 책임 경계 / Project Stella 구조 기준에서 기능 흐름이 성립하는지 판단
-> Source of Truth가 충돌하면 현재 구현 / 문서 의도 / 후속 설계 후보를 분리

구현 비용
-> 새 클래스 / 컴포넌트 / Asset / Blueprint 생성 범위 확인
-> 기존 호출 흐름 수정 범위 확인
-> Branch 단위로 감당 가능한 작업량인지 판단

변경 위험
-> 수정이 기존 기능, 호출 흐름, 데이터 계약을 깨뜨릴 가능성 판단
-> Blueprint / Asset / Montage / Collision 참조가 끊길 가능성 판단
-> 후속 Branch 범위와 섞일 가능성 판단

검증 위험
-> Build / Code Flow / PIE / Editor / Asset 검증으로 완료 여부를 확인할 수 있는지 판단
-> Codex가 직접 확인할 수 없는 검증이 있는지 판단
-> 미검증으로 남길 항목과 사용자 확인이 필요한 항목을 분리

문서화 영향
-> Work Checklist / System Architecture / System Design Records 반영 필요 여부 판단
-> Engine Technique Document / Engine Implementation Records 반영 필요 여부 판단
-> Verification Log / PR Document / Portfolio Technical Document 반영 필요 여부 판단
```

---

## 6. 작업 유형 판정 기준

작업 유형 판정과 다음 Prompt 선택 기준은 `05_Prompt_Flow_and_Routing_Blueprint (KR).md`를 따른다.

```yaml
신규 기능 구현
-> 새 기능 / 새 흐름 / 새 Component / 새 Asset 연결이 필요한 경우
-> 다음 Prompt 후보: Feature Work Planning Prompt

기존 기능 보완 / 업데이트
-> 기능은 존재하지만 동작, 범위, 정책, 검증, 문서가 부족한 경우
-> 다음 Prompt 후보: Feature Work Planning Prompt
-> 후속 후보: Update Work Planning Prompt

구조 변경 / 리팩터링
-> 책임 경계, 호출 흐름, 클래스 구조, 데이터 계약을 바꾸는 경우
-> 다음 Prompt 후보: Refactor Work Planning Prompt

검토 / 검증 중심
-> 구현보다 현재 상태 확인, 회귀 위험, 검증 결과 기록이 목표인 경우
-> 다음 Prompt 후보: Code Review / Verification Log / Asset Blueprint Validation Prompt

문서 작성 / 문서 정리
-> 코드 수정 없이 문서 생성, 재구성, 분류, 정합성 보완이 목표인 경우
-> 다음 Prompt 후보: Document Writing Prompt

Commit / PR 준비
-> 변경 내용 제출 전 Git 상태, 검증 상태, PR 가능 조건을 점검하는 경우
-> 다음 Prompt 후보: Git Commit PR Preflight Prompt
```

작업 유형이 섞여 있으면 Work Brief에서 포함 범위, 제외 범위, 우선순위를 먼저 분리한다.

작업 유형 판정 결과는 다음처럼 나눈다.

```yaml
Planning Prompt 후보
-> Feature Work Planning Prompt
-> Refactor Work Planning Prompt
-> 후속 후보: Update / Review Verification / Document Writing Planning Prompt

실행 계층 후보
-> 구현 계층 후보: Codex 구현 수행 또는 구현 Prompt 없음
-> 문서화 계층 후보: Document Writing / PR Document Writing Prompt
-> 검증 계층 후보: Code Review / Verification Log / Asset Blueprint Validation / Document Set Audit Prompt
-> Commit / PR 계층 후보: Git Commit PR Preflight Prompt

Work Checklist Writing 후보 / 판단 위치
-> Branch 단위 진행 관리, 완료 기준, 검증 상태, PR 가능 조건을 문서로 추적해야 하면 후보로 표시
-> 최종 작성 여부는 Work Planning 결과를 보고 확정
-> 단순 검토, 단일 문서 수정, 짧은 Git 점검처럼 관리 문서가 필요 없으면 생략 가능
```

---

## 7. 준비 상태 기준

준비 상태는 `Feature Work Planning`으로 넘어갈 수 있는지 판단하기 위한 기준이다.

```yaml
진행 불가
-> 목표 / 범위 / 구조가 불명확해 Feature Work Planning으로 넘어가기 위험함

검토 필요
-> 방향은 있지만 핵심 결정이나 입력 보완이 필요함

진행 가능
-> 계획차단 항목은 없고, 남은 비차단 항목은 선행 확인 / 후속 후보로 분리해 Feature Work Planning 진행 가능함
-> 구현 착수 가능 상태를 뜻하지 않음

준비 완료
-> 목표 / 범위 / 완료 기준 후보 / 검증 기준 후보가 확인되어 Feature Work Planning 진행 가능함
-> 구현 착수 여부는 Feature Work Planning 결과를 보고 별도로 판단
```

---

## 8. Work Brief 문서 갱신 기준

Work Brief는 사용자가 직접 채우는 작성 템플릿이 아니라, Codex가 채팅 내용을 바탕으로 작성 / 갱신하는 운영 문서다.

Work Brief에는 원문 보존 섹션과 해석 섹션을 병렬로 두지 않는다.

Work Brief에는 대화 과정을 길게 남기지 않고 현재 합의된 작업 개요, 정리된 기능 흐름, 확정된 결정, 남은 검토 항목, 선택 항목만 남긴다.

최종 Work Brief 문서에서는 상태별 섹션을 사용한다.

```md
## 검토필요 항목

### 계획차단 / 검토필요

- 현재 없음

### 비차단 / 검토필요

- 확인이 필요한 항목

## 선택필요 항목

### 계획차단 / 선택필요

- 현재 없음

### 비차단 / 선택필요

- 선택이 필요한 항목
```

상태 의미는 다음 기준을 따른다.

```yaml
계획차단 / 검토필요
-> 코드 / 문서 / Asset 탐색이 필요하고, 확인 전에는 Planning으로 넘어가기 위험한 항목

비차단 / 검토필요
-> 코드 / 문서 / Asset 탐색이 필요하지만, 선행 확인 항목으로 넘기고 Planning을 진행할 수 있는 항목

계획차단 / 선택필요
-> 사용자의 정책 선택 또는 범위 결정이 필요하고, 선택 전에는 Planning을 확정할 수 없는 항목

비차단 / 선택필요
-> 사용자의 선택이 필요하지만, 후속 범위나 구현 중 선택 항목으로 분리해 Planning을 진행할 수 있는 항목

확정된 결정
-> 사용자 답변 또는 Codex 권장안 수락으로 결정된 항목

후속 후보
-> 이번 Branch 범위에서 제외하고 후속 작업으로 넘길 항목
```

선택지가 필요한 항목은 표 대신 `-` 목록으로 작성한다.

```md
### 계획차단 / 선택필요

전달 방식

- Subsystem 사용 [권장]
  - 여러 Actor 사이의 중재와 확장성에 유리함
- Interface 사용 [선택]
  - 대상 Actor와의 계약을 명확히 하기 쉬움
- 직접 참조 [비권장]
  - 구현은 단순하지만 결합도가 높아짐
```

권장 Work Brief 구조는 다음을 따른다.

```yaml
작업 개요
-> 작업 목표 / 작업 유형 / 준비 상태 / Planning Prompt 정리

정리된 기능 흐름
-> 조율된 D20 기능 흐름과 후속 기능 흐름 정리

작업 범위
-> 이번 작업 범위 / 후속 범위 정리
-> 영구 제외 항목이 있으면 제외 범위 별도 정리

확정된 결정
-> 확정된 항목 정리

검토필요 항목
-> 계획차단 / 검토필요 항목 정리
-> 비차단 / 검토필요 항목 정리

선택필요 항목
-> 계획차단 / 선택필요 항목 정리
-> 비차단 / 선택필요 항목과 선택지 정리

위험 항목
-> 구조 / 구현 / 검증 위험 정리

Prompt 라우팅 결과
-> Planning Prompt / 실행 계층 후보 / Work Checklist Writing 후보와 판단 위치 정리

다음 단계
-> 다음에 호출할 Prompt 또는 사용자 답변 필요 항목 정리
```

---

## 9. Work Brief 처리 루프

Work Brief는 한 번에 완성하는 문서가 아니라, 사용자 자연어 요청과 Codex 검토 결과를 반복 반영해 현재 합의 상태로 갱신하는 문서다.

```yaml
처리 흐름
-> 사용자 자연어 요청 수신
-> Codex가 Work Brief 초안 또는 변경안 작성
-> 코드 / 문서 / Asset / Blueprint 확인이 필요한 항목을 계획차단 / 검토필요 또는 비차단 / 검토필요 섹션으로 분리
-> 사용자 정책 / 범위 / 구조 선택이 필요한 항목을 계획차단 / 선택필요 또는 비차단 / 선택필요 섹션으로 분리
-> Codex 탐색 또는 사용자 답변 결과를 Work Brief에 반영
-> 닫힌 항목은 확정된 결정 또는 후속 후보로 이동
-> 남은 계획차단 항목이 없으면 Planning Prompt로 라우팅
```

`계획차단 / 검토필요` 항목은 Codex가 먼저 코드 / 문서 / Asset / Blueprint를 확인해 Work Brief 단계에서 닫을 수 있는지 판단한다.

`비차단 / 검토필요` 항목은 Intake 단계에서 억지로 닫지 않고, Feature Work Planning의 선행 확인 항목으로 넘기는 것을 기본으로 한다.

```yaml
계획차단 / 검토필요 처리
-> 근거가 확인되면 확정된 결정으로 이동
-> 사용자 정책 선택이 필요하면 계획차단 / 선택필요 또는 비차단 / 선택필요로 전환
-> 이번 Branch 범위 밖이면 후속 후보로 이동
-> 추가 탐색이 필요하면 계획차단 / 검토필요로 유지하고 확인 대상을 구체화

비차단 / 검토필요 처리
-> Intake에서는 항목을 구체화해 남김
-> Feature Work Planning에서 선행 확인 항목 또는 구현 단위 전 탐색 항목으로 변환
-> Intake 중 명확히 범위 밖으로 확인되면 후속 후보로 이동
-> Intake 중 명확히 확정 가능한 근거가 확인된 경우에만 확정된 결정으로 이동
```

`계획차단 / 선택필요`와 `비차단 / 선택필요` 항목은 Codex가 임의 확정하지 않고 선택지와 권장안을 제시한다.

```yaml
계획차단 / 선택필요 처리
-> 선택지는 '-' 목록으로 작성
-> 각 선택지에 짧은 설명과 권장 정도를 표시
-> 사용자가 선택하거나 Codex 권장안을 승인하면 확정된 결정으로 이동
-> 이번 Branch에서 결정하지 않을 항목은 후속 후보로 이동

비차단 / 선택필요 처리
-> 선택지는 '-' 목록으로 작성
-> 각 선택지에 짧은 설명과 권장 정도를 표시
-> 선택 전에도 Planning 진행이 가능하면 후속 선택 항목 또는 구현 중 확인 항목으로 유지
-> 사용자가 선택하거나 Codex 권장안을 승인하면 확정된 결정으로 이동
```

빈 섹션은 삭제하지 않고 `현재 없음`으로 표시한다.

```yaml
검토필요 항목
-> 현재 없음

선택필요 항목
-> 현재 없음
```

Planning Prompt로 넘어가기 전에는 다음 조건을 확인한다.

```yaml
Planning 진입 조건
-> 계획차단 / 선택필요 항목 없음
-> 계획차단 / 검토필요 항목 없음
-> 남은 비차단 / 검토필요 항목은 Feature Work Planning의 선행 확인 항목으로 넘길 수 있음
-> Work Brief 준비 상태가 진행 가능 또는 준비 완료
```

Planning 진입 가능 상태는 구현 착수 가능 상태와 다르다.

```yaml
Planning 진입 가능
-> Feature Work Planning을 시작할 수 있음

구현 착수 가능
-> Feature Work Planning 결과에서 구현 단위 / 선행 조건 / 검증 기준이 정리된 뒤 판단
```

---

## 10. 입력 기준

```yaml
입력 기준
-> 사용자 자연어 요청 또는 기존 Work Brief
-> 현재 존재하는 구조
-> 이번 작업에 포함할 범위
-> 이번 작업에서 제외할 범위
-> 확인이 필요한 질문 / 미결정 항목
-> 관련 코드 / 문서 / Asset 경로
-> Feature Work Planning 진입 여부 판단에 필요한 정보
```

---

## 11. 출력 기준

```yaml
출력 기준
-> Work Brief Markdown
-> 작업 개요
-> 정리된 기능 흐름
-> 확정된 결정
-> 검토필요 항목
-> 선택필요 항목
-> 위험 항목
-> Planning Prompt 후보
-> 실행 계층 후보
-> Work Checklist Writing 후보 / 판단 위치
-> Work Brief 준비 상태 판정
-> 후속 Backlog 후보
```

---

## 12. 범위 / 비범위

```yaml
범위
-> 사용자 자연어 요청 정리
-> Work Brief 값 정리 / 진단
-> Work Brief 문서 작성 / 갱신
-> 계획차단 / 비차단 / 검토필요 / 선택필요 / 확정된 결정 / 후속 후보 섹션 분리
-> Work Brief 준비 상태 판정
-> Work Brief 작업 개요 작성

비범위
-> 최종 Work Checklist 작성
-> 구현 계획 확정
-> 코드 수정
-> Verification Log 작성
-> PR Document 작성
-> 공식 Architecture / Engine 문서 작성
```

---

## 13. 제약 조건

```yaml
제약 조건
-> 원문 보존 섹션과 해석 섹션을 병렬로 두지 않음
-> 결정되지 않은 정책을 확정한 것처럼 쓰지 않음
-> 검증하지 못한 항목은 미검증 또는 탐색 필요로 표시
-> 후속 범위를 현재 Branch 범위와 섞지 않음
-> 준비 상태가 진행 불가 또는 검토 필요이면 보완 질문과 권장 다음 단계를 먼저 제시
```

---

## 14. 검증 기준

```yaml
검증 기준
-> Work Brief가 현재 합의된 작업 개요 중심으로 작성되어 있는가
-> 정리된 기능 흐름과 작업 범위가 중복 없이 분리되어 있는가
-> 계획차단 / 비차단 / 검토필요 / 선택필요 / 확정된 결정 / 후속 후보가 섹션으로 분리되어 있는가
-> Planning Prompt 후보와 실행 계층 후보가 분리되어 있는가
-> 실행 계층 후보가 구현 / 문서화 / 검증 / Commit PR 계층으로 나뉘어 있는가
-> Work Checklist Writing 후보와 최종 판단 위치가 표시되어 있는가
-> Work Brief 준비 상태가 명확히 표시되어 있는가
-> Work Brief 작업 개요가 작업 범위 / 후속 범위 / 완료 기준 후보 / 검증 기준 후보로 나뉘어 있는가
-> Feature Work Planning으로 넘어갈 수 있는 상태인지 판단되어 있는가
```

---

## 15. 기존 Prompt와 역할 경계

```yaml
Work Brief Intake Prompt
-> 자연어 요청을 Work Brief 값으로 정리하고 검토필요 / 선택필요 항목을 닫아 Planning 진입 가능 상태를 판정

Feature Work Planning Prompt
-> 준비된 Work Brief를 기능 구현 단위 / 실행 순서 / 검증 계획으로 분해

Refactor Work Planning Prompt
-> 구조 변경 / 리팩터링 중심 작업의 변경 단위 / 위험 / 검증 계획 작성

Work Checklist Writing Prompt
-> 준비된 Work Brief와 Work Planning 결과를 바탕으로 최종 Work Checklist 작성

Document Set Audit Prompt
-> 문서군 정합성 / 운용 가능성 감사
```

---

## 16. 계속 수정할 항목

```yaml
후속 보완 후보
-> 실제 D20 Work Brief / Work Checklist 작성 결과를 기준으로 섹션 분리 기준 보완
-> Feature Work Planning Prompt와의 역할 중복 여부 검토
-> Work Brief 파일과 최종 Work Checklist 파일의 경로 / 명명 규칙 정리
```
