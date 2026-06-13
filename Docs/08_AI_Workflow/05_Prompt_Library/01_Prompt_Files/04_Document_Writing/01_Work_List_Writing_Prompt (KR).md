# Work List Writing Prompt

## 1. 목적

준비된 `Work Brief`와 Work Planning 결과를 바탕으로 최종 `Work List`를 작성하거나 보완한다.

Work List는 현재 작업의 목표 / 범위 / 완료 기준 / 검증 상태 / 후속 범위를 관리하는 상태 문서다.

이 Prompt의 전용 책임은 Work List의 목표, 범위, 완료 기준, 체크리스트, 검증 기준, PR 가능 조건, 후속 작업 범위를 정리하는 것이다.

---

## 2. 사용 시점

```yaml
사용 시점
-> Work Brief가 진행 가능 또는 준비 완료 상태일 때
-> Feature Work Planning 또는 Refactor Work Planning 결과가 준비되었을 때
-> 새 Branch 또는 주요 작업 단위의 최종 Work List를 작성할 때
-> 기존 Work List를 PR 가능한 기준으로 보완할 때
-> 완료 항목과 후속 작업 범위를 분리해야 할 때
-> 검증 상태와 PR 가능 조건을 명확히 해야 할 때
```

---

## 3. 사용 방법

대상 Branch / 작업명, Work Brief 경로와 준비 상태, Feature 또는 Refactor Work Planning 결과, Branch 작업 개요, 완료 기준 후보, 검증 기준 후보, 후속 범위 / Backlog 후보, 관련 문서와 현재 검증 상태를 제공하고 `복사용 Prompt`를 사용한다.

상단 메타, 날짜, 상태, 브랜치, 공백 같은 공통 양식은 `00_Document_Common_Format_Prompt` 기준을 따르되, Work List 전용 세부 양식은 이 Prompt를 우선한다.

입력이 부족하거나 Work Brief 준비 상태가 `진행 불가` 또는 `검토 필요`이면 `Work Brief Intake Prompt`로 돌아간다.

---

## 4. 복사용 Prompt

````text
아래 입력을 바탕으로 최종 Work List를 작성하거나 기존 Work List를 보완해줘.

준비된 Work Brief와 Work Planning 결과를 최종 Work List의 목표 / 완료 기준 / 체크 항목 / 검증 기준 / PR 가능 조건으로 변환해줘.

대상 작업:
- Branch / 작업명:
- Work Brief 경로:
- Work Planning 결과 경로:
- Work List 경로:
- 신규 작성 또는 기존 문서 보완 여부:

준비 상태:
- 진행 가능 / 준비 완료:
- 확정된 결정:
- 남은 검토필요 항목:
- 남은 선택필요 항목:
- 후속 범위:

Branch 작업 개요:
- Branch 목표:
- 포함 범위:
- 제외 범위:
- 완료 기준 후보:
- 검증 기준 후보:
- Backlog 후보:

관련 문서:
- AI Workflow Overview:
- Project Context:
- Work Pipeline:
- Operation Guide:
- Work Brief:
- Feature Work Planning:
- Refactor Work Planning:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- Portfolio Technical Document:

작성 목표:
- 작업 결과 설명보다 목표 / 범위 / 완료 기준 / 검증 상태 / 후속 작업 범위를 우선 정리해줘.
- Work List 상단 메타는 `제목 / 날짜 / 상태 / 브랜치` 형식으로 작성해줘.
- 상태는 `- [ ] **진행중**` 또는 `- [x] **완료**`로 작성해줘.
- 브랜치는 bullet list로 작성하고 브랜치 값은 인라인 코드로 작성해줘.
- Work Brief의 `확정된 결정` 항목은 완료 기준 또는 결정 항목으로 반영해줘.
- Work Brief의 `검토필요 항목`은 선행 확인 또는 체크리스트 미완료 항목으로 반영해줘.
- Work Brief의 `선택필요 항목`은 사용자 결정 전까지 완료 항목으로 처리하지 마.
- Work Brief의 `후속 후보` 항목은 제외 범위 / 후속 작업 범위로 반영해줘.
- 현재 작업 범위와 후속 작업 범위를 분리해줘.
- 완료된 체크 항목은 `- [x]`, 남은 체크 항목은 `- [ ]`로 구분해줘.
- 검증하지 못한 항목은 미검증 또는 후속 작업으로 분리해줘.
- PR 가능 조건은 완료 기준과 검증 상태를 근거로 작성해줘.

