# AI Workflow 운영 체계 및 Prompt Library v1 초안 구성

## 1. 제목

docs: AI Workflow 운영 체계 및 Prompt Library v1 초안 구성

---

## 2. 관련 브랜치

- `feature/ai-workflow`

---

## 3. 요약

### 작업 요약

본 PR은 Codex를 적극 활용하기 위한 `AI Workflow v1` 문서 구조와 `Prompt Library v1` 초안을 구성한 작업이다.

이번 작업은 실제 UE C++ 기능 구현이 아니라, 자연어 요청을 `Work Brief -> Feature Work Planning -> Work List Draft`로 변환하는 AI 기반 작업 흐름을 문서화하고 제한적으로 검증하는 것을 목표로 한다.

### 이전 구조의 한계점

기존 문서 체계에서는 AI와 함께 작업할 때 다음 기준이 명확히 분리되어 있지 않았다.

```yaml
한계점
- AI Workflow 전체 문서 구조
- Project Stella 자체의 프로젝트 맥락
- Codex와의 작업 운영 기준
- 작업 파이프라인 단계
- Prompt 제작 기준
- Prompt 호출 흐름과 라우팅 기준
- Work Brief / Planning / Work List의 역할 경계
- 후속 Refactor 범위
```

### 현재 구조의 보완사항

이를 다음 다섯 축으로 정리했다.

```yaml
1. AI Workflow 문서 구조 구성
- Index / Overview / Project Context / Operation / Work Pipeline / Backlog / Drafts 분리

2. Prompt Blueprint 구성
- Prompt 제작 기준, Custom 기준, 유지보수 기준, Flow / Routing 기준 정리

3. Prompt Files v1 초안 구성
- Work Planning / Document Writing / Review Verification / Git Operation Prompt 초안 작성

4. W02 제한적 실사용 검증
- Parry 자연어 요청을 Work Brief / Feature Work Planning / Work List Draft로 변환

5. 후속 Refactor 범위 분리
- 실제 구현, 문서 체계 재편, Prompt 품질 검수, History 체계 등을 후속 작업으로 분리
```

---

## 4. 변경 범위

### A. AI Workflow 문서 구조 구성

`Docs/08_AI_Workflow`를 AI 기반 작업 운영 체계의 루트로 정리했다.

```yaml
구성 문서
- AI_Workflow_Index
- AI_Workflow_Overview
- Project_Stella_Overview
- AI_Workflow_Operation_Guide
- AI_Work_Pipeline
- AI_Workflow_Backlog
- AI_Workflow_Refactor_Notes
- Drafts
```

각 문서의 역할은 다음 기준으로 분리했다.

```yaml
Index
-> 위치 / 역할 / 상태 확인

Overview
-> AI Workflow 전체 구조 요약

Project Context
-> Project Stella 자체의 목적 / 범위 / 기술 맥락 설명

Operation Guide
-> Codex와 작업할 때의 운영 기준

Work Pipeline
-> 목표 확인 / 탐색 / 계획 / 적용 / 검증 / 문서화 공정

Backlog
-> 후속 작업 / 검토 후보 / 보류 항목 관리

Drafts
-> 사용자 원문 기반 구상과 근거 초안 보관
```

---

### B. Prompt Blueprint 구성

Prompt 제작 / 유지보수 / 라우팅 기준을 `00_Prompt_Blueprint` 아래에 정리했다.

```yaml
Prompt Blueprint
- Prompt Blueprint Overview
- Prompt Format Blueprint
- Prompt Engineering Blueprint
- Prompt Custom Blueprint
- Prompt Library Maintenance Blueprint
- Prompt Flow and Routing Blueprint
```

주요 정리 내용은 다음과 같다.

```yaml
Prompt Format Blueprint
-> Prompt의 섹션 구조와 기본 양식 기준

Prompt Engineering Blueprint
-> Prompt 내부 요청 내용 설계 기준

Prompt Custom Blueprint
-> Project Stella / AI Workflow 전용 Prompt 적용 기준

Prompt Library Maintenance Blueprint
-> Prompt 파일명 / 폴더 / 상태 / Archive 관리 기준

Prompt Flow and Routing Blueprint
-> 자연어 요청 이후 어떤 Prompt 계층을 거쳐 처리되는지 설명
```

