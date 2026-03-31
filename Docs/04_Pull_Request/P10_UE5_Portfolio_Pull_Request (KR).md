# AI BehaviorTree Core 구현 및 전투/상태 흐름 정리

## 제목

✨ feat: AI BehaviorTree Core 구축 및 Enemy 전투 상태 흐름 정리 (#26)

## 요약

- `CAIController` 생성 이후, Enemy AI의 Blackboard / BehaviorTree 기반 코어 구조를 전반적으로 구축.
  
- AI 상태 전이, 컨텍스트 갱신, Patrol / Investigate / Chase / Alert / Engage / HitReact / Dead 흐름을 BT 중심으로 정리.
  
- 공격은 `Engage` 내부 `SBT_Attack` 서브트리로 구성.


---

## 완료 항목

### 1. AI Controller / Blackboard / BT 코어 구조 구축

- `CAIController` 추가
  
- `Blackboard` / `BehaviorTree` 초기화 구조 구현
  
- `CAIKey` namespace 기반 blackboard key 체계 정리
  
- service 기반 AIState / context 갱신 구조 도입

### 2. AIState / Context 파이프라인 확장

- `CBTService_UpdateAIState`
  
- `CBTService_UpdateAIContext`
  
- target cache / target priority / stale memory timeout 흐름 추가
  
- 상태 전이 기준 정리
  
	- Idle
	  
	- Patrol
	  
	- Investigate
	  
	- Chase
	  
	- Alert
	  
	- Engage
	  
	- HitReact
	  
	- Dead

### 3. Patrol / Investigate / Chase / Alert 흐름 구현

- `CPatrolPoint`, `CPatrolPath` 및 patrol task/service 추가
  
- LastKnownLocation 기반 investigate 흐름 구현
  
- EQS investigate 도입
  
- chase hysteresis 및 alert 단계 task / BT asset 추가

### 4. Engage assignment 및 전투 컨텍스트 구현

- `UCWorldSubsystem_CombatEngage` 추가
  
- engage assignment request / result 흐름 구성
  
- 거리 기반 alert 판정과 assignment 기반 engage 판정을 분리
  
- `UpdateEngageContext`를 통해 `bInEngageRange`, `bCanAttack` 계산

### 5. Combat -> Engage 구조 전환

- `Combat` 중심 네이밍을 `Engage` 기반으로 정리
  
- `Combat` context / key / service / task 의미를 `Engage` 구조로 재정렬
  
- `Attack`은 상위 state가 아니라 `Engage` 내부 subtree로 이동

### 6. Attack subtree 추가

- `SBT_Attack`, `SBT_Engage_Positioning` 추가
  
- `StartAttack`
  
- `WaitAttackEnd`
  
- `CommitAttackCooldown`
  
- `SelectAttackIndex`
  
- `AnimNotify_EndEnemyAttack`
  
- 공격 중 상위 상태를 `Engage`로 유지하도록 `UpdateAIState` 보완

### 7. Reaction / HitReact 흐름 정리

- request-consume-execute reaction flow 재구성
  
- pending / active reaction 분리
  
- reaction priority 정책 추가
  
- `BT_HitReact`, `TryStartReaction`, `WaitEndReaction` 추가
  
- `AnimNotifyState_Reaction` 기반 reaction window 제어

### 8. Dead / Revive 상태 구조 확장

- `bIsDead` -> `Alive / Dying / Dead / Reviving`
  
- `DeadState`를 Blackboard / AnimInstance에 동기화
  
- dead / revive용 BT task 추가
- 
- `AnimNotify_EnterDeadState`, `AnimNotify_EnterAliveState` 추가
- 
- explicit kill / revive entry point 추가
  
- revive health initialization 정책 정리

### 9. Anim / Montage / Weapon / Asset 정리

- Dead / Dying / HitReact / Attack 관련 애니메이션 및 몽타주 정리
  
- sword / unarmed animation naming 정리
  
- AI 전용 sword attack / draw / sheath montage 추가
  
- sword weapon asset을 medieval sword set 기반으로 교체


---

## 테스트 방법

1. Enemy AI가 포함된 테스트 레벨 실행
	
2. Blackboard / BT 초기화 확인
	   
	- `CAIController` possess
	  
	- `BB_Default`, `BT_Default` 실행 여부 확인
	
3. 상태 전이 확인
	   
	- Idle / Patrol / Investigate / Chase / Alert / Engage / HitReact / Dead
	
4. 전투 흐름 확인
	   
	- Alert -> Engage 전이
	  
	- engage assignment 반영
	  
	- `bInEngageRange`, `bCanAttack` 계산 확인

5. Attack subtree 확인
	   
	- `SBT_Attack` 진입
	  
	- `StartAttack`
	  
	- `bIsAttacking == true`
	  
	- `WaitAttackEnd` 유지
	  
	- `AnimNotify_EndEnemyAttack` 정상 종료 확인
	
6. HitReact / Dead / Revive 확인
	   
	- reaction 시작/종료
	  
	- dead phase 전이
	  
	- revive 후 health / state / anim sync 복귀 확인
	
7. 이동 브랜치 제어 확인
	   
	- `CanMove` 데코레이터가 이동 sequence 진입을 제어하는지 확인


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/ai-behaviortree-core`
  
- 이슈: `#26`


---

## 노트

- 현재 movement gating은 단일 `bCanMove` 기반이며, lock ownership 문제는 후속 개선 필요


---
