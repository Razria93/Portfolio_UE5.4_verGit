# Portfolio Document Writing Prompt

## 1. 목적

여러 작업 문서에 축적된 내용을 평가자에게 보여줄 포트폴리오 제출용 주제 중심의 Portfolio Document로 압축 / 재구성한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> 제출용 포트폴리오 문서를 작성할 때
-> 여러 Work List / Bug Report / System Architecture / System Design Records / Engine Technique Document / Engine Implementation Records / Verification Log / PR Document를 하나의 포트폴리오 주제로 묶을 때
-> 작업 기록을 문제 정의 / 시스템 구조 / 엔진 사용 방식 / 설계 판단 / 구현 결과 / 검증 흐름으로 재구성할 때
```

---

## 3. 사용 방법

대상 기술 주제와 입력 문서 목록을 제공하고 `복사용 Prompt`를 사용한다.

---

## 4. 복사용 Prompt

````text
아래 자료를 기반으로 Portfolio Document를 작성 또는 보완해줘.

대상 기술 주제:
- [예: Action / Reaction Execution Pipeline]

관련 입력 문서:
- Work List:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- 기타 참고 문서:

작성 목표:
- 작업 기록을 그대로 나열하지 말고 기술 주제 중심 문서로 재구성해줘.
- 문제 정의 -> 시스템 구조 -> 엔진 사용 방식 -> 설계 판단 -> 구현 결과 -> 검증 / 결과 -> 정리 흐름을 기본으로 사용해줘.
- 코드 타입명, Unreal 용어, pipeline 명칭은 실제 프로젝트 기준을 유지해줘.
- 최신 구조와 특정 Branch 시점의 문서를 섞지 말아줘.
- 확인되지 않은 내용은 확인 필요 또는 미반영으로 분리해줘.

권장 구조:
1. 문서 목적
2. 관련 자료
3. 문제 정의
4. 시스템 구조
5. 엔진 사용 방식
6. 설계 판단
7. 구현 결과
8. 검증 / 결과
9. 정리
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 대상 기술 주제
-> 관련 작업 문서
-> 기준 시점
-> 핵심 코드 / 구조명
-> 검증 결과
```

---

## 6. 출력 기준

```yaml
출력 기준
-> 기술 주제 중심 설명
-> 문제 정의
-> 시스템 구조
-> 엔진 사용 방식
-> 설계 판단
-> 구현 결과
-> 검증 / 결과
-> 관련 문서 연결
```

---

## 7. 범위 / 비범위

```yaml
범위
-> Portfolio Document 작성 / 보완

비범위
-> 개별 PR Document 작성
-> 개별 Bug Report 작성
-> 개별 System Architecture 작성
-> 개별 Engine Technique Document 작성
-> 개별 Architecture / Engine Record 작성
-> 이미지 / 다이어그램 실제 생성
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 작업 기록을 시간순으로 단순 나열하지 않음
-> 검증되지 않은 PIE / Editor / Asset 항목을 완료로 쓰지 않음
-> 최신 구조와 과거 작업 시점을 섞지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 기준 시점을 분리
-> 충돌하는 근거는 검토 의견으로 분리
-> 이미지 필요 여부는 참조 후보로만 제시
```

---

## 10. 검증 기준

```yaml
검증 기준
-> 기술 주제가 명확한가
-> 개별 작업 기록이 주제 중심으로 압축되었는가
-> 검증 결과와 미검증 항목이 분리되었는가
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 기술 주제 중심 문서 구조가 구성됨

실패
-> 입력 문서 부족 또는 기준 시점 충돌

미검증
-> 코드 / 문서 / Asset 확인 필요
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Portfolio Document Writing Prompt
-> 여러 작업 문서를 포트폴리오 제출용 주제로 압축

System Architecture Writing Prompt
-> 현재 시스템 구조 / 책임 경계 / 실행 흐름 문서 작성

Engine Technique Document Writing Prompt
-> Unreal Engine 기능 / API / 시스템 사용 방식 설명
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> Project Overview / Pipeline / Troubleshooting 유형별 세부 구조
```