Work List 작성 양식:
1. 상단 메타
2. 작업 목표
3. 작업 개요 / 기능 흐름 요약
4. 구조 / 비용 / 위험 검토
5. 결정이 필요한 항목
6. 이번 작업 범위
7. 제외 범위 / 후속 작업 범위
8. 완료 기준
9. 필수 문서 / 산출 대상
10. 항목별 체크리스트
11. 검증 기준
12. 진행 중 변경 관리 기준
13. PR 가능 조건
14. 비고 / Backlog 후보

체크 항목은 완료 기준을 검증 가능한 작업 단위로 분해해서 작성해줘.
출력할 때는 즉시 문서에 붙여 넣을 수 있는 Markdown 형식으로 작성해줘.
````

---

## 5. Work List 작성 양식

이 섹션은 `00_Document_Common_Format_Prompt`의 공통 양식을 Work List 기준으로 구체화한 작성 기준이다. 실제 작업에 맞지 않는 섹션은 축약할 수 있지만, 목표 / 완료 기준 / 검증 상태 / 후속 작업 범위는 가능한 한 명시한다.

```md
# UE5 Portfolio - Work List

## 제목

**W##: 작업 제목**

## 날짜

**yyyy.mm.dd**

## 상태

- [ ] **진행중**

---

## 브랜치

- `branch-name`

---
```

```yaml
1. 상단 메타
-> H1은 문서 카테고리명으로 작성
-> 제목 섹션에는 파일명 넘버링과 같은 `W##:` 제목을 Bold로 작성
-> 날짜는 `**yyyy.mm.dd**`로 작성
-> 날짜가 확인되지 않으면 `**추가요망**`으로 작성
-> 상태는 `- [ ] **진행중**` 또는 `- [x] **완료**`로 작성
-> 상태와 브랜치 사이에는 `---` 구분선을 배치
-> 브랜치 섹션명은 `브랜치`로 작성
-> 브랜치는 단일 / 복수 모두 bullet list로 작성
-> 브랜치 값은 인라인 코드로 작성
-> 브랜치가 확인되지 않으면 `- **추가요망**`으로 작성

2. 작업 목표
-> 이번 작업의 목적
-> 현재 작업 범위
-> 후속 작업과 구분해야 할 범위

3. 작업 개요 / 기능 흐름 요약
-> Work Brief와 Work Planning 결과를 기준으로 기술 흐름 정리
-> 입력 / 상태 / 이벤트 / 컴포넌트 / 데이터 / 결과 흐름 분리
-> 작업 목표와 구현 흐름 구분

4. 구조 / 비용 / 위험 검토
-> 구조 타당성
-> 구현 비용
-> 변경 위험
-> 더 단순하거나 안전한 대안
-> 부족한 입력
-> 미검증 가능성이 높은 범위

5. 결정이 필요한 항목
-> 질문 또는 선택지
-> 사용자 결정 전까지 확정하지 않을 항목
-> Codex가 임의로 정하면 위험한 정책 / 구조 / 검증 기준

6. 이번 작업 범위
-> 이번 Branch 또는 작업 단위에 포함할 항목
-> 먼저 구현 / 작성 / 검토할 항목
-> 최소 연결 범위

7. 제외 범위 / 후속 작업 범위
-> 이번 작업에서 하지 않는 항목
-> 다음 Branch 또는 별도 작업으로 넘길 항목
-> 미검증 또는 사용자 확인 필요 항목

