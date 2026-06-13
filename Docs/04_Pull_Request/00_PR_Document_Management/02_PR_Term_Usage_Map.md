# Term Usage Map

현재 기준 범위: P01~P18 PR 문서

P01~P18 PR 문서 총검토에서 수집한 용어를 바탕으로, PR 문서 전체에 적용할 통합 용어 기준과 문서별 사용 현황을 정리한다.

이 문서는 PR 문서가 추가될 때마다 갱신하는 용어 기준 문서다. 확정된 기준은 본문 일괄 보완에 사용하고, 새 판단이 필요한 항목은 확정 전 별도 후보로 관리한다.

분류, 섹션별 적용, 통합 / 문서별 맵 구성 기준은 `01_PR_Document_Usage_Guide.md`를 따른다.

## 운영 기준

- 이 문서는 PR 문서 본문 보완에 사용할 실제 용어 목록과 문서별 사용 현황을 관리한다.
- 통합 기준으로 확정할 수 있는 용어는 `통합 용어 사용 맵`에 반영한다.
- 특정 PR 시점에서만 다르게 쓰이는 용어는 `문서별 용어 사용 맵` 또는 `문서별 Override`로 남긴다.
- 아직 확정하지 못한 흔들림 후보는 통합 기준에 섞지 않고 finding 문서에서 관리한다.
- 후속 브랜치에서 도입된 용어는 해당 브랜치 이후에만 적용하고, 과거 PR 문서에는 소급하지 않는다.

---

## 1. 통합 용어 사용 맵

P01~P18 PR 문서 전체에 공통 적용할 기본 용어 기준이다. 개별 문서에서 별도 override가 없으면 이 기준을 우선한다.

### 1.1 Only EN

다음 용어는 KR로 바꾸면 코드 / UE / 프로젝트 구조 추적성이 떨어지므로 Only EN을 기본으로 둔다.

#### UE / Actor / Asset

- Actor
- AI
- BehaviorTree
- Blackboard
- Blueprint
- Controller
- DataAsset
- Editor
- Git
- Player

#### Animation / Movement

- AnimBP
- Animation Retarget
- BlendSpace
- camera
- lifecycle
- locomotion
- montage
- MovementComponent
- movement
- notify
- parameter
- socket
- timing
- window

#### Combat / Damage / Collision

- collision
- cooldown
- damage
- feedback
- hit
- overlap
- reaction
- target
- SFX
- VFX

#### Runtime Data / State

- cache
- commit
- component
- context
- fallback
- input
- payload
- snapshot
- state
- terminal feedback

#### Event / Delegate / Lifecycle

- callback
- delegate
- event
- request
- result
- rollback

#### Execution Flow / Pipeline

- active execution
- chain
- combo
- entry
- Flow
- incoming execution
- intervention
- Pipeline
- routing
- runtime

#### 문서 / Prompt 운영 용어

- AI Workflow
- Backlog
- Codex
- Documentation
- Index
- Parry
- Prompt
- Prompt Blueprint
- Prompt Files
- Prompt Library
- Refactor Notes
- Prompt Routing
- Work Brief
- Work List

---

### 1.2 Conditional EN

다음 용어는 문서 시점에 따라 사용 범위를 제한한다.

#### orchestration / orchestrator

- P15 이후에만 구조명으로 사용한다.
- P08~P14에는 소급하지 않는다.
- P15 / P16 핵심 개념에서는 EN(KR) 병기 후 EN으로 사용한다.

#### intervention

- P17 이후에만 구조명으로 사용한다.
- P17 핵심 개념에서는 intervention(중단 개입)으로 병기 후 EN으로 사용한다.
- P16 이전 문서에는 소급하지 않는다.
- `Intervene`는 중심 용어로 확대하지 않는다.

#### cleanup

- Only EN 후보지만, 반드시 대상을 함께 쓴다.
- 권장:
  - runtime cleanup
  - state cleanup
  - effect cleanup
  - attack-end cleanup
- 비권장:
  - cleanup만 단독 반복

#### Pipeline

- 공식 구조명처럼 관리할 처리 축, 제목, 핵심 개념에서는 EN을 유지한다.
- 주요 처리 흐름 H3와 일반 설명에서는 `흐름`, `단계`, `처리 경로`로 풀어쓴다.
- 별도 구조 단위가 아닌 세부 구현에는 `Pipeline`을 붙이지 않는다.
- 예:
  - 제목: `TakeDamage Pipeline`
  - H3: `TakeDamage 처리 흐름`
  - 본문: `damage 수신 흐름`

#### entry / routing

- 코드 책임이나 route 분기 자체가 핵심이면 EN 유지 가능.
- 요약 / 변경 배경에서는 `진입점`, `전달`, `분기`로 풀어쓴다.

#### Flow / Routing

- P18의 Prompt 호출 기준에서는 문서 체계 용어로 EN 병기 가능.
- gameplay PR에서는 일반적으로 `흐름`으로 풀어쓴다.

---

### 1.3 Only KR

다음 용어는 KR만으로 의미가 충분하거나, EN으로 쓰면 오히려 문서가 기술어 나열처럼 보이므로 Only KR을 기본으로 둔다.

#### 문서 구조 / 설명 역할

- 공격 단계
- 개입 조건
- 검증용 문서
- 교전
- 문서 목록
- 문서 변환 흐름
- 문서 체계
- 변경 배경
- 실행 경로
- 실행 결과
- 실행 상태
- 안정성 보완

#### Gameplay 상태 / 처리 결과

- 이동 제한
- 이동 복구
- 입력 허용 구간
- 전투 역할
- 중단 결과
- 중단 허용 조건
- 처리 결과
- 판단 기준
- 피격
- 회복
- 후속 범위
- 완료 범위

#### AI Workflow 운영 표현

- 작업 목록
- 작업 운영 흐름

---

### 1.4 EN(KR)

다음 용어는 첫 등장 병기 후 같은 섹션 안에서는 EN만 사용한다.

#### P01~P03 플레이 기반 / 장착 준비

- TestRoom(테스트 레벨)
- Camera Rig(camera 위치 / 회전 보조 구조)
- Look Input(camera 회전 입력)
- Movement Runtime Value(이동 runtime 값)
- Locomotion Parameter(이동 애니메이션 전환 매개값)
- Speed Type(이동 속도 타입)
- Equipment(장착 실행 객체)
- Attachment(무기 부착 actor)
- Weapon Type(무기 장착 상태)
- FEquipmentData(equipment 실행 데이터)
- Equipment Notify(장착 timing 전달 notify)

#### P04~P07 Action 실행 / Damage 송신 경계

