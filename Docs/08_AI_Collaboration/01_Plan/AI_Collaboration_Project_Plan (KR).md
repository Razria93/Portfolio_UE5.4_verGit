# AI Collaboration Project Plan

## 1. 목적

이 문서는 UE5 Portfolio 프로젝트에서 AI와 협업하기 위한 상위 기획 문서다.

`Project Stella`를 진행하는 과정에서 AI 협업을 목표, 책임, 검증, 기록 체계 안에서 통제하는 기준을 정의한다.

문서 위계는 `Project Plan -> Work Pipeline / Operation Guide -> Prompt Library` 순서로 둔다. `Work Pipeline`은 실제 작업 순서와 단계별 완료 기준을 정리하는 작업 흐름 문서, `Operation Guide`는 Pipeline 수행 중 적용할 내부 운영지침, `Prompt Library`는 실제 작업 요청에 적용할 규칙 모음으로 사용한다.

---

## 2. 프로젝트 개요

본 프로젝트의 이름은 `Project Stella`다.

`Project Stella`는 원작명 `Stella Blade`와 구분되는 포트폴리오 프로젝트명이다. 프로젝트의 설명명은 `Stella Blade Action System Analysis & Implementation Portfolio`로 둔다.

```yaml
프로젝트명
-> Project Stella

설명
-> Stella Blade 액션 시스템 분석 및 구현 포트폴리오

프로젝트 기간
-> 2025.12.01 ~ 2026.06 말

프로젝트 목적
-> 게임 클라이언트 부문 신입 취업을 위한 Unreal Engine 기반 포트폴리오

장르 목표
-> 소울라이크
-> 스타일리시 액션

기술 기준
-> Unreal Engine 5.4
-> C++
-> Visual Studio 2022
-> Windows PC
-> Git / GitHub
-> SourceTree
-> Obsidian
```

Project Stella는 Stella Blade의 액션 시스템을 분석하고, 포트폴리오 범위에서 설명 가능한 핵심 전투 구조를 구현하는 프로젝트다.

```yaml
기술 구현 목표
-> 정교한 전투 공방
-> 가드 / 패링
-> 닷지 / 퍼펙트 닷지
-> 브랜치 콤보 시스템
-> 처형 / 카운터
-> 스킬 시스템
-> 블링크 / 임펄스
```

---

## 3. AI 협업 시스템 목표

AI 협업 시스템의 목표는 사용자의 아이디어를 목표 해석, 구조 탐색, 계획, 구현, 검증, 기록, 문서화로 이어지는 통제 가능한 흐름으로 만드는 것이다.

```yaml
AI 협업 시스템 목표
-> 사용자 의도를 기술적 언어로 해석
-> 구현 가능한 작업 단위로 분리
-> 책임 경계와 실행 흐름을 먼저 정리
-> 구현 전 계획과 비범위를 공유
-> 구현 후 검증과 미검증 항목을 분리
-> History와 Document로 판단 근거를 남김
-> Prompt를 통해 반복 가능한 작업 규칙으로 정리
```

---

## 4. 협업 개념 정의

AI 협업에서 `기획 / 기술 구현 / QA / 문서화`는 작업 단계다.

역할은 책임을 가진 주체 또는 운영 산출물을 기준으로 구분한다. 단계는 작업이 진행되는 흐름을 의미한다. 이 둘을 분리해야 AI가 어느 단계에서 어떤 책임을 수행해야 하는지 명확해진다.

---

## 5. 협업 주체와 운영 산출물의 책임

협업 주체와 운영 산출물의 책임은 다음처럼 구분한다.

```yaml
사용자
-> 프로젝트 목표와 우선순위 결정
-> 최종 의사결정
-> Editor / Asset / 플레이 감각처럼 직접 확인이 필요한 항목 검증

Codex
-> 코드와 문서 탐색
-> 사용자 의도의 기술적 해석
-> 구현 계획 제안
-> 코드 / 문서 수정
-> 가능한 범위의 검증 수행
-> 미검증 항목 명시

Prompt
-> 반복 적용할 작업 규칙 제공
-> 작업 방식, 검증 방식, 문서화 기준을 일관되게 유지

History
-> 질문, 판단, 변경 이유, 시행착오의 증거 제공
-> 최종 문서에 필요한 판단 근거 제공

Document
-> 작업 결과와 판단 근거를 공식 산출물로 정리
-> Work Checklist, System Architecture, Bug Report, Verification Log, PR Document, Technical Document 등으로 분리

Work Checklist
-> 현재 작업 단위의 목표, 범위, 완료 기준, 검증 상태, 후속 작업 범위 관리
```

Codex는 사용자의 목표와 프로젝트 규칙을 기준으로 탐색, 제안, 구현, 검증을 수행하는 작업 보조자다.

---

## 6. 작업 단계

AI 협업 기반 작업은 다음 단계로 진행한다.

```text
1. 기획
2. 기술 구현
3. QA
4. 문서화
```

### 1) 기획

사용자 아이디어를 구현 가능한 기술 목표로 해석하는 단계다.

