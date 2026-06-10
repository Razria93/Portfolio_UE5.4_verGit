# Prompt Flow and Routing Blueprint

## 1. 목적

본 문서는 자연어 요청이 어떤 Prompt 계층을 거쳐 처리되는지와 `Work Brief Intake`가 어떤 기준으로 다음 Prompt를 선택하는지 정리한다.

`AI Work Pipeline`은 전체 작업 공정을 정의하고, 이 문서는 그 공정 안에서 Prompt Files를 어떤 순서와 계층으로 호출할지 설명한다.

---

## 2. 기본 흐름

```text
자연어 요청
-> Work Brief Intake
-> 작업 유형 판정
-> 필요한 Planning Prompt 선택
-> Work List Writing 후보 판단
-> Planning 결과 기준으로 필요 시 Work List Writing
-> 작업 수행
-> 필요 시 문서화 계층
-> 필요 시 검증 계층
-> 필요 시 Commit / PR 계층
```

위 흐름은 모든 Prompt를 항상 호출한다는 뜻이 아니다.

작업 성격에 따라 필요한 Prompt만 선택하되, 선택된 계층은 기본적으로 `구현 -> 문서화 / 검증 -> Commit / PR` 순서를 따른다.

Work Brief 단계에서는 Work List Writing 필요성을 후보로 표시할 수 있다.

최종 작성 여부는 Planning 결과에서 작업 단위, 완료 기준, 검증 기준, PR 가능 조건이 정리된 뒤 판단한다.

Work List를 작성하기로 한 Branch 단위 작업은 실행 전에 Work List를 먼저 만든다.

실행 이후에는 새 Work List를 뒤늦게 만드는 것이 아니라, 기존 Work List의 완료 상태 / 검증 상태 / 후속 범위를 업데이트한다.

---

## 3. Prompt 계층 역할

```yaml
입력 / 라우팅 계층
-> Work Brief Intake Prompt
-> 자연어 요청을 받아 현재 합의된 Work Brief 값으로 작성 / 갱신
-> 작업 유형, 준비 상태, 다음 Prompt 후보를 판정

계획 계층
-> Feature Work Planning Prompt
-> Refactor Work Planning Prompt
-> 후속 후보: Update Work Planning Prompt
-> 후속 후보: Review / Verification Planning Prompt
-> 후속 후보: Document Writing Planning Prompt
-> 작업을 실행 가능한 단위, 순서, 위험, 검증 기준으로 분해

계획 시각화 / 관리 계층
-> Work List Writing Prompt
-> Planning 결과를 진행 관리, 완료 기준, PR 가능 조건 확인용 문서로 정리

실행 계층
-> 구현 계층
-> 문서화 계층
-> 검증 계층
-> Commit / PR 계층

구현 계층
-> 실제 코드 / Blueprint / Asset / Editor 변경 수행
-> v1 기준 별도 구현 Prompt 없음
-> Codex가 Work Planning 결과와 Working Rule / Reference를 바탕으로 수행

문서화 계층
-> Document Writing Prompt
-> PR Document Writing Prompt
-> System Architecture / Design Records / Engine Records / Portfolio Technical Document 작성

검증 계층
-> Code Review Prompt
-> Verification Log Prompt
-> Asset Blueprint Validation Prompt
-> Document Format Normalization Prompt
-> Document Set Audit Prompt

Commit / PR 계층
-> Git Commit PR Preflight Prompt

참조 계층
-> Working Rule Prompt
-> Working Reference Prompt
-> 입력 / 라우팅, 계획, 실행, 검증 전반에서 실행 규칙, 책임 경계, 기술 기준, 프로젝트 구조 판단 기준으로 참조
```

---

## 4. Work Brief Intake 역할

`Work Brief Intake Prompt`는 Prompt Flow의 진입 라우터다.

```yaml
역할
-> 자연어 요청 정리
-> Work Brief 값 작성 / 갱신
-> 사용자 의도와 현재 컨텍스트 해석
-> 작업 유형 판정
-> 준비 상태 판정
-> 부족한 입력을 계획차단 / 검토필요, 비차단 / 검토필요, 계획차단 / 선택필요, 비차단 / 선택필요로 분리
-> Planning Prompt 후보 제시
-> 실행 Prompt 후보 제시
-> Work List Writing 후보와 최종 판단 위치 제시
```

준비 상태가 `진행 불가` 또는 `검토 필요`이면 세부 Prompt로 넘어가지 않고 Work Brief를 보완한다.

`Work Brief Intake`는 별도 입력 양식을 강제하지 않는다.

