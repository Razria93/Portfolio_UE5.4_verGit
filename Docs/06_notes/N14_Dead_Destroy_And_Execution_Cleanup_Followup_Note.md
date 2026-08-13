# N14. Dead Destroy and Execution Cleanup Follow-up Note

## 문서 상태

이 노트에서 처음 제기한 두 안건의 현재 상태는 다음과 같다.

```text
Enemy Dead / Destroy Flow
-> W06에서 구현·에셋 연결·PIE 검증 완료
-> 최신 계약은 S31로 이관

Execution Runtime Cleanup Boundary
-> 별도 후속 검토 유지
```

Dead / Destroy의 구현 기준은 [S31 Enemy Dead / Presentation / Destroy 생명주기 설계](../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)다. 이 노트는 더 이상 Dead 구조의 실행 기준이 아니며, 해결된 안건과 남은 후속 범위를 구분하는 인덱스로 사용한다.

---

## 1. 해결됨: Enemy Dead / Destroy Flow

### 최종 결과

```text
Health Alive -> Dead
-> Enemy Death Lifecycle 시작
-> DeadIn Reaction
-> DeadIn Completed
-> CharacterFeedback Presentation 요청
-> Character / Weapon Dissolve
-> Character Niagara OnSystemFinished
-> 다음 Tick gameplay cleanup / Enemy Destroy
-> EndPlay teardown
```

확정된 핵심 정책은 다음과 같다.

- HealthComponent가 `Alive / Dead` 생명 상태의 단일 원본이다.
- DeadIn은 Reaction, DeadLoop는 AnimBP Locomotion이다.
- CharacterFeedback은 표현 요청과 결과 통지만 담당한다.
- Weapon Dissolve는 같은 Timeline을 따르는 동기 참여자이며 완료 barrier가 아니다.
- 정상 Destroy는 Character Niagara의 자연 완료 이벤트가 요청한다.
- Presentation listener 없음 또는 필수 자원 생성 실패 시에만 fallback delay를 사용한다.
- 정상 Presentation이 시작되면 fallback delay를 해제해 파티클 수명을 자르지 않는다.
- `FinalizeDeath()`는 멱등이며 callback stack 밖의 다음 Tick에서 Destroy한다.
- Destroy 직전 gameplay cleanup과 EndPlay teardown을 분리한다.
- Targeting은 Enemy 정책을 알지 않고 Actor `OnEndPlay`로 현재 Target을 해제한다.
- Player Destroy / Respawn / Revive는 범위에서 제외한다.

### 완료된 검증

```text
- 정상 DeadIn -> Dissolve -> Destroy
- Presentation 미구현 / 시작 실패 fallback
- Character와 SkeletalMesh Weapon Dissolve
- 현재 Target Enemy의 OnEndPlay 해제
- 사망 선행 해제 후 Destroy 중복 이벤트 없음
- 이전 Target Destroy가 새 Target을 해제하지 않음
- Debug Overlay Death 상태와 EventLog
- Development Build
```

세부 구현 파일, 에셋 연결 및 검증 기록은 [W06-01 Task Brief](task_briefs/W06_Enemy_Dead_Destroy_Lifecycle/TB_W06_01_Enemy_Dead_Destroy_Lifecycle_v1.md)에 남긴다.

---

## 2. 후속 유지: Execution Runtime Cleanup Boundary

### 문제

Action / Reaction 전환에는 Montage만이 아니라 다음 runtime 상태가 함께 존재한다.

```text
- active execution context
- weapon collision / hit window
- trail / attachment overlap
- feedback window
- observable overlay
- deferred action
- signal / result dispatch에 필요한 snapshot
```

Stop 또는 Intervention 시점에 모든 데이터를 즉시 지우면 결과 처리와 feedback이 필요한 snapshot을 잃을 수 있다. 반대로 너무 늦게 지우면 stale collision, stale overlay, 중복 result dispatch가 발생할 수 있다.

### 검토해야 할 순서

```text
1. 실행 중단 원인 확정
2. 결과와 feedback에 필요한 immutable snapshot 캡처
3. 신규 hit / action intent 차단
4. collision / hit window / trail 종료
5. result / feedback dispatch
6. active context와 overlay 정리
7. execution state 복구
```

모든 시스템에 이 순서를 강제하는 것이 목적은 아니다. 각 Action / Reaction 유형이 어떤 데이터를 언제까지 필요로 하는지 표로 만든 뒤 공통 계약과 예외를 분리해야 한다.

### 후속 완료 조건

- Complete / Stop / Intervention / unexpected MontageEnd의 cleanup 책임이 구분된다.
- result와 feedback이 cleanup 전후 어느 snapshot을 읽는지 설명할 수 있다.
- collision / trail / overlay가 다음 실행으로 누출되지 않는다.
- Dead / Collapse / Repulse 같은 높은 우선순위 전환에서도 중복 종료가 없다.
- PIE에서 Action interrupt, HitReact, Parry, Dead, Collapse 경계를 검증한다.

### 예상 검토 대상

```text
Source/Portfolio/Action/*
Source/Portfolio/Reaction/*
Source/Portfolio/Component/CActionComponent.*
Source/Portfolio/Component/CReactionComponent.*
Source/Portfolio/Component/CWeaponComponent.*
Source/Portfolio/Component/CApplyDamageComponent.*
Source/Portfolio/Core/Debug/*
```

---

## 3. 관련 문서

- [S26 실행 몽타주 생명주기 결정](../05_System_Architecture/S26_UE5_Portfolio_System_Architecture.md)
- [S31 Enemy Dead / Presentation / Destroy 생명주기 설계](../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)
- [W06 Enemy Dead / Destroy Lifecycle](task_briefs/W06_Enemy_Dead_Destroy_Lifecycle/README.md)
