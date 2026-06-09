# UE5 Portfolio - Work Checklist

## 제목

**M06-01: AI Workflow 운영 체계 및 Prompt Library v1 초안 구성**

### 날짜

- **Day 19**
- **Date : 2026.06.05**

---

### Branch

- `feature/codex-workflow`

---

## 1. Branch 목표

이번 Branch는 Codex를 적극 활용하기 위한 `AI Workflow v1` 문서 구조와 `Prompt Library v1` 초안을 구성한다.

```yaml
핵심 목표
- AI Workflow 전체 문서 구조 구성
- AI Workflow 운영 기준 / 작업 파이프라인 정리
- Project Stella 적용 맥락 분리
- Prompt Blueprint / Prompt Files v1 초안 구성
- 자연어 요청을 Work Brief / Feature Work Planning / Work Checklist Draft로 변환하는 예시 흐름 검증
- 후속 AI Workflow Refactor 범위 정리
```

```yaml
목표 수준
- AI Workflow 문서군은 위치 / 역할 / 상태를 확인할 수 있는 구조로 둠
- Prompt Library v1은 구조와 역할 기준을 갖춘 초안 라이브러리로 둠
- D20 Parry 예시는 실제 구현이 아니라 Workflow 변환 흐름 검증 예시로 둠
- Prompt 품질 최종 검수와 실제 기능 구현 검증은 후속 Branch 범위로 넘김
```

---

## 2. 완료 기준

이 Branch는 다음 조건을 만족하면 PR 가능한 상태로 본다.

```yaml
완료 기준
- AI Workflow 핵심 문서의 역할과 위치가 분리되어 있음
- Prompt Blueprint 6종의 역할이 분리되어 있음
- Prompt Files v1 초안과 역할별 폴더 구조가 구성되어 있음
- AI Workflow Backlog와 Refactor Notes가 후속 범위를 관리함
- D20 예시 문서 3종으로 Work Brief -> Feature Work Planning -> Work Checklist Draft 흐름이 확인됨
- 실제 Parry 구현 / UE C++ 빌드 / PIE / Asset 검증이 이번 Branch 범위에서 제외되어 있음
- Commit 대상과 제외 대상이 분리되어 있음
```

---

## 3. 필수 산출물

```yaml
AI Workflow 문서
- Docs/08_AI_Workflow/00_Index/AI_Workflow_Index (KR).md
- Docs/08_AI_Workflow/01_Overview/AI_Workflow_Overview (KR).md
- Docs/08_AI_Workflow/02_Project_Context/Project_Stella_Overview (KR).md
- Docs/08_AI_Workflow/03_Operation/AI_Workflow_Operation_Guide (KR).md
- Docs/08_AI_Workflow/04_Work_Pipeline/AI_Work_Pipeline (KR).md
- Docs/08_AI_Workflow/06_Backlog/AI_Workflow_Backlog (KR).md
- Docs/08_AI_Workflow/06_Backlog/AI_Workflow_Refactor_Notes (KR).md
- Docs/08_AI_Workflow/07_Drafts 하위 Draft 문서
```

```yaml
Prompt Library
- Docs/08_AI_Workflow/05_Prompt_Library/00_Prompt_Blueprint/00_Prompt_Blueprint_Overview (KR).md
- Docs/08_AI_Workflow/05_Prompt_Library/00_Prompt_Blueprint/01_Prompt_Format_Blueprint (KR).md
- Docs/08_AI_Workflow/05_Prompt_Library/00_Prompt_Blueprint/02_Prompt_Engineering_Blueprint (KR).md
- Docs/08_AI_Workflow/05_Prompt_Library/00_Prompt_Blueprint/03_Prompt_Custom_Blueprint (KR).md
- Docs/08_AI_Workflow/05_Prompt_Library/00_Prompt_Blueprint/04_Prompt_Library_Maintenance_Blueprint (KR).md
- Docs/08_AI_Workflow/05_Prompt_Library/00_Prompt_Blueprint/05_Prompt_Flow_and_Routing_Blueprint (KR).md
- Docs/08_AI_Workflow/05_Prompt_Library/01_Prompt_Files 역할별 폴더와 Prompt 초안
```

