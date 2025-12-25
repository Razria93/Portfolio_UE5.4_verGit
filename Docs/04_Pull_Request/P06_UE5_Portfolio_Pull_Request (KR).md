# 히트-콜리전 시스템 구현 및 AnimNotify 기반 Collision Window 도입

## 제목

✨ feat: AnimNotify 기반 Collision Window 및 Attachment Overlap → Action 라우팅 구현 (#16)

## 요약

- 히트 판정 테스트를 위한 Dummy Target(`ACEnemy`)를 추가하여 오버랩 기반 타격 파이프라인을 레벨에서 즉시 검증 가능하도록 구성함

- `ACAttachment`가 Root 하위 Children에서 `UShapeComponent`를 자동 탐색/캐싱하고, 각 ShapeCollision에 Begin/End Overlap 이벤트를 동적 바인딩하도록 구현함

- 공격 타이밍에 맞춰 Collision을 제어하기 위해 `UCAnimNotify_Collision`을 추가하고, `FlowType(Begin/End)` 기반으로 `CollisionEnabled/CollisionDisabled`를 호출하여 Collision Window를 몽타주 타이밍으로 제어하도록 구성함

- Overlap 처리에서 자기 자신 예외 처리(OwnerCharacter/Attacker 대상 충돌 무시)를 적용하여 자기 충돌로 인한 오작동을 차단함

- Attachment에서 발생한 이벤트를 `UCWeaponComponent`에서 받아 `UCAction`으로 라우팅하도록 바인딩하여, 후속 PR에서 인터페이스 기반 조회/데미지 처리로 확장 가능한 진입점을 마련함


---

## 완료 항목

### 1. Dummy Target 추가 (ACEnemy)

- `ACEnemy` 클래스를 신규 추가함

- 테스트 레벨 배치만으로 Overlap 기반 히트 검증이 가능한 최소 타깃을 제공함


---

### 2. Attachment: ShapeCollision 자동 탐색/캐싱 및 Overlap 바인딩

- `ACAttachment::BeginPlay()`에서 Root 하위 Children 컴포넌트를 수집함

- `UShapeComponent`로 캐스팅 가능한 컴포넌트를 `Collisions_Cached`에 캐싱함

- 캐싱된 ShapeCollision에 다음 Overlap 이벤트를 동적 바인딩함

  - `ACAttachment::OnComponentBeginOverlap`  

  - `ACAttachment::OnComponentEndOverlap`

- 기본 상태에서 `CollisionDisabled()`를 적용하여 Notify로 열기 전에는 충돌이 발생하지 않도록 구성함  

  - 공격 타이밍 외 오버랩 이벤트 노이즈를 차단하기 위함임


---

### 3. AnimNotify 기반 Collision Window 제어 (UCAnimNotify_Collision)

- `UCAnimNotify_Collision` 클래스를 신규 추가함

- AnimNotify의 `FlowType(Begin/End)`에 따라 Attachment Collision 제어를 수행함  

  - `Begin`: `CollisionEnabled(CollisionName)` 호출  

  - `End`: `CollisionDisabled()` 호출

- `CollisionName`으로 특정 Collision만 Enable 가능하도록 구성함  

  - Name 지정 시: 해당 이름과 매칭되는 Collision만 Enable

  - Name 미지정 시: 캐싱된 Collision 전체 Enable

- Collision Window의 개폐 타이밍을 몽타주 트랙에서 명시적으로 제어할 수 있도록 구성함


---

### 4. Attachment 이벤트 → Action 라우팅 (WeaponComponent 바인딩)

- `UCWeaponComponent`에서 Attachment의 델리게이트를 `UCAction` 콜백으로 연결함

- 라우팅 이벤트를 다음과 같이 구성함  

  - Collision Window 이벤트

    - `OnAttachmentCollisionEnabled` → `UCAction::OnAttachmentCollisionEnabled`

    - `OnAttachmentCollisionDisabled` → `UCAction::OnAttachmentCollisionDisabled`

  - Overlap 이벤트

    - `OnAttachmentBeginOverlap` → `UCAction::OnAttachmentBeginOverlap`

    - `OnAttachmentEndOverlap` → `UCAction::OnAttachmentEndOverlap`

- Action이 Overlap을 수신한 뒤, 후속 PR에서 필요 데이터 정리 → 인터페이스 기반 조회/데미지 처리로 확장 가능하도록 진입점만 마련함  

  - 이번 PR에서는 라우팅 파이프라인까지만 포함함


---

### 5. 자기 자신 Overlap 예외 처리

- `ACAttachment` Overlap 처리에서 다음 케이스를 예외 처리함 

  - `OtherActor == OwnerCharacter` (Owner 충돌 무시)

  - `OtherActor == Attacker` 또는 동등 의미의 자기 참조 케이스(구현 기준) (자기 충돌 무시)

- 이를 통해 Owner/Attacker와의 자기 충돌로 인해 히트 판정이 오작동하는 문제를 차단함


---

## 테스트 방법

1. 프로젝트 실행 후 테스트 레벨 진입함

2. 레벨에 `ACEnemy`(Dummy)가 배치되어 있는지 확인함

3. Sword 무기 장착 상태 확인함(Attachment 생성/부착 확인)

4. Collision Window 테스트 수행함  

   - 공격 몽타주 재생 중 `UCAnimNotify_Collision(Begin)` 시점에 Collision이 Enable 되는지 확인함  

   - `UCAnimNotify_Collision(End)` 시점에 Collision이 Disable 되는지 확인함

5. Overlap 라우팅 테스트 수행함  

   - 타깃과 오버랩 발생 시 Attachment Overlap 이벤트가 호출되는지 확인함  

   - 이벤트가 WeaponComponent 바인딩을 통해 Action 콜백으로 전달되는지 확인함

6. 예외 처리 테스트 수행함  

   - Owner/Attacker와의 Overlap이 로직에서 무시되는지 확인함


---

## 관련 이슈 / 브랜치

- 브랜치: `feature/combat-hit-collision`

- 이슈: #16


---

## 노트

- **Collision Window**

  - 실제 타격 판정을 허용하는 시간 구간을 의미함

  - AnimNotify로 열고 닫혀서 의도된 프레임에서만 히트가 발생하도록 제어됨


- **CollisionName 선택 활성화**

  - 하나의 Attachment에 여러 ShapeCollision이 존재할 때, 몽타주 구간별로 유효 Collision을 선택할 수 있도록 구성함

- **책임 분리**

  - `UCAnimNotify_Collision`: 애니메이션 타이밍 기반 Collision Window 제어를 담당함

  - `ACAttachment`: Collision 자동 탐색/캐싱, Collision Enable/Disable, Begin/End Overlap 발생을 담당함

  - `UCWeaponComponent`: Attachment 이벤트를 Action으로 라우팅(바인딩)함
  
  - `UCAction(파생)`: Overlap 수신 후 히트 처리/데이터 정리/인터페이스 조회로 확장함(후속)

---

## In Scope (이번 PR 포함)

- Dummy Target(`ACEnemy`) 추가함

- Attachment의 ShapeCollision 자동 탐색/캐싱 및 Overlap 이벤트 바인딩 구현함

- AnimNotify 기반 Collision Window 제어(`UCAnimNotify_Collision` + Enable/Disable) 구현함

- `CollisionName` 기반 특정 Collision 선택 활성화 지원함

- 자기 자신 Overlap 예외 처리(Owner/Attacker 대상 충돌 무시) 적용함

- Attachment 이벤트를 Action으로 라우팅하는 바인딩 파이프라인 구축함


---

## Out of Scope (이번 PR 제외)

- DamageComponent / ReceiveDamageComponent 구현 및 Overlap 시 실제 데미지 적용 제외함

- 외부와 인터페이스 기반으로 조회/전달하는 시스템 구현 제외함

- 중복 히트 방지(동일 타깃 1회 히트 보장), 팀/상태 기반 타깃 필터링, 히트 리액션 등 전투 로직 완성 단계 제외함


---

## Follow-ups (후속 작업)

- [ ] Action 수신 단계에서 히트 이벤트 데이터 구조 확정함(최소 필드 정의 및 표준화)

- [ ] 인터페이스(예: `IDamageable / IHitReceiver`) 기반 조회/호출 연결함

- [ ] 중복 히트 방지(예: Notify Window 동안 HitActor Set 관리) 추가함

- [ ] 타깃 필터링(팀/상태/무적) 및 히트 처리 정책(관통/다중 히트 등) 정의함

- [ ] 디버그 로그/시각화(Enable 구간, 오버랩 발생 타깃, 현재 CollisionName 등) 보강함


---