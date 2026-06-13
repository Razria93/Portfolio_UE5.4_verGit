# PR Goal Flow Map

현재 기준 범위: P01~P18 PR 문서

P01~P18 PR 문서가 하나의 작업 흐름으로 읽히는지 확인하기 위해, 각 PR의 목표와 앞뒤 연결을 한 곳에 모은다.

이 문서는 PR 문서가 추가될 때마다 갱신하는 목표 / 흐름 기준 문서다. 새 PR이 추가되면 전체 흐름, 의미 단위 흐름, PR별 목표와 흐름을 같은 구조로 갱신한다.

## 운영 기준

- 전체 흐름은 PR의 큰 진행 순서를 한눈에 확인하는 요약으로 유지한다.
- 의미 단위 흐름은 PR 번호 묶음이 아니라 기능 흐름 단위로 구분한다.
- PR별 항목은 `요약 / 목표 / 연결 흐름 / 브랜치 검증` 구조를 유지한다.
- 연결 흐름에는 이어받은 것, 새로 만든 흐름, 후속으로 넘긴 범위를 구분해 적는다.
- 중간 검토 단계의 진행 상태 문장은 남기지 않고, 최종 연결 흐름과 확정 기준만 남긴다.

---

## 전체 흐름

```text
P01 TestRoom / Player / Camera
-> P02 Movement / Locomotion
-> P03 Weapon Equip / Attachment Socket
-> P04 Action Execution Pipeline 기본 구성
-> P05 ComboAttack 규칙 확장
-> P06 Hit Collision / Target Overlap 전달
-> P07 ApplyDamage / TakeDamage 송신 경계
-> P08 TakeDamage 수신 / HP 반영
-> P09 Reaction 실행
-> P10 Enemy AI 행동 결정
-> P11 Player Combat Receiver
-> P12 Player / Enemy 공통 Damage Pipeline
-> P13 Player Combat Loop 1사이클
-> P14 Combat Feedback
-> P15 Action 공용 실행 Pipeline
-> P16 Reaction 공용 실행 Pipeline / Feedback 재분리
-> P17 Intervention Rule / Runtime Cleanup
-> P18 AI Workflow / Prompt Library
```

한 문장으로 보면, P01~P18은 **3인칭 플레이 기반에서 Player 조작, 장착, action, combo, hit collision, damage, reaction, AI, feedback, orchestration을 단계적으로 확장하고, 마지막에 해당 작업을 반복 가능하게 만들기 위한 AI Workflow와 Prompt Library를 구성한 흐름**이다.

---

## 의미 단위 흐름

### P01~P03 플레이 기반 / 장착 준비

- Player 배치, camera, movement, locomotion, weapon equip / unequip, attachment socket 전환까지 구성한다.

### P04~P07 Action 실행 / Damage 송신 경계

- Player action input을 montage 실행과 combo 규칙으로 확장하고, hit collision과 target overlap을 ApplyDamage / TakeDamage 송신 경계까지 연결한다.

### P08~P14 Damage 수신 / Reaction / Feedback

- damage 수신, HP 반영, reaction 실행, Player / Enemy 수신 구조, combat feedback을 구성한다.

### P15~P17 공용 실행 / 중단 개입 구조

- Action / Reaction 실행을 공통 request / orchestration 구조로 정리하고, 이후 intervention rule과 runtime cleanup 순서를 보강한다.

### P18 AI Workflow / Prompt Library

- gameplay 코드 변경이 아니라, AI 협업 방식과 Prompt 사용 방식을 문서 체계로 고정하고, 자연어 요청을 작업 문서로 변환하는 흐름을 제한적으로 검증한다.

---

## PR별 목표와 흐름

### P01

#### 요약

- 3인칭 플레이 확인을 위한 TestRoom, Player, PlayerController, Camera Rig를 구성한 PR.

#### 목표

- 레벨 안에서 Player와 camera를 확인할 수 있는 최소 플레이 환경 구성.

#### 연결 흐름