8. 완료 기준
-> PR 가능 여부를 판단할 기준
-> 완료 항목과 미완료 항목의 구분 기준
-> 검증 상태와 연결되는 기준

9. 필수 문서 / 산출 대상
-> 이번 작업에서 작성 또는 보완해야 하는 문서
-> Prompt / 코드 / 문서 / 검증 기록 등 관리 대상
-> 파일 존재와 검토 완료 상태를 분리

10. 항목별 체크리스트
-> 문서별 / 기능별 / 단계별 체크 항목
-> 완료 항목은 [x]
-> 미완료 항목은 [ ]
-> 검토 필요 항목은 상태를 문장으로 표시

11. 검증 기준
-> Build / Code Flow / PIE / Editor / Asset / 문서 검색 등 확인 기준
-> 실행 가능한 검증과 실행 불가능한 검증 분리
-> 미검증 항목 기록 방식

12. 진행 중 변경 관리 기준
-> 작업 중 변경된 범위
-> 새로 생긴 후속 작업
-> Backlog로 넘길 항목

13. PR 가능 조건
-> 완료 기준 충족 여부
-> 검증 상태
-> 남은 미검증 항목의 영향
-> Commit / PR 전에 확인해야 할 항목

14. 비고 / Backlog 후보
-> 특이사항
-> 명칭 병행 상태
-> 임시 기준
-> 후속 정리 필요 사항
```

```yaml
체크 항목 작성 기준
-> 완료 기준을 검증 가능한 작업 단위로 분해
-> 각 체크 항목은 완료 여부를 확인할 수 있는 결과 중심으로 작성
-> 파일 생성 / 수정 / rename / 참조 갱신 / 상태 반영 / 검증 확인을 필요한 경우 분리
-> `- [x]`는 실제 완료 확인된 항목에만 사용
-> `- [ ]`는 남은 작업, 검토 필요, 미검증 항목에 사용
-> 추상 표현보다 확인 가능한 결과 중심으로 작성
```

---

## 6. 입력 기준

```yaml
입력 기준
-> Branch명 또는 작업명
-> Work Brief 경로
-> Work Brief 준비 상태
-> Work Brief의 확정된 결정
-> Work Brief의 검토필요 항목
-> Work Brief의 선택필요 항목
-> Work Brief의 후속 후보
-> Work Planning 결과
-> Branch 작업 개요
-> 작업 목표
-> 작업 개요 / 기능 흐름 요약
-> 이번 작업에 포함할 범위
-> 이번 작업에서 제외할 범위
-> 완료 기준 후보
-> 검증 기준 후보
-> 후속 작업 후보
-> 사용자 확인이 필요한 항목
```

Work Brief와 Work List의 필드 연결은 다음 기준을 따른다.

```yaml
Work Brief 작업 개요
-> Work List 작업 목표
-> Work List Branch 작업 개요

Work Brief 정리된 기능 흐름
-> Work List 작업 개요 / 기능 흐름 요약

Work Brief 작업 범위
-> Work List 이번 작업 범위
-> Work List 제외 범위 / 후속 작업 범위

Work Brief 확정된 결정
-> Work List 결정이 필요한 항목 중 확정된 결정
-> Work List 완료 기준 후보

Work Brief 검토필요 항목
-> Work List 선행 확인 항목
-> Work List 미완료 체크 항목

Work Brief 선택필요 항목
-> Work List 결정이 필요한 항목
-> 사용자 결정 전까지 완료 처리하지 않는 항목

Work Brief 후속 후보
-> Work List 제외 범위 / 후속 작업 범위
-> Work List Backlog 후보

Work Brief 위험 항목
-> Work List 구조 / 비용 / 위험 검토

