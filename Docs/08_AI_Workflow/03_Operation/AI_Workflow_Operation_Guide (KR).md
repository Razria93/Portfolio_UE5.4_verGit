# AI Workflow Operation Guide

## 1. 목적

본 문서는 `Project Stella`에서 Codex와 협업할 때 적용할 내부 운영지침이다.

이 문서는 `../04_Work_Pipeline/AI_Work_Pipeline (KR).md`를 운용할 때 지켜야 할 원칙, 책임 경계, 판단 기준, 산출물 사용 기준, 검증, 기록, Commit / PR 기준을 자체 기준으로 정리한다.

작업 순서는 Work Pipeline에서 관리하고, 본 문서는 Pipeline 각 단계에서 어떤 기준으로 판단하고 진행할지 정의한다.

`../01_Overview/AI_Workflow_Overview (KR).md`는 AI 기반 작업 운영 체계의 상위 개요 문서이며, 본 문서는 실제 운영에 필요한 기준을 독립적으로 포함한다.

---

## 2. 적용 범위

이 운영지침은 `Project Stella`의 UE5.4 C++ 전투 시스템 구현, 리팩터링, 검증, 문서화 작업에 적용한다.

`Project Stella`는 `Stella Blade`의 액션 시스템을 분석하고, 포트폴리오 범위에서 설명 가능한 핵심 전투 구조를 구현하는 프로젝트다. Codex와의 협업은 목표, 책임 경계, 변경 위험, 검증 상태, 문서화 근거를 통제하는 방식으로 운영한다.

---

## 3. 운영 원칙

```yaml
운영 원칙
-> 목표와 완료 기준을 먼저 고정
-> 책임 경계를 먼저 판단
-> 구현 범위와 비범위를 분리
-> 구현과 함께 검증 가능성을 판단
-> 확인하지 못한 항목은 미검증으로 남김
-> 사용자 결정이 필요한 항목은 다음 선택지로 분리
-> 관련 없는 사용자 변경은 되돌리지 않음
```

AI 기반 작업 운영은 구현 속도보다 판단의 명확성, 변경 범위 통제, 검증 가능성을 우선한다.

---

## 4. 핵심 통제 기준

Pipeline을 운용할 때 Codex는 요청된 작업을 다음 네 가지 기준으로 먼저 분석한다.

```yaml
작업 목표
-> 무엇을 끝내야 하는지 확인
-> 완료 기준, 범위, 비범위 판단
-> Work List 생성 / 갱신 필요 여부 판단

변경 위험
-> 코드 / 문서 / Asset / Blueprint / Git 변경 영향 판단
-> 책임 경계, public API / struct / enum, montage lifecycle, AI behavior 영향 판단
-> 계획 수립, 리뷰 강도, Commit 분리 필요성 판단

검증 필요성
-> Build / Code Flow / PIE / Editor / Asset 검증 필요 여부 판단
-> 실패 원인과 미검증 항목 분리
-> 사용자 Editor 확인이 필요한 항목 분리

문서화 필요성
-> Work List / Bug Report / System Architecture / System Design Records / Engine Technique Document / Engine Implementation Records / Verification Log / PR Document / Portfolio Technical Document 반영 필요 여부 판단
-> History와 공식 산출물 연결 필요성 판단
-> Portfolio Technical Document 또는 Prompt Library 후속 반영 필요 여부 판단
```

이 기준은 작업을 제한하기 위한 절차가 아니라, Codex가 목표를 오해하거나 검증되지 않은 결과를 완료로 표현하지 않게 하기 위한 최소 체크 기준이다.

---

## 5. 사용자 / Codex 책임 경계

```yaml
사용자
-> 프로젝트 목표와 우선순위 결정
-> 최종 의사결정
-> Editor / Asset / 플레이 감각처럼 직접 확인이 필요한 항목 검증
-> 선택지 중 진행 방향 결정

Codex
-> 코드와 문서 탐색
-> 사용자 의도의 기술적 해석
-> 구현 계획 제안
-> 코드 / 문서 수정
-> 가능한 범위의 검증 수행
-> 미검증 항목 명시
-> 사용자 결정이 필요한 항목 분리
```

