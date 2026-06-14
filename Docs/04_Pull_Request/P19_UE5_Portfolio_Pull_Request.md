# UE5 Portfolio Pull Request

## 제목

**P19: AI 협업 기반 문서 운영 체계 정리**

## 날짜

**2026.06.14**

## 상태

- [x] **완료**

---

## 브랜치

- `docs/portfolio-documentation-update`

---

## 요약

이번 PR에서는 **구형 문서 구조를 AI 협업 기준에 맞는 문서 운영 체계로 재정리했다.**

주요 문서 유형의 작성 기준과 문서 간 연결 방식을 정리해 사용자와 AI가 같은 기준으로 문서를 작성, 검토, 갱신할 수 있도록 만들었다.

PR 문서는 프로젝트 전반의 변경 흐름을 압축해 보여주는 핵심 문서로 재구성하고, Issue Checklist / Work List / Bug Report는 PR 기준 용어 톤을 공유하면서 각 문서 목적에 맞게 양식과 연결 기준을 맞췄다.

성격별 핵심 변경은 다음과 같다.

### Documentation / Refactoring

- **문서 유형 연결 기준 정리**: Issue Checklist / Work List / Bug Report / PR Document가 같은 용어 톤과 관련 문서 기준으로 연결되도록 정리했다.

- **PR 변경 기록 체계 정리**: P01~P18 PR 문서가 브랜치별 목표, 변경 흐름, 검증 결과, 관련 문서를 정식 변경 기록으로 설명하도록 정리했다.

- **문서 탐색 기준 재구성**: 전체 Documentation Index를 상위 라우터로 두고, 상세 목록은 문서 유형별 Index에서 찾도록 분리했다.

- **문서 식별 기준 정리**: 구형 `Technical Document` / `Txx` 기준을 제거하고, System Architecture / Portfolio Document / archive 문서가 현재 문서 체계 안에서 추적되도록 정리했다.

- **AI 협업 운영 기준 보강**: AI Workflow / Prompt Library / Prompt Management 기준을 보강해 Prompt 반영 후보, Index 작성, Git preflight 기준을 관리할 수 있게 했다.

- **PR 문서 audit 기준 정리**: PR 문서 검증 과정에서 만든 용어, 목표 흐름, 문서 품질 기준을 장기 유지 기준으로 축약했다.

- **후속 재분류 범위 분리**: System Architecture 본문 재작성과 Engine Technique 문서 유형 신설은 다음 Branch에서 처리하도록 notes 문서로 분리했다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복해서 사용할 문서 운영 용어를 먼저 정리한다.

```text
AI Workflow(AI 작업 운영 흐름)
-> AI와 함께 작업할 때 요청 확인, 계획, 실행, 검증, 문서화, 후속 관리를 어떤 기준으로 진행할지 정리한 운영 문서 묶음
```

```text
Prompt Library(Prompt 모음)
-> 반복 작업에 사용할 Prompt와 Prompt 제작 / 관리 기준을 역할별로 모아둔 문서 집합
```

```text
Documentation Index(문서 유형 상위 목차)
-> 전체 문서 유형의 ID 규칙, 역할, 문서 유형별 Index 위치를 안내하는 상위 라우터 문서
```

```text
PR Document Audit(PR 문서 audit 기준)
-> PR 문서가 같은 용어, 목표 흐름, 문서 품질 기준으로 작성되어 있는지 점검하고 후속 PR 문서 보완에 재사용하는 검토 기준
```

```text
Portfolio Document(포트폴리오 제출용 문서)
-> 내부 System / Engine / PR / Bug Report / Workflow 문서를 바탕으로 외부 독자에게 보여줄 설명 문서
```

---

## 변경 배경

이 섹션은 이번 PR이 필요했던 이유와 문서 운영 체계에서 먼저 정리해야 했던 문제를 설명한다.

### AI 협업 기준 안정화 필요성

AI와 협업해 문서를 작성할 때 기준이 대화에만 남으면 Branch마다 문서 구조, 용어, 검증 기준이 흔들릴 수 있었다.

따라서 문서 작성 / 검토 / 갱신 기준을 Prompt Library와 AI Workflow 안에 명시하고, 반복 가능한 운영 기준으로 남길 필요가 있었다.

### 문서 유형별 연계성 정리 필요성

기존 문서 구조는 문서 유형별 역할과 Index 연결이 충분히 분리되어 있지 않아, 어떤 문서를 어디서 찾아야 하는지 추적하기 어려웠다.

이를 해결하기 위해 전체 Documentation Index는 상위 라우터로 두고, 상세 문서 목록은 각 문서 유형별 Index로 나누는 구조가 필요했다.

