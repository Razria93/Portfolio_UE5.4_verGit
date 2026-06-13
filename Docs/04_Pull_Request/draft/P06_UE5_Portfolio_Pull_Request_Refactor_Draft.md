# UE5 Portfolio Pull Request

## 제목

**P06: Hit Collision 시스템 구현 및 Collision Window 도입**

## 날짜

**2025.12.25**

## 상태

- [x] **완료**

---

## 브랜치

- `feature/combat-hit-collision`

---

## 요약

이번 PR에서는 **입력으로 실행된 action이 이후 damage 처리로 이어질 수 있도록, 공격 montage의 특정 timing에 hit collision을 열고 target overlap 결과를 Action Callback까지 전달하는 흐름을 구현했다.**

damage 계산은 아직 처리하지 않고, damage 처리로 이어지는 collision 발생 / 제어 / 전달 구조를 구성했다.

성격별 핵심 변경은 다음과 같다.

### Feature

- **Dummy Enemy 배치**: 테스트 레벨에서 공격 overlap을 확인할 수 있도록 기본 Enemy 캐릭터를 추가했다.

- **Collision Window 구성**: 공격 montage notify timing에 맞춰 attachment collision을 켜고 끌 수 있도록 구성했다.

- **Attachment Overlap Event broadcast 구성**: attachment collision에서 target overlap이 발생하면 유효한 overlap 결과만 Attachment Overlap Event로 broadcast하도록 구성했다.

- **Action Callback 수신 연결**: Attachment Overlap Event가 Action Callback까지 도달하도록 연결했다.

### Refactoring

- **collision 제어 책임 분리**: notify는 montage timing만 전달하고, 실제 collision 활성화 / 비활성화는 attachment가 담당하도록 역할을 나눴다.

- **delegate binding 지점 정리**: attachment가 broadcast한 Attachment Overlap Event를 `UCWeaponComponent`가 action callback에 binding하도록 연결 지점을 정리했다.

- **후속 damage 처리 범위 분리**: P06에서는 hit collision 발생과 target overlap 전달까지만 다루고, 실제 damage 계산과 target 수신 처리는 후속 범위로 분리했다.

---

## 핵심 개념

이 섹션은 아래 설명에서 반복되는 프로젝트 고유 용어를 먼저 정리한다.

```text
Collision Window(충돌 허용 구간)
-> 공격 montage 중 attachment collision을 허용하는 구간
```

```text
Attachment Collision(attachment 충돌체)
-> attachment가 소유한 UShapeComponent 기반 공격 판정 collision
```

```text
Attachment Overlap Event(attachment 오버랩 이벤트)
-> attachment에서 target overlap 발생을 외부로 알리는 delegate event
```

```text
Action Callback(Attachment Overlap Event 수신 callback)
-> Attachment Overlap Event를 후속 damage 처리 책임 객체가 받을 수 있도록 연결한 callback
```

---

## 변경 배경

이 섹션은 P05의 입력 기반 action 실행 흐름 이후, action 실행 결과가 damage 처리로 이어지기 위해 hit collision 구조가 필요했던 이유를 정리한다.

### 공격 timing 기반 collision 제어 필요성

3D action combat에서는 입력이 damage로 바로 이어지지 않고, 입력으로 실행된 action montage의 특정 구간에서 공격 collision이 열려야 damage 처리로 이어질 수 있다.

따라서 montage notify가 공격 timing을 알려주고, attachment가 실제 collision 상태를 바꾸는 Collision Window 구조가 필요했다.

### Attachment Overlap Event 전달 필요성

공격 판정 collision은 attachment에서 발생하지만, 충돌 이후 처리는 action 측 callback 흐름에서 이어져야 했다.

따라서 attachment에서 확인한 target overlap 결과가 `UCWeaponComponent`를 거쳐 Action Callback까지 전달되는 연결 구조가 필요했다.

### 후속 damage 처리 확장 준비 필요성

P06 시점에서는 실제 damage 계산이나 target damage 수신을 구현하지 않는다.

대신 collision 발생 규칙과 전달 흐름을 먼저 마련해, 이후 damage 처리로 확장할 수 있는 기반을 준비해야 했다.

---

## 변경 범위

이 섹션은 문제를 어떻게 고쳤고, 그 결과 동작이 어떻게 달라졌는지 정리한다.

### 1. Dummy Enemy 기본 구성