- Action Object(action 실행 객체)
- Action Execution Pipeline(action 실행 흐름)
- LightAttack(단일 기본 공격)
- FActionData(action 실행 데이터)
- Action Notify(action timing 전달 notify)
- ComboAttack(연속 공격)
- Combo Step(연속 공격 단계)
- PreInput Window(선입력 허용 구간)
- PreInput(선입력)
- Action Notify Next(다음 combo 전환 notify)
- Collision Window(충돌 허용 구간)
- Attachment Collision(attachment 충돌체)
- Attachment Overlap Event(attachment overlap 전달 event)
- Action Callback(Attachment Overlap Event 수신 callback)
- ApplyDamage Pipeline(damage 송신 흐름)
- TakeDamage Boundary(damage 수신 경계)
- FAttachmentContext(attachment 기준 damage 설정 조회 정보)
- FEquipmentContext(equipment 기준 damage 설정 조회 정보)
- FActionContext(action 기준 damage 설정 조회 정보)
- FOverlapContext(target overlap 결과 정보)
- FHitContext(damage 요청에 필요한 타격 정보)
- FDamageSpec(damage 설정)
- FDamageResult(damage 결과)

#### P08~P14 Damage 수신 / Reaction / Feedback

- TakeDamage Pipeline(damage 수신 처리 흐름)
- Damage Pipeline(damage 처리 흐름)
- Reaction Execution Pipeline(피격 반응 실행 흐름)
- Reaction(리액션 실행 객체)
- Active Reaction(현재 실행 중인 피격 반응)
- Reaction Window(피격 반응 제어 구간)
- ActionFeedback(action timing 기반 표현 실행)
- ReactionFeedback(피격 반응 결과 기반 표현 실행)
- DamageFeedback(피격 순간 feedback)
- PlayerFeedback(local player 기준 표현 실행)
- Feedback Request(feedback 실행 요청)
- Damage Impact Info(damage impact 정보)
- Hit Window(타격 허용 구간)
- Combat Role Assignment(전투 역할 배정)
- AttackIndex(공격 단계 기록)
- AIContext(AI 행동 결정 정보)
- AIState(AI 상태)
- AIStateType(AI 상태 타입)
- BehaviorTree Branch(BehaviorTree 실행 경로)
- Combo Chain(연속 공격 흐름)
- Reaction Takeover(피격 반응 전환)
- Runtime Cleanup(runtime 정리)
- Failure Handling(실패 처리)
- Health Commit(HP 반영)
- Damage Payload(damage 원본 요청값)
- Damage Context(damage 처리 정보)
- Damage Result(damage 처리 결과)

#### P15~P17 공용 실행 / 중단 개입 구조

- Action Request(행동 실행 요청)
- Reaction Request(리액션 실행 요청)
- Request Result(요청 처리 결과)
- Action Component(action 실행 상태 component)
- Action Orchestrator(action 요청 조율자)
- Reaction Orchestrator(reaction 요청 조율자)
- Reaction Component(reaction 실행 상태 component)
- Cooldown Commit(cooldown 확정)
- intervention(중단 개입)
- active execution(현재 실행 중인 행동)
- incoming execution(새로 실행 요청된 행동)
- WantInterventionRules(개입 조건 규칙)
- AllowInterventionRules(중단 허용 규칙)
- FExecutionInterventionQuery(중단 판단 정보 구조체)
- FExecutionInterventionDirective(중단 실행 지시값)
- Runtime cleanup(실행 중 임시 상태 정리)

#### P18 AI Workflow / Prompt Library

- AI Workflow(AI 작업 운영 흐름)
- Prompt Library(Prompt 모음)
- Prompt Blueprint(Prompt 기준 문서)
- Prompt Files(Prompt 본문)
- Work Brief(작업 요청 요약)
- Feature Work Planning(기능 작업 계획)
- Work List(작업 목록)
- Prompt Flow / Routing(Prompt 호출 흐름)

---

## 2. 문서별 용어 사용 맵

각 PR 문서의 실제 용어 사용을 문서 단위로 정리한다. 문서 내부는 `Only EN / EN(KR) / Only KR / 검토 결과`로 나누어, 통합 기준과 다른 문서별 특수 맥락을 확인할 수 있게 한다.

### P01

#### Only EN

- Player
- PlayerController
- SpringArm
- CameraComponent
- camera
- mouse
- yaw / pitch
- axis mapping
- startup map
- capsule
- mesh
- component
- binding

#### EN(KR)

- TestRoom(테스트 레벨): 기능 검증을 위해 editor startup map으로 여는 기본 테스트 레벨.
- Camera Rig(camera 위치 / 회전 보조 구조): 3인칭 시점을 만들기 위해 SpringArm과 CameraComponent를 함께 구성한 camera 구조.
- Look Input(camera 회전 입력): mouse X / Y 입력을 controller yaw / pitch 회전으로 전달하는 입력 흐름.

#### Only KR

- 기본 플레이 환경: movement / combat 이전에 Player와 camera를 확인할 수 있는 최소 실행 환경.
- 3인칭 시점: camera가 Player 뒤쪽에서 따라가는 화면 구성.
- 기본 구성: P01에서는 capsule / mesh / SpringArm / CameraComponent를 갖춘 최소 Player actor 구성을 뜻한다.

#### 검토 결과

- `feature/character-camera-core` tip 기준으로 `TestRoom.umap`, `GM_Test`, `Label_Test`, `BP_CPlayer`, `BP_CPlayerController`, `ACPlayer`, `ACPlayerController`, `DefaultInput.ini`, `DefaultEngine.ini`가 확인됐다.
- P01 Draft의 용어 사용은 브랜치 시점과 대체로 맞다.
- `Camera Rig(camera 위치 / 회전 보조 구조)`은 핵심 개념에서 병기 후 본문에서는 `Camera Rig` 또는 `camera rig` 중 하나로 통일하는 것이 좋다.

### P02

#### Only EN

- Player
- PlayerController
- MovementComponent
- Animation Retarget
- AnimBP
- BlendSpace
- Jump Start / Jump Loop / Jump End
- Speed / Direction / bIsInAir
- CurrentSpeed / CurrentDirection / bIsFalling
- SpeedMap
- Walk / Run / Sprint
- Quinn
- IK Rig / IK Retargeter

#### EN(KR)

- Movement Runtime Value(이동 runtime 값): 매 tick 계산되는 현재 이동 속도, 이동 방향, 공중 상태 값.
- Locomotion Parameter(이동 애니메이션 전환 매개값): AnimBP가 locomotion / jump state를 전환할 때 사용하는 animation parameter.
- Speed Type(이동 속도 타입): Walk / Run / Sprint처럼 movement speed를 구분하는 값.

#### Only KR

- 이동 입력: WASD / Walk / Jump처럼 Player 이동에 들어오는 입력.
- 이동 방향: controller yaw를 기준으로 계산한 forward / right 방향.
- 공중 상태: character movement의 falling 여부.
- 이동 제한 / 복구: 후속 equipment / action에서 movement 가능 여부를 제어할 때 사용할 개념.

#### 검토 결과

- `feature/character-move-core` tip 기준으로 `UCMovementComponent`, `UCAnimInstance`, movement input binding, Walk / Run / Jump input, `ABP_Character`, `BS_Unarmed`, retarget 관련 asset이 확인됐다.
- P02 Draft의 큰 용어 사용은 브랜치 시점과 맞다.
- 코드 API 표기는 branch tip 기준으로 `Press_Walk` / `Release_Walk` / `Press_Jump` / `Release_Jump`와 현재 Draft 표기가 맞는지 유지 점검이 필요하다.
- `후속 gameplay 확장 기준 마련`은 용어 문제가 아니라 표현 품질 finding으로 관리한다.