```yaml
기획 흐름
-> 사용자 아이디어 / 목표 전달
-> 기술적 언어로 해석
-> 구현 단계 분리
-> 우선순위와 비범위 결정
-> 필요한 경우 Work Checklist 작성
```

### 2) 기술 구현

확정된 작업 단위를 코드와 구조로 구현하는 단계다.

```yaml
기술 구현 흐름
-> 관련 코드 / 문서 / 타입 / Asset 연결 지점 탐색
-> 구조와 책임 경계 제안
-> 사용자와 접근 방식 결정
-> 최소 연결 구조 구현
-> API / 데이터 구조 / 정책 구체화
-> 기능 구현
-> 구조 정리와 후속 보강 항목 분리
```

### 3) QA

구현 결과가 실제로 기대한 흐름을 만족하는지 확인하는 단계다.

```yaml
QA 흐름
-> Build 검증
-> Code Flow 확인
-> PIE 검증
-> Editor 검증
-> Asset 검증
-> 실패 원인과 다음 조치 기록
-> 미검증 항목 분리
```

### 4) 문서화

작업 과정과 결과를 목적에 맞는 공식 산출물로 정리하는 단계다.

```yaml
문서화 흐름
-> Work Checklist 갱신
-> 필요 시 System Architecture / Bug Report / Verification Log 작성
-> PR Document로 Branch 결과와 검증 상태 정리
-> 여러 작업 기록은 Technical Document로 압축
-> Prompt 개선 필요 항목은 Prompt Library 후속 작업으로 분리
```

---

## 7. 단계별 입력 / 출력 / 완료 기준

각 단계는 입력과 출력이 명확해야 다음 단계로 넘어갈 수 있다.

```yaml
기획
입력 -> 사용자 아이디어, 참고 기능, 목표 설명
출력 -> 작업 목표, 기술적 해석, 구현 단위, 비범위
완료 기준 -> 무엇을 구현하고 무엇을 제외할지 설명 가능

기술 구현
입력 -> 확정된 작업 목표, 관련 코드 / 문서, 구조 제안
출력 -> 코드 변경, 구조 변경, 후속 보강 항목
완료 기준 -> 핵심 실행 흐름이 코드에서 연결됨

QA
입력 -> 구현 결과, 예상 동작, 검증 기준
출력 -> 검증 결과, 실패 원인, 미검증 항목
완료 기준 -> 완료 / 실패 / 미검증 상태가 구분됨

문서화
입력 -> 작업 목표, 변경 결과, 검증 결과, History
출력 -> Work Checklist, PR Document, 기타 필요 문서
완료 기준 -> 작업 결과와 판단 근거를 추적 가능
```

---

## 8. Prompt / History / Document 관계

Prompt, History, Document는 서로 다른 역할을 가진다.

```yaml
Prompt
-> 작업 전에 AI에게 적용할 규칙
-> 반복 가능한 협업 방식의 입력

History
-> 작업 중 실제로 오간 질문과 판단의 기록
-> 결정이 어떻게 바뀌었는지 증명하는 근거

Document
-> 작업 후 정리된 공식 산출물
-> History를 그대로 복사하지 않고 목적에 맞게 압축
```

Prompt는 작업 전 규칙을 제공하고, History는 작업 중 판단 과정을 증명하며, Document는 작업 후 결과와 판단 근거를 공식 산출물로 정리한다.

---

## 9. 산출물 운영 기준

모든 작업을 같은 밀도로 문서화하지 않는다. 산출물은 목적에 따라 분리한다.

```yaml
Work Checklist
-> 현재 작업 단위의 목표, 범위, 완료 기준, 검증 상태 관리

System Architecture
-> 구조 변경, 책임 경계, 실행 흐름, 데이터 계약 정리

Bug Report
-> 재현 가능한 문제, 원인, 수정, 검증 기록

Verification Log
-> 실제 수행한 검증과 미검증 항목 기록

PR Document
-> Branch 결과, 변경 요약, 검증 상태, 후속 작업 범위 정리

Technical Document
-> 여러 작업 기록을 포트폴리오 제출용 기술 주제로 압축

AI Collaboration Index
-> AI Collaboration 문서의 위치, 역할, 상태를 확인하는 목차

Prompt Library Maintenance Guide
-> Prompt Library의 구조, 이름, 상태, 중복, Archive 관리 기준
```

---

## 10. 후속 구체화 항목

이 문서는 AI 협업 시스템의 압축 개요이자 상위 기획서다. 실제 작업 흐름, 운영 규칙, 프롬프트는 다음 문서에서 구체화한다.

```yaml
후속 구체화 항목
-> Work Pipeline이 Project Plan의 작업 단계를 작업 흐름으로 구체화하는지 재검토
-> Operation Guide가 Project Plan의 운영 원칙을 내부 규칙으로 구체화하는지 재검토
-> AI Collaboration Index가 전체 문서의 위치 / 역할 / 상태를 명확히 보여주는지 재검토
-> Prompt Library Maintenance Guide가 Prompt Library 유지보수 기준을 충분히 분리하는지 재검토
-> 작업 기록 단위를 Work Checklist 기준으로 구체화
-> 산출물별 작성 타이밍과 완료 기준 구체화
```

