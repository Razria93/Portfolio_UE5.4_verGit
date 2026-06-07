# Prompt Custom Blueprint

## 1. 목적

본 문서는 보편 Prompt Blueprint를 Unreal Engine, Project Stella, AI Workflow 문서 체계와 사용자 표현 기준에 적용할 때 필요한 Custom 규칙을 정리한다.

이 문서는 Unreal Engine Prompt, Project Stella Prompt, Document Writing Prompt, User Custom Prompt, Prompt Library 관리 기준에 적용한다.

---

## 2. 적용 범위

```yaml
적용 대상
-> Unreal Engine Prompt
-> Project Stella Prompt
-> Document Writing Prompt
-> User Custom Prompt
```

---

## 3. Prompt Unreal Engine Custom Rules

Unreal Engine 작업을 다루는 Prompt는 다음 기준을 추가로 따른다.

```yaml
Unreal / C++ / Blueprint / Asset / Editor 맥락
-> 입력 기준
-> 범위 / 비범위
-> 제약 조건

Build / Code Flow / PIE / Editor / Asset 검증 필요
-> 검증 기준
-> 완료 / 실패 / 미검증 처리 기준

검증 가능성 분리
-> C++ / Blueprint / Asset / Editor 검증 가능성 분리
-> Build / Code Flow / PIE / Editor / Asset / 미검증 항목 구분

영향 범위 분리
-> 코드 변경 영향
-> 문서 변경 영향
-> Asset / Blueprint 영향 가능성
```

---

## 4. Prompt Project Stella Custom Rules

Project Stella 작업을 다루는 Prompt는 다음 기준을 추가로 따른다.

```yaml
Project Stella 기준
-> Project Stella 작업 맥락을 명시
-> Stella Blade 액션 시스템 분석 / 구현 맥락을 반영
-> 포트폴리오 범위에서 설명 가능한 구조로 작성
```

---

## 5. Prompt Document Writing Custom Rules

Document Writing Prompt가 Work Checklist, PR Document, Verification Log, System Architecture, Bug Report를 작성하거나 보완하는 용도이면 응답에 다음 항목을 포함한다.

```yaml
Document 연결
Work Checklist
-> 작업 목표
-> 범위 / 비범위
-> 완료 기준
-> 검증 상태
-> 후속 작업 범위

PR Document
-> Branch 결과
-> 변경 요약
-> 검증 상태
-> 미검증 항목
-> 후속 작업 범위

Verification Log
-> 실제 수행한 검증
-> 실패한 검증
-> 미검증 항목
-> 다음 확인 항목

System Architecture
-> 책임 경계
-> 실행 흐름
-> 데이터 계약
-> 구조 변경 이유

Bug Report
-> 재현 조건
-> 원인
-> 수정
-> 검증
```

Document 유형별 정의와 운영 기준은 `../../03_Operation/AI_Workflow_Operation_Guide (KR).md`와 `../../04_Work_Pipeline/AI_Work_Pipeline (KR).md`를 따른다.

---

## 6. Prompt Library Maintenance Custom Rules

Project Stella Prompt Library는 다음 폴더 구조를 기준으로 관리한다.

```yaml
05_Prompt_Library/01_Prompt_Files
-> Prompt Files 루트

01_Working_Rules
-> 작업 세션에 직접 사용하는 규칙 Prompt

02_Working_Analysis
-> 작업 규칙의 분석 근거와 프로젝트 구조 대조 Prompt

03_Document_Writing
-> Work Checklist / PR Document / Bug Report / System Architecture / Technical Document 작성 Prompt

04_Feature_Work_Prompts
-> 특정 기능 작업을 재개하기 위한 Prompt

05_Review_Verification
-> 코드 리뷰, 검증 로그, Asset / Blueprint 검증 Prompt

06_Git_Operation
-> Commit / PR 전 점검 Prompt
```

Prompt Library의 위치, 역할, 상태는 `../../00_Index/AI_Workflow_Index (KR).md`에 반영한다.

후속 작업, 삭제 / Archive 판단 보류 항목, 실사용 검증 후보는 `../../06_Backlog/AI_Workflow_Backlog (KR).md`에 반영한다.

```yaml
관리 기준
-> 실제 Prompt Library 폴더 구조는 이 섹션에서 관리
-> Prompt 위치 / 역할 / 상태는 AI Workflow Index에 반영
-> Prompt Library에 등록하는 Prompt는 기존 Prompt와 역할 경계를 명시
-> 실사용 검증 후보, Archive 후보, 명칭 전환 대기 항목은 Backlog에 반영
-> Prompt 문장 품질 최종 검수 후보는 Backlog에 반영
-> 개별 Prompt 수정 후보는 Backlog 또는 해당 Prompt 파일에 반영
-> Prompt Custom Blueprint 자체에는 변동 항목을 길게 누적하지 않음
```

---

## 7. 상위 운영 기준 연결

Prompt 응답에 판단, 검증, 미검증, 다음 선택지, 작업 단계가 포함되는 경우 다음 상위 문서를 따른다.

```yaml
Operation Guide
-> 판단 / 검증 / 미검증 / 다음 선택지 기준
-> 사용자 결정이 필요한 항목 분리 기준
-> 검증하지 못한 항목을 완료로 표현하지 않는 기준

Work Pipeline
-> 작업 단계 / 입력 / 출력 / 완료 기준
-> 단계 전환 기준
-> 문서 작성 / 검증 기록 / PR 정리 시점
```

---

## 8. Prompt User Custom Rules

```yaml
문체
-> 한국어 기술 문서체
-> 반박형 메타 문장 지양
-> 같은 문맥 안의 문서 카테고리명 표기 통일

판단 표현
-> 객관적이고 보편적인 기준을 우선해 표현
-> 중복 / 구버전 Prompt 방치 위험을 명시
-> 삭제 / 대체 / 병합 판단은 근거와 선택지로 제시
```

---

## 9. Work Checklist / Backlog 연결

```yaml
Work Checklist
-> 현재 Prompt Library 검토 범위와 완료 기준 관리

AI Workflow Backlog
-> Prompt 실사용 검증
-> Prompt 문장 품질 최종 검수
-> Feature Work Prompt Archive 기준
-> Work Checklist 명칭 전환
-> 후속 보완 항목
-> 삭제 / Archive 판단 보류 항목
```

Prompt Custom Blueprint를 더 세분화할 필요가 생기면 `Prompt_Unreal_Engine_Custom_Blueprint`, `Prompt_Project_Stella_Custom_Blueprint`, `Prompt_Document_Writing_Custom_Blueprint`, `Prompt_Maintenance_Custom_Blueprint`로 나누는 것을 검토한다.