# PR Document Audit Findings

## 아카이브 정보

### 문서 성격

- P01~P18 PR Draft 승격 전 총검토 과정에서 사용한 audit finding 기록.

### 사용 이력

- 사용 시점: P01~P18 draft를 정식 PR 문서로 승격하기 전.
- 사용 목적: 본문 보완 기준, 문서별 finding, 사용자 판단 항목 관리.

### 아카이브 전환

- 전환 시점: 2026.06.14, P01~P18 PR 문서 승격 및 PR 문서 관리 파일 분리 이후.
- 전환 사유: 확정 기준은 상위 관리 문서로 흡수됐고, 이 문서는 승격 당시 판단 근거 확인용 기록으로 전환됐기 때문.

### 현재 사용 방식

- 새 PR 문서 운영 기준으로 직접 갱신하지 않고, P01~P18 승격 당시 검토 근거가 필요할 때만 참조한다.

---

## 원본 기준 범위

- P01~P18 PR Draft
- P01~P18 정식 PR 문서 승격 전 본문 보완 기준
- PR Draft 총검토 과정에서 확정한 공통 기준과 문서별 finding

## 당시 운영 기준

- 이 문서는 P01~P18 PR draft 본문 보완에 사용할 확정 기준과 문서별 finding을 관리했다.
- 같은 보완 기준이 여러 파일에 반복되면 하나의 공통 항목으로 합친다.
- 사용자 판단이 끝난 항목은 확정된 기준으로 이동하고, 남은 항목만 별도 표시한다.
- 사용자 판단이 필요한 항목이 없으면 `없음`으로 명시한다.
- PR draft가 추가되면 공통 finding, 문서별 finding, 확정 기준을 같은 구조로 갱신하는 방식이었다.

---

## 우선순위 요약

1. 통합 용어 기준 확정
2. 공통 섹션명 통일
3. 기본형 `변경 범위` 설명 문장 교체
4. 브랜치 시점 오염 방지 표현 정리
5. 추상 표현 축소
6. P17 intervention / cleanup 기준 추가
7. P18 문서 PR / 검증용 예시 문서 기준 추가
8. PR별 제목 / 목표 정합성 확인

---

## 1. 공통 섹션명 통일

대상:
- P06
- P07
- P08~P18

확정 기준:

- `## 관련 문서`로 통일한다.
- 관련 문서는 Issue Checklist / Bug Report / Issue Analysis Report / Work List 등 실제 연결 문서만 둔다.
- 같은 문서 카테고리에 역할이 다른 문서가 여러 개 있으면 `W01 Work List`, `W02 Work List`처럼 구체 라벨을 사용한다.

---

## 2. `변경 범위` 설명 문장 교체

대상:
- P01
- P02
- P04
- P05
- P06
- P07
- P10
- P14
- P15
- P16
- P17
- P18

확정 기준:

- PR별 변경 축을 한 문장으로 요약한다.
- 기본 템플릿 문장을 그대로 쓰지 않는다.
- P17 / P18도 같은 기준을 따르되, 구조 변경 PR / 문서 PR이라는 성격에 맞춰 설명문의 초점을 다르게 둔다.

문서별 방향:

- P17: intervention 판단 기준, 중단 결과, runtime cleanup 순서를 어떤 책임으로 나눴는지 정리.
- P18: 구성한 문서 체계와 각 산출물이 맡는 역할을 정리.

---

## 3. `기반 마련` 표현 정리

대상:
- P01
- P02
- P03
- P06
- P08
- P18 후속 범위 문장

확정 기준:

- 가능해진 동작이나 연결 결과로 바꾼다.
- P18에서는 "못 끝낸 범위"처럼 보이지 않게, 실사용 데이터 기반 재조정 후보나 다음 branch 검토 범위로 쓴다.

---

## 4. `현재 action` 표현 정리

대상:
- P04
- P06
- P07

확정 기준:

- P04: active 개념이 아니라면 `UCWeaponComponent가 소유한 action instance` 또는 `생성된 action instance`로 조정.
- P06: `Action Callback 수신 대상` 또는 `action callback` 중심 표현으로 정리.
- P07: `FActionContext`가 실제 현재 공격 action 정보를 담는 구조라면 유지하되, 핵심 개념과 연결.

