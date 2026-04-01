# Player Combat Receiver 구축 및 전투 수신 루프 연결

## 제목

`✨ feat: Player Combat Receiver 구축 및 전투 수신 루프 연결 (#32)`

## 요약

- 본 PR은 Player를 공격 전용 객체가 아니라 **전투 수신 가능한 엔티티**로 확장하고, `TakeDamage -> Health -> Reaction -> Dead` 기본 루프를 실제로 닫는 작업을 포함함.
  
- `ACPlayer`에 `TakeDamageComponent`, `HealthComponent`, `ReactionComponent`를 연결하고, AI 공격이 Player에게 정상적으로 적용되도록 수신 진입점을 구성함.
  
- Enemy 공격의 hit context 누락 문제를 함께 보완하여, 공격 인덱스와 액션 타입이 Player 수신 파이프라인까지 정상 전달되도록 정리함.
  
- 또한 dead-state lifecycle과 state sync를 보완하고, 관련 이슈 / 아키텍처 문서를 함께 정리함.


---

## 완료 항목

### 1. Player Combat Receiver 연결

- `ACPlayer`에 `TakeDamageComponent` 추가
  
- `ACPlayer`에 `HealthComponent` 추가
  
- `ACPlayer`에 `ReactionComponent` 추가
  
- Player 생성자 기준 컴포넌트 초기화 순서 정리
  
- `ACPlayer::TakeDamage()` 오버라이드 추가
  
- `TakeDamageComponent` 경유 damage 처리 흐름 연결

### 2. Player 피격 / Reaction 루프 연결

- `ACPlayer::Tick()`에서 pending reaction 소비 루프 추가
  
- `TryConsumePendingReaction()` + `TryExecuteReaction()` 기반 Player reaction 실행 연결
  
- Player 피격 시 `HitReact` 진입 확인
  
- 연속 피격 시 reaction replace / interrupt 흐름 확인

### 3. Health / Dead lifecycle 보완

- `UCHealthComponent::IsAlive()` / `IsDead()` 추가
  
- dead-state 변경 경로를 `ChangeDeadState()`로 통일
  
- `OnDeadStateChanged` delegate 추가
  
- `HealthComponent -> StateComponent` dead-state sync 연결
  
- `Alive` 이외 상태를 dead 계열 gameplay state로 처리하도록 정리

### 4. Player 입력 차단 정책 연결

- `CanActionInput()` 추가
  
- `Reaction` 상태에서 공격 / 장비 전환 입력 차단
  
- `IsAlive()` 기준으로 이동 / 액션 입력 차단
  
- `StopJump`는 release 성격을 고려하여 별도 차단 없이 유지

### 5. Enemy 공격 컨텍스트 보완

- `CBTTask_StartAttack`에 `AttackActionType` 추가
  
- Enemy 공격 시작 시 `WeaponComponent->PushContextToAttachment()` 호출
  
- Enemy 공격이 `Attachment / Equipment / Action / Index` 정보를 포함한 hit context를 Player에게 전달하도록 수정
  
- `AttackActionType == EActionType::Max` 시 런타임 가드 추가

### 6. State API 및 로그 정리

- `SetIdleMode / SetActionMode / SetReactionMode` 등을 `SetIdleState / SetActionState / SetReactionState`로 정리
  
- `PrintStateChangedInfo()` 분리
  
- state 전환 로그 출력 추가

### 7. 문서화

- `D12_UE5_Portfolio_Issue_Checklist (KR)` 업데이트
  
- `D12_UE5_Portfolio_Issue_Checklist (EN)` 업데이트
  
- `S02_UE5_Portfolio_System_Architecture (KR)` 추가
  
- `S02_UE5_Portfolio_System_Architecture (EN)` 추가
  
- `B04_UE5_Portfolio_Bug_Report (KR)` 추가
  
- `B04_UE5_Portfolio_Bug_Report (EN)` 추가


---

## 테스트 방법

1. Enemy가 Player를 인지하고 공격하도록 유도
  
2. 첫 타격 시 Player가 HP 감소와 함께 `Reaction` 상태에 진입하는지 확인
  
3. 연속 피격 시 `HitReact`가 interrupt / replace 정책에 따라 정상 재생되는지 확인
  
4. 누적 피격으로 HP가 0이 되면 `DeadState`가 `Alive -> Dying -> Dead`로 전이되는지 확인
  
5. Dead 상태 진입 이후 추가 피격이 `FinalAppliedDamage = 0`으로 무효 처리되는지 확인
  
6. Player 입력 확인
  
	- `Reaction` 중 공격 입력 차단
	  
	- `Dead` 상태에서 이동 / 액션 입력 차단
	  
	- `StopJump`는 정상 release 처리 유지
  
7. Enemy 공격 로그에서 hit context 확인
  
	- `AttachmentType`
	  
	- `EquipmentType`
	  
	- `ActionType`
	  
	- `ActionIndex`


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/player-combat-receiver`
  
- 이슈: `#32`


---

## 노트

- 이번 PR은 Player를 전투 수신 가능한 엔티티로 편입하는 작업에 집중하며, Player 공격 루프 자체는 후속 이슈에서 별도로 정리 예정임.
  
- 현재 구조에서는 `StateComponent`가 dead-state sync까지 일부 담당하고 있으나, 이후 `Combat Core Shared` 작업에서 상태 축 분리와 `StateComponent` 재정의가 필요할 가능성이 높음.
  
- `TakeDamageComponent`는 장기적으로 `CombatReceiver` 계층으로 확장될 수 있으나, 본 PR에서는 기능 연결과 루프 검증을 우선함.


---
