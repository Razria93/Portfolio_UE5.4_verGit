# UE5 Portfolio Pull Request

## 제목

**P27: Code Quality Cleanup Plan**

## 날짜

**2026.06.28**

## 상태

- [x] **완료**

---

## 브랜치

- `docs/code-quality-plan`

---

## 요약

이번 PR은 외부 리뷰 피드백과 `Source/Portfolio` 코드 스캔 결과를 바탕으로, 후속 코드 품질 개선 작업의 우선순위와 브랜치 분리 기준을 고정한다.

이 PR에서는 실제 C++ 코드를 수정하지 않는다. 목적은 이후 `Unreal reference safety`, `Component reference validation`, `Debug log policy` 같은 코드 리팩터링 PR들이 공통 기준을 갖고 진행되도록 작업 방향을 먼저 문서화하는 것이다.

---

## 변경 배경

외부 리뷰 과정에서 반복적으로 다음 문제가 지적되었다.

```text
- 구조와 문서가 크지만 실제 코드 품질 기준이 더 명확해야 한다.
- UE C++ 기본기, 특히 UObject reference / UPROPERTY / GC 기준이 중요하다.
- check 기반 component validation은 초기화 순서와 release build 관점에서 위험할 수 있다.
- 조건 없는 debug log / print는 shipping build와 성능 측면에서 정리되어야 한다.
- TODO, typo, hardcoded tuning value는 포트폴리오 신뢰도를 낮출 수 있다.
- 리팩터링은 작은 PR 단위로 나누어야 한다.
```

따라서 바로 코드 수정에 들어가기 전에, 코드 품질 개선 항목과 처리 순서를 먼저 고정한다.

---

## 변경 범위

### 1. W05 Work List 추가

다음 문서를 추가했다.

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
```

문서에는 다음 항목을 정리했다.

```text
- 우선 브랜치 순서
- 완료 기준
- 필수 산출물
- P0 / P1 / P2 작업 항목
- 작업 원칙
- 검증 기준
- 후속 문서 업데이트 후보
```

### 2. Code Quality Note 추가

다음 노트를 추가했다.

```text
Docs/06_notes/N08_Code_Quality_Cleanup_Plan_Note.md
```

노트에는 다음 기준을 정리했다.

```text
- Review feedback 요약
- 코드 스캔에서 확인한 위험
- Review feedback과 코드 스캔이 겹치는 항목
- P0 / P1 / P2 우선순위 판단
- 브랜치 분리 기준
- 문서 업데이트 기준
- Prompt update 후보
```

### 3. Work List Index 갱신

`W05` 항목을 Work List Index에 추가했다.

```text
Docs/01_Work_List/00_Work_List_Index.md
```

---

## 검증

### 문서 확인

다음 문서가 UTF-8로 정상 조회되는 것을 확인했다.

```text
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
Docs/06_notes/N08_Code_Quality_Cleanup_Plan_Note.md
Docs/01_Work_List/00_Work_List_Index.md
```

### 코드 변경 여부

이번 PR은 문서 계획 PR이므로 `Source/Portfolio` 코드는 변경하지 않는다.

---

## 제외 범위

이번 PR에서는 다음 작업을 의도적으로 제외한다.

```text
- UObject pointer / UPROPERTY 코드 수정
- check / ensure / component validation 코드 수정
- debug log gate 구현
- TODO 제거 또는 Phase 주석 변경
- hardcoded tuning value 정리
- naming / typo 코드 수정
- const API 정리
- AI Blackboard key registry 구현
- AI update interval profiling
- Enhanced Input migration
- Blink / Repulse / ResultOut 구현
```

---

## 후속 작업

권장 후속 순서는 다음과 같다.

```text
1. refactor/unreal-reference-safety-v1
2. refactor/component-reference-validation-policy
3. refactor/debug-log-policy-v1
4. refactor/todo-status-cleanup
5. refactor/tuning-constants-cleanup
```

첫 번째 코드 품질 PR은 `refactor/unreal-reference-safety-v1`로 진행한다.