---

## 5. `Pipeline` / `Flow` / `흐름` 정리

대상:
- P08
- P09
- P12
- P13
- P15
- P16
- P18

확정 기준:

- 공식 구조명처럼 관리할 처리 축, 제목, 핵심 개념: `Pipeline` 또는 `Flow / Routing` 유지.
- 주요 처리 흐름 H3: `... 흐름` 사용.
- 일반 설명: `흐름`, `단계`, `처리 경로`로 대체.
- 별도 구조 단위가 아닌 세부 구현에는 `Pipeline`을 붙이지 않는다.
- P18의 `Prompt Flow / Routing`은 문서 운영 용어이므로 병기 가능.

---

## 6. Feedback 시점 차이 정리

대상:
- P14
- P16
- P17

확정 기준:

- P14:
  - `P14 기준 ReactionFeedback`이라는 표현 유지.
  - 후속 P16에서 `DamageFeedback / ReactionFeedback`으로 재분리된다는 설명 유지.
- P16:
  - `DamageFeedback`: 피격 순간 feedback
  - `ReactionFeedback`: reaction montage timing feedback
- P17:
  - `terminal feedback`은 종료 시점 feedback임을 핵심 개념 또는 본문에서 설명한다.
  - runtime cleanup 이후에도 snapshot 기반으로 실행되는 feedback임을 유지한다.

---

## 7. `orchestration` / `intervention` 소급 금지

대상:
- P08~P18 전체

확정 기준:

- `orchestration`, `orchestrator`는 P15/P16 이후 구조명으로만 사용한다.
- `intervention`은 P17 이후 구조명으로만 사용한다.
- P08~P14에는 `orchestration` / `intervention`을 소급하지 않는다.
- `Intervene`는 P17 중심 용어로 확대하지 않는다.

---

## 8. 코드 식별자 inline code 기준

대상:
- P01~P18 전체

확정 기준:

- 타입명, 함수명, enum 값, 필드명, 파일명, 브랜치명은 inline code.
- 일반 도메인 용어는 inline code 없이 작성.
- 같은 문장 안에서 같은 식별자가 반복되면 첫 등장만 inline code로 충분하다.

예:

- `UCWeaponComponent`
- `TakeDamage()`
- `EStateType::Idle`
- Action Component
- BehaviorTree
- Blackboard
- Prompt Blueprint
- Work List

---

## 9. 문서 PR 작성 기준 추가

대상:
- P18
- 이후 문서 / Prompt / Workflow PR

확정 기준:

- 문서 PR에서 실제 변경이 문서 체계와 Prompt 체계 구성이라면 `Documentation` 단일 카테고리를 허용한다.
- 기준 문서와 실제 본문을 함께 만들었다면 상위 문서 묶음의 역할과 하위 문서의 역할을 구분해 설명한다.
- 변경 배경은 원인 / 계기 / 대략적인 방향성까지 쓰고, 실제 산출물 목록은 변경 범위에서 쓴다.
- 검증용 예시 문서는 실제 구현 완료 문서처럼 쓰지 않는다.
- `Work_List_Draft`는 파일명 또는 검증용 산출물 설명에서만 사용한다.

---

## 10. PR별 제목 / 목표 정합성 확인

대상:
- P07
- P08
- P09
- P12
- P15
- P16
- P17
- P18

확인 결과:

- P07: `Action Execution Pipeline`은 본문 핵심보다 넓으므로 ApplyDamage Pipeline 중심 제목으로 좁힌다.
- P08: `TakeDamage Pipeline` 유지 적합.
- P09: `Reaction Execution Pipeline` 유지 적합.
- P12: `Damage Pipeline 공유 구조` 유지 적합.
- P15: `Action 공용 실행 Pipeline` 유지 적합.
- P16: `Reaction 공용 실행 Pipeline` 유지 적합.
- P17: `Action / Reaction Intervention Rule 정리 및 Runtime Cleanup 보강` 유지 적합.
- P18: `AI Workflow 운영 체계 및 Prompt Library v1 초안 구성` 유지 적합.

