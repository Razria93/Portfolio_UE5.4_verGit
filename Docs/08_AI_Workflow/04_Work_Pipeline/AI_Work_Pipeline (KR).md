# AI Work Pipeline

## 1. 목적

본 문서는 `Project Stella`에서 Codex와 함께 작업을 진행할 때 사용할 작업 흐름 문서다.

이 문서는 실제 작업을 어떤 순서로 전개하고, 각 단계에서 어떤 입력을 받아 어떤 산출값을 만들지 자체 기준으로 정리한다.

`../03_Operation/AI_Workflow_Operation_Guide (KR).md`는 Pipeline을 운용하기 위한 내부 운영지침이며, `../01_Overview/AI_Workflow_Overview (KR).md`는 AI 기반 작업 운영 체계의 상위 개요 문서다. 본 문서는 실제 작업 흐름에 필요한 단계, 입력, 출력, 완료 기준을 독립적으로 포함한다.

---

## 2. 적용 범위

이 Pipeline은 UE5.4 C++ 전투 시스템 구현, 리팩터링, 검증, 문서화 작업에 적용한다.

`Project Stella`는 `Stella Blade`의 액션 시스템을 분석하고, 포트폴리오 범위에서 설명 가능한 핵심 전투 구조를 구현하는 프로젝트다.

Pipeline은 작업을 한 번에 끝내는 절차가 아니라, 필요한 단계만 선택해 순차적으로 진행할 수 있는 공정이다. 사용자는 단계를 수동으로 끊어 요청할 수 있고, Codex는 입력과 완료 기준이 충분할 때 여러 단계를 이어서 진행할 수 있다.

---

## 3. Pipeline 체크포인트

Pipeline은 단계 진행 전에 다음 네 가지 체크포인트를 확인한다.

```yaml
Pipeline 체크포인트
-> 작업 목표: 무엇을 끝내야 하는지, 완료 기준과 비범위가 무엇인지 확인
-> 변경 위험: 코드 / 문서 / Asset / Blueprint / Git 변경 영향 확인
-> 검증 필요성: Build / Code Flow / PIE / Editor / Asset 검증 필요 여부 확인
-> 문서화 필요성: Work List / Bug Report / System Architecture / System Design Records / Engine Technique Document / Engine Implementation Records / Verification Log / PR Document / Portfolio Document 반영 필요 여부 확인
```

이 네 가지는 AI 기반 작업 운영 체계에서 Codex의 작업을 통제하기 위한 핵심 기준이다. 상세 판단 기준은 `../03_Operation/AI_Workflow_Operation_Guide (KR).md`의 `핵심 통제 기준`을 따른다.

---

## 4. 공통 입력 / 최종 산출값

Pipeline의 공통 입력과 최종 산출값은 다음과 같다.

```yaml
공통 입력
-> 사용자 요청
-> 참고 기능 또는 참고 문서
-> 현재 코드 / 문서 / Asset / Blueprint 맥락
-> 기대 결과
-> 우려 지점

최종 산출값
-> 변경 결과
-> 검증 상태
-> 미검증 항목
-> 문서화 결과
-> 후속 작업 범위
```

공통 입력이 모두 필요한 것은 아니다. 다만 작업 목표, 기대 결과, 확인해야 할 위험이 불명확하면 다음 단계로 넘어가기 전에 요약하거나 질문한다.

---

## 5. 전체 Pipeline 흐름

Pipeline은 다음 8단계로 구성한다.

```text
1. 목표 확인
2. 현재 구조 탐색
3. 아이디어 계획
4. 구조 제안
5. 적용 및 수정
6. 검증 및 안정화
7. 병합
8. 문서화 및 관리
```

각 단계는 이전 단계의 출력값을 다음 단계의 입력값으로 사용한다.

Work Brief Intake Prompt는 목표 확인 단계에서 사용자 요청, Codex 해석, 범위, 위험, 미결정 항목을 정리할 때 사용한다.

Feature Work Planning Prompt와 Refactor Work Planning Prompt는 아이디어 계획 / 구조 제안 단계에서 나온 내용을 구현 단위, 변경 단위, 비범위, 위험, 검증 기준으로 정리해 적용 및 수정 단계의 입력으로 넘길 때 사용한다.

Prompt 호출 흐름과 작업 유형별 라우팅 기준은 `../05_Prompt_Library/00_Prompt_Blueprint/05_Prompt_Flow_and_Routing_Blueprint (KR).md`에서 관리한다.

---

## 6. 단계 전환 기준

```yaml
단계 전환 기준
-> 필수 입력이 없으면 다음 단계로 넘어가지 않고 사용자에게 요청
-> 이전 단계 출력이 불명확하면 요약 후 확인
-> Codex가 탐색으로 확인 가능한 정보는 먼저 확인
-> 사용자 의사결정이 필요한 항목은 다음 선택지로 분리
-> 검증 또는 문서화가 필요한 항목은 후속 단계 입력으로 넘김
```

