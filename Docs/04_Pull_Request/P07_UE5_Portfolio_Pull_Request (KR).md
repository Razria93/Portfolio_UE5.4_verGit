# ApplyDamage 파이프라인 구현: AnimNotify Push + Overlap → ApplyComp 구조로 ApplyDamage Flow 구현

## 제목

✨ feat: ApplyDamage 파이프라인 구현 (AnimNotify Push + Overlap → ApplyComp) (#16)

---

## 요약

- **이전 포트폴리오(기존 구조)** 는 `Attachment Overlap` 이벤트를 `UCAction`이 직접 수신하여 `FDamageEvent`를 구성/커스터마이징하고 `OtherActor->TakeDamage()`까지 호출하는 형태였음
- 해당 구조는 다음 리스크가 있었음

  1. **커플링(순환 의존) 우려**: 액션 실행의 결과(오버랩/피격)가 다시 Action으로 유입되어 원인/결과가 단일 객체에 집중될 수 있음

  2. **UObject 유효성 문제**: 전투 핵심 플로우인 데미지 연산/적용을 유효성 보장이 어려운 `UObject(UCAction)`에 두기 어려움

  3. **WeaponComp Super Object 문제**: `UCWeaponComponent`가 `Attachment/Equipment/Action`을 모두 보유한 상태에서 “라우팅 + 연산 + 적용”까지 몰리면 컴포넌트 역할이 비대화될 수 있음

- 이를 해결하기 위해 이번 포트폴리오에서는 다음 목표로 역할을 분리함

  1. Action 관련 기능들은 `UCActionComponent`로 분리

  2. `UCAction`은 “실행 의도/타이밍” 중심으로 축소

  3. `ACAttachment`는 `DamageCauser` 및 `ContextCarrier` 중심으로 축소

  4. Damage 관련 기능(연산/적용)들은 `UCApplyDamageComponent`로 분리
  
  5. 흐름을 다음과 같이 정리
     `Play Montage`
     → `UCAnimNotify_Action(Begin)`
     → `UCWeaponComponent::PushContextToAttachment()`
     → `ACAttachment::OnComponentBeginOverlap()`
     → `UCApplyDamageComponent::RequestApplyDamage()`
     → `UCApplyDamageComponent::ProcessApplyDamage(Validate → ResolveSpec → ComputeResult → TakeDamage)`


---

## 완료 항목

### 1. ApplyDamage 파이프라인 엔트리 구성: Attachment → ApplyDamageComponent 요청 경로 구축

- `ACAttachment`가 Overlap 발생 시 `FHitContext`를 구성하여 `UCApplyDamageComponent::RequestApplyDamage(const FHitContext&)`로 전달하도록 구현함

- `FHitContext`는 관련 컨텍스트를 결합한, “ApplyDamage 요청의 표준”으로서 다음과 같이 구성함

  - `FOverlapContext` (Overlap 시점의 사실)

  - `FAttachmentContext` / `FEquipmentContext` / `FActionContext` (AnimNotify Push로 백업된 상태 스냅샷)

---

### 2. AnimNotify 기반 Context Push 도입: Action 타이밍에서 “상태 스냅샷” 백업

- `UCAnimNotify_Action`가 `FlowType(Begin/End/Next)`에 따라 `UCAction::BeginPlayAction/EndPlayAction/NextPlayAction`을 호출하도록 구성함

- 액션 측에서 Begin/Next 타이밍에 `PushContextToAttachment()`를 호출하여, **ApplyDamage에 필요한 컨텍스트를 Attachment에 사전 백업**하도록 구현함

  - 현재 구현 기준: `UCAction_ComboAttack::BeginPlayAction()` 및 `UCAction_ComboAttack::NextPlayAction()`에서

    - `FActionContext{ CurrentActionType, ActionIndex }`를 생성

    - `UCWeaponComponent::PushContextToAttachment(const FActionContext&)` 호출

- `UCWeaponComponent::PushContextToAttachment()`는 다음을 수행함

  - `CurrentAttachmentType_Cached` → `FAttachmentContext` 구성

  - `CurrentEquipmentType_Cached` → `FEquipmentContext` 구성

  - 입력으로 받은 `FActionContext`와 함께

  - `IHitContextProducer(Attachment)`에 `SetLastAttachmentContext/SetLastEquipmentContext/SetLastActionContext`로 저장

> Push 단계는 “준비(백업)”로만 제한됨: 타깃 탐색/데미지 연산/Apply 호출을 수행하지 않음.


---

### 3. OverlapContext 표준화: Attachment가 Overlap 시점 컨텍스트를 일관된 형태로 구성

- `ACAttachment::BuildOverlapContext(...)`를 통해 `FOverlapContext`를 표준화함

  - `OwnerActor` = 공격자(Attachment의 OwnerCharacter)

  - `DamageCauser` = Attachment 자신

  - `OverlappedComponent / OverlapShape` = 공격 충돌 컴포넌트(UShapeComponent 캐스팅 결과 포함)

  - `OtherActor / OtherComponent` = 피격 대상 및 피격 컴포넌트 (Sweep 여부/HitResult 포함) 

- `ACAttachment::OnComponentBeginOverlap()`에서 자기 자신 충돌을 차단함

  - `OwnerCharacter_Cached == OtherActor` 즉시 리턴


---

### 4. ApplyDamageComponent 파이프라인 구현: Validate → SpecResolve → Compute → TakeDamage

- `UCApplyDamageComponent`에 ApplyDamage 처리 파이프라인을 구현함

  - `RequestApplyDamage()` → `ProcessApplyDamage()`

  - 처리 단계(현재 코드 기준)

    1. `ValidateRequest(FHitContext)`

    2. `CheckHitRule(FHitContext)` *(현재 TODO, 항상 true)*

    3. `ResolveDamageSpec(FHitContext, OutSpec)`

    4. `ComputeDamageResult(FHitContext, Spec, OutResult)`

    5. `ApplyDamageToTarget(FHitContext, Spec, Result)` → `Target->TakeDamage(...)`

- Spec/Result 구성 요소를 구조체로 분리함

  - `FDamageSpecKey` = (AttachmentType, EquipmentType, ActionType, ActionIndex)

  - `FDamageSpec` = (예: BaseDamage)

  - `FDamageResult` = (예: FinalDamage)

- Spec 조회는 `DamageSpecMap(TMap<FDamageSpecKey, FDamageSpec>)` 기반으로 수행함

  - `BuildSpecKey()`에서 `FHitContext`의 Attachment/Equipment/Action 컨텍스트를 조합해 키를 생성함


---

### 5. TakeDamage 이벤트 표준화: 커스텀 DamageEvent(FDefaultDamageEvent)로 전달 데이터 유지

- `ApplyDamageToTarget()`에서 `FDefaultDamageEvent`를 구성해 `Target->TakeDamage()`로 전달함

  - `FDefaultDamageEvent`는 `FDamageSpecKey / FDamageSpec / FDamageResult`를 포함함

  - `ClassID`는 `EDamageEventTypeId::DefaultDamage(0x1001)`로 정의됨

- InstigatorController는 `Attacker->GetInstigatorController()`를 우선 사용하고, 실패 시 Pawn 캐스팅 기반 fallback을 적용함

  - `DamageCauser`에 `Instigator`가 없을 경우 `Attacker`로부터 유추


---

## 테스트 방법

1. 캐릭터에 `UCWeaponComponent`, `UCActionComponent`, `UCApplyDamageComponent`가 부착되어 있는지 확인함

2. 공격 몽타주 재생 후 `UCAnimNotify_Action(Begin)`이 호출되는지 확인함

3. `UCAction_ComboAttack::BeginPlayAction()`에서 `PushContextToAttachment()`가 호출되는지 확인함

   - Attachment에 `LastAttachmentContext / LastEquipmentContext / LastActionContext`가 저장되는지 로그로 확인함

4. Collision Window 구간에서 타깃과 Overlap 발생 시

   - `ACAttachment::OnComponentBeginOverlap()` 호출 확인

   - `FHitContext`가 정상 구성되는지(OverlapContext + Last*Context) 로그 확인

5. `UCApplyDamageComponent::RequestApplyDamage()` 이후

   - `ValidateRequest → ResolveDamageSpec → ComputeDamageResult → ApplyDamageToTarget` 호출 흐름 확인

   - `Target->TakeDamage()`가 호출되고, 로그에 Request/Apply 데미지가 출력되는지 확인함


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/combat-apply-damage`

- 이슈: #16, #18


---

## 노트

- 본 PR의 메인은 “구조 리팩토링” 자체가 아니라, **전투 시스템 전체 흐름 구축 과정에서 ApplyDamage 파이프라인을 실제로 성립**시키는 것임

- Push는 “준비(상태 스냅샷 백업)”이고, Overlap은 “사실(히트 발생)”이며, ApplyDamageComponent는 “정책/연산/적용”의 단일 책임 지점으로 유지함

- 현재 `UCAction_ComboAttack` 기준으로 ActionContext가 Push되며, 다른 액션에도 동일 패턴 확장이 가능함(액션별 Begin/Next 타이밍에 Push)


---

## In Scope (이번 PR 포함)

- `UCApplyDamageComponent` 추가 및 ApplyDamage 파이프라인 구현(Request→Validate→ResolveSpec→Compute→TakeDamage)

- `FHitContext / FOverlapContext / FDamageSpecKey / FDefaultDamageEvent(EDamageEventTypeId)` 등 데이터 구조/이벤트 표준화

- `UCWeaponComponent::PushContextToAttachment()`를 통한 Attachment 컨텍스트 백업(Attachment/Equipment/Action)

- `ACAttachment` Overlap에서 HitContext 구성 후 ApplyDamageComponent로 요청 전달


---

## Out of Scope (이번 PR 제외)

- 중복 히트 방지(AlreadyHit Set), 팀/상태/무적 기반 타깃 필터링

- `CheckHitRule()` 정책 구현(현재 TODO)

- `RequestStopDamage()` 기반 지속 효과/DoT/오버랩 유지형 처리

- 피격 리액션(경직/넉백/히트스톱 등) 및 전투 연출

- 네트워크 권한(Authority) 및 동기화 정책

- Apply 입력/출력 표준(`FApplyRequest/Result`) 최종 확정 및 데이터 에셋(DB) 분리


---

## Follow-ups (후속 작업)

- [ ] `CheckHitRule()` 구현(중복 히트/팀/무적/상태 정책)

- [ ] `DamageSpecMap`을 DataAsset/DB로 분리(TODO 반영)

- [ ] `RequestStopDamage()` 처리(오버랩 기반 지속 효과/타이머/상태 해제 정책)

- [ ] `IDamageable / IHitReceiver` 인터페이스 기반 적용 호출로 확장(수신자 책임 분리)

- [ ] 디버그 출력 강화(Notify 타이밍/Push 컨텍스트/SpecKey/적용 결과 시각화)


---