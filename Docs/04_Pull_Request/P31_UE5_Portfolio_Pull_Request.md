# UE5 Portfolio Pull Request

## 제목

**P31: Component Lifecycle Cleanup Policy**

## 날짜

**2026.06.30**

## 상태

- [ ] 준비 중

---

## 브랜치

- `refactor/component-lifecycle-cleanup-policy`

---

## 커밋

```text
TBD
```

---

## 요약

이번 PR은 actor / component lifecycle cleanup 기준을 정리한다.

P29에서는 character component reference 주입과 복구 흐름을 정리했고, P30에서는 runtime lookup을 DI 대상과 구분했다. 이번 작업에서는 그 다음 단계로 `BeginPlay` / `EndPlay` / `OnPossess` / `OnUnPossess` / delegate / timer / spawned actor / runtime cache 정리 기준을 점검한다.

핵심 목표는 cleanup 책임이 있는 지점을 식별하고, 코드와 문서에서 설명 가능한 lifecycle 기준을 남기는 것이다.

---

## 작업 배경

P30과 F05에서 WeaponActor의 runtime actor / collision / trail cleanup 흐름을 정리하면서, 더 넓은 lifecycle cleanup 기준이 필요하다는 점이 드러났다.

특히 다음 질문에 답할 수 있어야 한다.

```text
BeginPlay에서 bind한 delegate는 어디서 해제되는가?
OnPossess에서 만든 controller runtime state는 OnUnPossess에서 정리되는가?
timer로 변경한 actor state는 world teardown에서도 복구 또는 정리되는가?
spawned actor의 Destroy 책임은 어느 component가 가지는가?
gameplay cleanup과 object teardown cleanup은 같은 API로 처리해도 되는가?
```

---

## 사전 조회 결과

```text
Lifecycle hook 보유 파일: 11개
Delegate / Timer 사용 파일: 8개
SpawnActor / NewObject 생성 경로: 3개
Runtime cleanup 명명 사용처: 약 20개+
```

---

## 작업 범위

### 1. Lifecycle 사용처 전수 확인

대상:

```text
BeginPlay
EndPlay
NativeInitializeAnimation
NativeUninitializeAnimation
OnPossess
OnUnPossess
```

확인 기준:

```text
- setup / bind / cache가 있는지
- 대응 cleanup이 있는지
- 없는 경우 의도적으로 없어도 되는지
```

### 2. Delegate / Timer cleanup 점검

대상:

```text
AddDynamic
AddUniqueDynamic
AddUObject
RemoveDynamic
RemoveAll
SetTimer
ClearTimer
```

우선 후보:

```text
ACAIController
UCWorldSubsystem_CombatFeedback
```

### 3. Spawned actor / executor ownership 점검

대상:

```text
SpawnActor
Destroy
NewObject
```

우선 후보:

```text
UCWeaponComponent
UCActionComponent
UCReactionComponent
```

`UCWeaponComponent`는 P30/F05에서 보강한 흐름을 기준으로 확인한다. `UCActionComponent` / `UCReactionComponent`는 executor lifecycle이 gameplay execution과 연결되므로 execution 흐름 영향도를 별도로 판단한다.

### 4. Cleanup 명명 기준 정리

구분:

```text
Gameplay Runtime Cleanup
-> action / reaction / feedback / guard 같은 실행 중 임시 상태 정리

Lifecycle Teardown Cleanup
-> EndPlay / OnUnPossess / NativeUninitializeAnimation / subsystem teardown에서 delegate, timer, spawned actor, cache 정리
```

이번 PR은 `Lifecycle Teardown Cleanup` 기준을 우선 정리한다.
`Gameplay Runtime Cleanup`은 현재 Action / Reaction interrupt 흐름 위에서 동작하고 있으며, ResultOut / Repulse처럼 외부 결과 전달 사례가 생기면 snapshot, cleanup 순서, result dispatch 시점을 후속 작업에서 다시 검토한다.

Dead 이후 Actor Destroy 프로세스는 후속 작업으로 분리하고, 이번 PR에서는 Destroy flow가 추가될 때 사용할 수 있는 `EndPlay`, `OnUnPossess`, subsystem teardown, delegate / timer / spawned actor cleanup hook의 책임을 정리한다.

Dead Destroy Flow와 Execution Runtime Cleanup Boundary는 후속 노트에서 별도 추적한다.

---

## 예상 수정 후보

```text
Source/Portfolio/Controller/CAIController.*
Source/Portfolio/System/Combat/CWorldSubsystem_CombatFeedback.*
Source/Portfolio/Component/CActionComponent.*
Source/Portfolio/Component/CReactionComponent.*
Docs/06_notes/N13_Component_Lifecycle_Cleanup_Policy_Note.md
Docs/01_Work_List/W05_Code_Quality_Plan/W05_UE5_Portfolio_Work_List.md
```

코드 변경은 조회 결과에서 수정 필요가 확인된 범위에 반영한다.

---

## 제외 범위

```text
- Action / Reaction 실행 종료 정책 재설계
- montage stop 정책 변경
- Guard / Reaction runtime cleanup 재설계
- 모든 _Injected reference를 EndPlay에서 일괄 null 처리
- Blink / Repulse / ResultOut 구현
- GAS 대응
```

---

## 검증 계획

```text
rg 기반 lifecycle / delegate / timer / spawn 사용처 전수 확인
git diff --check
PortfolioEditor Win64 Development 빌드
PIE 기본 combat loop smoke test
```

---

## 관련 문서

```text
Docs/06_notes/N13_Component_Lifecycle_Cleanup_Policy_Note.md
Docs/06_notes/N14_Dead_Destroy_And_Execution_Cleanup_Followup_Note.md
Docs/06_notes/N12_Runtime_Component_Lookup_Policy_Note.md
Docs/04_Pull_Request/P30_UE5_Portfolio_Pull_Request.md
Docs/04-02_Fix_Pull_Request/F05_UE5_Portfolio_Pull_Request_Fix.md
```

---