```yaml
Work Checklist / D20 예시
- Docs/01_Issue_CheckList/D19_UE5_Portfolio_Work_Checklist (KR).md
- Docs/01_Issue_CheckList/request/D20_UE5_Portfolio_Work_Brief (KR).md
- Docs/01_Issue_CheckList/request/D20_UE5_Portfolio_Feature_Work_Planning (KR).md
- Docs/01_Issue_CheckList/request/D20_UE5_Portfolio_Work_Checklist_Draft (KR).md
```

---

## 4. 현재 Branch 마감 체크리스트

### 4.1. AI Workflow 핵심 문서

- [x] `AI_Workflow_Index (KR).md` 작성
- [x] `AI_Workflow_Overview (KR).md` 작성
- [x] `Project_Stella_Overview (KR).md` 작성
- [x] `AI_Workflow_Operation_Guide (KR).md` 작성
- [x] `AI_Work_Pipeline (KR).md` 작성
- [x] `AI_Workflow_Backlog (KR).md` 작성
- [x] `AI_Workflow_Refactor_Notes (KR).md` 작성
- [x] Draft 문서를 `07_Drafts`에 분리

### 4.2. Prompt Blueprint

- [x] Prompt Blueprint Overview 작성
- [x] Prompt Format Blueprint 작성
- [x] Prompt Engineering Blueprint 작성
- [x] Prompt Custom Blueprint 작성
- [x] Prompt Library Maintenance Blueprint 작성
- [x] Prompt Flow and Routing Blueprint 작성
- [x] Prompt Blueprint 6종의 역할 경계 정리

### 4.3. Prompt Files v1 초안

- [x] `01_Working_Rules` 폴더와 Prompt 초안 구성
- [x] `02_Working_Reference` 폴더와 Prompt 초안 구성
- [x] `03_Work_Planning` 폴더와 Prompt 초안 구성
- [x] `04_Document_Writing` 폴더와 Prompt 초안 구성
- [x] `05_Feature_Work_Prompts` 폴더와 Prompt 초안 구성
- [x] `06_Review_Verification` 폴더와 Prompt 초안 구성
- [x] `07_Git_Operation` 폴더와 Prompt 초안 구성
- [x] Prompt Library v1은 실전 검증 완료본이 아니라 초안 라이브러리로 명시

### 4.4. D20 제한적 실사용 검증

- [x] D20 Parry 자연어 요청을 `D20_UE5_Portfolio_Work_Brief (KR).md`로 정리
- [x] D20 Work Brief를 `D20_UE5_Portfolio_Feature_Work_Planning (KR).md`로 변환
- [x] D20 Feature Work Planning을 `D20_UE5_Portfolio_Work_Checklist_Draft (KR).md`로 변환
- [x] Work Brief / Feature Work Planning / Work Checklist Draft의 입력-출력 연결 확인
- [x] 실제 Parry 구현 / Build / PIE / Editor / Asset 검증 항목은 완료 처리하지 않도록 분리
- [x] D20 실제 구현은 별도 Branch 후속 작업으로 분리

### 4.5. 후속 범위 인계

- [x] `AI_Workflow_Refactor_Notes (KR).md`에 다음 AI Workflow Refactor 후보 정리
- [x] `AI_Workflow_Backlog (KR).md`에 Refactor Notes 연결
- [x] D20 실제 구현과 Work Checklist Draft 공식 승격을 후속 Branch 범위로 분리
- [x] Document Writing / History / System Architecture / Engine Technique 체계 보완을 후속 Refactor 범위로 분리

---

## 5. 다음 AI Workflow Refactor 인계 범위

다음 항목은 현재 Branch 마감 조건이 아니다. `AI_Workflow_Refactor_Notes (KR).md`와 Backlog 기준으로 후속 Refactor에서 검토한다.

```yaml
계층 구조 / 카테고리
- Prompt Planning / Work Planning / 실행 / 검증 / 문서화 / Git 계층 재분류
- Prompt Flow and Routing Blueprint와 실제 폴더 구조 정합성 점검
- Work Brief / Planning / Checklist 필드 계약 정리

Overview / Pipeline
- AI Workflow Overview를 상위 요약 문서로 재정의
- 기존 Overview의 Draft 성격 내용 분리 여부 검토
- Work Checklist 위치와 실행 후 Update 규칙 정리

Prompt Library
- Prompt Files 내용 품질 정밀 검토
- Prompt 중복 축약 / 병합 / 삭제 후보 재점검
- Prompt 문장 품질 최종 검수
- Prompt Planning 계열 신설 여부 검토

문서 체계
- Document Writing Prompt 재구성
- System Architecture / System Design Records 체계 정리
- Engine Technique / Engine Implementation Records 체계 정리
- History 문서 신설 여부 검토
- 양식과 Prompt 기능 분리 여부 검토

검증 / Commit / PR
- Document Set Audit Prompt 보강
- Verification Log / PR Document 연계 검토
- Git Commit / PR Preflight Prompt 실사용 보완
```