### 구형 EN 문서와 구형 문서명 정리 필요성

구형 EN 문서와 `Technical Document` / `Txx` 표현은 System Architecture, Engine Technique, Portfolio Document의 역할을 흐리게 만들 수 있었다.

따라서 Portfolio 문서는 `Portfolio Document` / `PFxx` 기준으로 정리하고, System Architecture는 `Sxx` 파일명 기준으로 맞출 필요가 있었다.

### System / Engine 후속 재분류 필요성

System Architecture와 Engine Technique 본문은 단순 파일명 정리보다 작업량이 크고, 현재 구조 설명과 설계 기록을 다시 나눠야 했다.

이번 PR에서는 최종 재분류를 완료하지 않고, 다음 Branch에서 판단할 기준을 notes 문서로 분리했다.

---

## 변경 범위

이 섹션은 이번 PR에서 문서 운영 체계를 어떻게 정리했고, 그 결과 무엇이 달라졌는지 설명한다.

### 1. 문서 유형별 양식 / 연결 기준 정리

- **왜**:
  Issue Checklist, Work List, Bug Report, PR Document가 각자 다른 용어 톤과 연결 방식을 사용하면 문서 간 흐름과 관련 문서 추적 기준이 흔들릴 수 있었다.

- **어떻게**:
  Issue Checklist / Work List / Bug Report는 PR 기준 용어 톤을 공유하되, 각 문서 목적에 맞게 내용을 압축했다.
  문서 유형별 Index를 기준으로 각 문서 유형의 위치와 역할을 분리하고, 관련 문서 섹션과 파일명 참조가 실제 문서 체계와 맞도록 정리했다.
  공통 문서 양식은 유지하되, PR / Work List / Portfolio Document / Index처럼 문서 유형별로 필요한 작성 기준은 Prompt Library에 반영했다.

- **결과**:
  Issue Checklist / Work List / Bug Report / PR Document가 같은 문서 운영 체계 안에서 연결되고, 문서 유형별 차이는 작성 기준으로 관리된다.

### 2. PR 변경 기록 체계 정리

- **왜**:
  기존 PR 문서는 브랜치별 목표, 변경 흐름, 검증 결과, 관련 문서 연결 방식이 일정하지 않아 프로젝트 변경 이력을 한 기준으로 따라가기 어려웠다.
  따라서 PR 문서를 단순 작업 기록이 아니라 브랜치별 목표와 변경 흐름을 압축해 보여주는 정식 변경 기록으로 정리할 필요가 있었다.

- **어떻게**:
  기존 PR 문서와 보완안을 비교 검토하고, 사용자 검토와 audit 기준 반영을 거쳐 보완본을 확정했다.
  확정된 보완본은 P01~P18 정식 PR 문서 위치에 반영하고, PR 문서 목록은 `00_Pull_Request_Index.md`에서 관리하도록 구성했다.

- **결과**:
  P01~P18 PR 문서는 정식 변경 기록으로 정리되어 Pull Request Index 기준으로 추적할 수 있게 됐다.
  이번 Branch에서 가장 큰 재구성 범위는 프로젝트 변경 흐름을 압축해 보여주는 PR 문서였다.

### 3. PR 문서 audit 기준 정리

- **왜**:
  PR 문서 재구성 과정에서 용어, 목표 흐름, 문서 품질 기준을 따로 검증했지만, 중간 산출물이 많아지면 실제 유지 기준이 흐려질 수 있었다.

- **어떻게**:
  용어 분류 기준, 용어 사용 맵, PR 목표 흐름, PR 문서 품질 기준을 장기 유지 문서로 정리하고, 중간 산출물은 정리했다.

- **결과**:
  이후 PR 문서를 추가하거나 보완할 때 참조할 audit 기준이 역할별로 남았다.

### 4. 문서 탐색 기준 재구성

- **왜**:
  전체 문서 목록을 한 문서에서 직접 관리하면 문서가 늘어날수록 상위 Index가 비대해지고, 문서 유형별 역할을 보기 어려워진다.

- **어떻게**:
  전체 Documentation Index는 문서 유형을 찾는 상위 라우터 역할로 정리하고, 상세 목록은 문서 유형별 Index가 담당하도록 탐색 책임을 분리했다.

- **결과**:
  문서 ID 규칙과 문서 유형별 상세 목록이 분리되어, 사용자는 상위 Index에서 문서 유형을 찾고 각 문서 유형별 Index에서 실제 파일을 확인할 수 있다.

### 5. System / Portfolio 문서 식별 기준 정리