### P03

#### Only EN

- Player
- sword
- weapon type
- equipment
- attachment
- delegate
- binding
- broadcast
- hand socket
- holster socket
- montage
- state
- movement
- Details Panel
- Blueprint compile

#### EN(KR)

- Equipment(장착 실행 객체): weapon equip / unequip montage 재생, state 전환, movement 제어를 담당하는 UObject 기반 실행 객체.
- Attachment(무기 부착 actor): character mesh의 hand / holster socket에 붙는 weapon actor.
- Weapon Type(무기 장착 상태): 현재 character가 어떤 weapon type 상태인지 나타내는 값.
- FEquipmentData(equipment 실행 데이터): equip / unequip montage, play rate, movement 허용 여부를 담는 실행 데이터.
- Equipment Notify(장착 timing 전달 notify): montage timing에서 equip / unequip의 Begin / End를 알려주는 notify.

#### Only KR

- 장착 / 해제: sword input을 통해 weapon type을 바꾸고 equip / unequip montage를 실행하는 동작.
- 장착 완료 상태: `UCEquipment`가 equip 종료 후 기록하는 내부 상태.
- 이동 제한 / 복구: equipment 실행 중 movement를 멈추거나 다시 허용하는 처리.

#### 검토 결과

- `feature/character-weapon-equip` tip 기준으로 `UCWeaponComponent`, `UCEquipment`, `ACAttachment`, `FEquipmentData`, `UCAnimNotify_Equip`, `UCAnimNotify_Unequip`, sword montage, sword attachment asset이 확인됐다.
- P03 Draft의 용어 사용은 브랜치 시점과 대체로 맞다.
- Actor 생성은 `spawn`, UObject 생성은 `NewObject` 기준으로 구분하는 현재 설명을 유지한다.
- `Equipment(장착 실행 객체)`와 `Attachment(무기 부착 actor)`는 같은 섹션 첫 등장 병기 후 EN만 사용하는 구조가 적합하다.

### P04

#### Only EN

- Player
- sword
- action
- action instance
- montage
- notify
- timing
- state
- movement
- Idle
- ActionMode

#### EN(KR)

- Action Object(action 실행 객체): input을 받아 montage lifecycle을 실행하는 UObject 기반 action instance.
- Action Execution Pipeline(action 실행 흐름): input, action object, montage lifecycle, notify timing, state / movement 복구로 이어지는 action 실행 절차.
- LightAttack(단일 기본 공격): P04에서 처음 구성한 단일 공격 action.
- FActionData(action 실행 데이터): action montage, play rate, movement 허용 여부를 담는 실행 데이터.
- Action Notify(action timing 전달 notify): montage timing에서 action Begin / End를 action instance에 전달하는 notify.

#### Only KR

- 공격 입력: PlayerController에서 들어와 Player와 WeaponComponent를 거쳐 action 실행으로 이어지는 입력.
- 단일 공격: combo 이전 단계에서 하나의 공격 montage만 실행하는 공격 흐름.
- 이동 제한 / 복구: action 실행 중 movement를 막고 action 종료 시 다시 허용하는 처리.
- 실행 상태 복구: action 종료 후 state를 Idle로 되돌리는 처리.

#### 검토 결과

- `feature/combat-light-attack` tip 기준으로 `UCAction`, `UCAction_LightAttack`, `FActionData`, `UCAnimNotify_Action`, `UCWeaponComponent::PlayAction()`, `UCWeaponComponent::GetAction()`이 확인됐다.
- `PlayAction()`은 `UCWeaponComponent::PlayAction()` 진입점과 `UCAction::PlayAction()` method가 함께 있으므로 호출 주체를 문장 안에서 명확히 쓰는 기준을 유지한다.
- notify는 `UCWeaponComponent`가 소유한 action instance를 `GetAction()`으로 얻어 `Begin_PlayAction()` / `End_PlayAction()`을 호출하므로, `현재 action`보다 `UCWeaponComponent가 소유한 action instance` 표현이 더 정확하다.
- P04는 별도 LightAttack Pipeline이 아니라 Action Execution Pipeline의 기본 실행 골격을 구성한 문서로 정리한다.

### P05

#### Only EN

- Next timing
- Index
- bEnablePreInput
- bExistPreInput
- TArray
- montage

#### EN(KR)

- ComboAttack(연속 공격): 여러 공격 montage를 단계별로 이어서 실행하는 action.
- Combo Step(연속 공격 단계): ComboAttack 안에서 현재 실행 중인 공격 순서.
- PreInput Window(선입력 허용 구간): 다음 공격 입력을 미리 받을 수 있는 montage timing 구간.
- PreInput(선입력): PreInput Window 안에서 들어온 다음 공격 입력.
- Action Notify Next(다음 combo 전환 notify): 저장된 PreInput을 다음 Combo Step 실행으로 전환하는 notify timing.

#### Only KR

- 연속 공격: 여러 공격을 단계적으로 이어 실행하는 공격 흐름.
- 입력 허용 구간: 다음 공격 입력을 받을 수 있는 시간 구간.
- 재입력: 이미 공격 중일 때 다시 들어온 공격 입력.
- 다음 공격 후보: PreInput으로 저장된 뒤 Next timing에서 소비될 입력.

#### 검토 결과

- `feature/combat-combo-attack` tip 기준으로 `UCAction_ComboAttack`, `UCAnimNotify_PreInput`, `UCAnimNotify_Action`의 `Next` flow, `Index`, `bEnablePreInput`, `bExistPreInput`, `TArray<FActionData>`가 확인됐다.
- P05의 combo 관련 용어는 P06 이후 collision / damage 용어와 섞지 않고 combo lifecycle 안에서만 사용한다.
- `Next_PlayAction()`은 `bExistPreInput`이 있을 때 `Index`를 증가시키고 다음 montage를 재생하므로, Draft의 PreInput / Next timing 설명은 브랜치 시점과 맞다.
- P05는 별도 ComboAttack Pipeline이 아니라 P04의 Action Execution Pipeline 위에 ComboAttack 실행 규칙을 추가한 문서로 정리한다.

### P06

#### Only EN

- hit collision
- target overlap
- broadcast
- binding
- delegate
- overlap
- collision
- attachment
- damage
- ACEnemy

#### EN(KR)

- Collision Window(충돌 허용 구간): 공격 montage의 특정 timing에서만 attachment collision을 켜고 끄는 구간.
- Attachment Collision(attachment 충돌체): attachment가 소유한 `UShapeComponent` 기반 공격 판정 collision.
- Attachment Overlap Event(attachment overlap 전달 event): attachment collision에서 발생한 유효한 target overlap을 외부로 broadcast하는 delegate event.
- Action Callback(Attachment Overlap Event 수신 callback): Attachment Overlap Event를 action instance가 받을 수 있도록 연결한 callback.
- Dummy Enemy(검증용 enemy actor): hit collision 검증을 위해 테스트 레벨에 배치한 최소 enemy actor.
- CollisionName(collision 선택 이름): 여러 collision shape 중 특정 collision만 열 때 사용하는 이름.

#### Only KR

