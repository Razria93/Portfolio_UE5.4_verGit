# AI Workflow Index

## 1. 목적

본 문서는 `Docs/08_AI_Workflow` 하위 문서의 위치, 역할, 상태를 확인하기 위한 전체 인덱스다.

규칙, 사용법, 유지보수 기준, 후속 작업 상세는 각 담당 문서에서 관리한다.

---

## 2. AI Workflow 문서 구조

```text
Docs/08_AI_Workflow/
  00_Index/
    AI_Workflow_Index (KR).md

  01_Plan/
    AI_Workflow_Project_Plan (KR).md

  02_Project_Context/
    Project_Stella_Overview (KR).md

  03_Operation/
    AI_Workflow_Operation_Guide (KR).md

  04_Work_Pipeline/
    AI_Work_Pipeline (KR).md

  05_Prompt_Library/
    00_Prompt_Blueprint/
    01_Prompt_Files/

  06_Backlog/
    AI_Workflow_Backlog (KR).md

  07_Drafts/
    Project_Overview_Draft (KR).md
    Project_Rules_Draft (KR).md
    AI_Work_Plan_Draft (KR).md
```

---

## 3. Index

- 폴더 경로: `00_Index/`
- 폴더 역할: AI Workflow 문서군의 위치, 역할, 상태 확인

```yaml
AI_Workflow_Index (KR).md
-> [유지] Docs/08_AI_Workflow 하위 문서군의 전체 인덱스
```

---

## 4. Plan

- 폴더 경로: `01_Plan/`
- 폴더 역할: AI 기반 작업 운영 체계의 상위 기획

```yaml
AI_Workflow_Project_Plan (KR).md
-> [유지] Project Stella에서 AI 기반 작업 운영 체계를 구성하기 위한 상위 기획 문서
```

---

## 5. Project Context

- 폴더 경로: `02_Project_Context/`
- 폴더 역할: AI Workflow가 적용될 프로젝트 자체의 배경과 범위 관리

```yaml
Project_Stella_Overview (KR).md
-> [유지] Project Stella의 목적, 범위, 장르, 기술 스택, 핵심 구현 목표를 정리하는 본 개요 문서
```

---

## 6. Operation

- 폴더 경로: `03_Operation/`
- 폴더 역할: Pipeline 운용 원칙, 책임 경계, 검증, 기록, Commit / PR 기준 관리

```yaml
AI_Workflow_Operation_Guide (KR).md
-> [유지] AI Workflow를 실제 작업에서 운용하기 위한 내부 운영 기준
```

---

## 7. Work Pipeline

- 폴더 경로: `04_Work_Pipeline/`
- 폴더 역할: 작업 흐름, 단계별 입력 / 출력, 완료 기준, 단계 전환 기준 관리

```yaml
AI_Work_Pipeline (KR).md
-> [유지] AI 기반 작업을 단계별 공정으로 처리하기 위한 작업 흐름 문서
```

---

## 8. Prompt Library

- 폴더 경로: `05_Prompt_Library/`
- 폴더 역할: Prompt 제작 기준 문서와 Prompt Files 분류 기준 관리

```text
05_Prompt_Library/
  00_Prompt_Blueprint/
    00_Prompt_Blueprint_Overview (KR).md
    01_Prompt_Format_Blueprint (KR).md
    02_Prompt_Engineering_Blueprint (KR).md
    03_Prompt_Custom_Blueprint (KR).md
    04_Prompt_Library_Maintenance_Blueprint (KR).md

  01_Prompt_Files/
    01_Working_Rules/
    02_Working_Analysis/
    03_Document_Writing/
    04_Feature_Work_Prompts/
    05_Review_Verification/
    06_Git_Operation/
```

### 8.1. Prompt Blueprint

- 폴더 경로: `05_Prompt_Library/00_Prompt_Blueprint/`
- 폴더 역할: 반복 사용할 Prompt의 형식, 내용 설계, Custom 적용, 유지보수 기준을 관리하는 기준 문서 묶음

```yaml
00_Prompt_Blueprint_Overview (KR).md
-> [유지] Prompt Blueprint 묶음의 역할과 적용 흐름

01_Prompt_Format_Blueprint (KR).md
-> [유지] Prompt의 큰 틀과 섹션 구조 기준

02_Prompt_Engineering_Blueprint (KR).md
-> [유지] Prompt 내부 요청 내용 설계 기준

03_Prompt_Custom_Blueprint (KR).md
-> [유지] Project Stella / AI Workflow 전용 Prompt 규칙

04_Prompt_Library_Maintenance_Blueprint (KR).md
-> [유지] Prompt Library 폴더, 이름, 상태, 중복, Archive 관리 기준
```

### 8.2. Prompt Files

#### 8.2.1. 01_Working_Rules

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/01_Working_Rules/`
- 역할: 작업 세션용 규칙 Prompt 분류

```yaml
-> 작성 / 검토 대기
```

#### 8.2.2. 02_Working_Analysis

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/02_Working_Analysis/`
- 역할: 작업 규칙의 분석 근거 Prompt 분류

```yaml
-> 작성 / 검토 대기
```

#### 8.2.3. 03_Document_Writing

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/03_Document_Writing/`
- 역할: Work Checklist / PR Document / Bug Report / System Architecture / Technical Document 작성 Prompt 분류

```yaml
-> 작성 / 검토 대기
```

#### 8.2.4. 04_Feature_Work_Prompts

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/04_Feature_Work_Prompts/`
- 역할: 특정 기능 작업 재개 Prompt 분류

```yaml
-> 작성 / 검토 대기
```

#### 8.2.5. 05_Review_Verification

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/05_Review_Verification/`
- 역할: 코드 리뷰, 검증 로그, Asset / Blueprint 검증 Prompt 분류

```yaml
-> 작성 / 검토 대기
```

#### 8.2.6. 06_Git_Operation

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/06_Git_Operation/`
- 역할: Commit / PR 전 점검 Prompt 분류

```yaml
-> 작성 / 검토 대기
```

---

## 9. Backlog

- 폴더 경로: `06_Backlog/`
- 폴더 역할: 후속 작업, 검토 후보, 보류 항목 관리

```yaml
AI_Workflow_Backlog (KR).md
-> [유지] AI Workflow 문서군과 Prompt Library의 후속 작업 / 검토 후보 / 보류 항목 관리 문서
```

---

## 10. Drafts

- 폴더 경로: `07_Drafts/`
- 폴더 역할: 초기 구상과 원문 근거 보관

```yaml
Project_Overview_Draft (KR).md
-> [보관] Project Plan 작성을 위한 사용자 원문 기반 프로젝트 개요 초안

Project_Rules_Draft (KR).md
-> [보관] AI 기반 작업 운영 규칙 구상을 위한 사용자 원문 기반 초안

AI_Work_Plan_Draft (KR).md
-> [보관] 초기 작업 흐름과 운영 후보를 정리한 사용자 원문 기반 초안
```

---

## 11. 외부 연결 문서

```yaml
Docs/01_Issue_CheckList/D19_UE5_Portfolio_Work_Checklist (KR).md
-> [진행 중] feature/codex-workflow Branch의 목표, 완료 기준, 현재 작업 상태를 관리하는 Work Checklist
```

---

## 12. 상태 기준

```yaml
유지
-> 현재 역할이 명확하고 계속 사용

작성 / 검토 대기
-> 아직 개별 파일 작성 또는 Prompt Blueprint 기준 검토가 필요

보관
-> 최종 운영 문서는 아니지만 근거 문서로 보존

진행 중
-> 현재 Branch에서 검토 / 작성 / 보완 중
```