사용자는 채팅에 자연어로 요청하고, Codex는 Work Brief 문서에 현재 합의된 작업 개요와 남은 판단 항목만 반영한다.

Work Brief에는 원문 보존 섹션과 해석 섹션을 병렬로 두지 않는다.

---

## 5. Work Brief 운영 루프

Work Brief는 사용자 자연어 요청을 현재 합의된 작업 개요로 정리하고, Planning으로 넘어가기 전 남은 검토 / 선택 항목을 닫기 위한 운영 문서다.

```text
자연어 요청
-> Work Brief 초안 / 변경안 작성
-> 계획차단 / 검토필요, 비차단 / 검토필요, 계획차단 / 선택필요, 비차단 / 선택필요 섹션으로 항목 표시
-> Codex 탐색 또는 사용자 답변
-> Work Brief 갱신
-> 확정된 결정 / 후속 후보 항목 정리
-> Planning 진입 여부 판정
```

`계획차단 / 검토필요` 항목은 Codex가 코드 / 문서 / Asset / Blueprint를 확인하기 전에는 Planning으로 넘어가기 위험한 항목이다.

`비차단 / 검토필요` 항목은 Work Brief Intake에서 닫지 않고, Planning의 선행 확인 항목으로 넘기고 진행할 수 있는 항목이다.

`계획차단 / 선택필요` 항목은 사용자 정책, 범위, 구조 선택 전에는 Planning을 확정할 수 없는 항목이다.

`비차단 / 선택필요` 항목은 사용자 선택이 필요하지만 후속 선택 항목으로 분리하고 Planning을 진행할 수 있는 항목이다.

Planning 진입 조건은 다음과 같다.

```yaml
Planning 진입 조건
-> 계획차단 / 선택필요 항목 없음
-> 계획차단 / 검토필요 항목 없음
-> 남은 비차단 / 검토필요 항목은 Planning의 선행 확인 항목으로 넘길 수 있음
-> Work Brief 준비 상태가 진행 가능 또는 준비 완료
```

Planning 진입 가능 상태는 구현 착수 가능 상태와 구분한다.

```yaml
Planning 진입 가능
-> 필요한 Planning Prompt를 시작할 수 있음

구현 착수 가능
-> Planning 결과에서 구현 단위, 선행 조건, 검증 기준이 정리된 뒤 판단
```

조건을 만족하지 못하면 세부 Planning Prompt나 실행 Prompt로 넘어가지 않고 `Work Brief Intake Prompt`로 돌아가 Brief를 보완한다.

---

## 6. 작업 유형별 Planning 라우팅 기준

```yaml
신규 기능 구현
-> Planning Prompt: Feature Work Planning Prompt
-> 새 기능 / 새 흐름 / 새 Component / 새 Asset 연결이 필요한 경우

구조 변경 / 리팩터링
-> Planning Prompt: Refactor Work Planning Prompt
-> 책임 경계, 호출 흐름, 클래스 구조, 데이터 계약을 바꾸는 경우

기존 기능 보완 / 업데이트
-> v1 처리: Feature Work Planning Prompt
-> 후속 후보: Update Work Planning Prompt
-> 기능은 존재하지만 동작, 범위, 정책, 검증, 문서가 부족한 경우

검토 / 검증 중심 작업
-> v1 처리: Review / Verification 실행 Prompt로 직접 라우팅
-> 후속 후보: Review / Verification Planning Prompt
-> 구현보다 현재 상태 확인, 회귀 위험, 검증 결과 기록이 목표인 경우

문서 작성 / 문서 정리
-> v1 처리: Document Writing Prompt로 직접 라우팅
-> 후속 후보: Document Writing Planning Prompt
-> 코드 수정 없이 문서 생성, 재구성, 분류, 정합성 보완이 목표인 경우

Commit / PR 준비
-> v1 처리: Git Commit PR Preflight Prompt로 직접 라우팅
-> 변경 내용 제출 전 Git 상태, 검증 상태, PR 가능 조건을 점검하는 경우
```

작업 유형이 섞여 있으면 먼저 Work Brief에서 범위와 우선순위를 분리한다.

---

## 7. Work List Writing 위치

`Work List Writing Prompt`는 실행 Prompt가 아니라 Planning 결과를 시각화 / 관리 / 기록하는 선택 계층이다.