- 충돌 이후 처리: collision 발생 이후 다음 처리 대상으로 overlap 결과를 넘기는 흐름.
- 후속 damage 처리: P06 이후 브랜치에서 damage 요청 / 계산 / 수신으로 확장될 범위.
- 유효한 overlap 결과: self-overlap을 제외하고 필요한 actor / component 값이 검증된 overlap 결과.

#### 검토 결과

- `feature/combat-hit-collision` tip 기준으로 `ACEnemy`, `ACAttachment`, `UCAnimNotify_Collision`, `UCWeaponComponent`의 delegate binding, `OnAttachmentBeginOverlap.Broadcast(...)`, action callback이 확인됐다.
- P06 브랜치 목적은 damage 계산이 아니라 hit collision 발생 / 제어 / 전달 흐름이다.
- P06 본문에서는 `현재 실행 중인 action` 표현을 제거하고, `Action Callback 수신 대상` 또는 `action callback` 중심 표현으로 정리했다.
- delegate 동작 구분이 중요하므로 `broadcast` / `binding`은 Only EN으로 유지한다.

### P07

#### Only EN

- ApplyDamage Pipeline
- TakeDamage Boundary
- context
- DamageSpecMap
- FDamageSpecKey
- FCustomDamageEvent
- RequestApplyDamage
- ApplyDamageToTarget
- RequestStopDamage
- CheckHitRule
- BaseDamage
- FinalDamage

#### EN(KR)

- FAttachmentContext(attachment 기준 damage 설정 조회 정보): attachment type처럼 attachment 기준 damage 설정 조회에 필요한 정보를 담는 구조체.
- FEquipmentContext(equipment 기준 damage 설정 조회 정보): equipment type처럼 equipment 기준 damage 설정 조회에 필요한 정보를 담는 구조체.
- FActionContext(action 기준 damage 설정 조회 정보): action type과 action index처럼 action 기준 damage 설정 조회에 필요한 정보를 담는 구조체.
- FOverlapContext(target overlap 결과 정보): attacker, damage causer, overlap component, target actor, hit component 등 overlap 결과를 담는 구조체.
- FHitContext(damage 요청에 필요한 타격 정보): overlap / attachment / equipment / action 정보를 결합한 damage 요청 구조체.
- FDamageSpec(damage 설정): damage key로 조회되는 기본 damage 설정.
- FDamageResult(damage 결과): 계산된 damage와 attacker / damage causer / target을 담는 결과.
- ApplyDamage Pipeline(apply damage 송신 흐름): hit context를 검증하고 damage 설정 / 결과를 만든 뒤 target의 `TakeDamage()`까지 넘기는 흐름.
- TakeDamage Boundary(TakeDamage 수신 경계): Unreal `TakeDamage()` API로 damage 결과를 넘기는 경계.

#### Only KR

- 송신 측: damage 요청을 만들고 target에게 전달하는 쪽.
- 수신 측: `TakeDamage()`를 받아 HP / reaction 등 후속 처리를 수행할 쪽.
- damage 설정 조회: context 기반 key로 damage 설정을 찾는 처리.
- damage 결과 계산: 조회한 damage 설정으로 최종 damage 결과를 만드는 처리.

#### 검토 결과

- `feature/combat-apply-damage` tip 기준으로 `FActionContext`, `FAttachmentContext`, `FEquipmentContext`, `FOverlapContext`, `FHitContext`, `FDamageSpec`, `FDamageResult`, `UCApplyDamageComponent`, `DamageSpecMap`, `target->TakeDamage(...)`가 확인됐다.
- P07은 `weapon context` 같은 후속 / 미정의 alias를 쓰지 않고, branch-backed context 구조체명을 기준으로 설명해야 한다.
- `context`는 핵심 구조체와 직접 연결되므로 Only EN 유지가 적합하다.
- P07 제목과 목표는 Action Execution Pipeline 전체가 아니라 ApplyDamage Pipeline 중심으로 좁힌다.

### P08

#### Only EN

- TakeDamage Pipeline
- TakeDamage
- DamageEvent
- FDefaultDamageEvent
- Payload
- Context
- Result
- Health
- HP
- dead state
- DamageCauser
- Instigator
- Commit
- fallback

#### EN(KR)

- UCTakeDamageComponent(damage 수신 처리 component): Enemy가 받은 `TakeDamage()` event를 payload / context / result 단계로 처리하는 component.
- UCHealthComponent(HP 관리 component): HP clamp, damage / heal 적용, dead state 갱신을 담당하는 component.
- FTakeDamagePayload(damage 수신 원본 입력): engine entry와 apply damage 쪽 metadata를 모은 원본 입력 구조체.
- FTakeDamageContext(damage 수신 처리 정보): resolved instigator, damage causer, HP snapshot, damage amount를 담는 처리 중 정보 구조체.
- FTakeDamageResult(damage 수신 결과): accepted 여부, reject reason, 최종 적용 damage, killed 여부를 담는 결과.
- TakeDamage Pipeline(damage 수신 처리 흐름): Unreal `TakeDamage()` event를 받아 HP 반영 결과까지 만드는 receiver-side damage 흐름.

#### Only KR

- 수신 측 damage 처리: target actor가 damage event를 받은 뒤 내부 처리로 넘기는 흐름.
- HP 반영: accepted damage를 실제 HP 감소와 dead state 갱신으로 반영하는 처리.
- 처리 단계 분리: 원본 입력, 처리 중 상태, 최종 결과를 나누어 추적하는 방식.

#### 검토 결과

- `feature/combat-take-damage` tip 기준으로 `ACEnemy::TakeDamage()`, `UCTakeDamageComponent::RequestTakeDamage()`, `HandleDefaultDamageEvent()`, `FTakeDamagePayload`, `FTakeDamageContext`, `FTakeDamageResult`, `UCHealthComponent::TakeDamage()`가 확인됐다.
- P08 제목의 `TakeDamage Pipeline`은 receiver-side damage 수신 / HP 반영 흐름을 가리키므로 유지가 적합하다.
- Reaction / feedback은 branch tip 코드에서 TODO / 후속 범위로 남아 있으므로 P08 결과처럼 쓰지 않는다.

### P09

#### Only EN

- Reaction Execution Pipeline
- reaction
- active reaction
- new reaction
- Reaction Window
- Interruptible
- Cancelable
- ImmuneToReaction
- AllowInterruptionBy
- WantToInterrupt
- FReactionDataKey
- FReactionData
- FReactionQueryContext
- ReactionDataMap
- ReactionExecutor
- cache

#### EN(KR)

- UCReactionComponent(피격 반응 선택과 실행 관리 component): damage 결과를 받아 reaction type, reaction data, executor, active reaction 교체를 관리하는 component.
- UCReaction(피격 반응 실행 객체): reaction montage lifecycle과 interrupt / cancel flag를 관리하는 executor.
- Active Reaction(현재 실행 중인 피격 반응): 새 reaction이 들어왔을 때 유지하거나 중단될 수 있는 실행 중 reaction.
- New Reaction(새 피격 반응): damage 결과로 새로 요청된 reaction.
- Reaction Window(피격 반응 제어 구간): montage notify state로 active reaction의 interruptible / cancelable / immune flag를 조정하는 구간.
- Reaction Execution Pipeline(피격 반응 실행 흐름): damage 결과에서 reaction 선택, 교체 판단, montage 실행, 정리까지 이어지는 흐름.

