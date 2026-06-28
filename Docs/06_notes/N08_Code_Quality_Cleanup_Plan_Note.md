# N08. Code Quality Cleanup Plan Note

## 목적

이 노트는 외부 리뷰 피드백과 현재 `Source/Portfolio` 코드 스캔 결과를 합쳐, 코드 품질 개선 방향을 고정하기 위한 기준 문서다.

현재 우선순위는 기능 추가가 아니라 코드 품질 개선이다.

```text
이번 노트의 역할
-> 외부 리뷰에서 나온 지적 정리
-> 코드 스캔에서 확인한 실제 위험 정리
-> 두 항목이 겹치는 우선 작업 고정
-> 기능 회귀 위험이 있는 작업 분리
```

---

## 1. Review Feedback 요약

### 1.1 17 / 18회차

```text
핵심 피드백
-> 구조와 문서보다 실제 게임과 결과물이 보여야 한다.
-> 최근 작업 흐름과 PR 단위 설명이 중요하다.
-> 무엇을 왜 고쳤고, 결과가 어떻게 좋아졌는지 드러나야 한다.
```

코드 품질 관점에서는 PR 단위가 중요하다. 큰 리팩터링을 하나로 묶으면 리뷰가 어렵고, 개선 의도가 흐려진다.

### 1.2 22회차

```text
핵심 피드백
-> 기술문서가 너무 길고 추상적이다.
-> 구현 증거, 검증 과정, 다이어그램, 영상 링크가 부족하다.
-> TODO가 많으면 미완성 포트폴리오로 보일 수 있다.
-> AI 활용 사례는 "AI가 해줬다"처럼 보이면 약점이 된다.
-> GAS를 안 쓰는 이유는 명확히 설명해야 한다.
```

코드 품질 관점에서는 문서가 말하는 구현 상태와 코드 상태가 일치해야 한다.

### 1.3 24회차

```text
핵심 피드백
-> 채용자는 프로젝트 규모보다 본인이 무엇을 했는지 본다.
-> 문제 해결 경험이 중요하다.
-> Interface / Component / Inheritance / DataAsset 선택 이유를 설명할 수 있어야 한다.
-> UE 기본기, UObject / Actor / Component / UPROPERTY / GC / Tick 최적화 이해가 드러나야 한다.
```

코드 품질 관점에서는 Unreal 기본기가 실제 코드에 반영되어야 한다.

### 1.4 25회차

```text
핵심 피드백
-> 구조 설계는 괜찮지만 견고함이 부족하다.
-> check 기반 컴포넌트 검증은 릴리즈 빌드와 원인 추적 측면에서 위험하다.
-> 컴포넌트 참조는 lazy caching / ensure / dependency injection 등 장단점을 판단해야 한다.
-> 현재 프로젝트는 필수 컴포넌트를 기대하는 구조이므로 injection이 설득력 있다.
-> const 일관성, 하드코딩 값, TODO, legacy input, AI update interval이 코드 품질 리스크다.
-> 리팩터링은 작은 PR 단위로 나누어야 한다.
```

25회차는 직접 코드 리뷰 성격이 가장 강하므로 W05의 우선순위 기준으로 사용한다.

---

## 2. 코드 스캔에서 확인한 위험

### 2.1 UObject 참조 안정성

`UObject` / `AActor` / Component raw pointer가 헤더 전반에 존재한다.

```text
주요 후보
-> Action / Reaction executor
-> Player / Enemy component fields
-> AIController asset / perception fields
-> WeaponActor component / owner fields
-> AnimInstance cached component fields
-> Combat / Feedback component cached fields
```

모든 raw pointer가 잘못은 아니다. 다만 UObject 계열 포인터는 소유 / 강한 참조 / 약한 참조 / 일시 참조 기준을 코드에서 설명할 수 있어야 한다.

### 2.2 Component lookup / check 사용

`check`, `FindComponentByClass`, `GetComponentByClass`가 주요 초기화 경로에 넓게 존재한다.

```text
위험
-> 개발 빌드에서는 즉시 crash
-> Shipping 빌드에서는 check 제거 가능
-> 이후 null 접근 시 원인 추적 어려움
-> Blueprint에서 컴포넌트 누락 시 발견 지점이 늦어질 수 있음
-> 컴포넌트 수가 늘어날수록 탐색 / 의존성 관리 비용 증가
```

현재 구조는 "있으면 사용한다"보다 "반드시 있어야 한다"에 가깝다. 따라서 필수 의존성은 injection 또는 명시적 validation으로 정리하는 것이 맞다.

### 2.3 Debug log / print 함수

`FLog::Log`, `Print...Info`, `SummaryInfo` 계열 함수가 많다.

```text
위험
-> Debug dump와 error log가 섞임
-> Hot path에서 호출될 경우 성능 영향 가능
-> Shipping 빌드 포함 여부 기준 불명확
-> 로그가 많으면 실제 문제 신호가 묻힘
```

로그를 없애는 것이 목표가 아니라, 로그의 목적과 활성화 조건을 분리하는 것이 목표다.

### 2.4 TODO / 구현 상태 불일치