- 이어받은 것: 신규 프로젝트 초기 상태.
- 새로 만든 흐름: Editor startup map -> TestRoom -> Player / Camera 확인, mouse input -> controller rotation -> camera 회전.
- 후속으로 넘긴 범위: movement, locomotion, weapon equip, combat action.

#### 브랜치 검증

- `feature/character-camera-core` tip 기준으로 TestRoom asset, Player / Controller Blueprint, `ACPlayer`, `ACPlayerController`, `LookYaw` / `LookPitch`, `EditorStartupMap` 구성이 확인됐다.

### P02

#### 요약

- Player 이동 입력을 movement 처리 component로 넘기고, 현재 이동 상태 값을 AnimBP에 전달해 기본 locomotion을 구동한 PR.

#### 목표

- WASD / Walk / Jump 입력과 Idle / Walk / Run / Jump animation 연결.

#### 연결 흐름

- 이어받은 것: P01의 Player / Controller / Camera 기반.
- 새로 만든 흐름: movement input -> UCMovementComponent -> CurrentSpeed / CurrentDirection / bIsFalling -> UCAnimInstance -> AnimBP.
- 후속으로 넘긴 범위: equip / action 중 movement 제한, 고급 locomotion, combat movement.

#### 브랜치 검증

- `feature/character-move-core` tip 기준으로 `UCMovementComponent`, `UCAnimInstance`, movement / walk / jump input binding, locomotion AnimBP, BlendSpace, retarget asset 구성이 확인됐다.

### P03

#### 요약

- sword 입력으로 weapon equip / unequip을 실행하고, montage timing에 맞춰 attachment를 hand / holster socket 사이에서 전환한 PR.

#### 목표

- 무기 장착 상태와 attachment socket 전환을 combat action 이전 단계로 구성.

#### 연결 흐름

- 이어받은 것: P01~P02의 Player / input / movement 기반.
- 새로 만든 흐름: sword input -> weapon type toggle -> UCEquipment equip / unequip montage -> equipment begin broadcast -> ACAttachment socket 재부착.
- 후속으로 넘긴 범위: sword 장착 상태를 조건으로 하는 LightAttack, action execution, hit collision.

#### 브랜치 검증

- `feature/character-weapon-equip` tip 기준으로 `UCWeaponComponent`, `UCEquipment`, `ACAttachment`, equipment notify, sword montage, sword attachment asset 구성이 확인됐다.

### P04

#### 요약

- Player action input이 sword 장착 조건을 거쳐 단일 LightAttack montage 실행으로 이어지는 첫 action 실행 흐름을 만든 PR.

#### 목표

- Action Execution Pipeline의 기본 실행 골격 구성.

#### 연결 흐름

- 이어받은 것: P03의 weapon type과 sword 장착 상태.
- 새로 만든 흐름: action input -> UCWeaponComponent -> UCAction_LightAttack -> FActionData montage 재생 -> UCAnimNotify_Action End -> state / movement 복구.
- 후속으로 넘긴 범위: ComboAttack, hit collision, damage 처리.

#### 브랜치 검증

- `feature/combat-light-attack` tip 기준으로 `UCAction`, `UCAction_LightAttack`, `FActionData`, `UCAnimNotify_Action`, `UCWeaponComponent::PlayAction()` 흐름이 확인됐다.

### P05

#### 요약

- 단일 LightAttack을 3단계 ComboAttack으로 확장하고, 입력 허용 구간 안의 재입력만 다음 공격으로 소비되도록 만든 PR.

#### 목표

- P04의 Action Execution Pipeline 위에 ComboAttack 실행 규칙 추가.

#### 연결 흐름

- 이어받은 것: P04의 action input과 단일 montage 실행 흐름.
- 새로 만든 흐름: action input -> ComboAttack 1타 -> PreInput Window -> bExistPreInput 저장 -> Action Notify Next -> 다음 Combo Step montage.
- 후속으로 넘긴 범위: hit collision, damage, AI 공통 action 실행, 피격 중단.