```yaml
사용하는 경우
-> Branch 단위 작업을 장기적으로 관리해야 하는 경우
-> 진행 상태와 완료 기준을 시각적으로 추적해야 하는 경우
-> PR 가능 조건과 검증 상태를 한 문서에서 확인해야 하는 경우
-> 작업 범위와 후속 범위를 기록으로 남겨야 하는 경우

건너뛸 수 있는 경우
-> 단순 검토
-> 단일 문서 수정
-> 짧은 Git 점검
-> 관리 문서가 필요 없는 일회성 작업
```

Work List는 작업을 수행하는 Prompt가 아니라, Planning 결과를 실행 가능한 관리 문서로 변환하는 Prompt다.

Work Brief에서는 Work List Writing을 후보로 표시할 수 있다.

Work Planning 결과에서 최종 작성 여부를 판단한다.

Work List를 사용하기로 한 작업은 실행 전에 작성한다.

실행 후에는 Work List를 새로 만드는 것이 아니라 기존 Work List의 체크 상태, 검증 상태, PR 가능 조건, 후속 범위를 갱신한다.

Work List 작성 이후에도 실제 실행 계층은 작업 성격에 따라 `구현 계층`, `문서화 계층`, `검증 계층`, `Commit / PR 계층`으로 나뉜다.

---

## 8. v1 기준 한계점

```yaml
현재 있는 Planning Prompt
-> Work Brief Intake Prompt
-> Feature Work Planning Prompt
-> Refactor Work Planning Prompt

현재 없는 Planning Prompt
-> Update Work Planning Prompt
-> Review / Verification Planning Prompt
-> Document Writing Planning Prompt

v1 처리 기준
-> 없는 Planning Prompt가 필요한 작업은 Work Brief Intake가 직접 실행 Prompt 후보로 라우팅
-> 필요성이 반복 확인되면 별도 Planning Prompt로 분리
```

---

## 9. Prompt 참조 기준

Working Rule과 Working Reference는 직접 라우팅되는 작업 결과물이 아니라, 각 단계에서 판단 기준으로 참조하는 Prompt다.

참조 계층은 실행 계층의 하위 계층이 아니라, 입력 / 라우팅, 계획, 실행, 검증 전반에서 필요할 때 사용하는 횡단 계층이다.

```yaml
Working Rule
-> 작업 중 적용할 실행 규칙
-> Unreal Engine 공통 작업 규칙
-> Project Stella 작업 세션 규칙

Working Reference
-> 책임 경계 / 기술 기준 / 프로젝트 구조 판단 기준
-> Unreal Engine C++ 책임 경계
-> Project Stella 구조 / 용어 / Source of Truth 확인 기준
```

Planning, Implementation, Verification 단계에서 구조 판단이 필요하면 Working Rule / Reference를 함께 참조한다.

---

## 10. 주요 라우팅 예시

```yaml
새 기능 구현 요청
-> Work Brief Intake
-> Feature Work Planning
-> 필요 시 Work List Writing
-> 구현 계층
-> 필요 시 검증 계층
-> 필요 시 문서화 계층
-> 필요 시 Commit / PR 계층

리팩터링 요청
-> Work Brief Intake
-> Refactor Work Planning
-> 필요 시 Work List Writing
-> 구현 계층
-> 필요 시 검증 계층
-> 필요 시 문서화 계층
-> 필요 시 Commit / PR 계층

문서 정리 요청
-> Work Brief Intake
-> 필요 시 Work List Writing
-> 문서화 계층
-> 필요 시 검증 계층

검증 요청
-> Work Brief Intake
-> 필요 시 Work List Writing
-> 검증 계층
-> 필요 시 문서화 계층
-> 필요 시 Commit / PR 계층
```

---

## 11. AI Work Pipeline과의 관계

```yaml
AI Work Pipeline
-> 목표 확인 / 탐색 / 계획 / 구조 제안 / 적용 / 검증 / 문서화의 전체 공정

Prompt Flow and Routing Blueprint
-> 각 공정에서 어떤 Prompt 계층을 사용할지 정하는 호출 흐름

Prompt Files
-> 실제 작업에 사용하는 Prompt
```

Prompt Flow는 Pipeline을 대체하지 않고, Pipeline 안에서 Prompt를 선택하는 기준으로 사용한다.

---

## 12. 계속 수정할 항목

```yaml
후속 보완 후보
-> Update Work Planning Prompt 필요 여부
-> Review / Verification Planning Prompt 필요 여부
-> Document Writing Planning Prompt 필요 여부
-> Work Brief Intake Prompt의 작업 유형 판정 기준과 동기화
-> Work List Writing 사용 / 생략 기준의 실제 운용 결과 확인
```