Codex는 독립 의사결정자가 아니라 작업 보조자다. 사용자의 목표를 기술적 작업 단위로 해석하고, 확인 가능한 정보는 먼저 탐색하며, 결정이 필요한 항목은 계획 또는 다음 선택지로 분리한다.

---

## 6. 산출물 정의와 사용 기준

Pipeline의 입력과 출력에 사용되는 주요 산출물은 다음 기준으로 사용한다.

```yaml
Prompt Library
-> 전체 프롬프트 체계
-> Prompt Blueprint와 Prompt Files를 포함하는 상위 개념

AI Workflow Index
-> AI Workflow 전체 문서의 위치, 역할, 상태를 확인하는 목차 문서

Prompt Format Blueprint
-> Prompt 양식과 섹션 구조 기준 문서

Prompt Engineering Blueprint
-> Prompt 내부 요청 내용 설계 기준 문서

Prompt Custom Blueprint
-> Project Stella / AI Workflow 전용 Prompt 적용 기준 문서

Prompt Library Maintenance Blueprint
-> Prompt Library의 폴더, 파일명, 상태, 중복, Archive 관리 기준 문서

Prompt Files
-> 개별 작업 규칙 / 작업 계획 / 문서 카테고리별 작성 / 보완 / 검증 / Git 운영 Prompt

Prompt
-> 작업 전에 반복 적용할 규칙
-> 작업 방식, 검증 방식, 문서화 기준을 일관되게 유지하기 위한 입력

History
-> 작업 중 질문, 판단, 결정 변화, 시행착오를 증명하는 기록
-> 최종 문서에 필요한 판단 근거

Work List
-> 현재 작업 단위의 목표, 범위, 완료 기준, 검증 상태, 후속 작업 범위 관리

Bug Report
-> 재현 가능한 문제, 원인, 수정, 검증 기록

System Architecture
-> 현재 시스템 구조, 책임 경계, 실행 흐름, 데이터 계약 정리

System Design Records
-> 시스템 구조 설계 결정과 선택 이유 기록
-> 시스템 구조 / 책임 경계 / 설계 흐름에서 발생한 문제와 위험 기록

Engine Technique Document
-> Unreal Engine 기능 / API / 시스템 사용 방식 정리
-> 엔진 기능을 프로젝트에서 어떻게 적용하는지 설명

Engine Implementation Records
-> Unreal Engine 기능 사용 방식에 대한 결정 기록
-> Unreal Engine 동작, 설정, API 사용 중 발생한 기술 이슈 분석

Verification Log
-> 실제 수행한 검증과 미검증 항목 기록
-> Build / Code Flow / PIE / Editor / Asset 검증 구분

PR Document
-> Branch 결과, 변경 요약, 검증 상태, 미검증 항목, 후속 작업 범위 정리

Portfolio Technical Document
-> 여러 작업 기록을 포트폴리오 제출용 기술 주제로 압축

Commit / PR
-> 실제 변경 결과를 확정하고 공유하는 단위
-> 계획과 기록이 코드 / 문서 변경으로 반영되었는지 확인하는 단위
```

Prompt는 규칙, History는 증거, Document는 판단 근거, Verification Log는 품질 보증, Commit / PR은 실제 결과로 취급한다.

---

## 7. Codex 기능 사용 기준

Codex 기능 용어는 다음처럼 구분한다.

```yaml
IDE Context
-> 현재 코드 / 문서 / 파일 상태를 대화에 제공하는 기능

Plan Mode
-> 구현 전 목표, 범위, 책임, 검증 기준을 결정하는 계획 모드

Goal
-> 여러 턴에 걸친 장기 작업을 추적하는 기능

Default 작업
-> 계획 확정 뒤 실제 수정 / 검증 / 문서화를 수행하는 기본 작업 흐름
```

```yaml
IDE Context
-> 현재 코드 / 문서 / 파일 상태를 빠르게 제공할 때 사용
-> 특정 파일, 타입, 함수, 문서 흐름을 확인할 때 유효
-> 실제 파일 검색 / 빌드 / 검증과 함께 사용

Plan Mode
-> 구현 전 목표, 범위, 책임 경계, 검증 기준을 확정할 때 사용
-> 대안과 trade-off가 있거나 사용자 결정이 필요한 경우 우선 사용
-> final plan은 구현자가 바로 실행할 수 있을 만큼 decision complete해야 함

Goal
-> 장기 리팩터링, 여러 턴에 걸친 구조 변경, 대규모 문서 정리처럼 지속 추적이 필요한 작업에 사용
-> 단순 버그 수정이나 단일 문서 작성에는 기본 사용하지 않음

Default 작업
-> 계획이 확정된 뒤 구현 / 문서 작성 / 검증을 수행할 때 사용
-> 작업 중 새 위험이나 범위 확대가 보이면 다시 계획으로 되돌림
```