CombatSignalTarget, Health delegate, Feedback loop, Weapon spawn, DataAsset 분리 등 핵심 흐름 주변 TODO가 남아 있다.

```text
위험
-> 문서에서는 구현 완료처럼 보이는데 코드에는 TODO가 남음
-> "기본을 왜 안 지켰지"라는 인상을 줄 수 있음
-> TODO가 기능 미완성과 설계 보류를 구분하지 못함
```

TODO는 제거하거나 Phase / 이유 / 후속 브랜치를 명시해야 한다.

### 2.5 Data-driven 주장과 하드코딩 값

AI sight, movement speed, BT service interval, combat multiplier 등이 코드에 직접 존재한다.

```text
위험
-> data-driven 문서와 실제 코드 불일치
-> 튜닝을 위해 리컴파일 필요
-> 값의 의미를 찾기 어려움
```

바로 모든 값을 DataAsset으로 옮기지 않는다. 먼저 상수명 / config / DataAsset 후보를 분류한다.

### 2.6 Naming / typo / API consistency

`ReactionExcutor`, `Seperate`, `Deffered`, `InValid` 같은 오타가 존재한다.

```text
위험
-> 포트폴리오 검증 부족 인상
-> public API 오타는 오래 남을수록 수정 비용 증가
-> 문서와 코드 용어 불일치 가능
```

Blueprint / asset reference가 얽힌 rename은 별도 확인이 필요하다.

### 2.7 AI update interval / performance

AI BT Service 다수가 `0.1s`, `0.2s` interval로 동작한다.

```text
위험
-> 적 수 증가 시 비용이 선형 이상으로 체감될 수 있음
-> 모든 계산이 같은 주기로 돌아감
-> 성능 설명 / 검증 자료 부족
```

바로 최적화하지 않고, 먼저 측정 기준과 비용 분리 후보를 정한다.

---

## 3. 우선순위 판단

### 3.1 P0

```yaml
P0:
  - Unreal Reference Safety
  - Component Reference / Initialization Policy
  - Debug / Logging Policy
  - TODO / 미완성 신호 정리
  - Hardcoded tuning value 1차 정리
  - Naming / typo quick cleanup
```

P0는 외부 리뷰 지적과 코드 스캔 결과가 겹치며, 기능 추가 없이 포트폴리오 신뢰도를 바로 올릴 수 있는 항목이다.

### 3.2 P1

```yaml
P1:
  - Const / read-only API consistency
  - AI Blackboard key registry
  - AI update interval audit
  - Combat cue API naming consistency
```

P1은 구조 설득력을 높이지만, 일부는 수정 범위가 커질 수 있으므로 P0 이후 진행한다.

### 3.3 P2

```yaml
P2:
  - Enhanced Input migration
  - DataAsset 전면 전환
  - Player / Enemy combat receiver 공통화
  - Orchestrator shared flow extraction
  - Combat data type header split
  - Blink / Repulse / ResultOut implementation
  - GAS migration / adapter 검토
```

P2는 기능 회귀 또는 범위 확장 위험이 있으므로 1차 코드 품질 작업에서 제외한다.

---

## 4. 브랜치 분리 기준

```text
한 브랜치에 하나의 리뷰 질문만 담는다.
```

예시는 다음과 같다.

```yaml
좋은 브랜치
- refactor/unreal-reference-safety-v1
- refactor/debug-log-policy-v1
- refactor/api-const-consistency

나쁜 브랜치
- refactor/code-cleanup-all
- refactor/code-quality-fixes
- feature/guard-parry-action-refactor
```

Guard / Parry / Action / AI / Feedback을 한 PR에 묶으면 리뷰 포인트가 흐려진다.

---

## 5. 문서 업데이트 기준

코드 변경 후 문서 업데이트는 다음 기준으로 판단한다.

```yaml
문서 업데이트 필요:
  - public API 이름이 바뀐다
  - component responsibility가 바뀐다
  - 구현 상태 / Phase 상태가 바뀐다
  - PR에서 review feedback 대응을 설명해야 한다

문서 업데이트 불필요:
  - 내부 typo만 수정한다
  - const만 추가한다
  - 로그 gate만 추가하고 외부 동작이 바뀌지 않는다
```

단, PR 문서에는 모든 변경의 검증 결과를 남긴다.

---

## 6. Prompt update 후보

이번 W05 자체로 즉시 prompt를 수정하지 않는다. 실제 코드 작업을 진행한 뒤 아래 후보를 검토한다.

```yaml
후보
- Code Review Prompt에 Unreal reference safety 체크 추가
- Refactor Work Planning Prompt에 branch split 기준 추가
- Document Writing Prompt에 구현됨 / TODO / Future Work 구분 강화
- PR Document Prompt에 review feedback 대응 항목 추가
```

---

## 7. 현재 결론

첫 번째 코드 작업은 다음 중 하나로 시작한다.

```text
1순위: refactor/unreal-reference-safety-v1
2순위: refactor/component-reference-validation-policy
```

둘 다 P0이지만, `UPROPERTY` / GC는 UE C++ 기본기 질문과 직접 연결되고 수정 범위도 비교적 명확하므로 먼저 진행한다.

그 다음 `check` / component reference policy를 정리한다.
