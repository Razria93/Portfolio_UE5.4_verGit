# UE5 Portfolio Pull Request

## 제목

**P18: AI Workflow 운영 체계 및 Prompt Library v1 초안 구성**

## 날짜

**2026.06.09**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/ai-workflow`

---

## 요약

본 PR은 Codex를 적극 활용하기 위한 **AI Workflow v1 문서 구조**와 **Prompt Library v1 초안**을 구성한 작업이다.

또한 자연어 요청을 요청 요약(Work Brief), 기능 작업 계획(Feature Work Planning), 작업 목록(Work List)으로 정리하는 문서 흐름을 만들고, Parry 요청을 예시로 해당 흐름이 동작하는지 확인하는 것을 목표로 한다.

P18에서 확인한 문서 변환 흐름은 다음과 같다.

```text
자연어 요청
-> 요청 요약(Work Brief)
-> 기능 작업 계획(Feature Work Planning)
-> 작업 목록(Work List)
```

핵심 변경은 다음과 같다.

### Documentation

- **작업 운영 문서 구성**: AI 협업에 필요한 문서 목록(Index), 개요 문서(Overview), 프로젝트 맥락 문서(Project Context), 운영 지침(Operation Guide), 작업 절차 문서(Work Pipeline), 후속 관리 문서(Backlog), 초안 보관 문서(Drafts)를 나누어 정리했다.

- **Prompt 기준과 본문 구성**: 제작 / 관리 기준은 Prompt 기준 문서(Prompt Blueprint)로 정리하고, 실제 작업에 사용할 본문은 Prompt 본문(Prompt Files)으로 분리했다.

- **Prompt 호출 흐름 정리**: 자연어 요청 이후 어떤 Prompt 계층으로 넘어갈지 판단하는 호출 흐름(Flow / Routing) 기준을 정리했다.

- **요청 변환 예시 작성**: Parry 자연어 요청을 요청 요약(Work Brief), 기능 작업 계획(Feature Work Planning), 작업 목록(Work List)으로 변환해 작업 문서 흐름을 예시로 남겼다.

- **후속 범위 정리**: 실제 Parry 구현, Prompt 실사용 검증, 문서 체계 재정리는 후속 관리 문서(Backlog)와 리팩터링 메모(Refactor Notes)로 분리했다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복해서 사용할 문서 운영 용어를 먼저 정리한다.

```text
AI Workflow(AI 작업 운영 흐름)
-> Codex와 함께 작업할 때 요청 확인, 계획, 실행, 검증, 문서화를 어떤 순서로 진행할지 정리한 운영 흐름
-> 이 PR에서는 Docs/08_AI_Workflow 문서군으로 구성했다.
```

```text
Prompt Library(Prompt 모음)
-> 반복 작업에 사용할 Prompt와 Prompt 관리 기준을 역할별로 모아둔 문서 집합
-> 이 PR에서는 Prompt Blueprint와 Prompt Files v1 초안으로 구성했다.
```

```text
Prompt Blueprint(Prompt 기준 문서)
-> Prompt의 작성 형식, 제작 원칙, 커스텀 기준, 유지보수 기준, 호출 흐름을 정리한 기준 문서
```

```text
Prompt Files(Prompt 본문)
-> 실제 작업 중 호출해 사용할 Prompt 본문 파일
-> 작업 계획, 문서 작성, 검토, Git 작업처럼 역할별 폴더에 배치했다.
```

```text
Project Context(프로젝트 맥락 문서)
-> 프로젝트 개요와 작업 전제 정보를 AI가 먼저 확인할 수 있게 정리한 문서군
```

```text
Work Pipeline(작업 절차 문서)
-> 요청 확인부터 계획, 실행, 검증, 문서화까지 이어지는 작업 절차를 정리한 문서군
```

```text
Drafts(초안 보관 문서)
-> 검증용 또는 작성 중인 초안 산출물을 임시로 모아두는 문서군
```

```text
Work Brief(작업 요청 요약)
-> 사용자의 자연어 요청을 작업 목적, 범위, 준비 상태, 다음 계획 입력으로 정리한 문서
```

```text
Feature Work Planning(기능 작업 계획)
-> Work Brief를 바탕으로 기능 구현 범위, 실행 순서, 검증 기준을 구체화한 계획 문서
```

```text
Work List(작업 목록)
-> Planning 결과를 실행 상태, 완료 기준, 검증 기준을 확인할 수 있는 작업 목록으로 정리한 문서
-> P18에서는 W03 예시 문서를 통해 Work List 작성 흐름이 동작하는지만 확인했고, 항목별 구현 정합성은 후속 Parry 구현 Branch에서 검토한다.
```

```text
Prompt Flow / Routing(Prompt 호출 흐름)
-> 자연어 요청을 정리한 뒤 어떤 Prompt 계층으로 넘길지 판단하는 기준
```

---

## 변경 배경

이 섹션은 AI와의 협업 체계를 구축해야 했던 필요성과, 그 내부 구성을 AI Workflow / Prompt Library / W03 예시 검증으로 나누어 구성한 이유를 요약한다.

### AI 협업 절차 문서화 필요성

Codex와 작업할 때 필요한 목표 확인, 계획, 실행, 검증, 문서화 기준이 대화에만 남으면 Branch마다 작업 방식이 흔들릴 수 있었다.

따라서 AI 협업 절차를 문서화하되, 한 문서에 모두 모으지 않고 문서 위치 확인, 운영 기준, 작업 절차, 후속 관리를 구분할 수 있는 체계로 나눌 필요가 있었다.

### 자연어 요청 변환 흐름 필요성

사용자의 자연어 요청은 바로 구현으로 들어가기 전에 목적, 범위, 위험, 검증 기준으로 정리되어야 했다.

이를 위해 요청을 먼저 요약하고, 계획 단계에서 실행 단위와 검증 기준을 구체화한 뒤, 필요한 경우 작업 목록으로 관리할 수 있는 흐름이 필요했다.

### Prompt 기준과 본문 분리 필요성

Prompt가 늘어나면 작성 기준, 유지보수 기준, 호출 순서, 실제 Prompt 본문이 섞일 수 있었다.

따라서 Prompt를 만들고 관리하는 기준과 실제 작업 중 호출할 본문을 분리할 필요가 있었다.

### W03 예시 검증과 실제 구현 분리 필요성

P18에서는 Parry 요청을 예시로 사용해 자연어 요청이 작업 문서로 변환되는 흐름을 확인했다.

실제 Parry 구현과 UE 검증은 문서 변환 흐름 확인 이후의 작업이므로, 후속 Parry 구현 Branch에서 구체적으로 진행한다.

---

## 변경 범위

이 섹션은 이번 PR에서 구성한 문서 체계와 각 산출물이 맡는 역할을 정리한다.

### 1. AI Workflow 문서 구조 구성

- **왜**:
  AI 기반 작업 운영 기준에는 문서 위치 확인, 전체 개요, 운영 규칙, 작업 절차, 후속 관리처럼 서로 다른 역할이 필요했다.

- **어떻게**:
  `Docs/08_AI_Workflow` 아래에 문서 목록(Index), 개요 문서(Overview), 프로젝트 맥락 문서(Project Context), 운영 지침(Operation Guide), 작업 절차 문서(Work Pipeline), 후속 관리 문서(Backlog), 초안 보관 문서(Drafts)를 나누었다.

- **결과**:
  AI Workflow 문서군은 문서 위치와 상태 확인, 전체 구조 설명, 운영 기준, 작업 절차, 후속 항목 관리를 각각 다른 문서에서 담당한다.

### 2. Prompt Blueprint 구성

- **왜**:
  Prompt를 안정적으로 늘리려면 실제 Prompt 본문보다 먼저 작성 형식, 제작 원칙, 커스텀 기준, 유지보수 기준, 호출 흐름 기준이 필요했다.

- **어떻게**:
  Prompt 작성 형식(Format), 제작 원칙(Engineering), 프로젝트 적용 기준(Custom), 유지보수 기준(Maintenance), 호출 흐름(Flow / Routing)을 Prompt Blueprint로 구성했다.

- **결과**:
  Prompt Library v1은 Prompt 본문과 제작 / 관리 기준을 함께 가진 초안 구조가 됐다.

### 3. Prompt Files v1 초안 구성

- **왜**:
  실제 작업에서 사용할 Prompt는 작업 계획, 문서 작성, 검토, Git 작업처럼 사용 목적별로 찾을 수 있어야 했다.

- **어떻게**:
  Work Planning, Document Writing, Review Verification, Git Operation 등 역할별 폴더를 만들고, Work Brief Intake / Feature Work Planning / Work List Writing / Document Set Audit / Git Commit PR Preflight Prompt 초안을 배치했다.

- **결과**:
  자연어 요청 정리, 기능 계획, 작업 목록 작성, 문서 검토, Git 작업 전 확인에 사용할 Prompt 본문이 역할별로 분리됐다.

### 4. Prompt Flow / Routing 기준 정리

- **왜**:
  사용자의 요청이 들어왔을 때 매번 어떤 Prompt 계층으로 넘어갈지 새로 판단하면 작업 흐름이 흔들릴 수 있었다.

- **어떻게**:
  자연어 요청을 Work Brief Intake에서 정리한 뒤, 작업 유형과 준비 상태에 따라 Planning Prompt, Work List Writing, 문서화, 검증, Git 작업 계층으로 이어지는 기준을 정리했다.

- **결과**:
  작업 요청은 목적과 유형에 따라 필요한 Prompt 계층으로 이동할 수 있는 기본 Routing 기준을 갖게 됐다.

### 5. W03 제한적 실사용 검증

- **왜**:
  AI Workflow와 Prompt Library 초안이 실제 자연어 요청을 작업 문서로 변환할 수 있는지 확인할 예시가 필요했다.

- **어떻게**:
  Parry 자연어 요청을 `W03_UE5_Portfolio_Work_Brief.md`, `W03_UE5_Portfolio_Feature_Work_Planning.md`, `W03_UE5_Portfolio_Work_List_Draft.md`로 변환했다.

- **결과**:
  자연어 요청이 Work Brief -> Feature Work Planning -> Work List로 변환되는 흐름을 예시 문서로 확인했다.
  `Work_List_Draft`는 변환 흐름 확인을 위한 검증용 문서로 남겼다.

### 6. Backlog / Refactor Notes 정리

- **왜**:
  P18은 AI 협업 작업환경을 처음 구축하고, 계획 생산까지의 흐름을 확인한 단계다.
  이후에는 실제 사용 데이터를 기준으로 문서 체계와 Prompt Library를 다시 조정해야 했다.

- **어떻게**:
  `AI_Workflow_Backlog (KR).md`에는 실사용 중 다시 확인할 후속 작업과 검토 후보를 정리하고, `AI_Workflow_Refactor_Notes (KR).md`에는 다음 AI Workflow Refactor에서 조정할 구조 후보를 분리했다.

- **결과**:
  P18은 AI Workflow 문서 체계와 Prompt Library v1 초안을 초기 작업환경으로 구성하고, 다음 단계에서는 실제 사용 데이터를 바탕으로 후속 Refactor를 진행할 수 있게 했다.

---

## 주요 처리 흐름

이 섹션은 이번 PR에서 정리한 작업 문서 변환 흐름과 Prompt 호출 흐름을 설명한다.

### W03 예시 문서 변환 흐름

```text
사용자 자연어 요청
-> Work Brief 작성
-> Feature Work Planning 작성
-> Work List 작성
```

이 흐름은 P18에서 Parry 요청을 예시로 사용해 자연어 요청이 작업 문서로 변환되는 과정을 확인한 것이다.
P18에서는 작업 목록이 생성되는 흐름까지만 확인했다.
작업 목록 내용 검토와 구현 / 검증 / 문서화는 후속 Parry 구현 Branch에서 진행한다.

### Prompt Routing 흐름

```text
사용자 자연어 요청
-> Work Brief Intake에서 요청 목적 / 작업 유형 / 준비 상태 정리
-> 필요한 Planning Prompt 후보 판단
-> 선택된 Planning Prompt로 작업 계획 작성
-> Work List Writing 필요 여부 판단
-> 실행 / 문서 / 검증 / Git 단계로 이동
```

이 흐름은 `Work Brief Intake`가 자연어 요청을 정리하고 다음 Prompt 후보를 판단한 뒤, 작업 유형에 맞는 계층으로 넘기는 과정을 의미한다.

---

## 구현 결과

이 섹션은 P18 이후 문서 운영 체계와 Prompt Library가 어떤 산출물로 남았는지 정리한다.

- **AI Workflow 문서군**: 문서 목록(Index), 개요 문서(Overview), 프로젝트 맥락(Project Context), 운영 지침(Operation Guide), 작업 절차(Work Pipeline), 후속 관리(Backlog), 초안 보관(Drafts) 문서로 나뉘었다.

- **Prompt Blueprint**: Prompt 작성 형식, 제작 원칙, 프로젝트 적용 기준, 유지보수 기준, 호출 흐름 기준을 관리하는 기준 문서로 구성됐다.

- **Prompt Files v1**: Work Planning, Document Writing, Review Verification, Git Operation 등 실제 작업 중 호출할 Prompt 본문을 역할별 폴더로 분리했다.

- **W03 예시 문서**: Parry 요청을 Work Brief, Feature Work Planning, Work List까지 변환해 문서 변환 흐름 예시로 남겼다.

- **Backlog / Refactor Notes**: 실제 사용 중 다시 확인할 후속 작업과 다음 AI Workflow Refactor에서 조정할 구조 후보를 관리하도록 분리했다.

---

## 주요 커밋

- `be43b53`: `docs(ai-workflow): establish workflow document structure`

- `939ba31`: `docs(prompt): define prompt blueprint and routing model`

- `fab6874`: `docs(prompt): add work planning prompts`

- `8c54eef`: `docs(prompt): add document review and git operation prompts`

- `234a114`: `docs(ai-workflow): record backlog and d20 workflow validation`

---

## 테스트 방법

### 문서 출력 확인

- W01 Work List와 AI Workflow 문서군이 UTF-8 기준으로 정상 출력되는지 확인했다.

### 검색 검증

- Branch 목표, Prompt Library v1, AI Workflow, W03 흐름이 문서에 반영되어 있는지 검색으로 확인했다.

### 배치 확인

- Prompt Blueprint와 Prompt Files가 역할별 폴더에 배치되어 있는지 확인했다.

### W03 흐름 확인

- W03 Work Brief / Feature Work Planning / Work List 문서가 자연어 요청 -> 계획 -> 작업 목록 흐름으로 이어지는지 확인했다.

- W03 작업 목록에서 실제 구현 / Build / PIE / Editor / Asset 검증 항목이 완료 처리되지 않고 후속 Parry 구현 Branch 범위로 분리되어 있는지 확인했다.

---

## 검증 결과

- W01 Work List와 AI Workflow 문서군의 출력 상태를 확인했다.

- Branch 목표, Prompt Library v1, AI Workflow, W03 문서 흐름이 관련 문서에 반영되어 있음을 확인했다.

- Prompt Blueprint 6종과 Prompt Files 역할별 폴더 배치를 확인했다.

- W03 Work Brief / Feature Work Planning / Work List가 제한적 실사용 검증 예시로 남아 있고, 실제 구현 완료 문서처럼 처리되지 않았음을 확인했다.

---

## 미검증 항목

- UE C++ 빌드

- PIE 검증

- Editor / Asset 검증

- 실제 Parry 구현 검증

- Prompt Library v1 전면 실사용 검증

본 PR은 문서 / Prompt Library 구성 작업이므로 UE C++ 빌드와 PIE 검증은 수행하지 않았다.

---

## 비범위

- W03 Parry 실제 구현

- Guard / Counter 내부 설계 확정

- W03 작업 목록의 실제 구현 기준 정합성 검토

- Prompt Library v1 전면 실사용 검증

- Prompt 문장 품질 최종 검수

- 기존 Docs 전체 리팩터링

- System Architecture / Engine Technique 문서 체계 재작성

- UE C++ 코드 변경과 빌드 검증

---

## 후속 작업

- 후속 Parry 구현 Branch에서 W03 작업 목록의 항목별 구현 정합성을 검토한다.

- 후속 Parry 구현 Branch에서 Build / Code Flow / PIE / Editor / Asset 검증을 수행한다.

- Prompt Library v1을 실제 작업에 반복 적용하면서 보완 후보를 Backlog와 Refactor Notes에 반영한다.

- AI Workflow Refactor Notes 기준으로 Prompt Flow, 문서 체계, Work List 갱신 규칙을 재검토한다.

---

## 관련 문서

- W01 Work List: `W01_UE5_Portfolio_Work_List.md`

- Work Brief: `W03_UE5_Portfolio_Work_Brief.md`

- Work Planning: `W03_UE5_Portfolio_Feature_Work_Planning.md`

- W03 Work List: `W03_UE5_Portfolio_Work_List_Draft.md`

- AI Workflow:
  - `AI_Workflow_Index (KR).md`
  - `AI_Workflow_Overview (KR).md`
  - `Project_Stella_Overview (KR).md`
  - `AI_Workflow_Operation_Guide (KR).md`
  - `AI_Work_Pipeline (KR).md`
  - `Project_Overview_Draft (KR).md`
  - `Project_Rules_Draft (KR).md`
  - `AI_Work_Plan_Draft (KR).md`

- Prompt Blueprint: `05_Prompt_Flow_and_Routing_Blueprint (KR).md`

- Backlog:
  - `AI_Workflow_Backlog (KR).md`
  - `AI_Workflow_Refactor_Notes (KR).md`

---

## 정리

P18은 Codex 기반 작업을 반복 가능하게 만들기 위한 AI Workflow 문서 체계와 Prompt Library v1 초안을 구성한 문서 PR이다.

W03 Parry 요청은 자연어 요청이 작업 문서로 변환되는 흐름을 확인하기 위한 예시로 사용했다.
실제 구현과 UE 검증은 후속 Parry 구현 Branch로 분리했다.