#### 브랜치 검증

- `feature/combat-combo-attack` tip 기준으로 `UCAction_ComboAttack`, `UCAnimNotify_PreInput`, `UCAnimNotify_Action`의 `Next` flow, `Index`, `bEnablePreInput`, `bExistPreInput` 흐름이 확인됐다.

### P06

#### 요약

- 공격 montage timing에 hit collision을 열고, target overlap 결과를 action callback까지 전달한 PR.

#### 목표

- damage 처리 전 단계로 collision 발생 / 제어 / 전달 흐름 구성.

#### 연결 흐름

- 이어받은 것: P04~P05의 action montage와 notify timing.
- 새로 만든 흐름: action montage notify -> attachment collision enable / disable -> target overlap -> Attachment Overlap Event broadcast -> UCWeaponComponent binding -> Action Callback.
- 후속으로 넘긴 범위: damage 요청 생성, damage 계산, target damage 수신, duplicate hit 방지.

#### 브랜치 검증

- `feature/combat-hit-collision` tip 기준으로 `ACAttachment` collision 수집 / enable / disable, `UCAnimNotify_Collision`, `OnAttachmentBeginOverlap.Broadcast(...)`, `UCWeaponComponent` delegate binding이 확인됐다.

### P07

#### 요약

- action / attachment / equipment context와 target overlap을 결합해 damage 요청으로 만들고, target의 TakeDamage 경계까지 전달한 PR.

#### 목표

- ApplyDamage Pipeline과 target damage 수신 경계 구성.

#### 연결 흐름

- 이어받은 것: P06의 hit collision과 target overlap 전달 흐름.
- 새로 만든 흐름: action context cache -> FHitContext 구성 -> UCApplyDamageComponent::RequestApplyDamage -> DamageSpec 조회 -> DamageResult 계산 -> Target->TakeDamage.
- 후속으로 넘긴 범위: HP 반영, reaction, feedback, duplicate hit policy, data asset 분리.

#### 브랜치 검증

- `feature/combat-apply-damage` tip 기준으로 `FHitContext`, `FAttachmentContext`, `FEquipmentContext`, `FActionContext`, `FOverlapContext`, `UCApplyDamageComponent::RequestApplyDamage`, `DamageSpecMap`, `target->TakeDamage(...)` 흐름이 확인됐다.

### P08

#### 요약

- Enemy가 Unreal `TakeDamage()`를 수신하고, damage 처리 결과를 HP에 반영하는 receiver-side damage 처리 흐름을 만든 PR.

#### 목표

- damage 수신 entry, damage 처리 component, HP 관리 component 책임 분리.

#### 연결 흐름

- 이어받은 것: P07의 ApplyDamage 송신과 `Target->TakeDamage()` 호출 경계.
- 새로 만든 흐름: Enemy `TakeDamage()` -> `UCTakeDamageComponent` -> payload / context / result -> `UCHealthComponent` HP commit.
- 후속으로 넘긴 범위: reaction 실행, feedback 실행, Player 수신 구조, 공통 damage 규칙.

#### 브랜치 검증

- `feature/combat-take-damage` tip 기준으로 `ACEnemy::TakeDamage()`, `UCTakeDamageComponent::RequestTakeDamage()`, `HandleDefaultDamageEvent()`, `FTakeDamagePayload`, `FTakeDamageContext`, `FTakeDamageResult`, `UCHealthComponent::TakeDamage()`가 확인됐다.

### P09

#### 요약

- damage 결과를 기준으로 hit / dead reaction을 선택하고 montage 기반 reaction 실행으로 연결한 PR.

#### 목표

- damage 처리와 reaction 선택 / 실행 책임 분리.

#### 연결 흐름

- 이어받은 것: P08의 accepted damage 결과와 HP / dead state 반영.
- 새로 만든 흐름: TakeDamage result -> Reaction Component -> reaction type 결정 -> reaction data / executor 조회 -> active reaction 교체 판단 -> montage lifecycle.
- 후속으로 넘긴 범위: Player / Enemy 공통 reaction 실행 구조, orchestration 기반 reaction request, feedback 재분리.

