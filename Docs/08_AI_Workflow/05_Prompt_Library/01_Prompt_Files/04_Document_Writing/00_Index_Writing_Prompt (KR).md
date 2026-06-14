# Index Writing Prompt

## 1. 목적

문서 유형별 Index와 전체 Documentation Index를 작성하거나 보완할 때, 상위 라우터와 문서 유형별 상세 목록의 역할을 분리해 정리한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> 새 문서 유형별 Index를 만들 때
-> 기존 전체 Documentation Index가 상세 목록을 과하게 들고 있을 때
-> 문서 유형 prefix / 파일명 / 폴더명이 변경되어 Index 갱신이 필요할 때
-> 문서 유형별 상세 목록과 상위 Index의 역할이 섞였는지 점검할 때
```

---

## 3. 사용 방법

대상 문서 유형, 실제 파일 경로, 문서 ID prefix, 현재 폴더 구조, 관련 문서 유형을 입력하고 `복사용 Prompt`를 사용한다.

상위 Index와 문서 유형별 Index를 함께 작성할 경우, 상위 Index는 문서 유형 라우터로 두고 상세 문서 목록은 각 문서 유형별 Index에 둔다.

---

## 4. 복사용 Prompt

````text
다음 문서 유형의 Index를 작성하거나 보완해줘.

대상 범위:
- 전체 Index:
- 문서 유형:
- 대상 폴더:
- 문서 ID prefix:

현재 파일 구조:
- [Get-ChildItem 또는 파일 목록 붙여넣기]

작성 / 보완 목적:
- [새 Index 작성 / 파일명 변경 반영 / 문서 유형 재분류 반영 / 구형 명칭 제거 등]

작성 기준:
1. 전체 Index는 상위 라우터로 작성한다.
   - 문서 유형별 목적
   - 문서 ID prefix
   - 문서 유형별 Index 링크
   - 참조 규칙
   - 상세 문서 목록은 넣지 않는다.

2. 문서 유형별 Index는 상세 목록으로 작성한다.
   - 실제 파일명 기준으로 작성한다.
   - 제목은 가능하면 문서 H1 또는 본문 제목을 확인해 작성한다.
   - 파일명에서 추정할 수밖에 없으면 `확인 필요`를 남긴다.
   - GitHub PR 번호, branch, 관련 문서가 불확실하면 임의로 채우지 않는다.

3. 문서 유형별 컬럼은 문서 성격에 맞춘다.
   - Issue Checklist: ID / 제목 / 파일 / 관련 브랜치 / 관련 PR / 상태
   - Bug Report: ID / 제목 / 파일 / 관련 PR / 문제 유형 / 해결 상태
   - Pull Request: ID / 제목 / 파일 / 브랜치 / GitHub PR / 관련 문서
   - System Architecture: ID / 제목 / 파일 / 현재 역할 / 다음 분류 후보 / 비고
   - Portfolio Document: ID / 제목 / 파일 / 문서 목적 / 참조 내부 문서 / 비고

4. 명칭과 prefix는 현재 기준을 따른다.
   - Issue Checklist: `Dxx`
   - Work List: `Wxx`
   - Bug Report: `Bxx`
   - Pull Request: `Pxx`
   - System Architecture: `Sxx`
   - Portfolio Document: `PFxx`
   - 구형 prefix나 구형 폴더명은 남기지 않는다.

5. 불확실한 항목은 추측하지 않는다.
   - 확인 가능한 값만 채운다.
   - 불확실한 값은 빈 칸 또는 `확인 필요`로 남긴다.
   - 완료되지 않은 재분류는 확정된 구조처럼 쓰지 않는다.

출력 형식:
1. Index 역할 판단
2. 적용한 문서 유형 prefix
3. 생성 / 보완할 Index 파일
4. 문서 유형별 Index 초안
5. 확인 필요 항목
6. 구형 명칭 제거 확인
7. 검증 명령 제안
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 전체 Index 경로
-> 문서 유형별 Index 경로
-> 대상 폴더 파일 목록
-> 문서 ID prefix
-> 문서 유형 역할
-> 최신 파일명 / 폴더명 기준
-> 관련 문서 연결 기준
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 상위 Index와 문서 유형별 Index 역할 분리
-> 문서 유형별 상세 목록
-> 확인 필요 항목
-> 구형 명칭 / 경로 제거 확인
-> 검증 명령
```

---

## 7. 범위 / 비범위

```yaml
범위
-> 전체 Documentation Index 작성 / 보완
-> 문서 유형별 Index 작성 / 보완
-> 문서 ID prefix 정리
-> 실제 파일명 기준 목록 작성

비범위
-> 본문 문서 내용 재작성
-> 문서 유형 최종 재분류 확정
-> 불확실한 관련 문서 임의 연결
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 전체 Index에 상세 문서 목록을 중복 작성하지 않음
-> 파일명만 보고 문서 제목 / 상태 / 관련 문서를 과도하게 추정하지 않음
-> 구형 prefix / 구형 폴더명을 최신 기준처럼 유지하지 않음
-> 다음 브랜치에서 결정할 재분류 후보를 현재 확정 구조처럼 쓰지 않음
```

---

## 9. 검증 기준

```yaml
검증 기준
-> 전체 Index가 문서 유형 라우터 역할만 하는가
-> 문서 유형별 Index에 상세 목록이 있는가
-> 실제 파일명과 Index 파일명이 일치하는가
-> 문서 ID prefix가 현재 기준과 맞는가
-> 구형 명칭 / 구형 prefix가 남지 않는가
-> 불확실한 값이 추측으로 채워지지 않았는가
```

---

## 10. 기존 Prompt와 역할 경계

```yaml
Index Writing Prompt
-> 전체 Index / 문서 유형별 Index 작성과 보완

Document Common Format Prompt
-> 공통 파일명 / 상단 메타 / 문체 / Markdown 양식 기준

Document Set Audit Prompt
-> 여러 문서 간 역할 / 경로 / 상태 / 참조 정합성 감사
```