Plan Mode는 책임 경계 변경, 여러 파일 영향, 검증 기준 불명확, 사용자 trade-off 결정, 리팩터링 단위 분리, 현재 구조와 목표 구조 사이의 해석 차이가 있을 때 우선 사용한다.

Work Brief Intake Prompt는 작업 시작 전 사용자 요청, Codex 해석, 범위, 위험, 미결정 항목, 준비 상태를 정리할 때 사용한다.

Feature Work Planning Prompt는 새 기능 구현 작업을 구현 단위, 실행 순서, 위험, 검증 기준으로 정리할 때 사용한다.

Refactor Work Planning Prompt는 구조 변경 / 리팩터링 중심 작업을 변경 단위, 책임 경계, 위험, 검증 기준으로 정리할 때 사용한다.

Goal은 여러 세션에 걸칠 가능성이 있고, 단계별 완료 기준이나 중간 산출물 추적이 필요한 경우에만 사용한다.

---

## 8. History 기록 기준

History는 판단을 증명할 수 있는 형태로 압축한다.

```yaml
남길 것
-> 목표가 바뀐 이유
-> 선택한 접근과 버린 접근
-> 책임 경계 판단
-> trade-off
-> 사용자 결정
-> 검증 실패와 수정
-> 후속 작업 범위

버릴 것
-> 단순 진행 보고
-> 반복된 설명
-> 임시 검색 과정의 모든 세부 로그
-> 최종 판단에 영향을 주지 않은 아이디어
```

```yaml
History 연결 기준
작업 범위 결정 -> Work List
검증 실패 -> Bug Report / Verification Log
현재 시스템 구조 -> System Architecture
시스템 구조 판단 변화 -> System Design Records
엔진 기능 사용 방식 -> Engine Technique Document
엔진 사용 판단 변화 -> Engine Implementation Records
구현 결과 -> PR Document
포트폴리오 요약 -> Portfolio Technical Document
반복 가능한 규칙 -> Prompt Format Blueprint / Prompt Library Maintenance Blueprint / Prompt Files / Operation Guide
```

History 문서가 아직 정리되지 않은 경우, 현재 시스템 구조와 책임 경계는 System Architecture를 우선 참조하고, 엔진 기능 사용 방식은 Engine Technique Document 또는 관련 코드 / Unreal 문서를 우선 참조한다. 작업 결정 / 범위 / 검증 상태는 Work List와 PR Document에 남긴다.

---

## 9. 검증과 오류 검출 기준

검증은 작업 완료 조건이다.

```yaml
Build
-> UE C++ build 성공 / 실패 / 미수행 이유

Code Flow
-> 호출 순서, guard path, reject reason, lifecycle, cleanup 확인

PIE
-> 입력, runtime behavior, montage, collision / overlap, AI behavior 확인

Editor
-> Unreal Blueprint compile, parent class, exposed property, redirector 확인

Asset
-> montage notify, DataAsset / DataTable entry, feedback reference, serialized value 확인

미검증
-> 현재 환경에서 확인하지 못했거나 사용자가 Editor에서 확인해야 하는 항목
```

검증을 늘릴 때는 항목을 기계적으로 추가하지 않고 변경 위험과 연결한다. 책임 경계, public API / struct / enum, montage lifecycle, Asset / Blueprint reference, AI behavior / Blackboard, damage / feedback / execution pipeline 영향이 있으면 검증 강도를 높인다.

```yaml
최소 검증 기준
-> 문서 작업: UTF-8 출력 확인과 핵심 키워드 검색
-> 코드 작업: 가능하면 Build와 Code Flow 확인
-> Asset / Blueprint 영향 작업: Editor / Asset 확인 또는 미검증 항목 명시

검증 생략 / 보류 기준
-> 현재 환경에서 PIE / Editor / Asset 확인이 불가능하면 미검증으로 명시
-> 실패 또는 미검증 항목이 있어도 Commit / PR이 필요하면 원인, 영향, 다음 조치를 문서에 남김
-> 검증 부족이 핵심 완료 기준을 흔들면 Commit / PR을 보류
```