- **왜**:
  attachment collision이 실제 target actor와 overlap되는지 테스트 레벨에서 확인할 대상이 필요했다.

- **어떻게**:
  `ACEnemy`를 `ACharacter` 기반으로 추가하고, capsule, mesh, movement, state component 기본값을 설정했다.

- **결과**:
  테스트 레벨에서 attachment collision overlap을 확인할 수 있는 최소 Enemy target이 준비됐다.

### 2. Attachment collision 수집과 기본 비활성화

- **왜**:
  Attachment에는 여러 collision shape가 있을 수 있고, 공격 구간이 아닐 때는 collision이 꺼져 있어야 했다.

- **어떻게**:
  `ACAttachment::BeginPlay()`에서 하위 `UShapeComponent`를 찾아 overlap delegate를 binding하고, 시작 시 collision을 `NoCollision`으로 비활성화했다.

- **결과**:
  Attachment는 자신이 가진 공격 collision 목록을 관리하고, Collision Window가 열릴 때만 활성화할 수 있다.

### 3. AnimNotify 기반 Collision Window 구성

- **왜**:
  공격 montage의 특정 timing에서만 attachment collision을 켜고 꺼야 했다.

- **어떻게**:
  `UCAnimNotify_Collision`이 owner의 `UCWeaponComponent`를 찾고, 현재 attachment의 collision 활성화 / 비활성화를 담당하는 `CollisionEnabled()` 또는 `CollisionDisabled()`를 호출하도록 구성했다.

- **결과**:
  montage notify timing에 맞춰 attachment collision 상태를 제어할 수 있다.

### 4. CollisionName 기반 선택 활성화

- **왜**:
  하나의 attachment 안에 여러 collision shape가 있을 수 있어, 필요할 때 특정 collision만 열 수 있어야 했다.

- **어떻게**:
  `CollisionEnabled(FName)`에서 이름이 지정되면 해당 이름의 `UShapeComponent`만 활성화하고, 이름이 없으면 저장된 collision 전체를 활성화하도록 구성했다.

- **결과**:
  montage notify에서 전체 collision 또는 특정 collision을 선택적으로 열 수 있다.

### 5. Attachment Overlap Event 구성

- **왜**:
  충돌 이후 처리는 action 측 callback 흐름에서 이어져야 하므로, attachment에서 확인한 target overlap 결과를 Action Callback까지 전달할 수 있어야 했다.

- **어떻게**:
  `ACAttachment::OnComponentBeginOverlap()`에서 `attacker`, `damageCauser`, `attackCollision`, `targetActor`, `hitComponent`의 유효성을 검증했다.
  이후 owner self-overlap을 차단한 뒤에 `OnAttachmentBeginOverlap` event로 관련된 target overlap 결과를 전달하도록 구성했다.

- **결과**:
  Attachment는 유효한 target overlap 결과를 Attachment Overlap Event로 broadcast할 수 있다.

### 6. Attachment Overlap Event와 Action Callback 연결

- **왜**:
  Attachment에서 broadcast한 target overlap 결과는 Action Callback까지 도달해야 충돌 이후 처리 흐름에서 사용할 수 있었다.

- **어떻게**:
  `UCWeaponComponent::BeginPlay()`에서 Attachment Overlap Event를 `UCAction`의 collision / overlap callback에 binding했다.

- **결과**:
  Attachment Overlap Event는 `UCWeaponComponent`를 거쳐 Action Callback까지 전달된다.
  P06 시점에서는 target overlap 결과가 Action Callback까지 도달하는 흐름을 확인하고, 실제 damage 요청 구성은 후속 범위로 남는다.

---

## 주요 처리 흐름

이 섹션은 action montage timing에서 hit collision이 열리고, target overlap 결과가 Action Callback까지 전달되는 대표 흐름을 정리한다.

### Collision Window 흐름

```text
action montage notify
-> UCAnimNotify_Collision 실행
-> UCWeaponComponent에서 현재 attachment 확인
-> FlowType 확인
   - Begin -> attachment collision enable
   - End   -> attachment collision disable
-> UShapeComponent collision 상태 변경
```

이 흐름은 공격 montage의 특정 timing이 실제 attachment collision 상태 변경으로 이어지는 과정을 의미한다.

### Attachment Overlap Event 흐름