---

## 11. 확정된 기준

다음 항목은 audit 기준으로 확정한 본문 보완 기준이다.

- 전체 PR draft의 관련 문서 섹션명은 `## 관련 문서`로 통일한다.
- `Pipeline`은 공식 구조 단위 / 제목 / 핵심 개념에서만 유지하고, 본문 설명에서는 `흐름`, `단계`, `처리 경로`로 풀어쓴다.
- P04~P05는 별도 Pipeline이 아니라 Action Execution Pipeline의 기본 구성과 combo 규칙 확장으로 정리한다.
- P06의 `현재 실행 중인 action` 표현은 `Action Callback 수신 대상` 중심으로 정리 완료했다.
- P07 제목은 ApplyDamage Pipeline 중심으로 좁힌다.
- P03의 장착 관련 흐름은 `Equipment Pipeline` 같은 공식 Pipeline 용어로 올리지 않고, 장착 실행 흐름으로 설명한다.
- P15 `Request Result`는 branch-backed result 값 기준으로 쓰고, `Chained`는 후속 combo 요청 accepted 결과로 설명한다.
- P16 `Reaction Result` 단독 표준화는 피하고, 외부 반환 결과는 `Reaction Request Result`, component 적용 결과는 `Reaction Execution Result`로 구분한다.
- P17 cleanup 관련 API 이름은 용어 기준으로 확대하지 않고, 코드 검증 근거로만 사용한다.
- P18 Prompt Files 하위 카테고리는 통합 용어 기준으로 올리지 않고, 필요한 경우 해당 문서 안의 세부 설명으로만 쓴다.
- P14 `ReactionFeedback`은 `P14 기준 ReactionFeedback`으로 유지한다.
- P17 `intervention`의 한글 병기는 `중단 개입`으로 유지한다.
- P18 `Prompt Flow / Routing`은 병기 유지가 가능하며, 문맥상 한쪽만 필요하면 `Prompt Flow` 또는 `Prompt Routing` 중 하나만 사용한다.

사용자 판단 필요:

- 없음.

---

## 문서별 Finding 요약

### P01

- TestRoom / Player / Camera 구성 목표는 명확하다.
- `feature/character-camera-core` tip 기준으로 TestRoom asset, Player / Controller Blueprint, `ACPlayer`, `ACPlayerController`, camera component 구성, `LookYaw` / `LookPitch`, `EditorStartupMap` 구성이 확인됐다.
- Draft는 Non-Draft의 핵심 범위를 누락하지 않고 최신 PR 양식으로 재구성되어 있다.
- 남은 항목은 코드 정합성 문제가 아니라 표현 품질 점검에 가깝다. `변경 범위` 설명문은 P01의 TestRoom / Player / Camera / Look Input 구성 기준으로 더 구체화할 수 있다.
- `기반 마련` 계열 표현은 가능해진 동작 중심으로 관리한다.

### P02

- movement input / runtime value / AnimBP parameter 연결 흐름은 유지한다.
- `feature/character-move-core` tip 기준으로 `UCMovementComponent`, `UCAnimInstance`, movement / walk / jump input binding, locomotion AnimBP, BlendSpace, retarget asset 구성이 확인됐다.
- Draft는 Non-Draft의 movement / locomotion 핵심 범위를 최신 양식으로 재구성하고 있다.
- `이동 결과`, `공급 분리`처럼 의미가 추상적인 표현은 구체 동작 중심으로 관리한다.
- branch tip 코드 기준 API 표기는 `Press_Walk` / `Release_Walk` / `Press_Jump` / `Release_Jump`와 Draft 표기의 차이가 없는지 유지 관리한다.
- `후속 gameplay 확장 기준 마련`은 실제로 가능해진 movement 제어 지점 중심 표현으로 관리한다.
- `MovementComponent`는 Only EN 기준으로 유지하므로, Draft 본문에서 불필요한 병기를 만들지 않는다.

### P03

