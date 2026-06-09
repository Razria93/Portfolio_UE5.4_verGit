# Project Stella Overview

## 1. 목적

본 문서는 `Project Stella`의 목표, 범위, 장르, 기술 스택, 핵심 구현 대상을 정리하는 현재 기준 프로젝트 개요 문서다.

`../01_Overview/AI_Workflow_Overview (KR).md`가 AI 기반 작업 운영 체계를 설명한다면, 본 문서는 그 운영 체계가 적용될 포트폴리오 프로젝트 자체를 설명한다.

---

## 2. 프로젝트 개요

```yaml
프로젝트명
-> Project Stella

설명명
-> Stella Blade Action System Analysis & Implementation Portfolio

설명
-> Stella Blade 액션 시스템 분석 및 구현 포트폴리오

프로젝트 기간
-> 2025.12.01 ~ 2026.06 말

프로젝트 목적
-> 게임 클라이언트 부문 신입 취업을 위한 Unreal Engine 기반 포트폴리오
```

`Project Stella`는 원작명 `Stella Blade`와 구분되는 포트폴리오 프로젝트명이다.

---

## 3. 프로젝트 목표

`Project Stella`는 `Stella Blade`의 액션 시스템을 분석하고, 포트폴리오 범위에서 설명 가능한 핵심 전투 구조를 구현하는 프로젝트다.

원작 전체를 재현하는 것이 아니라, 전투 공방, 입력 반응, 상태 전환, 카메라 / 연출 흐름처럼 게임 클라이언트 구현 역량을 보여줄 수 있는 구조를 선별해 구현한다.

---

## 4. 장르 목표

```yaml
장르 목표
-> 소울라이크
-> 스타일리시 액션
```

소울라이크 계열의 정교한 전투 공방과 스타일리시 액션 계열의 빠른 입력 반응, 연계, 연출 흐름을 함께 다룬다.

---

## 5. 핵심 구현 목표

```yaml
소울라이크 액션
-> 가드
-> 패링
-> 닷지
-> 퍼펙트 닷지

스타일리시 액션
-> 브랜치 콤보 시스템
-> 처형
-> 카운터
-> 스킬 시스템

Stella Blade 액션
-> 블링크
-> 임펄스
```

각 기능은 단순 동작 구현보다 책임 경계, 실행 흐름, 입력 처리, 검증 가능성을 설명할 수 있는 구조로 구현하는 것을 우선한다.

---

## 6. 기술 스택

```yaml
Engine
-> Unreal Engine 5.4

Language
-> C++

IDE / OS
-> Visual Studio 2022
-> Windows PC

Version Control
-> Git
-> GitHub
-> SourceTree

Documentation
-> Obsidian
```

---

## 7. 포트폴리오 범위

```yaml
포함 범위
-> 핵심 전투 기능 구현
-> 기능별 책임 경계 정리
-> 코드 흐름과 상태 전환 설명
-> Build / Code Flow / PIE / Editor / Asset 검증 상태 분리
-> 주요 작업 결과 문서화

비범위
-> 원작 전체 콘텐츠 재현
-> 상용 게임 수준의 전체 Asset 제작
-> 모든 연출 / UI / 레벨 구성의 완전 재현
```

포트폴리오 범위는 “얼마나 많이 만들었는가”보다 “핵심 구조를 분석하고 설명 가능한 방식으로 구현했는가”를 기준으로 둔다.

---

## 8. AI Workflow와의 관계

`Project Stella`는 AI Workflow의 적용 대상 프로젝트다.

AI Workflow는 `Project Stella`의 작업을 목표 해석, 구조 탐색, 구현, 검증, 기록, 문서화 흐름으로 통제하기 위한 운영 체계다.

```yaml
Project Stella
-> 구현 대상 프로젝트

AI Workflow
-> Project Stella 작업을 운영하고 검증하고 기록하기 위한 작업 체계
```