---

## 10. 리팩터링 권유 기준

리팩터링은 Codex가 임의로 수행하지 않고, 필요성과 비용을 먼저 설명한 뒤 사용자와 결정한다.

```yaml
리팩터링 권유 순서
1. 문제 인식
2. 현재 구조 근거
3. 리팩터링 필요성 분석
4. 추천안
5. 사용자 결정 필요 여부
6. 일정 / 단계 분리 필요 여부
```

리팩터링 필요성 분석에는 목표, 구현 방식, 안정성, 확장성, 구현 비용, trade-off를 포함한다.

---

## 11. 문서화 / Git / PR 기준

문서화는 작업 단위의 판단과 결과를 추적하기 위한 연결 구조다.

```yaml
Commit 권장 시점
-> Branch 목표의 독립 산출물이 완료됨
-> 한 번에 되돌릴 수 있는 의미 단위가 형성됨
-> 파일 이동 / 폴더 구조 변경이 끝나 이후 diff를 흐릴 수 있음
-> 문서 구조 변경과 내용 변경을 분리할 수 있음
-> 검증 또는 재검토 기준이 통과됨
-> 다음 작업을 진행하면 현재 변경과 섞일 위험이 있음

Commit 보류 시점
-> 핵심 체크 항목이 미완료
-> 경로 이동 후 참조 검증이 끝나지 않음
-> 재검토 예정 문서를 완료처럼 보이게 만들 위험이 있음
-> unrelated 변경과 섞여 있음
```

Commit / PR 전에는 changed files, staged / unstaged / untracked, 관련 문서, 검증 상태, 미검증 항목, unrelated 변경 제외 여부를 확인한다.

Branch는 작업 목표와 구현 범위를 관리하는 버전 컨트롤 단위이고, PR은 작업 결과와 검증 상태를 제출하는 단위이며, Merge는 PR에서 설명한 변경과 검증 상태를 기준으로 결정한다.

---

## 12. 최소 운영 규칙

```yaml
주석 작성 최소 기준
-> 복잡한 책임 경계, UObject lifetime, delegate binding / unbinding, asset 연동 근거처럼 코드만으로 의도가 불명확한 경우에만 작성
-> 날짜나 장문 설명보다 관련 문서 ID, 의도, 비자명한 제약을 짧게 남김
-> 단순 구현 설명이나 변경 이력성 주석은 남기지 않음

네이밍 최소 기준
-> Unreal Engine 보편 용어를 우선 사용
-> 프로젝트 특화 용어가 필요하면 보편 용어와 함께 의미를 설명
-> 같은 개념은 문서 / 코드 / PR에서 같은 표기를 유지
-> 사용자가 납득하기 쉬운 프로젝트 용어라도 책임 경계가 흐려지면 대안을 제안

파일 / 폴더 관리 최소 기준
-> active 문서, reference, archive, ignore, prompt 역할을 분리
-> 현재 Branch Commit 대상이 아닌 실험 / 초안은 Commit 범위에서 제외
-> ignore 폴더는 Git에 남길 필요가 없는 자료에만 사용
```

---

## 13. 다음 선택지 기준

응답 마지막의 추천은 현재 대화 흐름만 따르지 않고 사용자가 다음 행동을 선택할 수 있게 제시한다.

```yaml
이어서 진행
-> 현재 흐름에서 가장 자연스러운 다음 작업

분기 작업
-> 같은 Branch 안에서 이어갈 수 있는 대안 작업

작업 전환 제안
-> 현재 Work List 범위를 벗어나 새 Work List가 필요한 후속 작업

점검 / 검토
-> 불확실성, 중복, 누락, stale 여부 확인

마감 작업
-> Commit, PR, 문서화, Branch 종료
```

여러 선택지가 경쟁하면 `필수 / 우선 / 권장 / 선택 / 보류 / 비권장` 우선순위와 `반복 점검 / 안정화 전 주의 / 문서화 시점 / Commit 시점 / 작업 전환 / 검증 필요` 상태를 필요한 만큼 표시한다.