Feature / Refactor Work Planning 결과
-> Work List 항목별 체크리스트
-> Work List 검증 기준
-> Work List PR 가능 조건
```

관련 문서는 다음 기준을 따른다.

```yaml
관련 문서 작성 기준
-> 확인 가능한 실제 파일명만 작성
-> 재작성 예정이거나 매칭이 불확실한 문서는 확정 관련 문서로 작성하지 않음
-> 확인이 필요한 문서는 후속 검토 항목으로 분리
-> 같은 파일명이 여러 폴더에 중복될 때만 경로를 함께 작성
-> 문서 존재와 검토 완료 상태를 분리
```

---

## 7. 출력 기준

```yaml
출력 기준
-> Markdown 형식의 Work List
-> Work_List 신규 파일명에는 `(KR)`을 붙이지 않음
-> 파일명과 본문 제목의 `W##` 넘버링 일치
-> 상단 메타의 제목 / 날짜 / 상태 / 브랜치 형식 일치
-> 확인 가능한 관련 문서 기준 적용
-> 작업 목표
-> 작업 개요 / 기능 흐름 요약
-> 구조 / 비용 / 위험 검토
-> 결정이 필요한 항목
-> 확정된 결정
-> 검토필요 항목
-> 후속 작업 범위
-> 이번 작업 범위
-> 완료 기준
-> 필수 문서 / 산출 대상
-> 항목별 체크리스트
-> 진행 중 변경 관리 기준
-> 제외 범위 / 후속 작업 범위
-> PR 가능 조건
-> 비고 / Backlog 후보
```

---

## 8. 범위 / 비범위

```yaml
범위
-> Work List 신규 작성
-> 기존 Work List 보완
-> 준비된 Work Brief와 Work Planning 결과를 최종 Work List로 변환
-> 작업 범위 / 완료 기준 / 검증 상태 정리
-> 후속 작업 범위 분리
-> PR 가능 조건 작성