#### 브랜치 검증

- `feature/combat-reaction` tip 기준으로 `UCReactionComponent`, `UCReaction`, `FReactionDataKey`, `FReactionData`, `FReactionQueryContext`, `AllowInterruptionBy()`, `WantToInterrupt()`, `CAnimNotifyState_Reaction` 흐름이 확인됐다.

### P10

#### 요약

- Enemy AI가 상황에 따라 순찰, 추격, 교전, 피격, 사망 실행 경로를 선택하도록 AI 행동 결정 흐름을 구성한 PR.

#### 목표

- AIContext / AIState / BehaviorTree branch 기반 Enemy AI 실행 경로 구성.

#### 연결 흐름

- 이어받은 것: P08~P09의 damage / reaction 결과와 전투 상태 흐름.
- 새로 만든 흐름: AIContext 갱신 -> AIState 결정 -> BehaviorTree branch 선택 -> 이동 / 교전 / 공격 / 피격 / 사망 흐름.
- 후속으로 넘긴 범위: AI action 실행을 Player와 같은 공통 action request로 통합, AI combo chain, orchestration.

#### 브랜치 검증

- `feature/ai-behaviortree-core` tip 기준으로 `ACAIController` 초기화, `FAIContext`, `EAIStateType`, `CAIKey`, `CBTService_UpdateAIContext`, `CBTService_UpdateAIState`, `UCWorldSubsystem_CombatEngage`, `AttackIndex`, `SBT_Attack` 흐름이 확인됐다.

### P11

#### 요약

- Player도 Enemy처럼 damage를 받고 HP / reaction / dead 흐름에 들어갈 수 있도록 전투 수신 구조를 구성한 PR.

#### 목표

- Player damage receive, health, reaction, dead, input blocking 연결.

#### 연결 흐름

- 이어받은 것: P08/P09의 Enemy-side TakeDamage / Reaction 구조.
- 새로 만든 흐름: Player `TakeDamage()` -> Player receiver components -> health / reaction / dead state -> input blocking.
- 후속으로 넘긴 범위: Player / Enemy damage 규칙 공유, 공통 Hit Window, feedback.

#### 브랜치 검증

- `feature/player-combat-receiver` tip 기준으로 `ACPlayer::TakeDamage()`, `UCTakeDamageComponent`, `UCHealthComponent`, `UCReactionComponent`, `ConsumePendingReaction()`, `CanActionInput()`, Enemy attack task의 `FActionContext` 전달 흐름이 확인됐다.

### P12

#### 요약

- Player와 Enemy가 같은 damage 송신 / 수신 규칙을 사용하도록 ApplyDamage와 TakeDamage 단계를 정리한 PR.

#### 목표

- 공통 Damage Pipeline, Hit Window 기반 중복 타격 방지, damage amount 의미 분리.

#### 연결 흐름

- 이어받은 것: P07 ApplyDamage, P08 TakeDamage, P11 Player receiver.
- 새로 만든 흐름: Hit Window -> ApplyDamage request -> duplicate hit guard -> TakeDamage -> HP commit -> reaction 연결.
- 후속으로 넘긴 범위: 실제 플레이 combat loop 안정화, feedback, action orchestration.

#### 브랜치 검증

- `feature/combat-core-shared` tip 기준으로 `NotifyHitWindowOpened()` / `NotifyHitWindowClosed()`, `FApplyDamageHitWindowKey`, `DamagedTargetContainer`, duplicate hit guard, `CommittedDamage`, dead state before / after 기준이 확인됐다.

### P13

#### 요약

- Player의 입력, combo, hit window, damage 적용이 하나의 공격 1사이클로 이어지는지 안정화한 PR.

#### 목표

- Player 공격 진입 / 종료, Combo Chain, Hit Window / Damage Pipeline 연결 확인.

#### 연결 흐름

