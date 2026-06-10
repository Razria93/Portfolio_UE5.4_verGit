# Verification Log Prompt

## 1. 목적

작업 후 검증 결과를 Build / Code Flow / PIE / Editor / Asset / 미검증으로 분리해 기록한다.

---

## 2. 사용 시점

```yaml
사용 시점
-> 작업 완료 후 검증 결과를 정리할 때
-> PR Document 또는 Work List에 검증 상태를 연결해야 할 때
-> 미검증 항목을 완료 항목과 분리해야 할 때
```

---

## 3. 사용 방법

대상 작업과 수행한 검증 내용을 제공하고 `복사용 Prompt`를 사용한다.

---

## 4. 복사용 Prompt

````text
아래 작업의 검증 결과를 Verification Log 형태로 정리해줘.

대상 작업:
- [Branch / PR / commit range / 작업 요약]

관련 문서:
- Work List:
- Bug Report:
- System Architecture:
- System Design Records:
- Engine Technique Document:
- Engine Implementation Records:
- Verification Log:
- PR Document:
- Portfolio Technical Document:
- Code Review:

검증 목표:
- 실제 수행한 검증만 완료로 기록해줘.
- 코드 읽기 기반 확인은 Build나 PIE가 아니라 Code Flow로 분리해줘.
- Build / PIE / Editor / Asset 검증은 서로 대체하지 말고 별도 항목으로 기록해줘.
- 실패한 검증은 실패 원인, 관련 로그, 다음 조치를 분리해줘.
- 확인하지 못한 항목은 미검증으로 남겨줘.

출력 형식:

Verification Log

Build
- 상태:
- 명령:
- 결과:
- 비고:

Code Flow
- 상태:
- 확인 범위:
- 결과:
- 비고:

PIE
- 상태:
- 확인 범위:
- 결과:
- 비고:

Editor
- 상태:
- 확인 범위:
- 결과:
- 비고:

Asset
- 상태:
- 확인 범위:
- 결과:
- 비고:

미검증
- [확인하지 못한 항목]
````

---

## 5. 입력 기준

```yaml
입력 기준
-> 작업 요약
-> 수행한 검증
-> 실패한 검증
-> 확인하지 못한 검증
-> 관련 문서
```

---

## 6. 출력 기준

```yaml
출력 기준
-> Build
-> Code Flow
-> PIE
-> Editor
-> Asset
-> 미검증
```

---

## 7. 범위 / 비범위

```yaml
범위
-> 검증 결과 기록

비범위
-> 검증 실행
-> 코드 리뷰
```

---

## 8. 제약 조건

```yaml
제약 조건
-> 수행하지 않은 검증을 완료로 표현하지 않음
-> Code Flow 확인을 Runtime 검증으로 쓰지 않음
-> Build 성공을 Editor / Asset 검증 완료로 쓰지 않음
```

---

## 9. 모호성 처리 기준

```yaml
모호한 경우
-> 확인한 범위와 확인하지 못한 범위를 분리
-> 사용자가 확인해야 하는 항목을 미검증으로 남김
```

---

## 10. 검증 기준

```yaml
검증 기준
-> 실제 수행한 검증만 완료로 기록되었는가
-> 미검증 항목이 구체적인가
```

---

## 11. 완료 / 실패 / 미검증 처리 기준

```yaml
완료
-> 검증 상태가 항목별로 분리됨

실패
-> 검증 결과 근거 확인 불가

미검증
-> 현재 환경에서 확인하지 못한 항목
```

---

## 12. 기존 Prompt와 역할 경계

```yaml
Verification Log Prompt
-> 검증 결과 기록

Asset Blueprint Validation Prompt
-> Editor / Asset / Blueprint 확인 항목 점검
```

---

## 13. 계속 수정할 항목

```yaml
후속 보완 후보
-> 검증 로그 문서 양식
```
