# UE5 Portfolio Pull Request

## 제목

**P48: CPP Include Order Cleanup**

## 날짜

**2026.07.24**

## 상태

- [x] `.cpp` include ordering 정규화
- [x] `.cpp` matching header first 기준 유지
- [x] `ProjectGlobal.h` 위치 기준 유지
- [x] project header / Unreal Engine header 배치 정리
- [x] AI BehaviorTree 일부 미사용 include 제거
- [x] `.h` / `generated.h` / UHT 대상 선언 변경 없음
- [x] include order 재스캔 통과
- [x] `git diff --check` 통과
- [x] `PortfolioEditor Win64 Development` build 통과
- [ ] PIE smoke 미실행

## 브랜치

- `refactor/include-order-cleanup`

## 요약

이번 PR은 P46 Type Header Organization 이후 남아 있던 `.cpp` include 배치 흔들림을 정리한다.

기존 구현 파일 일부는 Unreal / Engine header 뒤에 project header가 다시 나오는 형태가 남아 있었다. 이 구조는 빌드 동작을 직접 바꾸지는 않지만, include dependency를 점검할 때 실제 의존성과 단순 배치 문제가 섞여 보이게 만든다.

이번 PR에서는 `.cpp` include block을 규칙에 맞춰 정렬하고, 에이전트 교차 검토와 빌드 검증을 거친 AI BehaviorTree 일부 미사용 include만 제거했다. `.h`, UHT 대상 선언, 타입명, 필드명, gameplay 실행 흐름은 변경하지 않는다.

## 변경 배경

P46 / P47에서 Type 헤더 책임과 타입 의미 정리는 완료했지만, 구현 파일의 include 배치는 아직 파일마다 다른 관성이 남아 있었다.

특히 `.cpp`에서 다음 순서가 흔들리는 케이스가 있었다.

```text
matching header
-> ProjectGlobal.h
-> project internal headers
-> Unreal / Engine headers
```

이 PR은 다음 header dependency 축소 작업으로 넘어가기 전에 `.cpp` include block의 기준선을 먼저 맞추는 작업이다.

## 변경 범위

### 1. `.cpp` include order 정규화

왜:

구현 파일마다 include group 순서가 다르면, 실제 dependency 문제와 단순 정렬 차이가 같은 diff에 섞인다.

어떻게:

`Source/Portfolio` 하위 `.cpp` 파일의 include block을 다음 기준으로 정리했다.

```text
1. Matching header
2. ProjectGlobal.h
3. Project internal headers
4. Unreal / Engine headers
5. ThirdParty / STL headers
```

결과:

`main...HEAD` 기준 106개 `.cpp` 파일의 include block이 정리됐다.

### 2. AI BehaviorTree 미사용 include 제거

왜:

include order 정리 이후 일부 AI BehaviorTree 구현 파일에서 본문 직접 사용이 없는 include가 남아 있었다.

어떻게:

컨텍스트 없는 에이전트 검토와 로컬 확인을 거친 뒤, 빌드 검증 가능한 범위에서 6개 파일의 include 11개만 제거했다.

```text
CBTServiceIntervalHelper.cpp
CBTService_UpdateAIIntentState.cpp
CBTTask_ClearFocus.cpp
CBTTask_SetFocus.cpp
CBTTask_StartReaction.cpp
CBTTask_WaitEndReaction.cpp
```

결과:

AI BehaviorTree 구현 파일 일부의 직접 의존성이 줄었고, 변경 후 `PortfolioEditor Win64 Development` build가 통과했다.

### 3. header dependency 축소 범위 제외

왜:

`.h` include 축소와 forward declaration 전환은 UHT, Blueprint 노출 타입, by-value struct return, transitive include 의존성에 영향을 줄 수 있다.

어떻게:

이번 PR에서는 `.h`를 건드리지 않고, 명확한 `.cpp` include order와 검증된 일부 미사용 include 제거만 처리했다.

결과:

header hygiene 후보는 후속 pass로 분리됐다.

## 주요 처리 흐름

```text
Source/Portfolio .cpp include block 스캔
-> matching header / ProjectGlobal / project / engine group 기준 적용
-> include order 재스캔
-> 과밀 include 후보 에이전트 검토
-> AI BehaviorTree 일부 미사용 include만 제거
-> git diff --check
-> PortfolioEditor Win64 Development build
```

## 구현 결과

```text
main...HEAD
-> 106 files changed, 274 insertions(+), 229 deletions(-)

commit range
-> 92e4d40c style(include): normalize cpp include ordering
-> 2441e4fb refactor(ai): remove unused behavior tree includes
```

이번 PR에서 변경한 범위는 `Source/Portfolio` 하위 C++ 구현 파일의 include block이다.

```text
변경함:
-> .cpp include group 정렬
-> AI BehaviorTree 일부 미사용 include 제거

변경하지 않음:
-> .h include 축소
-> generated.h 위치
-> USTRUCT / UENUM / UPROPERTY
-> 타입명 / 필드명 / enum entry
-> gameplay logic
-> CombatSignal reserved scaffold
-> Feedback key model
```

## 테스트 방법

```text
1. git diff --stat main...HEAD
2. git log --oneline main..HEAD
3. include order 재스캔
4. git diff --check main...HEAD
5. PortfolioEditor Win64 Development build
6. 에이전트 교차 검토로 상위 include 과밀 후보 확인
```

## 검증 결과

### Branch scan

```text
Branch:
refactor/include-order-cleanup

main 대비 변경:
106 files changed, 274 insertions(+), 229 deletions(-)
```

### Include order scan

```text
Matching header first violations:
0

ProjectGlobal position violations:
0

.cpp project-after-engine violations:
0

duplicate includes:
0

header / UHT 변경:
없음
```

### Static check

```text
git diff --check main...HEAD
Result: Pass
```

### Build

```text
PortfolioEditor Win64 Development
Result: Pass
```

### Review

```text
Context-unaware agent review
Result: Must fix now 항목 없음

추가 optional 후보:
-> 일부 .cpp include 삭제 후보
-> 일부 .h forward declaration 후보

판정:
-> 이번 PR 범위에서는 제외
```

## 비범위 / 후속 작업

- `.h` include 축소와 forward declaration 전환
- IWYU 수준의 include dependency 재설계
- `ProjectGlobal.h` 제공 기능 축소
- Feedback MatchKey / PlaybackKey 구조 변경
- CombatSignal generic pipeline scaffold 제거
- PIE smoke

## 관련 문서

- Work List: `W05_UE5_Portfolio_Work_List.md`
- Type Header Rules: `W05_Type_Header_Organization_Rules.md`
- Type Header Work Plan: `W05_Type_Header_Organization_Work_Plan.md`
- Previous PR: `P46_UE5_Portfolio_Pull_Request.md`
- Previous PR: `P47_UE5_Portfolio_Pull_Request.md`

## 정리

이번 PR은 `.cpp` include 배치를 규칙에 맞춰 정렬하고, 검증된 AI BehaviorTree 미사용 include만 제거한다.

동작 변경, 타입 rename, public header 재설계는 포함하지 않았다. 이후 header dependency 축소는 UHT / Blueprint / transitive include 영향을 별도 검증하는 후속 pass에서 다룬다.