- weapon equip / unequip, hand socket / holster socket 전환 흐름은 유지한다.
- `feature/character-weapon-equip` tip 기준으로 `UCWeaponComponent`, `UCEquipment`, `ACAttachment`, `FEquipmentData`, equipment notify, sword montage, sword attachment asset 구성이 확인됐다.
- Draft는 Non-Draft의 weapon equip / unequip 핵심 범위를 누락하지 않고 최신 양식으로 재구성되어 있다.
- `spawn` / `생성` 표현은 Actor와 UObject 생성 방식 차이를 반영해 구분한다.
- P03의 `기반을 마련했다` 표현은 문맥상 허용 가능하지만, 최종 일괄 보완 시 가능해진 동작 중심으로 더 좁힐 수 있다.
- 장착 관련 흐름은 공식 `Pipeline`처럼 쓰기보다 equipment 실행 흐름, 장착 / 해제 lifecycle, socket 전환 흐름으로 설명한다.

### P04

- action input -> action instance -> montage lifecycle 흐름은 유지한다.
- `PlayAction()`처럼 같은 이름의 API와 action instance method가 겹치는 표현은 호출 주체를 명확히 한다.
- 브랜치 검증 결과, `UCWeaponComponent::PlayAction()` 진입점과 `UCAction::PlayAction()` method가 함께 있으므로 `PlayAction()` 단독 표기는 피한다.
- `UCAnimNotify_Action`은 `UCWeaponComponent::GetAction()`으로 얻은 action instance에 `Begin_PlayAction()` / `End_PlayAction()`을 호출하므로, `현재 action`보다 `UCWeaponComponent가 소유한 action instance` 표현이 더 정확하다.
- P04는 Action Execution Pipeline의 기본 실행 골격을 구성한 문서로 정리한다.
- `Action Execution Pipeline(action 실행 흐름)`은 P04에서 처음 정의하고, P05에서는 이 흐름 위에 ComboAttack 규칙을 확장한 것으로 연결한다.

### P05

- Combo Step / PreInput Window / Next timing 흐름은 유지한다.
- Combo 관련 용어는 P06 이후 damage / collision 흐름과 섞이지 않게 유지한다.
- 브랜치 검증 결과, `UCAnimNotify_PreInput`이 `bEnablePreInput`을 열고 닫고, `UCAnimNotify_Action`의 `Next` flow가 `Next_PlayAction()`으로 연결되는 구조가 확인됐다.
- `Index`, `bEnablePreInput`, `bExistPreInput`은 branch-backed 용어이므로 핵심 흐름 설명에서 유지 가능하다.
- P05는 P04의 Action Execution Pipeline 위에 ComboAttack 실행 규칙을 추가한 문서로 정리한다.
- P05 제목 / 목표는 별도 ComboAttack Pipeline이 아니라 Action Execution Pipeline의 ComboAttack 확장으로 정리한다.

### P06

- hit collision 발생 / 제어 / 전달 흐름이 핵심이다.
- `현재 실행 중인 action` 표현은 Action Callback 수신 대상 중심으로 정리 완료했다.
- broadcast / binding은 delegate 동작을 구분하기 위해 EN 유지가 적합하다.
- 브랜치 검증 결과, `ACAttachment`가 `UShapeComponent` 기반 collision을 수집하고 `UCAnimNotify_Collision`이 collision enable / disable을 호출하는 구조가 확인됐다.
- `ACAttachment::OnComponentBeginOverlap()`은 유효한 overlap을 검증한 뒤 `OnAttachmentBeginOverlap.Broadcast(...)`로 전달하므로, 이 구간은 `정리한다`보다 `broadcast한다` / `전달한다`가 더 정확하다.
- `UCWeaponComponent`가 Attachment Overlap Event를 action callback에 binding하므로, P06의 핵심 동사는 collision 제어, broadcast, binding, 전달로 고정한다.
- `CollisionName`은 Only EN 단독 용어가 아니라 `CollisionName(collision 선택 이름)`으로 병기해, 여러 collision 중 선택 활성화에 쓰는 이름임을 설명한다.

### P07