- 이어받은 것: P04~P05 action / combo, P06 collision, P07~P12 damage 처리 흐름.
- 새로 만든 흐름: Player input -> ComboAttack -> PreInput / Combo Chain -> Hit Window -> ApplyDamage -> TakeDamage -> reaction.
- 후속으로 넘긴 범위: combat feedback, AI action 공용 실행, reaction orchestration.

#### 브랜치 검증

- `feature/player-combat-loop` tip 기준으로 `UCAction_ComboAttack`, `bEnablePreInput`, `bExistPreInput`, `NextPlayAction()`, `FActionContext`, hit window open / close, `CommittedDamage` 흐름이 확인됐다.

### P14

#### 요약

- 전투 결과와 action timing을 trail / VFX / SFX / camera shake 같은 체감 feedback으로 연결한 PR.

#### 목표

- ActionFeedback, ReactionFeedback, PlayerFeedback 1차 분리.

#### 연결 흐름

- 이어받은 것: P12/P13의 damage / hit / action timing 흐름.
- 새로 만든 흐름: notify 또는 damage result -> Feedback Request -> ActionFeedback / ReactionFeedback / PlayerFeedback component.
- 후속으로 넘긴 범위: P16의 DamageFeedback / ReactionFeedback 재분리, reaction orchestration 기반 feedback.

#### 브랜치 검증

- `feature/combat-feedback` tip 기준으로 `UCActionFeedbackComponent`, `UCReactionFeedbackComponent`, `UCPlayerFeedbackComponent`, `UCWorldSubsystem_CombatFeedback`, action feedback notify / notify state, Enemy attack-end notify / BT task cleanup 흐름이 확인됐다.

### P15

#### 요약

- Player와 AI가 같은 Action Request와 공통 action 실행 경로를 사용하도록 action 실행 구조를 재구성한 PR.

#### 목표

- action 실행 의도 / 요청 조율 / 실행 상태 / 실제 montage lifecycle 책임 분리.

#### 연결 흐름

- 이어받은 것: P10 AI 행동 결정, P13 Player combat loop, P14 feedback.
- 새로 만든 흐름: Player input 또는 AI decision -> Action Request -> Action Orchestrator -> Action Component -> Action -> Request Result.
- 후속으로 넘긴 범위: reaction request도 같은 방식으로 공용화, intervention / cleanup 고도화.

#### 브랜치 검증

- `feature/action-orchestration` tip 기준으로 `UCActionOrchestratorComponent`, `RequestCombatAction()`, `RequestEquipmentAction()`, `RequestMovementAction()`, `FActionRequestResult`, `EActionRequestResultType`, Started / Rejected / Ignored / Chained / Interrupted 결과가 확인됐다.

### P16

#### 요약

- Player와 AI의 reaction 실행을 공통 Reaction Request / Orchestrator / Component / Reaction 구조로 전환하고 feedback 책임을 재분리한 PR.

#### 목표

- pending reaction consume 제거, reaction request 판단 / 실행 상태 / montage lifecycle / feedback 책임 분리.

#### 연결 흐름

- 이어받은 것: P09 reaction execution, P14 feedback, P15 orchestration 패턴.
- 새로 만든 흐름: Reaction Request -> Reaction Orchestrator -> Reaction Component -> Reaction execution -> DamageFeedback / ReactionFeedback.
- 후속으로 넘긴 범위: intervention rule / stop reason / runtime cleanup 구조 정리.

#### 브랜치 검증

- `feature/reaction-orchestration` tip 기준으로 `UCReactionOrchestratorComponent`, `FReactionOrchestrationQuery`, `FReactionOrchestrationResult`, `FReactionRequestResult`, `CanInterruptActiveReaction()`, DamageFeedback / ReactionFeedback 분리 흐름이 확인됐다.

### P17

#### 요약

- 새 행동이 현재 행동을 끊고 들어올 수 있는지 판단하는 기준을 데이터 규칙으로 분리하고, 행동 종료 후 runtime side effect와 terminal feedback 순서를 보강한 PR.