```text
Collision Window open
-> UShapeComponent overlap 발생
-> attacker / damage causer / attack collision 확인
-> target actor / hit component 확인
-> owner self-overlap 차단
-> OnAttachmentBeginOverlap event broadcast
```

이 흐름은 attachment가 engine overlap을 받아 유효한 target overlap 결과를 Attachment Overlap Event로 broadcast하는 과정을 의미한다.

### Action Callback binding 흐름

```text
Attachment Overlap Event broadcast
-> UCWeaponComponent binding
-> Action Callback 호출
-> Action Callback에서 target overlap 결과 수신
```

이 흐름은 attachment에서 발생한 target overlap 결과가 `UCWeaponComponent`의 delegate binding을 거쳐 Action Callback까지 전달되는 과정을 의미한다.

---

## 구현 결과

- `ACEnemy` 기반 dummy target을 테스트 레벨에서 사용할 수 있다.

- `ACAttachment`는 하위 `UShapeComponent`를 수집하고, 시작 시 공격 collision을 비활성화한다.

- `UCAnimNotify_Collision`은 montage timing에 맞춰 attachment collision을 켜고 끌 수 있다.

- `CollisionName`을 사용하면 특정 collision만 선택적으로 활성화할 수 있다.

- Attachment Overlap Event는 owner self-overlap을 차단한 뒤 Action Callback까지 전달된다.

- 실제 damage 계산과 target damage 수신은 후속 damage 처리 흐름에서 확장할 수 있는 상태로 남았다.

---

## 테스트 방법

### Dummy Enemy

- 테스트 레벨에 `ACEnemy` 또는 Enemy Blueprint를 배치하고 overlap target으로 인식되는지 확인한다.

- Enemy capsule, mesh, movement 기본값이 테스트 가능한 상태인지 확인한다.

### Collision Window

- 공격 montage에서 `UCAnimNotify_Collision` `Begin` timing에 collision이 활성화되는지 확인한다.

- 공격 montage에서 `UCAnimNotify_Collision` `End` timing에 collision이 비활성화되는지 확인한다.

- `CollisionName`이 지정된 경우 해당 이름의 `UShapeComponent`만 활성화되는지 확인한다.

- `CollisionName`이 없는 경우 저장된 collision 전체가 활성화되는지 확인한다.

### overlap 전달

- Collision Window 안에서 target과 overlap 시 `ACAttachment::OnComponentBeginOverlap()`이 호출되는지 확인한다.

- Owner self-overlap이 무시되는지 확인한다.

- `UCWeaponComponent` binding을 통해 `UCAction::OnAttachmentBeginOverlap()`까지 전달되는지 확인한다.

- Collision Window가 닫힌 뒤 불필요한 Attachment Overlap Event가 발생하지 않는지 확인한다.

---

## 검증 결과

- `ACEnemy` dummy target을 통해 테스트 레벨에서 overlap 진입을 확인했다.

- `UCAnimNotify_Collision` `Begin` / `End` timing 기준으로 collision enable / disable 동작을 확인했다.

- `ACAttachment` 내부 `UShapeComponent` 자동 수집과 overlap delegate binding을 확인했다.

- Owner self-overlap 차단을 확인했다.

- Attachment Overlap Event가 `UCAction` callback까지 전달되는 binding 흐름을 확인했다.

---

## 비범위

- 실제 damage 계산, duplicate hit 방지, target 상태 필터, HP / reaction 처리는 이번 PR에서 구현하지 않는다.

- `UCAction` callback 내부의 damage 요청 구성은 후속 damage 처리 흐름에서 확장한다.

- Attachment Overlap Event는 damage 처리 완료가 아니라 target overlap 결과를 Action Callback까지 전달하기 위한 event로 둔다.

---

## 관련 문서

- Issue Checklist: `D07_UE5_Portfolio_Issue_Checklist.md`

---

## 정리

- P06은 damage 처리로 이어지는 첫 단계로, montage timing에 맞춰 attachment collision을 제어하고 target overlap 결과를 Action Callback까지 전달하는 hit collision 기반 PR이다.

- `UCAnimNotify_Collision`, `ACAttachment`, `UCWeaponComponent`, `UCAction`이 각각 timing 전달, collision / overlap 관리, delegate 연결, 후속 callback 수신 역할을 나눠 갖도록 정리했다.

- damage 계산과 target 수신 처리는 후속 damage 처리 흐름에서 확장할 수 있도록 남겼다.