- ApplyDamage Pipeline과 target의 `TakeDamage()` 경계가 핵심이다.
- `weapon context`처럼 브랜치 시점에 없는 용어가 섞이지 않도록 용어표 기준을 유지한다.
- `context`는 핵심 구조체와 직접 연결되므로 Only EN 유지가 적합하다.
- 브랜치 검증 결과, P07의 canonical context는 `FAttachmentContext`, `FEquipmentContext`, `FActionContext`, `FOverlapContext`, `FHitContext`다.
- `UCApplyDamageComponent`는 `ValidateRequest -> CheckHitRule -> ResolveDamageSpec -> ComputeDamageResult -> ApplyDamageToTarget` 순서로 처리하므로, ApplyDamage 단계 설명은 이 흐름을 기준으로 유지한다.
- `target->TakeDamage(...)`는 branch tip 코드에서 확인되는 최종 송신 경계이므로 `TakeDamage Boundary` 표현은 유지 가능하다.
- P07 제목과 목표는 ApplyDamage Pipeline 중심으로 좁힌다.
- `Action Execution Pipeline`은 P07 핵심보다 넓으므로, P07 본문에서는 ApplyDamage Pipeline / TakeDamage Boundary 중심으로 설명한다.

### P08

- TakeDamage 수신 entry, damage 처리 component, HP commit 책임 분리는 유지한다.
- `기반 마련` 표현은 가능해진 damage 수신 / 반영 동작으로 구체화해 관리한다.
- 브랜치 검증 결과, `ACEnemy::TakeDamage()`는 entry / routing을 담당하고, 실제 처리 단계는 `UCTakeDamageComponent`의 Payload / Context / Result 흐름과 `UCHealthComponent` HP commit으로 분리된다.
- Reaction / feedback은 P08 코드에서 직접 실행되지 않으므로, P08 완료 결과가 아니라 후속 범위로 유지한다.

### P09

- active reaction / new reaction 판단은 해당 브랜치 코드 API와 맞으므로 유지 가능하다.
- P16의 Reaction Orchestrator 용어를 소급하지 않는다.
- 브랜치 검증 결과, P09의 실제 용어는 `active reaction` / `new reaction`, `AllowInterruptionBy()` / `WantToInterrupt()`이며, priority 기반 단순 교체가 아니다.
- `Reaction Window`는 notify state가 active reaction의 interruptible / cancelable / immune flag를 조정하는 구간으로 유지한다.

### P10

- AIContext / AIState / BehaviorTree / Blackboard 용어는 핵심 개념 이후 유지한다.
- 요약은 KR 우선 원칙을 유지하고, 본문에서만 필요한 EN 용어를 사용한다.
- 브랜치 검증 결과, `AIContext -> AIState -> BehaviorTree branch` 흐름과 `CAIKey` 기반 Blackboard key, `AttackIndex`, `SBT_Attack` asset이 확인됐다.
- P10은 action request 공용화 이전 브랜치이므로 P15 이후의 Action Request / Orchestrator 용어를 소급하지 않는다.
- Engage와 Attack은 같은 전투 흐름에서 역할이 다르다. Engage는 교전 참여 상태, Attack은 Engage 안에서 조건이 맞을 때 실행되는 상세 행동으로 설명한다.
- `AttackableTime(공격 가능 시간)`은 Engage 안에서 Attack branch로 들어갈 수 있는 시간 조건으로 설명한다.

### P11

- Player damage receive / health / reaction / dead / input blocking 흐름은 유지한다.
- 여러 component를 나열할 때는 같은 문장 안에서 inline code 기준을 일관되게 적용한다.
- 브랜치 검증 결과, `ACPlayer::TakeDamage()`는 `UCTakeDamageComponent::RequestTakeDamage()`로 damage 수신을 넘기고, `UCHealthComponent` / `UCReactionComponent` / `UCStateComponent`가 HP, pending reaction, dead state, input blocking에 연결된다.
- `ConsumePendingReaction()`은 Player tick에서 pending reaction context를 소비하므로, P11은 Player가 damage를 받은 뒤 피격 반응 흐름에 진입할 수 있게 한 브랜치로 정리한다.
- Enemy attack task는 `FActionContext`에 `AttackActionType`과 `AttackIndex`를 담아 attachment context로 전달하므로, Player 수신 구조와 Enemy 공격 context 연결을 함께 유지한다.
- Enemy 공격 기준 정보를 설명할 때 `FHitContext(타격 정보)`가 damage 요청에 전달하는 action / attachment / equipment / overlap 정보를 함께 확인한다.