---

### C. Prompt Files v1 초안 구성

실제 복사용 Prompt 파일을 역할별 폴더로 구성했다.

```yaml
Prompt Files
- Working Rules
- Working Reference
- Work Planning
- Document Writing
- Feature Work Prompts
- Review Verification
- Git Operation
```

이번 PR에서 특히 다음 Prompt 흐름을 구성했다.

```yaml
Work Brief Intake Prompt
-> 자연어 요청을 Work Brief로 정리하고 작업 유형 / 준비 상태 / 다음 Prompt 후보 판정

Feature Work Planning Prompt
-> Work Brief를 기능 구현 단위 / 실행 순서 / 검증 기준으로 분해

Refactor Work Planning Prompt
-> 구조 변경 / 리팩터링 작업의 변경 단위 / 위험 / 검증 기준 작성

Work List Writing Prompt
-> Work Brief와 Work Planning 결과를 실행 관리용 Work List로 변환

Document Set Audit Prompt
-> 문서군의 역할 / 구조 / 용어 / 연계성 / 운용 가능성 감사

Git Commit PR Preflight Prompt
-> Commit / PR 전 Git 상태와 검증 상태 점검
```

---

### D. Prompt Flow / Routing 기준 정리

자연어 요청 이후의 Prompt 호출 흐름을 다음 계층으로 정리했다.

```text
자연어 요청
-> Work Brief Intake
-> 작업 유형 판정
-> 필요한 Planning Prompt 선택
-> Work List Writing 후보 판단
-> Planning 결과 기준으로 필요 시 Work List Writing
-> 작업 수행
-> 필요 시 문서화 계층
-> 필요 시 검증 계층
-> 필요 시 Commit / PR 계층
```

Prompt 계층은 다음 기준으로 분리했다.

```yaml
입력 / 라우팅 계층
-> Work Brief Intake

계획 계층
-> Feature Work Planning
-> Refactor Work Planning

계획 시각화 / 관리 계층
-> Work List Writing

실행 계층
-> 구현
-> 문서화
-> 검증
-> Commit / PR

참조 계층
-> Working Rule
-> Working Reference
```

---

### E. W02 제한적 실사용 검증

Parry 구현 요청을 예시로 사용해 AI Workflow의 문서 변환 흐름을 검증했다.

```yaml
W02 문서 흐름
- W02 Work Brief
- W02 Feature Work Planning
- W02 Work List Draft
```

검증한 내용은 다음과 같다.

```yaml
검증 내용
- 자연어 요청을 Work Brief로 정리할 수 있는가
- Work Brief를 Feature Work Planning으로 변환할 수 있는가
- Feature Work Planning을 Work List Draft로 변환할 수 있는가
- 실제 구현 / Build / PIE / Editor / Asset 검증 항목을 완료 처리하지 않고 분리할 수 있는가
- 실제 구현 Branch로 넘길 범위를 명확히 분리할 수 있는가
- S26 / S27 / S28과 GuardAndParry 애셋을 현재 PR의 확정 기준이 아니라 실제 구현 Branch에서 확인할 입력 항목으로 분리할 수 있는가
```

W02 문서 3종은 AI Workflow 제한적 실사용 검증 예시이며, 실제 Parry 구현 완료 문서가 아니다.

---

### F. 리뷰 대응 반영

PR 리뷰에서 지적된 W02 입력 기준의 확정성 문제를 반영했다.

```yaml
S26 / S27 / S28
-> 현재 PR의 확정 기준이 아니라 실제 구현 Branch에서 확인할 후속 Architecture baseline 후보로 정리

GuardAndParry Animation Asset
-> 현재 PR의 전제가 아니라 실제 구현 Branch에서 확인할 입력 Asset 항목으로 정리
```

---

### G. Backlog / Refactor Notes 정리

현재 Branch에서 닫지 않을 범위를 후속 작업으로 분리했다.

