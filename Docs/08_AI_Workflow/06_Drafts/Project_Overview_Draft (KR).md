# Project Overview Draft

## 1. 목적

본 문서는 사용자 본인이 AI와 협업을 통한 결과물을 도출해내기 위해, 프로젝트 목표와 배경 정보를 압축하여 공유하기 위해 작성한 초안이다.

AI가 프로젝트의 장르, 구현 목표, 기술 스택, AI 활용 흐름을 빠르게 이해하고 이후 협업 규칙과 Prompt Library를 구성하는 데 참고할 수 있도록 작성한다.

현재 운영 기준은 `../00_Index/AI_Workflow_Index (KR).md`, `../01_Plan/AI_Workflow_Project_Plan (KR).md`, `../02_Operation/AI_Workflow_Operation_Guide (KR).md`, `../03_Work_Pipeline/AI_Work_Pipeline (KR).md`에서 관리한다. 본 문서는 초기 구상과 원문 근거를 보관하는 Draft다.

---

## 2. 프로젝트 제목

```yaml
프로젝트명
-> Project Stella

설명
-> Stella Blade 액션 시스템 분석 및 구현 포트폴리오
```

`Project Stella`는 원작명 `Stella Blade`와 구분되는 포트폴리오 프로젝트명이다.

---

## 3. 프로젝트 목적

해당 프로젝트는 게임 클라이언트 부문 신입 취업을 목적으로 한 Unreal Engine 기반 포트폴리오이다.

Stella Blade를 그대로 재현하는 것이 아니라, 액션 시스템을 분석하고 포트폴리오 범위에서 핵심 전투 구조를 구현하는 것을 목표로 한다.

---

## 4. 프로젝트 기간

```yaml
작업 기간
-> 2025.12.01 ~ 2026.06 말
```

---

## 5. 프로젝트 장르

해당 프로젝트는 ShiftUp의 Stella Blade가 보여주는 `소울라이크 + 스타일리시 액션` 계열 전투 경험을 참고한다.

```yaml
장르 목표
-> 소울라이크
-> 스타일리시 액션
```

---

## 6. 프로젝트 목표

### 요약

Stella Blade의 액션 시스템을 분석하고, 포트폴리오 범위에서 설명 가능한 구조로 구현하는 것을 목표로 한다.

### 기술 구현 목표

```yaml
소울라이크
-> 정교한 전투 공방
-> 가드 / 패링
-> 닷지 / 퍼펙트 닷지

스타일리시 액션
-> 브랜치 콤보 시스템
-> 처형 / 카운터
-> 스킬 시스템

Stella Blade 고유 액션
-> 블링크 / 임펄스
```

---

## 7. 프로젝트 기술 스택

### 개발 환경

```yaml
개발 환경
-> Unreal Engine 5.4
-> C++
-> Visual Studio 2022
-> Windows PC
```

### 버전 관리

```yaml
버전 관리
-> Git
-> GitHub
-> SourceTree
```

### 이슈 / 문서 관리

```yaml
관리 도구
-> GitHub Issues & Projects
-> Obsidian
```

---

## 8. AI 기반 작업 운영 방식 메모

AI 활용 방식은 Milestone에 따라 다음처럼 변화했다.

```yaml
Milestone 01 ~ 02
-> ChatGPT Web 사용
-> 코드와 문서를 수동으로 공유하며 설계, 구현 방향, 오류 원인을 질의응답 방식으로 검토

Milestone 03 ~ 05
-> Codex 사용
-> 로컬 Workspace와 Git 변경사항을 기반으로 구현, 리팩터링, 검증, 문서화를 진행

Milestone 06 이후
-> Codex + Prompt Workflow 사용
-> IDE Context, Plan Mode, Goal, Prompt Library 기반의 반복 가능한 AI 기반 작업 운영 체계로 확장
```

이 항목은 프로젝트의 기술 목표보다 AI 기반 작업 운영 방식의 변화에 대한 히스토리성 메모로 둔다.

---
