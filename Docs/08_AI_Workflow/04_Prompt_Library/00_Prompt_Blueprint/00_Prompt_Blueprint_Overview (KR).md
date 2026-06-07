# Prompt Blueprint Overview

## 1. 목적

본 문서는 Prompt Blueprint 묶음의 역할과 적용 흐름을 정리한다.

Prompt Blueprint는 반복 사용할 Prompt를 만들기 위한 형식, 내용 설계, Custom 적용, 유지보수 기준을 묶은 제작 기준이다.

---

## 2. Prompt Library 정의

```yaml
Prompt Library
-> Prompt Blueprint
-> Prompt Files
```

Prompt Blueprint는 Prompt를 만들고 관리하기 위한 기준 문서 묶음이다.

Prompt Files는 작업 세션에 사용하거나 문서 작성 / 검증 / Git 운영에 적용하는 개별 Prompt다.

---

## 3. Prompt Blueprint 이해 순서

Prompt Blueprint를 처음 검토할 때는 다음 순서로 읽는다.

```yaml
Prompt Blueprint 이해 순서
1. Prompt Format Blueprint
-> Prompt의 형식과 섹션 구조 이해

2. Prompt Engineering Blueprint
-> 각 섹션 안에 들어갈 요청 내용 설계 기준 이해

3. Prompt Library Maintenance Blueprint
-> 작성된 Prompt의 위치, 이름, 상태, 중복, 참조, 수명주기 관리 기준 이해

4. Prompt Custom Blueprint
-> Project Stella / AI Workflow 전용 적용 기준 이해
```

---

## 4. Prompt 제작 적용 순서

새 Prompt를 작성할 때는 다음 순서로 적용한다.

```yaml
적용 요약
-> Format -> Engineering -> Custom -> Maintenance
```

```yaml
Prompt 제작 적용 순서
1. Prompt Format Blueprint
-> 필요한 섹션과 포함 여부 결정

2. Prompt Engineering Blueprint
-> 복사용 Prompt, 입력 기준, 출력 기준, 제약 조건, 검증 기준의 내용 설계

3. Prompt Custom Blueprint
-> Project Stella / AI Workflow 맥락, Document 연결, 사용자 표현 기준 반영

4. Prompt Library Maintenance Blueprint
-> 위치, 파일명, 상태, 중복, Backlog 반영 여부 판단
```

Prompt Blueprint 이해 순서와 실제 제작 적용 순서가 다른 이유는 Custom 규칙이 Prompt 작성 중에 먼저 반영되고, Maintenance 판단은 작성 후 Library에 등록하거나 갱신할 때 적용되기 때문이다.

---

## 5. 작업 유형별 Prompt Blueprint 참조

```yaml
신규 Prompt 작성
-> Prompt Format Blueprint
-> Prompt Engineering Blueprint
-> Prompt Custom Blueprint
-> Prompt Library Maintenance Blueprint

기존 Prompt 응답 품질 개선
-> Prompt Engineering Blueprint
-> Prompt Custom Blueprint
-> Prompt Library Maintenance Blueprint

Prompt Library 구조 정리
-> Prompt Library Maintenance Blueprint
-> Prompt Custom Blueprint
-> AI Workflow Index
-> AI Workflow Backlog

Project Stella 전용 기준 반영
-> Prompt Custom Blueprint
```

---

## 6. Index와 Overview 역할 경계

```yaml
AI Workflow Index
-> 문서와 Prompt의 위치 / 역할 / 상태 확인

Prompt Blueprint Overview
-> Prompt Blueprint 묶음의 역할과 적용 흐름 설명
```

Index는 상세 사용법을 길게 설명하지 않는다.

Overview는 Prompt 제작 흐름을 설명하되, 각 Prompt Blueprint의 세부 규칙을 복사하지 않는다.
