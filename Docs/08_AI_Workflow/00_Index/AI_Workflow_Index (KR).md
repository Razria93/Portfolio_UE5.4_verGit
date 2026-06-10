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

  01_Overview/
    AI_Workflow_Overview (KR).md

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

## 4. Overview

- 폴더 경로: `01_Overview/`
- 폴더 역할: AI 기반 작업 운영 체계의 상위 개요 관리

```yaml
AI_Workflow_Overview (KR).md
-> [유지] Project Stella에서 AI 기반 작업 운영 체계의 목적, 구성요소, 책임, 작업 단계, 문서 관계를 설명하는 상위 개요 문서
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
    05_Prompt_Flow_and_Routing_Blueprint (KR).md

  01_Prompt_Files/
    01_Working_Rules/
    02_Working_Reference/
    03_Work_Planning/
    04_Document_Writing/
    05_Feature_Work_Prompts/
    06_Review_Verification/
    07_Git_Operation/
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

05_Prompt_Flow_and_Routing_Blueprint (KR).md
-> [생성됨 / 검토 필요] 자연어 요청부터 Work Brief Intake, Work Planning, Document Writing, Review / Verification까지의 Prompt 호출 흐름과 라우팅 기준
```

### 8.2. Prompt Files

#### 8.2.1. 01_Working_Rules

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/01_Working_Rules/`
- 역할: 작업 세션용 규칙 Prompt 분류

```yaml
01_Unreal_Engine_Working_Rule_Prompt (KR).md
-> [생성됨 / 검토 필요] Unreal Engine C++ 작업 공통 규칙 Prompt

02_Project_Stella_Working_Rule_Prompt (KR).md
-> [생성됨 / 검토 필요] Project Stella 작업 세션 실행 규칙 Prompt
```

#### 8.2.2. 02_Working_Reference

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/02_Working_Reference/`
- 역할: 작업 규칙을 적용할 때 참고하는 기술 기준 / 책임 경계 / 검증 기준 Prompt 분류

```yaml
01_Unreal_Engine_Working_Reference_Prompt (KR).md
-> [생성됨 / 검토 필요] Unreal Engine C++ 책임 경계 / 구현 원칙 / 검증 기준 Reference Prompt

02_Project_Stella_Working_Reference_Prompt (KR).md
-> [생성됨 / 검토 필요] Project Stella 구조와 Unreal Engine 공통 규칙 대조 Reference Prompt
```

#### 8.2.3. 03_Work_Planning

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/03_Work_Planning/`
- 역할: 작업 실행 전 목표 / 범위 / 변경 단위 / 위험 / 검증 기준을 정리하는 계획 Prompt 분류

```yaml
01_Work_Brief_Intake_Prompt (KR).md
-> [생성됨 / 검토 필요] 새 Branch 작업 시작 전 사용자 요청 / Codex 진단 / 준비 상태를 조율하는 Intake Prompt

02_Feature_Work_Planning_Prompt (KR).md
-> [생성됨 / 검토 필요] 준비된 Work Brief를 기능 구현 단위 / 실행 순서 / 검증 계획으로 분해하는 Planning Prompt

03_Refactor_Work_Planning_Prompt (KR).md
-> [생성됨 / 검토 필요] 리팩터링 착수 전 변경 단위 / 위험 / 검증 기준 계획 Prompt
```

#### 8.2.4. 04_Document_Writing

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/04_Document_Writing/`
- 역할: 문서 카테고리별 작성 / 보완 Prompt 분류
- 대상 문서: Work List / Bug Report / System Architecture / System Design Records / Engine Technique Document / Engine Implementation Records / Verification Log / PR Document / Portfolio Technical Document