- **왜**:
  구형 EN 문서와 구형 문서명 기준이 남아 있으면 System Architecture, Portfolio Document, archive 문서의 역할과 후속 재분류 대상이 흐려질 수 있었다.

- **어떻게**:
  구형 `Technical Document` / `Txx` 기준을 제거하고, System Architecture / Portfolio Document / archive 문서가 현재 문서 체계 안에서 추적되도록 식별 기준을 정리했다.
  현재 문서와 보관 문서가 섞이지 않도록 System Architecture archive 문서는 별도 archive 기준으로 분리했다.

- **결과**:
  System / Engine 후속 재분류에서 현재 문서와 보관 문서를 구분해 추적할 수 있게 됐다.

### 6. AI Workflow / Prompt Library 갱신

- **왜**:
  문서 유형 명칭과 운영 기준이 바뀌면 AI Workflow와 Prompt Library가 구형 기준을 계속 참조할 수 있었다.

- **어떻게**:
  AI Workflow, Prompt Blueprint, Prompt Files, Prompt Management 문서에서 `Portfolio Document` / `PFxx` 기준을 반영했다.
  Index 작성 기준은 별도 Prompt로 정리하고, Prompt 후보 감지 / 기록 기준과 Git preflight 기준을 AI 협업 운영 흐름에 추가했다.

- **결과**:
  AI가 문서 작업을 수행할 때 최신 문서 유형 명칭, Index 작성 기준, Prompt 유지관리 기준을 함께 참조할 수 있게 됐다.

### 7. System / Engine 후속 재분류 범위 분리

- **왜**:
  System Architecture 본문 재작성과 Engine Technique 문서 유형 신설은 파일명 정리보다 큰 구조 리팩터링이며, 이번 PR에서 함께 처리하면 범위가 과도하게 커질 수 있었다.

- **어떻게**:
  System / Engine / Portfolio 문서 역할과 다음 Branch 재분류 기준을 별도 notes 문서로 정리했다.

- **결과**:
  이번 PR은 문서 운영 체계와 문서 식별 / 탐색 기준 정리까지 완료하고, System / Engine 본문 재분류는 다음 Branch에서 이어갈 수 있게 됐다.

---

## 주요 처리 흐름

이 섹션은 이번 PR에서 정리한 문서 참조 흐름과 Prompt 반영 후보 관리 흐름을 설명한다.

### 문서 참조 흐름

```text
문서 탐색 필요
-> Documentation Index에서 문서 유형 확인
-> 문서 유형별 Index로 이동
-> 실제 문서 ID / 파일명 / 관련 문서 확인
-> 필요한 PR / Bug Report / System / Portfolio 문서 참조
```

이 흐름은 전체 Index가 상세 목록을 직접 들고 있지 않고, 문서 유형별 Index로 연결되는 구조를 의미한다.

### Prompt 반영 후보 관리 흐름

```text
작업 단위 완료
-> Prompt 반영 후보 자체 점검
-> 반영 불필요 / 후보 기록 / 즉시 반영 권장 분류
-> 필요 시 Prompt Pattern Candidates에 기록
-> 사용자 승인 후 Prompt 반영
-> 반영 후 구형 표현 / 충돌 기준 검증
```

이 흐름은 작업 중 발견한 문서 작성 기준을 바로 Prompt에 자동 반영하지 않고, 후보 기록과 승인 절차를 거쳐 관리하는 과정을 의미한다.

---

## 구현 결과

- Issue Checklist / Work List / Bug Report / PR Document는 공통 용어 톤과 관련 문서 기준 안에서 연결된다.

- PR 문서는 브랜치별 변경 흐름을 압축해 보여주는 정식 변경 기록으로 기능한다.

- 전체 Documentation Index는 문서 유형 라우터 역할을 맡고, 상세 목록은 문서 유형별 Index가 담당한다.

- System Architecture / Portfolio Document / archive 문서는 현재 문서 체계 안에서 구분해 추적할 수 있다.

- AI Workflow와 Prompt Library는 최신 문서 유형 명칭과 문서 작성 기준을 기준으로 판단할 수 있다.

- Prompt 변경 후보 감지, staged format-only 변경 분리, Index 작성 기준이 AI 협업 운영 흐름에 포함됐다.

- System / Engine 본문 재분류는 notes 문서 기준으로 다음 Branch에서 이어갈 수 있게 됐다.

---

## 테스트 방법

### 문서 존재 / 번호 확인

- Issue Checklist / Bug Report / Work List 관련 목록이 문서 유형별 Index 또는 관련 목록 기준으로 연결되는지 확인했다.