단계 전환은 자동 진행과 수동 진행을 모두 허용한다. 자동 진행은 입력과 완료 기준이 충분할 때만 사용하고, 범위 확대나 책임 경계 변경이 보이면 사용자 확인을 우선한다.

---

## 7. Pipeline 단계

### 1) 목표 확인

```yaml
목적
-> 사용자가 달성하려는 목표와 해석을 구현 가능한 작업 목표로 정리

입력
-> 사용자 아이디어
-> 참고 기능
-> 기대 결과
-> 우려 지점

출력
-> 작업 목표
-> 성공 기준
-> 비범위 / 후속 작업 범위
-> Work List 생성 / 갱신 필요 여부

완료 기준
-> 무엇을 바꾸려는지 설명 가능
-> 완료 여부를 판단할 기준이 있음
-> 하지 않을 범위가 분리됨

입력 부족 시 처리
-> 기대 결과가 불명확하면 목표 예시를 요청
-> 범위가 넓으면 Work List 단위로 분리
-> 구현 여부보다 분석이 먼저 필요하면 현재 구조 탐색으로 이동
```

### 2) 현재 구조 탐색

```yaml
목적
-> 현재 코드, 문서, Asset 연결 지점을 확인하고 기준 시점을 구분

입력
-> 작업 목표
-> 관련 파일 / 타입 / 문서

출력
-> 관련 코드 / 문서 / Asset 연결 지점 목록
-> 현재 구조 요약
-> 문서와 코드의 기준 시점 차이

완료 기준
-> 현재 구조가 어떤 책임 경계로 동작하는지 설명 가능
-> 확인한 근거와 추정이 분리됨

입력 부족 시 처리
-> 관련 파일을 모르면 검색으로 후보를 찾음
-> 문서와 코드가 충돌하면 현재 코드 기준과 문서 기준을 분리
-> Asset / Editor 확인이 필요하면 미검증 항목으로 넘김
```

### 3) 아이디어 계획

```yaml
목적
-> 가능한 접근을 비교하고 추천 방향과 trade-off를 정리

입력
-> 작업 목표
-> 현재 구조
-> 제약 조건

출력
-> 접근 후보
-> 추천안
-> trade-off
-> 사용자 결정이 필요한 항목

완료 기준
-> 왜 이 방향을 택하는지 설명 가능
-> 더 단순한 접근과 구조적 접근의 차이를 비교함

입력 부족 시 처리
-> 제약 조건이 불명확하면 선택지로 분리
-> 사용자 결정이 필요한 trade-off는 구현 전에 질문
-> 단순 수정과 구조 변경이 갈리면 Plan Mode 대상으로 분리
```

### 4) 구조 제안

```yaml
목적
-> 책임 경계, 데이터 구조, 정책 결정, 외부 연결, 디버깅 방식을 설계

입력
-> 추천 접근
-> 현재 구조
-> 책임 경계 후보

출력
-> 책임 경계
-> 구조적 흐름
-> 데이터 구조 / 구현 알고리즘
-> 정책 결정
-> 외부 요소 연결 방식
-> 디버깅 / 검증 방식

완료 기준
-> 변경 전후 책임이 비교 가능
-> 적용 범위와 제외 범위가 구분됨
-> 검증 기준이 구조와 연결됨

입력 부족 시 처리
-> 책임 주체가 불명확하면 Actor / Component / Subsystem / UObject / Asset / Blueprint 기준으로 다시 분리
-> 데이터 흐름이 불명확하면 Payload / Context / Result 또는 현재 프로젝트의 동등한 흐름으로 정리
-> 검증 기준이 없으면 검증 및 안정화 단계의 입력으로 넘김
```

### 5) 적용 및 수정

```yaml
목적
-> 확정된 계획에 따라 필요한 범위만 수정

입력
-> 확정된 계획
-> 수정 대상 파일
-> 검증 기준

출력
-> 변경 파일
-> 변경 이유
-> 직접 수정한 범위
-> 건드리지 않은 범위

완료 기준
-> 모든 변경이 목표와 직접 연결됨
-> 사용자가 만든 unrelated 변경을 되돌리지 않음
-> 임시 대응과 구조적 해결이 구분됨

입력 부족 시 처리
-> 수정 범위가 불명확하면 구현 전 계획으로 되돌림
-> 사용자 변경과 겹치면 변경 의도를 확인
-> 검증 기준이 없으면 최소 Build / Code Flow 확인 기준을 먼저 정의
```

### 6) 검증 및 안정화

```yaml
목적
-> 변경이 계획한 책임과 흐름을 실제로 만족하는지 확인

입력
-> 변경 결과
-> 예상 동작
-> 검증 기준

출력
-> Build 결과
-> Code Flow 확인
-> PIE / Editor / Asset 검증 결과 또는 미검증 항목
-> Code Review / Verification Log 작성 필요 여부

완료 기준
-> 수행한 검증과 수행하지 못한 검증이 분리됨
-> 실패한 검증은 원인과 다음 조치가 기록됨

입력 부족 시 처리
-> 실행 환경이 없으면 Code Flow 확인과 미검증 항목으로 분리
-> Asset / Editor 확인이 필요하면 사용자 확인 항목으로 분리
-> 실패 로그가 있으면 원인 분석 후 수정 또는 후속 작업 범위로 분리
```

