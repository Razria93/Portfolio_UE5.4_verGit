# UE5 Portfolio Pull Request

## 제목

**P28: Unreal 참조 안전성 v1**

## 날짜

**2026.06.28**

## 상태

- [x] **완료**

---

## 브랜치

- `refactor/unreal-reference-safety-v1`

---

## 요약

이번 PR은 `Source/Portfolio`의 UObject 참조 중 UHT / GC / runtime lifetime 관점에서 바로 설명이 필요한 부분을 작게 정리한다.

핵심은 모든 raw pointer를 한 번에 교체하는 것이 아니라, 명확한 안전성 이슈와 리뷰 질문이 생길 수 있는 필드만 우선 보완하는 것이다.

---

## 변경 배경

W05 코드 품질 정리 계획에서 첫 번째 코드 작업으로 `Unreal Reference Safety`를 선택했다.

UE C++에서는 UObject 참조가 다음 기준을 명확히 가져야 한다.

```text
- Unreal reflection / GC가 알아야 하는 필드인가
- 런타임 캐시지만 저장되면 안 되는 필드인가
- 참조 대상 actor를 살려두면 안 되는 임시 추적 데이터인가
- raw pointer가 함수 경계 / payload / local access 수준에서만 쓰이는가
```

이번 PR은 이 기준을 문서화하고, 현재 코드에서 바로 수정 가능한 작은 범위에 적용한다.

---

## 변경 범위

### 1. HealthComponent cached owner 참조 보완

`UCHealthComponent::OwnerActor_Cached`에 `UPROPERTY(Transient)`와 `nullptr` 초기화를 추가했다.

```text
변경 전
-> raw AActor* member
-> UPROPERTY 없음
-> 기본 초기화 없음

변경 후
-> UPROPERTY(Transient)
-> nullptr 초기화
```

이 필드는 런타임에 `GetOwner()` 결과를 캐시하는 필드다. 저장 대상은 아니지만 Unreal GC / reflection 관점에서 명확한 런타임 UObject 참조로 보이도록 정리했다.

### 2. CombatFeedback hit-stop actor 추적을 weak reference로 변경

`UCWorldSubsystem_CombatFeedback`의 hit-stop 추적 맵을 raw actor key에서 `TWeakObjectPtr<AActor>` key로 변경했다.

```text
ActiveHitStopMap
CachedTimeDilationMap
```

hit-stop 대상 actor는 subsystem이 소유하는 객체가 아니다. 타이머 대기 중 actor가 먼저 파괴될 수 있으므로, 추적 맵이 actor를 붙잡거나 raw pointer 보관 의미를 주지 않도록 weak reference를 사용한다.

`RestoreHitStop`도 `TWeakObjectPtr<AActor>`를 받아 실행 시점에 actor 유효성을 다시 확인한다.

### 3. CombatSignalSource duplicate hit target cache를 weak reference로 변경

`UCCombatSignalSourceComponent::DamagedTargetContainer`의 target set을 `TSet<TWeakObjectPtr<AActor>>`로 변경했다.

이 컨테이너는 hit window 안에서 이미 damage를 보낸 target을 추적하는 임시 cache다. target actor를 소유하거나 수명을 연장하는 것이 목적이 아니므로 weak reference가 더 적절하다.

### 4. UObject reference safety 기준 문서 추가

다음 노트를 추가했다.

```text
Docs/06_notes/N09_Unreal_Reference_Safety_Policy_Note.md
```

문서에는 다음 기준을 정리했다.

```text
- UPROPERTY 사용 기준
- Transient runtime cache 기준
- TObjectPtr 사용 후보
- TWeakObjectPtr 사용 후보
- raw pointer 허용 범위
- TSoftObjectPtr / TSoftClassPtr 사용 기준
- UObject에 일반 스마트 포인터를 사용하지 않는 기준
- timer / delegate payload 기준
```

### 5. Code Quality Note 연결

`N08_Code_Quality_Cleanup_Plan_Note`의 UObject 참조 안정성 항목에서 세부 기준 문서 `N09`를 참조하도록 갱신했다.

---

## 검증

### 빌드

```text
PortfolioEditor Win64 Development
```

결과:

```text
성공
```

### 정적 확인

```text
git diff --check
```

결과:

```text
성공
```

### 필드 스캔

헤더의 raw UObject pointer member 중 즉시 `UPROPERTY`가 없는 후보를 재스캔했다.

결과:

```text
No missing immediate UPROPERTY candidates for raw UObject pointer fields.
```

---

## 제외 범위

이번 PR에서는 다음 작업을 의도적으로 제외한다.

```text
- 프로젝트 전체 TObjectPtr migration
- 모든 cached pointer의 nullptr 초기화 통일
- FCombatSignalHitWindowKey::DamageCauser raw actor key 구조 변경
- Component reference validation / check / ensure 정책 변경
- Debug log gate 구현
- Blink / Repulse / ResultOut 기능 구현
```

`FCombatSignalHitWindowKey::DamageCauser`는 현재 raw actor key를 유지한다. 해당 key를 weak reference 또는 stable id로 바꾸면 hash / equality 의미가 바뀔 수 있으므로 별도 브랜치에서 다룬다.

---

## 후속 작업

권장 후속 순서는 다음과 같다.

```text
1. refactor/component-reference-validation-policy
2. refactor/cached-pointer-default-init
3. refactor/debug-log-policy-v1
4. refactor/todo-status-cleanup
```

이번 PR은 W05의 첫 번째 코드 품질 브랜치로, Unreal reference safety 기준을 코드와 문서 양쪽에 반영하는 것을 완료 조건으로 한다.