#### Only KR

- 피격 반응 실행: damage 결과에 따라 hit / dead reaction을 선택하고 montage를 실행하는 흐름.
- 교체 판단: active reaction과 new reaction 양쪽 정책을 기준으로 새 reaction을 받아들일지 결정하는 처리.
- 실행 상태 정리: reaction 종료 후 movement / state와 active reaction 정보를 복구하는 처리.

#### 검토 결과

- `feature/combat-reaction` tip 기준으로 `UCReactionComponent::RequestReaction()`, `ResolveReactionType()`, `ResolveReactionData()`, `QueryAcceptNewReaction()`, `AllowInterruptionBy()`, `WantToInterrupt()`, `UCReaction`, `CAnimNotifyState_Reaction`이 확인됐다.
- P09의 `active reaction` / `new reaction`은 branch-backed 용어이므로 유지한다.
- `Reaction Orchestrator` 같은 P16 이후 용어는 P09에 소급하지 않는다.

### P10

#### Only EN

- BehaviorTree
- Blackboard
- AIContext
- AIState
- AIStateType
- EAIStateType
- AttackIndex
- Engage
- Alert
- Attack
- SBT_Attack
- Combat Role Assignment
- Blackboard key
- CanMove
- Task
- Service
- Decorator

#### EN(KR)

- AIContext(AI 행동 결정 정보): perception, target, distance, engage, reaction, dead 관련 값을 모아 AIState 결정에 쓰는 context.
- AIState(AI 행동 상태): BehaviorTree 실행 경로를 선택하는 상태 값.
- Engage(교전 상태): AI가 target과 전투에 참여하는 상태.
- Alert(경계 상태): target은 인식했지만 교전에 참여하지 않는 상태.
- AttackIndex(공격 단계 기록): Engage 안에서 어떤 공격 montage를 실행할지 고르는 단계 값.
- AttackableTime(공격 가능 시간): Engage 상태 안에서 attack branch로 들어갈 수 있는 시간 조건.
- Combat Role Assignment(전투 역할 배정): 여러 AI 중 누가 Engage 역할을 가질지 정하는 배정 결과.
- SBT_Attack(공격 실행 서브트리): Engage 상태 내부에서 실제 attack task를 실행하는 BehaviorTree 하위 asset.

#### Only KR

- AI 행동 결정 흐름: 감지 결과와 Blackboard 값을 바탕으로 AIState를 고르고 해당 실행 경로로 들어가는 흐름.
- 이동 상태별 목적지: 순찰, 조사, 추격, 경계 상태에서 각자 사용하는 이동 기준 위치.
- 교전 / 공격 책임 분리: Engage 상태 유지와 실제 Attack 실행 조건을 나누어 관리하는 기준.
- 잔여값 정리: 유효하지 않은 target, engage 판단 값, AttackIndex가 다음 행동 결정에 남지 않게 비우는 처리.

#### 검토 결과

- `feature/ai-behaviortree-core` tip 기준으로 `ACAIController`의 BehaviorTree / Blackboard 초기화, `FAIContext`, `EAIStateType`, `CBTService_UpdateAIContext`, `CBTService_UpdateAIState`, `CAIKey`, `AttackIndex`, `SBT_Attack` asset과 attack task가 확인됐다.
- `UCWorldSubsystem_CombatEngage`가 Alert / Engage 역할 배정을 수행하고, attack 실행은 `SBT_Attack`과 `StartAttack` / `SelectAttackIndex` / `EndEnemyAttack` 흐름에서 처리된다.
- P10은 action request 공용화 이전 브랜치이므로 P15 이후의 Action Request / Orchestrator 용어를 소급하지 않는다.

### P11

#### Only EN

- Player
- Enemy
- TakeDamage
- Combat Receiver
- Pending Reaction
- DeadState
- HitReact
- FActionContext
- AttackIndex
- StateComponent
- CanActionInput
- Tick

#### EN(KR)

- Combat Receiver(전투 수신자): damage를 받은 actor가 HP / reaction / dead 흐름으로 들어가는 수신 역할.
- Pending Reaction(대기 중인 피격 반응): damage commit 이후 저장되고 Player tick에서 소비되는 reaction context.
- DeadState(사망 상태): Health 기준으로 Player의 alive / dying / dead 상태를 구분하는 값.
- FHitContext(타격 정보): Enemy 공격의 action / attachment / equipment / overlap 정보를 damage 요청에 전달하는 hit context.

#### Only KR

- 입력 차단: 사망 또는 피격 반응 중 Player 입력을 막는 처리.
- Enemy 공격 기준 정보: Enemy 공격의 action type / action index가 hit context로 전달되는 기준.
- 피격 반응 소비: 대기 중인 reaction context를 Player tick에서 꺼내 실행하는 처리.
- 사망 동기화: HP component의 사망 상태를 StateComponent와 연결하는 처리.
- 전투 수신 흐름: Player가 damage를 받고 HP / reaction / dead 처리로 이어지는 흐름.

#### 검토 결과

- `feature/player-combat-receiver` tip 기준으로 `ACPlayer::TakeDamage()`, `UCTakeDamageComponent`, `UCHealthComponent`, `UCReactionComponent`, `ConsumePendingReaction()`, `CanActionInput()`이 확인됐다.
- Enemy attack task는 `FActionContext`에 `AttackActionType`과 `AttackIndex`를 담아 attachment context로 전달한다.
- P11은 Player가 Enemy와 같은 damage / reaction 수신 흐름에 들어가는 브랜치이며, 공통 Hit Window / damage 규칙은 P12로 넘긴다.

### P12

#### Only EN

- Damage Pipeline
- FApplyDamageHitWindowKey
- DamageCauser
- HitWindowId
- RequestDamage
- MitigatedDamage
- FinalTakenDamage
- CommittedDamage
- DeadState_Before
- DeadState_After
- ApplyDamage
- TakeDamage

#### EN(KR)

- Hit Window(타격 유효 구간): collision이 켜져 damage를 처리할 수 있는 공격 구간.
- FApplyDamageHitWindowKey(타격 구간 식별 key): `DamageCauser`와 `HitWindowId`를 묶어 같은 타격 구간을 식별하는 key.
- FApplyDamagePayload(ApplyDamage 원본 요청 정보): ApplyDamage 단계로 들어온 hit context와 요청 metadata를 담는 원본 입력 구조체.
- FApplyDamageContext(ApplyDamage 처리 정보): damage 설정 조회와 결과 계산에 필요한 송신 측 처리 정보를 담는 구조체.
- FApplyDamageResult(ApplyDamage 처리 결과): target에게 전달할 damage 결과와 송신 측 처리 결과를 담는 구조체.
- FTakeDamagePayload(TakeDamage 원본 수신 정보): Unreal TakeDamage 경계로 들어온 damage event 원본 정보를 담는 구조체.
- FTakeDamageContext(TakeDamage 처리 정보): HP 반영과 dead state 판단에 필요한 수신 측 처리 정보를 담는 구조체.
- FTakeDamageResult(TakeDamage 처리 결과): accepted 여부, 최종 반영 damage, dead state 변화를 담는 구조체.
- CommittedDamage(실제 반영 피해량): 최종적으로 HP에 반영된 damage 값.
- RequestDamage(요청 피해량): damage spec에서 계산되어 target에게 요청되는 damage 값.
- FinalTakenDamage(최종 수신 피해량): 피격자 쪽에서 실제 수신 결과로 정리되는 damage 값.
- DeadState_Before / DeadState_After(사망 상태 변경 전후 값): damage 처리 전후 사망 상태 변화 기준.