비범위
-> Work Brief 입력 양식 제공
-> 사용자 요청의 최초 해석 / Intake 진단
-> 기능 구현 단위 / 실행 순서 계획
-> 코드 수정
-> PR Document 작성
-> Verification Log 작성
-> Bug Report 작성
-> System Architecture 작성
```

---

## 9. 제약 조건

```yaml
제약 조건
-> Work Brief가 진행 불가 또는 검토 필요 상태이면 최종 Work List를 확정하지 않음
-> Work Brief의 확정된 결정을 임의로 다시 열지 않음
-> Work Brief의 계획차단 / 선택필요 항목이 남아 있으면 최종 Work List를 확정하지 않음
-> Work Brief의 계획차단 / 검토필요 항목이 남아 있으면 최종 Work List를 확정하지 않음
-> 현재 작업 범위와 후속 작업 범위를 분리함
-> 사용자 결정이 필요한 항목을 Codex가 임의로 확정하지 않음
-> Work_List 신규 문서 파일명에 `(KR)`을 붙이지 않음
-> 파일 존재와 검토 완료 상태를 구분함
-> 검증하지 않은 항목을 완료 처리하지 않음
-> 완료 기준과 PR 가능 조건을 연결함
-> W01 같은 기존 Work List의 구조는 참고하되 고유 내용을 복사하지 않음
```

---

## 10. 모호성 처리 기준

```yaml
모호한 경우
-> 준비 상태가 불명확하면 Work Brief Intake Prompt로 돌아감
-> 작업 범위가 불명확하면 질문 또는 선택지로 분리
-> 완료 여부가 불명확하면 미완료 또는 검토 필요로 표시
-> 검증 여부가 불명확하면 미검증으로 표시
-> 관련 문서의 기준이 충돌하면 충돌 내용을 비고 또는 후속 작업으로 분리
-> Legacy Issue Checklist / Work List 명칭이 충돌하면 현재 대상 문서의 파일명과 본문 제목을 기준으로 판단
```

---

## 11. 검증 기준

```yaml
검증 기준
-> Work_List 신규 파일명에 `(KR)`이 없는가
-> 파일명과 본문 제목의 `W##` 넘버링이 일치하는가
-> 상단 메타가 제목 / 날짜 / 상태 / 브랜치 형식으로 작성되었는가
-> 상태가 `- [ ] **진행중**` 또는 `- [x] **완료**` 형태인가
-> 브랜치가 bullet list이고 브랜치 값이 인라인 코드인가
-> Work Brief의 준비 상태가 진행 가능 또는 준비 완료인가
-> Work Brief의 확정된 결정 / 검토필요 항목 / 선택필요 항목 / 후속 후보가 Work List에 올바르게 반영되었는가
-> Work Planning 결과가 구현 단위 / 검증 계획 / 후속 범위를 분리하는가
-> 문서 역할이 Work List에 맞는가
-> 작업 개요 / 기능 흐름 요약이 작업 목표와 연결되는가
-> 구조 / 비용 / 위험 검토가 결정 필요 항목과 연결되는가
-> 관련 문서가 확인 가능한 실제 파일명 기준으로 작성되었는가
-> 작업 목표와 완료 기준이 연결되는가
-> 항목별 체크리스트가 완료 기준에서 도출되었는가
-> 필수 문서 / 산출 대상과 항목별 체크리스트가 일치하는가
-> 현재 작업 범위와 후속 작업 범위가 분리되어 있는가
-> 완료 항목과 미검증 항목이 섞이지 않았는가
-> PR 가능 조건이 검증 상태와 연결되는가
```

---

## 12. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> Work Brief와 Work Planning 결과가 최종 Work List로 변환됨
-> Work Brief의 확정된 결정 / 검토필요 항목 / 선택필요 항목 / 후속 후보가 누락 없이 반영됨
-> 작업 개요 / 결정 필요 항목 / 작업 범위 / 완료 기준 / 검증 상태 / 후속 작업 범위가 분리됨
-> PR 가능 조건을 판단할 수 있음
-> 미검증 항목이 완료처럼 표현되지 않음

실패
-> Work Brief 준비 상태가 진행 불가 또는 검토 필요임
-> 필수 입력 부족으로 작업 범위를 정할 수 없음
-> 사용자 결정 없이는 범위나 정책을 확정할 수 없음
-> 완료 기준과 검증 상태를 연결할 수 없음

미검증
-> 실제 검증 여부가 확인되지 않은 항목
-> 사용자 확인이 필요한 상태값
-> 관련 문서 경로 또는 최신 기준을 확인하지 못한 항목
```

---

## 13. 기존 Prompt와 역할 경계

```yaml
00_Document_Common_Format_Prompt
-> 파일명 / 상단 메타 / 날짜 / 상태 / 브랜치 / 공백 같은 공통 양식 기준

Work Brief Intake Prompt
-> 사용자 요청 / Codex 해석 / 진단 / 질문 / 선택지를 정리하고 Feature Work Planning 진입 가능 상태를 판정

Feature Work Planning Prompt
-> 준비된 Work Brief를 기능 구현 단위 / 실행 순서 / 검증 계획으로 분해

Refactor Work Planning Prompt
-> 구조 변경 / 리팩터링 중심 작업의 변경 단위 / 위험 / 검증 계획 작성

Work List Writing Prompt
-> 준비된 Work Brief와 Work Planning 결과를 바탕으로 최종 Work List 작성

Verification Log Prompt
-> 수행한 검증과 미검증 항목 기록

PR Document Writing Prompt
-> Branch 결과 제출 문서 작성

Document Set Audit Prompt
-> 문서군의 역할, 구조, 용어, 경로, 상태값, 운용 가능성 감사
```

---

## 14. 후속 보완 후보

```yaml
후속 보완 후보
-> W02 Work List 작성 결과를 기준으로 작성 양식 보완
-> Work List 양식을 별도 Format Reference로 분리할지 검토
-> Work Brief Intake Prompt와 Feature Work Planning Prompt의 입력 / 출력 연결 기준 점검
```