- Work List Index에서 W01~W03 항목을 추적할 수 있는지 확인했다.

- W01~W03 Work List 문서가 존재하는지 확인했다.

- P01~P19 PR 문서가 존재하는지 확인했다.

### 구형 명칭 검색

- 구형 EN 문서명과 구형 문서명 흔적을 확인하기 위해 `Portfolio Technical Document`, `Technical_Documents`, `Txx`, `Refactor_Draft`, `관련 PR / 문서` 잔존 여부를 검색했다.

### Index 확인

- `W02`, `W03`, `P19`, `PFxx`, `S29`, `Portfolio Document`가 관련 Index와 문서에 반영되어 있는지 확인했다.

### 공백 검증

- 문서 변경 범위에 대해 `git diff --check`를 실행했다.

### Git preflight

- `git status --short`로 staged / unstaged / untracked 변경을 분리했다.
- Parry asset / unrelated 변경이 문서 커밋 후보에 섞이지 않는지 확인했다.
- staged format-only 변경과 내용 변경을 분리해 확인했다.

---

## 검증 결과

- W02 Work List와 P19 PR 문서가 존재함을 확인했다.

- Issue Checklist / Bug Report / Work List가 문서 유형별 Index와 관련 문서 기준 안에서 연결되어 있음을 확인했다.

- Work List Index에서 W01~W03 항목을 확인했다.

- Pull Request Index에서 P19 항목을 확인했다.

- 전체 Documentation Index와 문서 유형별 Index가 현재 문서 ID 체계를 기준으로 연결되어 있음을 확인했다.

- AI Workflow와 Prompt Library에서 구형 `Portfolio Technical Document`, `Technical Document`, `Txx`, `07_Technical_Documents` 표현이 제거되었음을 확인했다.

- `git diff --check` 기준 공백 오류가 없고, LF / CRLF warning은 공백 오류와 분리해 확인했다.

- Parry asset / unrelated 변경은 문서 커밋 후보에서 제외했음을 확인했다.

- staged format-only 변경과 내용 변경을 분리해 확인했다.

---

## 미검증 항목

- UE C++ 빌드

- PIE 검증

- Editor / Asset 검증

- System Architecture 본문 최신성 검증

- Engine Technique 문서 유형 최종 구조 검증

본 PR은 문서 운영 체계 정리 작업이므로 UE C++ 빌드와 PIE 검증은 수행하지 않았다.

---

## 비범위

- System Architecture 본문 재작성

- Engine Technique 문서 유형 신설

- Engine Implementation Records 최종 분리

- Portfolio Document 본문 재구성

- History / Records 체계 최종 확정

- UE C++ 코드 변경과 빌드 검증

---

## 후속 작업

- 다음 Branch에서 `N01_System_Engine_Document_Reclassification_Note.md` 기준으로 System / Engine 문서를 재분류한다.

- System Architecture를 현재 시스템 구조 설명 중심으로 재작성한다.

- Engine Technique Document 문서 유형 신설 여부와 폴더 번호를 결정한다.

- Portfolio Document를 내부 System / Engine / AI Workflow 문서를 기반으로 제출용 설명 문서로 재구성한다.

- History / Records / Archive 체계를 별도 문서 유형으로 둘지 재검토한다.

---

## 관련 문서

- Work List: `W02_UE5_Portfolio_Work_List.md`

- Work List Index: `00_Work_List_Index.md`

- Documentation Index: `00_Documentation_Index.md`

- Pull Request Index: `00_Pull_Request_Index.md`

- System / Engine Note: `N01_System_Engine_Document_Reclassification_Note.md`

- AI Workflow:
  - `AI_Workflow_Index (KR).md`
  - `AI_Workflow_Operation_Guide (KR).md`
  - `AI_Work_Pipeline (KR).md`

- Prompt Management:
  - `01_Prompt_Change_Management_Rule (KR).md`
  - `02_Prompt_Pattern_Candidates (KR).md`

- Prompt Files:
  - `00_Index_Writing_Prompt (KR).md`
  - `Git_Commit_PR_Preflight_Prompt (KR).md`

---

## 정리

P19는 구형 문서 구조를 AI 협업 기준에 맞는 문서 운영 체계로 재정리한 문서 PR이다.

이번 PR에서는 문서 작성 기준, 문서 탐색 기준, Prompt 유지관리 기준을 같은 운영 흐름 안에서 정리했다.

그중 PR 문서는 브랜치별 변경 흐름을 압축해 보여주는 핵심 변경 기록으로 재구성했고, System / Engine 본문 재분류는 다음 Branch에서 이어갈 수 있도록 범위를 분리했다.
