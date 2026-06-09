# PR Document Writing Prompt

## 1. 목적

Branch에서 해결한 문제, 변경 범위, 책임 분리, 주요 Pipeline, 검증 결과를 PR Document로 정리한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> PR 작성 전후
-> Branch 결과를 제출용 문서로 정리할 때
-> 변경 요약과 검증 상태를 함께 정리해야 할 때
```

---

## 3. 사용 방법

대상 Branch, 변경 파일, 관련 Work Checklist, 검증 결과를 제공하고 `복사용 Prompt`를 사용한다.

---

## 4. 복사용 Prompt

````text
아래 작업을 PR Document 형식으로 작성 또는 보완해줘.

대상 작업:
- [Branch / PR / commit range / changed files]

관련 문서:
- Work Checklist:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- Portfolio Technical Document:
- Code Review:

작성 목표:
- 변경 목록 나열이 아니라 작업 의도와 책임 분리를 이해할 수 있게 정리해줘.
- 코드의 실제 타입명 / API / struct / enum / 파일명을 기준으로 작성해줘.
- 기능이 어떤 flow / pipeline / lifecycle 안에서 동작하는지 설명해줘.
- 검증 결과와 미검증 항목을 분리해줘.
- 현재 Branch 범위에 없던 최신 구조를 끌어와서 완료 내용처럼 쓰지 말아줘.

권장 구조:
1. 제목
2. 관련 Branch
3. 요약
4. 변경 범위
5. 주요 Pipeline
6. 안정성 보완
7. 검증 결과
8. 미검증 항목
9. 관련 문서
10. 정리
````

---

## 5. 입력 기준

```yaml
입력 기준
-> Branch명
-> 변경 파일 또는 commit range
-> 관련 Work Checklist
-> 검증 결과
-> 관련 구조 문서
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 작업 요약
-> 변경 범위
-> 주요 Pipeline
-> 검증 결과
-> 미검증 항목
-> 관련 문서
```

---

## 7. 범위 / 비범위

```yaml
범위
-> PR Document 작성 / 보완

비범위
-> Work Checklist 작성
-> 코드 리뷰 수행
-> 실제 GitHub PR 생성
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 실제 변경 범위만 작성
-> 검증하지 않은 항목을 완료로 표현하지 않음
-> 변경 목록보다 의도 / 책임 / 흐름을 우선 설명
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> Branch 시점과 현재 최신 구조를 분리
-> 관련 문서 번호를 자동 가정하지 않음
-> 확인 불가 항목은 검토 의견 또는 미검증으로 분리
```

---

## 10. 검증 기준

```yaml
검증 기준
-> Work Checklist 목표와 PR 결과가 연결되는가
-> 변경 범위와 검증 결과가 일치하는가
-> 미검증 항목이 PR note로 남을 수 있는가
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> PR Document에 변경 결과와 검증 상태가 분리됨

실패
-> Branch 결과 또는 검증 근거 확인 불가

미검증
-> PIE / Editor / Asset / Blueprint 확인 필요
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
01_Work_Checklist_Writing_Prompt
-> 작업 목표 / 완료 기준 관리

PR Document Writing Prompt
-> Branch 결과 제출 문서 정리
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> GitHub PR 템플릿과의 연결 기준
```