#### Only KR

- 중복 타격 방지: 같은 hit window 안에서 같은 target에게 damage가 여러 번 적용되지 않게 막는 처리.
- 중복 타격: 같은 hit window 안에서 같은 target에게 damage가 여러 번 적용되는 상황.
- 자기 자신 타격: attacker 자신이 damage target으로 잡히는 상황.
- 아군 대상: 같은 team 또는 friendly 관계의 target.
- 공통 damage 규칙: Player와 Enemy가 같은 ApplyDamage / TakeDamage 단계로 damage를 처리하는 기준.
- 송신 측 gate: ApplyDamage 단계에서 invalid hit window, 자기 자신 타격, 중복 타격, spec 조회 실패를 막는 기준.
- 수신 측 commit: TakeDamage 단계에서 HP 반영과 dead state 변화를 확정하는 처리.

#### 검토 결과

- `feature/combat-core-shared` tip 기준으로 `NotifyHitWindowOpened()`, `NotifyHitWindowClosed()`, `FApplyDamageHitWindowKey`, `DamagedTargetContainer`, `CanApplyDamage()`, `CommittedDamage`가 확인됐다.
- P12 branch의 damage amount 결과 용어는 `CommittedDamage` 중심이므로, P08/P07의 과거 damage result 명칭을 소급하지 않는다.
- Draft의 `CommittedDamage` / Hit Window 설명은 브랜치 코드와 맞다.

### P13

#### Only EN

- ComboAttack
- PreInput
- PreInput Window
- Combo Chain
- Hit Window
- ActionIndex
- FActionContext
- ApplyDamage
- TakeDamage
- CommittedDamage
- bEnablePreInput
- bExistPreInput

#### EN(KR)

- Player Combat Loop(Player 전투 1사이클): Player 입력에서 combo, hit window, damage 적용, reaction 확인까지 이어지는 검증 흐름.
- Combo Chain(combo 연계 흐름): PreInput과 Next timing을 기준으로 다음 Combo Step으로 이어지는 흐름.
- Hit Window Damage(타격 허용 구간 내 damage 적용 흐름): collision window 안에서 damage가 적용되고 중복 타격이 차단되는 흐름.

#### Only KR

- 공격 진입 / 종료: ComboAttack 시작과 종료에서 action state와 attachment context를 열고 닫는 처리.
- 한 사이클 검증: 입력, combo, hit, damage, reaction이 실제 플레이 흐름으로 이어지는지 확인하는 검증.
- 상태 복귀: 공격 종료 이후 action index, preinput, attachment context를 초기화하는 처리.

#### 검토 결과

- `feature/player-combat-loop` tip 기준으로 `UCAction_ComboAttack`, `bEnablePreInput`, `bExistPreInput`, `NextPlayAction()`, `FActionContext`, `NotifyHitWindowOpened()`, `NotifyHitWindowClosed()`, `CommittedDamage`가 확인됐다.
- P13은 새 공식 Pipeline 도입보다 P04~P12의 action / combo / hit window / damage 흐름이 하나의 Player combat loop로 이어지는지 검증하는 브랜치다.
- P13의 `Hit Window`는 P12에서 이미 도입된 용어를 재사용하므로, P13에서는 새 병기 대상이 아니라 검증 흐름 안의 기존 구조명으로 유지한다.
- `action orchestration`은 후속 범위 문장에만 남기고 P13 완료 구조처럼 쓰지 않는다.

### P14

#### Only EN

- Combat Feedback
- ActionFeedback
- ReactionFeedback
- PlayerFeedback
- Feedback Request
- ActionFeedbackRequestProvider
- CWorldSubsystem_CombatFeedback
- FTakeDamagePacket
- FHitStopRequest
- FCameraShakeRequest
- AnimNotify_ActionFeedback
- AnimNotifyState_ActionFeedback
- AnimNotify_EndEnemyAttack
- CBTTask_EndAttack
- attack-end cleanup
- shared feedback
- trail
- VFX
- SFX
- camera shake

#### EN(KR)

- Combat Feedback(전투 체감 표현): 전투 결과와 action timing을 trail / VFX / SFX / camera shake 같은 표현으로 연결하는 계층.
- ActionFeedback(action timing 기반 표현 실행): action montage timing을 기준으로 실행되는 feedback.
- ReactionFeedback(피격 반응 결과 기반 표현 실행): P14 기준에서 `TakeDamage` 이후 hit result를 기준으로 실행되는 feedback.
- PlayerFeedback(local player 기준 표현 실행): local player controller / camera 기준으로 실행되어야 하는 feedback.
- Feedback Request(feedback 실행 요청): feedback component가 실행할 표현과 timing을 담는 요청 값.

#### Only KR

- 공격 종료 신호: Enemy attack montage notify가 공격 종료 시점을 알리는 신호.
- Player local 표현: local player controller 기준으로만 실행되어야 하는 camera shake 같은 표현.

#### 검토 결과

- `feature/combat-feedback` tip 기준으로 `UCActionFeedbackComponent`, `UCReactionFeedbackComponent`, `UCPlayerFeedbackComponent`, `UCWorldSubsystem_CombatFeedback`, `AnimNotify_ActionFeedback`, `AnimNotifyState_ActionFeedback`이 확인됐다.
- `UCReactionFeedbackComponent`는 `FTakeDamagePacket`의 accepted / `CommittedDamage` 결과를 기준으로 hit stop, hit VFX, hit SFX, camera shake request를 처리한다.
- `UCPlayerFeedbackComponent`는 subsystem의 camera shake request를 local player controller 기준으로 소비하므로, 모든 feedback을 한 component가 직접 실행한다고 쓰지 않는다.
- Enemy attack 종료는 notify 신호와 `CBTTask_EndAttack` 정리 책임을 구분해 설명한다.

### P15

#### Only EN

- Action Request
- Action Orchestrator
- Action Component
- Action
- Request Result
- Combat Action Request
- Movement Action Request
- Equipment Action Request
- Combo Chain
- cooldown
- rollback
- BehaviorTree
- Blackboard

#### EN(KR)