#### 목표

- Action / Reaction 중단 판단 기준, `Interrupted` 중심 중단 결과, runtime cleanup 순서 정리.

#### 연결 흐름

- 이어받은 것: P15의 Action Orchestrator 구조, P16의 Reaction Orchestrator 구조, P14~P16의 feedback / montage lifecycle 흐름.
- 새로 만든 흐름: incoming execution -> Want rule -> active Allow rule -> directive -> Interrupted stop -> incoming 적용.
- 후속으로 넘긴 범위: 공통 중단 판단 알고리즘 추출, domain별 책임 분리, notify window 개선, directive source 추적 로그.

#### 브랜치 검증

- `feature/orchestration-refactor` tip 기준으로 Action / Reaction orchestrator 모두 `FExecutionInterventionQuery`, `FExecutionInterventionDirective`, `EExecutionStopReason::Interrupted`, Want / Allow intervention rule을 사용한다.

### P18

#### 요약

- Codex 기반 작업을 반복 가능하게 만들기 위해 AI Workflow 문서 체계와 Prompt Library v1 초안을 구성하고, Parry 요청을 예시로 자연어 요청이 작업 문서로 변환되는 흐름을 확인한 문서 PR.

#### 목표

- AI Workflow 문서군, Prompt Blueprint / Prompt Files 분리, Work Brief / Feature Work Planning / Work List 변환 흐름 확인.

#### 연결 흐름

- 이어받은 것: P17까지의 gameplay PR 문서 작성 경험, Work List / Bug Report / PR 문서 보완 과정에서 정리된 문서 운영 방식.
- 새로 만든 흐름: 자연어 요청 -> Work Brief -> Feature Work Planning -> Work List.
- 후속으로 넘긴 범위: W02 Parry 실제 구현, UE 검증, Prompt Library 전면 실사용 검증, AI Workflow / Prompt Library refactor.

#### 브랜치 검증

- `feature/ai-workflow` tip 기준으로 AI Workflow 문서군, Prompt Blueprint, Prompt Files, W02 Work Brief / Feature Work Planning / Work List 예시 문서, Backlog / Refactor Notes가 확인됐다.

---

## 연속성 기준

- P06의 목표는 damage 처리 전 단계의 collision 발생 / 제어 / 전달 흐름이다.
- P07의 목표는 ApplyDamage Pipeline과 target damage 수신 경계다.
- P09의 `active reaction` / `new reaction`은 해당 브랜치 코드 API와 맞으므로 유지 가능하다.
- P10은 AI 행동 결정 흐름을 구성하지만, action 실행은 아직 공통 request 구조로 통합되기 전이다.
- P14는 feedback의 1차 구조이며, P16에서 feedback 책임이 다시 세분화된다.
- P15~P16은 action과 reaction을 각각 request / orchestrator / component / executor 구조로 재정리한다.
- P17의 `intervention` 용어는 P15~P16의 `orchestration` 이후에만 등장해야 한다.
- P18의 문서 / Prompt 운영 용어는 gameplay PR에 소급하지 않는다.
- P18의 `Work_List_Draft`는 검증용 파일명일 뿐, 이후 Work List 운영의 필수 단계처럼 쓰지 않는다.

---

## 확정 기준

- 전체 PR 문서의 관련 문서 섹션명은 `## 관련 문서`로 통일한다.
- P04~P05는 별도 Pipeline이 아니라 Action Execution Pipeline의 기본 구성과 combo 규칙 확장으로 정리한다.
- P06의 `현재 실행 중인 action` 표현은 `Action Callback 수신 대상` 중심으로 정리 완료했다.
- P07 제목은 ApplyDamage Pipeline 중심으로 좁힌다.
- P17 `intervention`의 한글 병기는 `중단 개입`으로 유지한다.
- P18 `Prompt Flow / Routing`은 병기 유지가 가능하며, 문맥상 한쪽만 필요하면 `Prompt Flow` 또는 `Prompt Routing` 중 하나만 사용한다.
