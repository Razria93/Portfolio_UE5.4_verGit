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

Prompt Files의 1차 분류는 작업 흐름 기준으로 유지한다. Unreal Engine 기준과 Project Stella 기준은 같은 폴더 체계 안에서 Prompt 내용과 Custom Rule로 적용한다.

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

`01_Unreal_Engine_Working_Rule_Prompt (KR).md`는 Unreal Engine 작업에 공통 적용할 실행 규칙을 담당한다.

`01_Unreal_Engine_Working_Reference_Prompt (KR).md`는 Unreal Engine 작업의 책임 경계, 구현 원칙, Blueprint / Asset 경계 기준을 담당한다.

---

## 4. Prompt Project Stella Custom Rules

Project Stella 작업을 다루는 Prompt는 다음 기준을 추가로 따른다.

```yaml
Project Stella 기준
-> Project Stella 작업 맥락을 명시
-> Stella Blade 액션 시스템 분석 / 구현 맥락을 반영
-> 포트폴리오 범위에서 설명 가능한 구조로 작성
```

`02_Project_Stella_Working_Reference_Prompt (KR).md`는 Project Stella 구조가 Unreal Engine 공통 규칙과 어떻게 맞거나 다른지 판단하는 Reference 기준을 담당한다.

`02_Project_Stella_Working_Rule_Prompt (KR).md`는 Project Stella 작업 세션에서 현재 기준 확인, 구현 전 계획, 검증 상태 분리, 문서화 필요 여부 판단에 적용하는 실행 규칙을 담당한다.

---

## 5. Document Writing Prompt Custom Rules

Document Writing Prompt는 문서 카테고리별 작성 / 보완에 적용한다. 대상 문서가 Work List, Bug Report, System Architecture, System Design Records, Engine Technique Document, Engine Implementation Records, Verification Log, PR Document, Portfolio Technical Document이면 응답에 다음 기준을 반영한다.

```yaml
Document 연결
Work List
-> 작업 목표
-> 범위 / 비범위
-> 완료 기준
-> 검증 상태
-> 후속 작업 범위

Bug Report
-> 재현 조건
-> 기대 결과 / 실제 결과
-> 원인
-> 수정
-> 검증

System Architecture
-> 현재 시스템 구조
-> 구성 요소
-> 책임 경계
-> 실행 흐름
-> 데이터 계약

System Design Records
-> Architecture Decision Record
-> Architecture Issue Report
-> 설계 결정 / 구조 문제 / 책임 충돌 기록

Engine Technique Document
-> Unreal Engine 기능 / API / 시스템 사용 방식
-> 적용 기준
-> 사용 시 주의점

Engine Implementation Records
-> Engine Decision Record
-> Engine Issue Report
-> 엔진 기능 사용 결정 / 엔진 동작 이슈 분석

Verification Log
-> 실제 수행한 검증
-> 실패한 검증
-> 미검증 항목
-> 다음 확인 항목

PR Document
-> Branch 결과
-> 변경 요약
-> 검증 상태
-> 미검증 항목
-> 후속 작업 범위

Portfolio Technical Document
-> 평가자에게 보여줄 기술 주제 중심 설명
-> 문제 정의 / 설계 판단 / 구현 구조 / 검증 결과 압축
```

문서 카테고리별 정의와 운영 기준은 `../../03_Operation/AI_Workflow_Operation_Guide (KR).md`와 `../../04_Work_Pipeline/AI_Work_Pipeline (KR).md`를 따른다.

---

## 6. Prompt Library Maintenance Custom Rules

Project Stella Prompt Library는 다음 폴더 구조를 기준으로 관리한다.

```yaml
05_Prompt_Library/01_Prompt_Files
-> Prompt Files 루트

01_Working_Rules
-> 작업 세션에 직접 사용하는 규칙 Prompt

02_Working_Reference
-> 작업 규칙을 적용할 때 참고하는 기술 기준 / 책임 경계 / 검증 기준 Prompt

03_Work_Planning
-> 작업 실행 전 목표 / 범위 / 변경 단위 / 위험 / 검증 기준을 정리하는 계획 Prompt

04_Document_Writing
-> 문서 카테고리별 작성 / 보완 Prompt

05_Feature_Work_Prompts
-> 특정 기능 작업을 재개하기 위한 Prompt

06_Review_Verification
-> 코드 리뷰, 검증 로그, Asset / Blueprint 검증 Prompt

07_Git_Operation
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

## 9. Work List / Backlog 연결

```yaml
Work List
-> 현재 Prompt Library 검토 범위와 완료 기준 관리

AI Workflow Backlog
-> Prompt 실사용 검증
-> Prompt 문장 품질 최종 검수
-> Feature Work Prompt Archive 기준
-> Work List 작성 / 갱신 규칙
-> 후속 보완 항목
-> 삭제 / Archive 판단 보류 항목
```

Prompt Custom Blueprint를 더 세분화할 필요가 생기면 `Prompt_Unreal_Engine_Custom_Blueprint`, `Prompt_Project_Stella_Custom_Blueprint`, `Prompt_Document_Writing_Custom_Blueprint`, `Prompt_Maintenance_Custom_Blueprint`로 나누는 것을 검토한다.