- Action Request(행동 요청): Player 입력이나 AI 판단 결과를 공통 실행 요청 형식으로 만든 값.
- Action Orchestrator(행동 요청 조율자): action request를 검증하고 실행 계층으로 넘기는 component.
- Action Component(action 실행 상태 component): 실제 action 실행 상태와 active action을 관리하는 component.
- Request Result(요청 처리 결과): action request가 Rejected / Ignored / Handled / Started / Chained / Enqueued / Interrupted 중 어떤 결과로 처리됐는지 나타내는 값.
- Combo Chain(combo 연계 흐름): chain window와 후속 request를 통해 다음 공격 단계로 이어지는 흐름.
- ChainWindowOpened(연계 가능 구간 열림 event): Combo Chain에서 후속 action request를 받을 수 있는 timing이 열렸음을 알리는 event.
- Cooldown Commit(cooldown 확정): 실제 action이 시작된 경우에만 다음 실행 가능 시간을 확정하는 처리.

#### Only KR

- 공통 실행 진입점: Player와 AI가 같은 action request 경로로 들어가는 지점.
- 실행 의도: Player 입력이나 AI 판단이 만든 "무엇을 실행할지"에 대한 요청.
- 실행 생명주기: request 처리, component 상태 적용, montage 실행, 종료 cleanup까지 이어지는 action 실행 과정.
- 실패 처리: request가 거절되거나 무시됐을 때 state / context / cooldown을 남기지 않는 처리.

#### 검토 결과

- `feature/action-orchestration` tip 기준으로 `UCActionOrchestratorComponent`, `RequestCombatAction()`, `RequestEquipmentAction()`, `RequestMovementAction()`, `FActionRequestResult`, `EActionRequestResultType`이 확인됐다.
- P15는 Player와 AI의 action 실행 진입점을 Action Request 기반으로 표준화하고, 실제 실행은 `Action Orchestrator -> Action Component -> Action`으로 넘기는 브랜치다.
- `Action Request`, `Action Orchestrator`, `Action Component`, `Request Result`는 P15 이후 구조명으로 유지한다.
- P15 이전 문서에는 Action Orchestrator / orchestration 용어를 소급하지 않는다.
- `Chained`는 P15 branch-backed request result 용어이며, 즉시 새 montage를 시작한 결과가 아니라 후속 combo 요청이 accepted 된 결과로 설명한다.

### P16

#### Only EN

- Reaction Request
- Reaction Orchestrator
- Reaction Component
- Reaction
- Reaction Request Result
- Reaction Execution Result
- DamageFeedback
- ReactionFeedback
- Damage Impact Info
- active reaction
- incoming reaction
- FReactionOrchestrationQuery
- FReactionOrchestrationResult

#### EN(KR)

- Reaction Request(리액션 요청): TakeDamage 이후 hit / dead reaction 실행 여부를 판단하기 위해 만든 요청.
- Reaction Orchestrator(reaction 요청 조율자): reaction request를 해석하고 실행 여부를 판단하는 component.
- Reaction Component(reaction 실행 상태 component): active reaction state를 적용하고 실행 객체에 넘기는 component.
- Reaction Request Result(리액션 요청 처리 결과): 외부 호출자에게 반환되는 reaction 요청 결과.
- Reaction Execution Result(reaction 실행 적용 결과): Reaction Component가 소비하는 실행 적용 결과.
- FReactionExecutionPolicy(reaction 실행 정책): Reaction Orchestrator가 reaction request를 해석해 실행 적용 방식을 정리한 정책 값.
- DamageFeedback(피격 순간 feedback): damage event와 impact metadata 기준으로 실행되는 hit feedback.
- ReactionFeedback(피격 반응 결과 기반 표현 실행): reaction montage timing과 trigger key 기준으로 실행되는 feedback.
- Damage Impact Info(damage impact 정보): hit VFX / SFX 위치와 방향을 정하기 위해 전달하는 damage impact metadata.

#### Only KR

- 대기 중인 피격 반응 소비 제거: Player tick / AI task에서 pending reaction을 소비하던 흐름을 공통 request 진입점으로 바꾸는 처리.
- 피격 순간 표현: damage가 발생한 순간 위치 / 방향을 기준으로 실행되는 feedback.
- 리액션 실행 표현: reaction montage timing에 맞춰 실행되는 feedback.
- AI 관찰 구조: AI가 reaction을 직접 실행하지 않고 결과 상태를 관찰하는 구조.

#### 검토 결과

- `feature/reaction-orchestration` tip 기준으로 `UCReactionOrchestratorComponent`, `FReactionOrchestrationQuery`, `FReactionOrchestrationResult`, `FReactionRequestResult`, `EReactionRequestResultType`, `ResolveReactionType()`, `CanInterruptActiveReaction()`이 확인됐다.
- P16은 P09의 reaction 실행을 공통 Reaction Request / Orchestrator / Component / Reaction 구조로 재구성한 브랜치다.
- P14의 넓은 `ReactionFeedback` 의미는 P16에서 `DamageFeedback`과 `ReactionFeedback`으로 재분리된다.
- P16 이후에는 피격 순간 feedback과 reaction montage timing feedback을 같은 의미로 쓰지 않는다.
- `Reaction Result` 단독 표준화는 피하고, 외부 반환 결과는 `Reaction Request Result`, component 적용 결과는 `Reaction Execution Result`로 구분한다.

### P17

#### Only EN

- active execution
- incoming execution
- FExecutionInterventionQuery
- FExecutionInterventionDirective
- WantInterventionRules
- AllowInterventionRules
- Interrupted
- StopSource
- SourceDomain
- TargetDomain
- StopReason
- AfterStopAction
- runtime cleanup
- terminal feedback
- snapshot

#### EN(KR)

- intervention(중단 개입): 새로 실행 요청된 행동이 현재 실행 중인 행동을 중단하고 들어올 수 있는지 판단하고 처리하는 구조.
- active execution(현재 실행 중인 행동): 새로 실행 요청된 행동의 중단 대상이 될 수 있는 action 또는 reaction.
- incoming execution(새로 실행 요청된 행동): 현재 실행 중인 행동을 중단하고 진입할 수 있는 후보 action 또는 reaction.
- FExecutionInterventionQuery(중단 판단 정보 구조체): active execution과 incoming execution의 중단 가능성을 평가하기 위한 context 구조체.
- WantInterventionRules(개입 조건 규칙): incoming execution이 active execution을 중단 대상으로 보는지 판단하는 data rule.
- AllowInterventionRules(중단 허용 규칙): active execution이 incoming execution에 의해 중단될 수 있는지 판단하는 data rule.
- FExecutionInterventionDirective(중단 실행 지시값): active execution을 중단한 뒤 incoming execution을 어떻게 적용할지 component에 전달하는 지시값.
- Interrupted(외부 중단 결과): 외부 실행 요청으로 active execution이 멈춘 결과.
- Dodge(회피 action): intervention rule 검증에 사용된 회피 action.
- terminal guard(최종 상태 보호 정책): Dead reaction처럼 일반 intervention rule보다 강하게 유지되는 최종 상태 보호 기준.
- terminal feedback(종료 시점 feedback): action / reaction 종료 시점에 실행되는 feedback.
- runtime cleanup(실행 중 임시 상태 정리): trail / collision / hit context 같은 실행 중 임시 상태를 정리하는 처리.

#### Only KR