### P12

- Player / Enemy 공통 Damage Pipeline, Hit Window, damage amount 의미 분리는 유지한다.
- `Shared Damage Flow` / `Damage Pipeline` 표기는 통합 용어 기준에 맞춰 관리한다.
- 브랜치 검증 결과, `FApplyDamageHitWindowKey`, `NotifyHitWindowOpened()`, `NotifyHitWindowClosed()`, `DamagedTargetContainer`를 기준으로 같은 hit window 안의 duplicate hit을 차단한다.
- P12 branch의 결과 damage 명칭은 `CommittedDamage` 중심이다. 과거 branch의 `FinalAppliedDamage` 같은 표현을 P12에 소급하지 않는다.
- `CommittedDamage`, dead state before / after, hit / dead reaction 연결 기준은 Draft와 코드가 맞으므로 유지한다.
- ApplyDamage / TakeDamage의 Payload / Context / Result 구조체는 각각 원본 요청 / 처리 정보 / 처리 결과로 병기해 설명한다.
- P12에서 `Hit Window`는 EN(KR) 병기 기준으로 유지하고 Only EN 단독 중복 분류를 만들지 않는다.

### P13

- Player combat loop 1사이클 검증 흐름은 유지한다.
- 입력, combo, Hit Window, ApplyDamage, reaction이 하나의 흐름으로 이어지는지 정리 문장을 유지한다.
- 브랜치 검증 결과, `UCAction_ComboAttack`은 `bEnablePreInput` / `bExistPreInput` / `NextPlayAction()`으로 combo 입력을 소비하고, `FActionContext`를 attachment에 전달한다.
- P13의 핵심은 새 공식 Pipeline 도입이 아니라 P04~P12에서 만든 action, combo, hit window, ApplyDamage / TakeDamage 흐름이 실제 Player combat loop로 이어지는지 확인하는 것이다.
- `action orchestration`은 후속 PR 범위 문장에만 남기고, P13 완료 구조처럼 쓰지 않는다.

### P14

- ActionFeedback / ReactionFeedback / PlayerFeedback 1차 분리 구조는 유지한다.
- P14 기준 ReactionFeedback과 P16 이후 DamageFeedback / ReactionFeedback 재분리의 시점 차이를 명시한다.
- 브랜치 검증 결과, `UCActionFeedbackComponent`는 action timing 기반 `FActionFeedbackRequest`를 받아 trail / VFX / SFX를 처리한다.
- `UCReactionFeedbackComponent`는 `FTakeDamagePacket`의 accepted 여부와 `CommittedDamage`를 기준으로 hit stop, hit VFX, hit SFX, camera shake request를 처리한다.
- `UCPlayerFeedbackComponent`는 `UCWorldSubsystem_CombatFeedback`의 camera shake request를 local player controller 기준으로 소비하므로, player-local feedback은 별도 책임으로 유지한다.
- Enemy attack 종료는 `UCAnimNotify_EndEnemyAttack`의 signal과 `CBTTask_EndAttack`의 AttackIndex / AttackActionType / attachment context / movement cleanup 책임을 구분해 설명한다.

### P15

- Action Request / Orchestrator / Component / Action 책임 분리 흐름은 유지한다.
- Action Request 표준화, AI Action 실행 경로 재구성, AI Combo Chain 재호출 구조의 표현 기준을 유지한다.
- 브랜치 검증 결과, `UCActionOrchestratorComponent`는 movement / equipment / combat request를 받고 `FActionRequestResult`로 Rejected / Ignored / Handled / Started / Chained / Enqueued / Interrupted 같은 결과를 반환한다.
- `RequestCombatAction()`은 resolved action type을 `UCActionComponent::ExecuteAction()`으로 넘기므로, P15는 Player와 AI의 실행 의도를 공통 Action Request로 표준화한 브랜치로 정리한다.
- AI는 BehaviorTree 안에서 montage를 직접 실행하지 않고 Action Request 경로로 넘기는 구조이므로, P10의 AI 행동 결정 흐름과 P15의 공통 action 실행 흐름을 구분한다.
- P15 이전 문서에는 Action Orchestrator / orchestration 용어를 소급하지 않는다.
- `ChainWindowOpened`는 Only EN 단독보다 `ChainWindowOpened(연계 가능 구간 열림 event)`로 병기해 Combo Chain의 event 성격을 설명한다.
- `Request Result`는 branch-backed enum 값 기준으로 설명하고, `Chained`는 즉시 새 montage 시작이 아니라 후속 combo 요청 accepted 결과로 쓴다.