```yaml
후속 Refactor 후보
- Prompt Flow / Routing 계층과 폴더 구조 정합성 점검
- Work Brief / Planning / Work List 필드 계약 정리
- Work List 위치와 갱신 규칙 정리
- Document Writing Prompt와 문서 카테고리 체계 재정리
- System Architecture / Engine Technique 문서 역할 분리
- History 문서 체계 신설 여부 검토
- Prompt 문장 품질 최종 검수
```

실제 W02 Parry 구현은 별도 Branch에서 진행하도록 분리했다.

---

## 5. 주요 커밋

```yaml
be43b53
-> docs(ai-workflow): establish workflow document structure

939ba31
-> docs(prompt): define prompt blueprint and routing model

fab6874
-> docs(prompt): add work planning prompts

8c54eef
-> docs(prompt): add document review and git operation prompts

234a114
-> docs(ai-workflow): record backlog and d20 workflow validation
```

---

## 6. 검증

### 수행한 검증

```yaml
문서 출력 확인
-> W01 Work List UTF-8 출력 확인

검색 검증
-> Branch 목표 / Prompt Library v1 / AI Workflow / W02 흐름 반영 여부 확인

배치 확인
-> Prompt Blueprint 6종 파일 배치 확인
-> Prompt Files 역할별 폴더 배치 확인

W02 검증
-> W02 Work Brief / Feature Work Planning / Work List Draft 문서 흐름 확인
-> W02 Work List Draft에서 실제 구현 / Build / PIE / Editor / Asset 항목이 완료 처리되지 않았는지 확인
```

### 수행하지 않은 검증

```yaml
미수행
-> UE C++ 빌드
-> PIE 검증
-> Editor / Asset 검증
-> 실제 Parry 구현 검증
```

본 PR은 문서 / Prompt Library 구성 작업이므로 UE C++ 빌드는 수행하지 않는다.

---

## 7. 비범위

```yaml
이번 PR 비범위
- W02 Parry 실제 구현
- Guard / Counter 세부 설계 확정
- W02 Work List Draft 공식 승격
- Prompt Library v1 전면 실사용 검증
- Prompt 문장 품질 최종 검수
- 기존 Docs 전체 리팩터링
- System Architecture / Engine Technique 문서 체계 재작성
- UE C++ 코드 변경 및 빌드 검증
```

---

## 8. 후속 작업

```yaml
실제 구현 Branch
-> W02 Parry 구현
-> W02 Work List Draft 공식 Work List 승격 여부 판단
-> Build / Code Flow / PIE / Editor / Asset 검증
-> Verification Log / PR Document 작성

AI Workflow Refactor
-> AI_Workflow_Refactor_Notes 기준으로 Prompt Flow / 문서 체계 / Work List 갱신 규칙 재검토
-> 실제 W02 구현 중 Workflow 사용성 확인 후 필요한 부분만 반영
```

---

## 9. 참고 문서

```yaml
Work List
-> Docs/01_Work_List/W01_Codex_Workflow/W01_UE5_Portfolio_Work_List.md

W02 예시
-> Docs/01_Work_List/W02_Parry/W02_UE5_Portfolio_Work_Brief.md
-> Docs/01_Work_List/W02_Parry/W02_UE5_Portfolio_Feature_Work_Planning.md
-> Docs/01_Work_List/W02_Parry/W02_UE5_Portfolio_Work_List_Draft.md

AI Workflow
-> Docs/08_AI_Workflow/00_Index/AI_Workflow_Index (KR).md
-> Docs/08_AI_Workflow/01_Overview/AI_Workflow_Overview (KR).md
-> Docs/08_AI_Workflow/03_Operation/AI_Workflow_Operation_Guide (KR).md
-> Docs/08_AI_Workflow/04_Work_Pipeline/AI_Work_Pipeline (KR).md

Prompt Library
-> Docs/08_AI_Workflow/05_Prompt_Library/00_Prompt_Blueprint/05_Prompt_Flow_and_Routing_Blueprint (KR).md

Backlog
-> Docs/08_AI_Workflow/06_Backlog/AI_Workflow_Backlog (KR).md
-> Docs/08_AI_Workflow/06_Backlog/AI_Workflow_Refactor_Notes (KR).md
```