- 개입 조건: 새로 실행 요청된 행동이 기존 행동을 끊고 들어오려는 조건.
- 중단 허용 조건: 현재 실행 중인 행동이 새 행동에게 중단을 허용하는 조건.
- 최종 우선권: Dead reaction처럼 일반 data rule보다 강하게 유지되는 정책.
- 후속 작업: 공통 알고리즘 추출, domain 책임 분리, directive source 추적 로그 보강.

#### 검토 결과

- `feature/orchestration-refactor` tip 기준으로 Action / Reaction orchestrator 모두 `FExecutionInterventionQuery`와 `FExecutionInterventionDirective`를 생성하고, `EExecutionStopReason::Interrupted`를 사용한다.
- Action / Reaction data는 Want / Allow intervention rule을 분리해 사용하고, Dead reaction은 일반 intervention rule보다 강한 최종 우선권을 유지한다.
- `Cancel / Interrupt` 분리는 `Interrupted` 중심 stop result로 통합하는 방향이 맞고, `Intervene`는 중심 용어로 확대하지 않는다.
- runtime cleanup과 terminal feedback은 snapshot을 먼저 확보한 뒤 cleanup 이후 실행되는 순서로 설명한다.

### P18

#### Only EN

- AI Workflow
- Prompt Library
- Prompt Blueprint
- Prompt Files
- Work Brief
- Feature Work Planning
- Work List
- Work Brief Intake
- Document Set Audit
- Git Commit PR Preflight
- Backlog
- Refactor Notes
- W01
- W02

#### EN(KR)

- AI Workflow(AI 작업 운영 흐름): Codex와 함께 작업할 때 요청 확인, 계획, 실행, 검증, 문서화를 어떤 순서로 진행할지 정리한 운영 흐름.
- Prompt Library(Prompt 모음): 반복해서 사용할 prompt와 prompt 제작 / 관리 기준을 모은 문서 체계.
- Prompt Blueprint(Prompt 기준 문서): Prompt 작성 형식, 제작 원칙, 프로젝트 적용 기준, 유지보수 기준, 호출 흐름 기준을 관리하는 문서.
- Prompt Files(Prompt 본문): 실제 작업 중 호출할 prompt 본문.
- Project Context(프로젝트 맥락 문서): 프로젝트 개요와 작업 전제 정보를 담는 AI Workflow 문서군.
- Work Pipeline(작업 절차 문서): 요청 확인부터 계획, 실행, 검증까지의 작업 절차를 정리한 문서군.
- Drafts(초안 보관 문서): 검증용 또는 작성 중인 초안 산출물을 임시로 모아두는 문서군.
- Work Brief(작업 요청 요약): 자연어 요청을 작업 목적, 범위, 준비 상태로 정리한 문서.
- Feature Work Planning(기능 작업 계획): Work Brief를 바탕으로 구현 범위, 실행 순서, 검증 기준을 구체화한 계획 문서.
- Work List(작업 목록): 실행 가능한 작업 항목과 완료 기준을 정리한 문서.
- Prompt Flow / Routing(Prompt 호출 흐름): 자연어 요청 이후 어떤 prompt 계층으로 넘어갈지 판단하는 기준.

#### Only KR

- 작업 운영 흐름: AI와 협업할 때 작업이 진행되는 순서.
- 문서 체계: 문서 위치, 개요, 운영 기준, 작업 절차, 후속 관리를 나누는 구조.
- 검증용 예시 문서: 실제 구현 완료 문서가 아니라 자연어 요청 변환 흐름을 확인하기 위한 예시 문서.
- 후속 관리: 실사용 데이터를 기준으로 다시 확인할 작업과 refactor 후보를 관리하는 범위.

#### 검토 결과

- `feature/ai-workflow` tip 기준으로 AI Workflow 문서군, Prompt Blueprint, Prompt Files, W02 Work Brief / Feature Work Planning / Work List 예시 문서가 확인됐다.
- P18은 gameplay 코드 변경이 아니라 Documentation PR이며, `Documentation` 단일 요약 카테고리를 허용한다.
- `Work_List_Draft`는 P18의 검증용 파일명일 뿐, Work List 운영에서 Draft 단계를 반드시 거친다는 의미로 확장하지 않는다.
- UE C++ 빌드 / PIE / Editor / Asset / 실제 Parry 구현 검증은 P18 완료 결과가 아니라 미검증 / 비범위 / 후속 작업으로 유지한다.

## 문서별 Override

### P09

- `active reaction` / `new reaction`은 해당 브랜치 코드 API와 맞으므로 유지 가능하다.
- P16의 `Reaction Orchestrator` 용어를 P09에 소급하지 않는다.

### P10

- 이미 사용자 조율을 많이 거친 문서이므로 대규모 용어 변경은 피한다.
- 요약은 KR 우선, 핵심 개념 이후 `AIContext`, `AIState`, `BehaviorTree`, `Blackboard`, `AttackIndex` 유지.

### P14 / P16

- P14의 `ReactionFeedback`은 P14 기준의 넓은 hit feedback 의미다.
- P16 이후 통합 기준에서는 `DamageFeedback` / `ReactionFeedback` 분리를 기본으로 둔다.
- P14 문서에서는 `P14 기준 ReactionFeedback` 표현을 유지해 시점 차이를 드러낸다.

### P15 / P16

- `orchestrator`, `orchestration`, `request`, `result`는 구조명으로 유지 가능하다. `cleanup`은 `runtime cleanup`, `state cleanup`처럼 대상을 함께 쓴다.
- 단, P08~P14에 소급하지 않는다.

### P17

- `intervention(중단 개입)`, `active execution`, `incoming execution`, `Want / Allow`, `Interrupted`, `runtime cleanup`은 P17 핵심 용어로 유지한다.
- `Intervene`는 중심 용어로 올리지 않는다.
- `Cancel / Interrupt` 통합은 `Interrupted` 중심 stop result 정리로 설명한다.

### P18

- `AI Workflow`, `Prompt Library`, `Prompt Blueprint`, `Prompt Files`, `Work Brief`, `Feature Work Planning`, `Work List`는 문서 체계 용어로 유지한다.
- `Prompt Flow / Routing`은 병기 유지가 가능하며, 문맥상 한쪽만 필요하면 `Prompt Flow` 또는 `Prompt Routing` 중 하나만 사용한다.
- 문서 PR은 `Documentation` 단일 카테고리를 허용한다.
- `Work_List_Draft`는 파일명 또는 검증용 산출물 설명에서만 사용한다.
- 같은 문서 카테고리 안에서 역할이 다르면 `W01 Work List`, `W02 Work List`처럼 구체 라벨을 쓴다.

---

## 확정 기준

- `Pipeline`은 공식 구조 단위 / 제목 / 핵심 개념에서만 유지하고, 본문 설명에서는 `흐름`, `단계`, `처리 경로`로 풀어쓴다.
- P14 `ReactionFeedback`은 `P14 기준 ReactionFeedback`으로 유지한다.
- P17 `intervention`의 한글 병기는 `중단 개입`으로 유지한다.
- P18 `Prompt Flow / Routing`은 병기 유지가 가능하며, 문맥상 한쪽만 필요하면 `Prompt Flow` 또는 `Prompt Routing` 중 하나만 사용한다.