### P16

- Reaction Request / Orchestrator / Component / Reaction 구조와 feedback 재분리 흐름은 유지한다.
- action flow와 reaction flow의 표현 방식은 유사하되, reaction이 더 최신 구조라는 차이는 유지한다.
- 브랜치 검증 결과, `UCReactionOrchestratorComponent`는 `FDamageReactionRequest`를 받아 reaction type, reaction context, execution policy, active / incoming 관계를 판단한다.
- `FReactionRequestResult`는 외부 반환용이고, orchestration / execution result는 component 적용용이므로 결과 구조 분리를 유지한다.
- `FReactionExecutionPolicy(reaction 실행 정책)`는 Reaction Orchestrator가 실행 적용 방식을 정리한 정책 값으로 병기한다.
- `Reaction Result` 단독 표현은 모호하므로, 요청 반환인지 component 적용 결과인지에 따라 `Reaction Request Result`와 `Reaction Execution Result`로 구분한다.
- P16부터 `DamageFeedback`과 `ReactionFeedback`이 분리된다. P14의 넓은 ReactionFeedback 의미를 P16 이후 기준으로 소급 해석하지 않는다.
- reaction flow는 P15 action flow와 유사한 request / orchestrator / component / executor 구조를 갖지만, active reaction 판단과 hit / dead reaction 우선순위가 포함된다는 차이를 유지한다.

### P17

- intervention, Want / Allow, Interrupted, runtime cleanup, terminal feedback 기준을 통합 용어 기준에 반영한다.
- `Intervene`는 중심 용어로 확대하지 않는다.
- 완료 결과와 후속 작업이 섞이지 않게 유지한다.
- 브랜치 검증 결과, Action / Reaction orchestrator 모두 `FExecutionInterventionQuery`를 만들고 Want / Allow rule을 확인한 뒤 `FExecutionInterventionDirective`를 생성한다.
- 외부 요청으로 현재 행동이 멈춘 결과는 `EExecutionStopReason::Interrupted` 중심으로 정리되므로, `Cancel / Interrupt` 분리 표현을 되살리지 않는다.
- Dead reaction은 일반 intervention rule보다 강한 최종 우선권을 유지하므로, P17 문서에서는 data rule 편입 범위와 terminal guard 범위를 분리한다.
- runtime cleanup은 terminal feedback / finish event snapshot 확보 이후 수행되는 순서로 유지한다.
- `Dodge(회피 action)`는 intervention rule 검증에 사용된 action으로 병기 가능하다.
- `terminal guard(최종 상태 보호 정책)`는 Dead reaction처럼 일반 rule보다 강하게 유지되는 최종 상태 보호 기준으로 설명한다.
- `CleanupRuntimeEffects()`, `ClearRuntime()`, `PlayFeedbackRequest` 같은 API 이름은 본문 용어로 확대하지 않고, 코드 검증 근거로만 사용한다.

### P18