---

## 6. 실제 구현 Branch 인계 범위

다음 항목은 AI Workflow Branch가 아니라 실제 D20 구현 Branch에서 처리한다.

```yaml
D20 Parry 구현
- D20 Work Checklist Draft 공식 Work Checklist 승격 여부 판단
- UCCombatResolutionComponent 최소 구성
- CAction_Parry 구현
- Parry Montage / Window 연결
- TakeDamage 진입부 Combat Resolution 선처리
- Parry 성공 시 Damage 무효화
- Parry Reaction interrupt / Feedback 연결

검증
- Build 검증
- Code Flow 검증
- PIE 검증
- Editor / Asset 검증
- Verification Log 작성
- PR Document 작성
```

---

## 7. 제외 범위

```yaml
이번 Branch 제외 범위
- D20 Parry 실제 구현
- Guard / Counter 세부 설계 확정
- D20 Work Checklist Draft 공식 승격
- Prompt Library v1 전면 실사용 검증
- Prompt 문장 품질 최종 검수
- 기존 Docs 전체 리팩터링
- System Architecture / Engine Technique 문서 체계 재작성
- UE C++ 코드 변경 및 빌드 검증
- 영어 Work Checklist 작성
```

---

## 8. PR 가능 조건

### 8.1. 산출물 상태

- [x] AI Workflow 핵심 문서 작성 완료
- [x] Prompt Blueprint 6종 작성 완료
- [x] Prompt Files v1 초안 구성 완료
- [x] Backlog / Refactor Notes 작성 완료
- [x] D19 Work Checklist 작성 완료
- [x] D20 Work Brief / Feature Work Planning / Work Checklist Draft 작성 완료

### 8.2. 검증 상태

- [x] D19 Work Checklist가 UTF-8로 정상 출력되는지 확인
- [x] 핵심 항목 검색으로 Branch 목표 / Prompt Library v1 / AI Workflow / D20 흐름 반영 여부 확인
- [x] Prompt Blueprint 6종 파일 배치 확인
- [x] Prompt Files 역할별 폴더 배치 확인
- [x] D20 Work Checklist Draft에서 실제 구현 / Build / PIE / Editor / Asset 항목이 완료 처리되지 않았는지 확인
- [x] 문서 작업이므로 UE C++ 빌드를 수행하지 않는 것으로 검증 범위 명시

### 8.3. Commit / PR 준비 상태

- [ ] 이번 Branch Commit 후보를 `Docs/08_AI_Workflow/`, `Docs/01_Issue_CheckList/D19_UE5_Portfolio_Work_Checklist (KR).md`, `Docs/01_Issue_CheckList/request/`의 D20 예시 문서로 제한
- [ ] 기존 대량 Docs 변경, `Docs/08_Reference`, unrelated README 변경을 별도 판단 대상으로 분리
- [ ] PR 문서 작성 시 AI Workflow Overview / Work Pipeline / Operation Guide / AI Workflow Index / D19 Work Checklist / D20 예시 문서를 근거로 연결

---

## 9. 비고

- D19는 AI Workflow v1 구조와 Prompt Library v1 초안을 닫기 위한 Work Checklist다.
- Prompt Library v1은 완성품이 아니라 실제 작업에 적용하기 전의 초안 구조다.
- D20 Parry 문서 3종은 AI Workflow 제한적 실사용 검증 예시이며, 실제 Parry 구현 완료 문서가 아니다.
- 다음 AI Workflow Refactor 범위는 `AI_Workflow_Refactor_Notes (KR).md`에서 관리한다.
- `Docs/08_Reference` 자료는 작업 구상용 참고 자료로 보고, 이번 Branch Commit 대상에서는 제외한다.