```yaml
01_Work_List_Writing_Prompt (KR).md
-> [생성됨 / 검토 필요] 준비된 Work Brief와 Work Planning 결과를 바탕으로 최종 Work List를 작성 / 보완하는 Prompt

Bug_Report_Writing_Prompt (KR).md
-> [생성됨 / 검토 필요] Bug Report 작성 / 보완 Prompt

System_Architecture_Writing_Prompt (KR).md
-> [생성됨 / 검토 필요] System Architecture 작성 / 보완 Prompt

PR_Document_Writing_Prompt (KR).md
-> [생성됨 / 검토 필요] PR Document 작성 / 보완 Prompt

Portfolio_Technical_Document_Writing_Prompt (KR).md
-> [생성됨 / 검토 필요] Portfolio Technical Document 작성 / 보완 Prompt

Troubleshooting_Summary_Prompt (KR).md
-> [생성됨 / 검토 필요] 여러 Bug Report / Architecture Issue Report / Engine Issue Report를 Portfolio Technical Document 하위 Troubleshooting 유형으로 압축하는 Prompt
```

#### 8.2.5. 05_Feature_Work_Prompts

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/05_Feature_Work_Prompts/`
- 역할: 특정 기능 작업 재개 Prompt 분류

```yaml
Cross_Domain_Interrupt_Work_Prompt (KR).md
-> [생성됨 / 검토 필요] Cross-domain interrupt / intervention 기능 작업 재개 Prompt
```

#### 8.2.6. 06_Review_Verification

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/06_Review_Verification/`
- 역할: 코드 리뷰, 문서 양식 / 정합성 감사, 검증 로그, Asset / Blueprint 검증 Prompt 분류

```yaml
Document_Format_Normalization_Prompt (KR).md
-> [생성됨 / 검토 필요] 문서군 파일명 / 상단 메타 / 문체 / 공백 양식 정리 Prompt

Document_Set_Audit_Prompt (KR).md
-> [생성됨 / 검토 필요] 문서군 정합성 / 운용 가능성 감사 Prompt

Code_Review_Prompt (KR).md
-> [생성됨 / 검토 필요] 코드 리뷰 Findings 도출 Prompt

Verification_Log_Prompt (KR).md
-> [생성됨 / 검토 필요] Build / Code Flow / PIE / Editor / Asset 검증 결과 기록 Prompt

Asset_Blueprint_Validation_Prompt (KR).md
-> [생성됨 / 검토 필요] Editor / Asset / Blueprint 영향 점검 Prompt
```

#### 8.2.7. 07_Git_Operation

- 분류 경로: `05_Prompt_Library/01_Prompt_Files/07_Git_Operation/`
- 역할: Commit / PR 전 점검 Prompt 분류

```yaml
Git_Commit_PR_Preflight_Prompt (KR).md
-> [생성됨 / 검토 필요] Commit / PR 전 Git 상태와 검증 상태 점검 Prompt
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
-> [보관] AI Workflow Overview 작성을 위한 사용자 원문 기반 프로젝트 개요 초안

Project_Rules_Draft (KR).md
-> [보관] AI 기반 작업 운영 규칙 구상을 위한 사용자 원문 기반 초안

AI_Work_Plan_Draft (KR).md
-> [보관] 초기 작업 흐름과 운영 후보를 정리한 사용자 원문 기반 초안
```

---

## 11. 외부 연결 문서

```yaml
Docs/01_Work_List/W01_Codex_Workflow/W01_UE5_Portfolio_Work_List.md
-> [완료] feature/codex-workflow Branch의 목표, 완료 기준, 현재 작업 상태를 관리하는 Work List
```

---

## 12. 상태 기준

```yaml
유지
-> 현재 역할이 명확하고 계속 사용

작성 / 검토 대기
-> 아직 개별 파일 작성 또는 Prompt Blueprint 기준 검토가 필요

생성됨 / 검토 필요
-> 파일이 실제로 존재하고 Prompt Blueprint 기준의 초안 검토가 필요

보관
-> 최종 운영 문서는 아니지만 근거 문서로 보존

진행 중
-> 현재 Branch에서 검토 / 작성 / 보완 중
```