- 문서 PR은 Documentation 단일 카테고리를 허용한다.
- Prompt Blueprint / Prompt Files, Work Brief / Feature Work Planning / Work List 역할 구분을 유지한다.
- `Work_List_Draft`는 파일명 또는 검증용 산출물 설명에서만 사용한다.
- 브랜치 검증 결과, `feature/ai-workflow`에는 AI Workflow 문서군, Prompt Blueprint, Prompt Files, W02 Work Brief / Feature Work Planning / Work List 예시 문서가 포함된다.
- P18은 실제 UE C++ 기능 구현 PR이 아니므로 Build / PIE / Editor / Asset / Parry 구현 검증은 완료 결과가 아니라 미검증 항목, 비범위, 후속 작업으로 유지한다.
- `Work_List_Draft`는 P18의 검증용 파일명이며, 일반 Work List 작성 프로세스가 항상 Draft 단계를 거친다는 의미로 확장하지 않는다.
- Prompt Library v1은 전면 실사용이 아니라 초안 구성과 제한적 W02 변환 검증 단계로 설명한다.
- `Project Context`, `Work Pipeline`, `Drafts`는 AI Workflow 문서군 안의 역할을 설명하는 병기 대상이다.
- Prompt Files 하위 카테고리인 Work Planning / Document Writing / Review Verification / Git Operation은 통합 용어 기준으로 올리지 않고, 필요하면 P18 본문 안의 세부 구성 설명으로만 쓴다.

## 최종 반영 상태

- P01~P18 draft 총검토에서 확인한 finding은 최종 finding 문서에 반영했다.
- `orchestration` 소급 금지, feedback 시점 차이, inline code 기준은 확정 기준으로 유지한다.
- P17/P18에서 확정된 `intervention`, `Documentation` 단일 카테고리, 검증용 예시 문서 기준을 반영했다.
- 이 문서에는 archive 전환 당시 확정된 기준과 PR draft 본문 finding만 남겼다.

## 당시 PR Draft 보완 절차

1. `02_PR_Term_Usage_Map.md` 기준 확정
2. 섹션명 / 설명 문장 / 추상 표현 같은 저위험 항목 일괄 수정
3. P04~P07처럼 확정된 기준은 draft 보완 시 일괄 반영
4. P14 / P16 / P17 feedback 시점 차이 확인
5. P18 문서 PR 기준을 후속 문서 PR에 적용
6. `git diff --check`
7. 잔존 표현 검색

---

## Audit 중간 산출물 정리 결과

대상:

- `P01-P07_Draft_Audit_Findings.md`
- `P01-P07_PR_Goal_Flow_Map.md`
- `P01-P07_Term_Usage_Map.md`
- `P01-P16_Draft_Batch_Fix_Candidates.md`
- `P01-P16_Integrated_Term_Standard.md`
- `P01-P18_Draft_Batch_Fix_Candidates.md`
- `P01-P18_Integrated_Term_Standard.md`
- `P08-P16_Draft_Audit_Findings.md`
- `P08-P16_PR_Goal_Flow_Map.md`
- `P08-P16_Term_Usage_Map.md`
- `P17-P18_Draft_Audit_Findings.md`
- `P17-P18_PR_Goal_Flow_Map.md`
- `P17-P18_Term_Usage_Map.md`
- `Term_Usage_Reclassification_Draft.md`
- `Term_Usage_Reclassification_Findings.md`
- `Term_Usage_Consistency_Findings.md`

판단:

- 위 문서들은 장기 유지 기준 문서가 아니라 P01~P18 draft 총검토 과정에서 생성된 중간 산출물이다.
- 용어 분류 기준은 `01_PR_Document_Usage_Guide.md`에 흡수됐다.
- 실제 용어 목록과 문서별 사용 현황은 `02_PR_Term_Usage_Map.md`에 반영됐다.
- PR별 목표와 흐름은 `03_PR_Goal_Flow_Map.md`에 반영됐다.
- 본문 finding과 확정 기준은 archive 전환 전 `04_Draft_Audit_Findings.md`에 반영됐고, 이후 이 문서로 보관됐다.
- 삭제 전 미결 항목 검색 결과, 남은 항목은 현재 미결이 아니라 과거 검토 이력으로 판단했다.

처리 방침:

- 중간 audit 산출물은 삭제 완료했다.
- 이후 새 기준을 추가할 때는 중간 산출물이 아니라 현재 PR 문서 관리 문서에 직접 반영한다.

현재 PR 문서 관리 문서:

- `01_PR_Document_Usage_Guide.md`
- `02_PR_Term_Usage_Map.md`
- `03_PR_Goal_Flow_Map.md`