### 7) 병합

```yaml
목적
-> Commit / PR로 올릴 변경 범위와 검증 상태를 정리

입력
-> 변경 파일
-> 검증 결과
-> 문서화 상태

출력
-> 변경 파일 그룹
-> staged 대상 / 제외 대상
-> Commit 메시지 후보
-> PR 전 점검 결과

완료 기준
-> 관련 없는 변경이 Commit에 섞이지 않음
-> 문서 / 검증 / 미검증 항목이 PR에 연결됨

입력 부족 시 처리
-> staged / unstaged 상태가 불명확하면 git status 기준으로 재점검
-> 검증 상태가 부족하면 Commit 보류 또는 미검증 명시
-> 관련 없는 변경이 섞이면 Commit 범위를 분리
```

### 8) 문서화 및 관리

```yaml
목적
-> 작업 흐름과 판단 근거를 포트폴리오 산출물로 남김

입력
-> 작업 목표
-> 변경 결과
-> 검증 결과
-> History

출력
-> Work List
-> Bug Report
-> System Architecture
-> System Design Records
-> Engine Technique Document
-> Engine Implementation Records
-> Verification Log
-> PR Document
-> Portfolio Document
-> AI Workflow Index / Prompt Format Blueprint / Prompt Library Maintenance Blueprint / Prompt Files / Operation Guide
-> Milestone / Roadmap

완료 기준
-> 작업 이유, 변경 범위, 검증 결과, 후속 작업 범위가 추적 가능
-> History가 문서와 PR로 압축됨

입력 부족 시 처리
-> History가 부족하면 실제 변경과 검증 결과를 기준으로 최소 요약
-> 구조 맥락이 필요하면 System Architecture를 우선 참조
-> 엔진 기능 / API 사용 맥락이 필요하면 Engine Technique Document 또는 관련 코드 / Unreal 문서를 우선 참조
-> Prompt 개선 항목은 Prompt Library 후속 작업으로 분리
```

```yaml
산출물 선택 기준
-> Work List: 작업 목표 / 범위 / 완료 기준 / 검증 상태 관리가 필요할 때
-> Bug Report: 재현 가능한 문제와 원인 / 수정 / 검증을 남길 때
-> System Architecture: 현재 시스템 구조 / 책임 경계 / 실행 흐름 / 데이터 계약을 설명할 때
-> System Design Records: 시스템 구조 설계 결정이나 구조 문제 / 책임 경계 위험을 기록할 때
-> Engine Technique Document: Unreal Engine 기능 / API / 시스템 사용 방식을 설명할 때
-> Engine Implementation Records: Unreal Engine 기능 사용 결정이나 엔진 동작 / 설정 / API 이슈를 기록할 때
-> Verification Log: 수행한 검증과 미검증 항목을 분리 기록할 때
-> PR Document: Branch 결과와 검증 상태를 제출할 때
-> Portfolio Document: 여러 작업 기록을 제출용 기술 주제로 압축할 때
-> AI Workflow Index: AI Workflow 문서의 위치 / 역할 / 상태를 갱신할 때
-> Prompt Format Blueprint / Prompt Library Maintenance Blueprint / Prompt Files: 반복 가능한 작업 규칙 또는 Prompt Library 관리 기준을 개선할 때
```

---

## 8. 단계별 참조 문서

```yaml
운영 규칙
-> ../03_Operation/AI_Workflow_Operation_Guide (KR).md

작업 범위 / 완료 기준
-> Work List

버그 기록
-> Bug Report

기술적 의도 / 구조 맥락
-> System Architecture

시스템 설계 결정 / 구조 문제
-> System Design Records

엔진 기능 / API 사용 맥락
-> Engine Technique Document

엔진 사용 결정 / 엔진 이슈
-> Engine Implementation Records

작업 판단 변화 / 맥락
-> History

반복 작업 규칙
-> Prompt Format Blueprint / Prompt Library Maintenance Blueprint / Prompt Files

검증 결과
-> Verification Log

Branch 결과
-> PR Document

포트폴리오 제출용 정리
-> Portfolio Document
```

History 문서가 아직 정리되지 않은 경우, 현재 시스템 구조와 책임 경계는 `System Architecture`를 우선 참조하고 엔진 기능 / API 사용 방식은 `Engine Technique Document` 또는 관련 코드 / Unreal 문서를 우선 참조한다. 작업 결정 / 범위 / 검증 상태는 `Work List` 또는 `PR Document`에 남긴다.

Pipeline은 각 산출물을 언제 호출하고 어떤 완료 기준으로 다음 단계로 넘길지 결정하는 작업 흐름 문서다.
