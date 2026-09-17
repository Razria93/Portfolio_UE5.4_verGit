# S39. Weapon Pivot and Socket Transition Architecture

> 상태: **현행 코드 계약**
> 갱신일: 2026-09-15
> 대상: `ACWeaponActor`, `UCWeaponComponent`, Weapon Transform Notify State

---

## 1. 결론

무기의 안정된 장착 자세는 **캐릭터 전용 소켓 하나만** 결정한다.

- 손 자세: 캐릭터의 Hand 소켓 Transform
- 수납 자세: 캐릭터의 Holster 소켓 Transform
- 무기 종류별 위치·회전·스케일 차이: 해당 캐릭터의 전용 소켓에서 조정
- 무기 Blueprint의 추가 Hand/Holster Relative Transform: 사용하지 않음
- `WeaponMeshComponent`의 Relative Transform: 허용하며 피벗 좌표 변환에 반영한다. Identity를 권장하지만 강제하지 않는다.

무기 내부 `HandGrip` 소켓은 장착 위치를 정하지 않는다. 이 소켓은 무기 연출을 위한 **회전 중심 `P`**만 표시한다.

---

## 2. 두 소켓의 역할

| 구분 | 소유자 | 역할 |
| --- | --- | --- |
| Hand/Holster 장착 소켓 | 캐릭터 Skeletal Mesh | 무기 액터 전체의 최종 장착 위치·회전·스케일 결정 |
| `HandGrip` 피벗 소켓 | 무기 Skeletal Mesh | 무기 내부 피벗의 위치와 방향 표시 |

캐릭터 소켓에 설정한 스케일도 장착 결과의 일부다. 예를 들어 Celeste 전용 소켓의 `(4,4,4)`는 무기 원본 에셋의 크기 차이를 보정하며, `ActorRootComponent`가 그 소켓을 부모로 삼아 그대로 상속한다.

Animation Editor의 Preview Asset은 `BP_CWeaponActor`를 생성하지 않는다. 따라서 애니메이션 제작 화면에서도 올바른 무기 크기와 자세가 필요하다면 캐릭터 전용 소켓에서 보정하는 현재 방식이 타당하다.

---

## 3. 컴포넌트 계층

```text
Character Skeletal Mesh
└─ Character Hand/Holster Socket
   └─ ActorRootComponent
      └─ PivotComponent
         └─ ContentRootComponent
            ├─ WeaponMeshComponent
            ├─ Collision Components
            └─ 함께 회전할 기타 콘텐츠
```

| 컴포넌트 | 책임 | 안정 상태 Relative Transform |
| --- | --- | --- |
| `ActorRootComponent` | 캐릭터 소켓 부착 및 손↔홀스터 전환 중 월드 보간 | Identity |
| `PivotComponent` | `HandGrip`의 기준 변환 `P`와 런타임 회전 `R` 합성 | `PivotRestTransform` |
| `ContentRootComponent` | 고정 역보정 `P⁻¹`을 모든 무기 콘텐츠에 공통 적용 | `PivotRestTransform.Inverse()` |
| `WeaponMeshComponent` | 무기 형상과 무기 소켓 제공. Relative Transform이 있으면 피벗 위치·방향 계산에도 반영 | 에셋에서 작성한 값(Identity 권장) |

`ContentRootComponent`가 존재하는 이유는 `P⁻¹`을 Mesh·Collision·FX에 각각 중복 적용하지 않고 한 곳에서 적용하기 위해서다. 피벗 회전을 따라야 하는 Blueprint 컴포넌트는 반드시 `ContentRootComponent` 아래에 둔다.

코드의 멤버 이름은 위 표를 따른다. 기존 Blueprint 직렬화 호환을 위해 네이티브 Default Subobject의 내부 객체 이름은 당분간 `RootScene`, `PresentationPivot`, `PresentationContentRoot`를 유지한다.

---

## 4. 적용되는 Transform 레벨

최종 월드 결과까지 적용 순서를 나누면 다음과 같다.

```text
1. 캐릭터 Bone 애니메이션 Transform
2. 캐릭터 Hand/Holster 소켓의 Relative Transform
3. ActorRootComponent의 Transform
   - 안정 장착 상태: Identity
   - 소켓 전환 중: Source/Target 소켓의 월드 Transform 사이 보간값
4. PivotComponent의 Transform
   - 기본: P
   - 회전 중: R과 P의 합성값
5. ContentRootComponent의 Transform
   - 항상 P⁻¹
6. WeaponMeshComponent의 Transform
   - 에셋에서 작성한 Relative Transform
   - 이 값은 피벗을 ContentRoot 좌표계로 올릴 때 합성된다.
```

즉, 제거한 `AttachmentRelativeTransform_Hand/Holster`나 `TargetRelativeTransform`이 중간에 추가되지 않는다.

---

## 5. 피벗 회전 수학

일반적인 열벡터 행렬 표기로 무기 로컬 점 `v`의 결과는 다음과 같다.

```text
v' = A × P × R × P⁻¹ × v
```

| 기호 | 의미 |
| --- | --- |
| `A` | 캐릭터 Bone과 장착 소켓을 거쳐 만들어진 외부 장착 Transform |
| `P` | `HandGrip`을 WeaponMesh Relative Transform과 합성하여 ContentRoot Space로 올린 뒤 Scale을 제거한 강체 Transform |
| `R` | Notify가 요청한 논리 회전 |
| `v` | 원래 무기 로컬 좌표의 점 |

오른쪽부터 보면 다음 순서다.

```text
P⁻¹ : 무기 좌표를 HandGrip 중심 좌표로 변환
R   : HandGrip이 원점인 좌표에서 회전
P   : 결과를 원래 무기 좌표로 복귀
A   : 캐릭터 소켓을 따라 월드에 배치
```

회전이 없으면 `R = Identity`이므로 다음처럼 상쇄된다.

```text
P × Identity × P⁻¹ = Identity
```

따라서 중심부터 `HandGrip`까지의 거리가 중복 반영되지 않으며, 피벗 초기화 전후의 기본 자세도 동일하다.

Unreal 코드에서는 `FTransform` 합성 규칙에 맞춰 다음처럼 구성한다.

```cpp
const FTransform WeaponMeshRelativeToContentRoot = WeaponMeshComponent->GetRelativeTransform();
const FTransform PivotSocketRelativeToWeaponMesh = WeaponMeshComponent->GetSocketTransform(PivotSocketName, RTS_Component);

FTransform PivotSocketRelativeToContentRoot = PivotSocketRelativeToWeaponMesh * WeaponMeshRelativeToContentRoot;
PivotSocketRelativeToContentRoot.SetScale3D(FVector::OneVector);

PivotRestTransform = PivotSocketRelativeToContentRoot;
PivotComponent->SetRelativeTransform(PivotRestTransform);
ContentRootComponent->SetRelativeTransform(PivotRestTransform.Inverse());

const FTransform RotationOffset(RuntimeRotation);
PivotComponent->SetRelativeTransform(RotationOffset * PivotRestTransform);
```

`PivotRestTransform`은 “계산 중인 회전값”이 아니다. 초기화 때 `HandGrip`과 WeaponMesh Relative Transform을 합성하여 계산한 고정 기준 Transform `P`다. 합성을 먼저 수행하므로 WeaponMesh Scale이 피벗 위치에 미친 결과는 보존하지만, 피벗 계층 자체의 Scale은 `(1,1,1)`로 정규화한다.

`bHasValidPivot`은 `P`를 안전하게 사용할 수 있는지 나타낸다. 다음 검증을 모두 통과한 뒤에만 `true`가 된다.

- `PivotComponent`, `ContentRootComponent`, `WeaponMeshComponent`가 유효함
- `WeaponMeshComponent`가 `ContentRootComponent`의 직접 자식임
- `PivotSocketName`이 무기 Mesh에 존재함
- WeaponMesh Relative Transform과 피벗 소켓을 합성한 Transform에 NaN이 없음

소켓 또는 WeaponMesh에 작성된 Scale은 피벗 위치 계산에는 반영되지만 `PivotRestTransform`의 Scale 채널로 전달하지 않는다. 회전식 `P × R × P⁻¹`에 Scale을 포함하면, 특히 비균일 Scale과 회전의 조합에서 왜곡이 발생할 수 있기 때문이다.

---

## 6. 장착과 소켓 전환

### 6.1. 안정 장착

`AttachToOwnerSocket()`은 다음 계약을 지킨다.

```text
캐릭터 소켓에 SnapToTargetIncludingScale로 부착
→ ActorRootComponent Relative Transform을 Identity로 확정
```

최종 자세의 단일 원천은 캐릭터 소켓이다.

### 6.2. 소켓 전환 중

`Weapon Socket Transform Transition`은 Source와 Target 소켓의 **현재 월드 Transform**을 매 Tick 다시 읽어 보간한다.

```text
Begin    : 현재 부착 소켓 이름 캡처
Tick     : SourceSocketWorld ↔ TargetSocketWorld 보간
Complete : Target 소켓에 Snap, ActorRoot Identity 확정
Cancel   : Source 소켓에 즉시 Snap, ActorRoot Identity 복구
```

Source/Target 소켓이 캐릭터 애니메이션에 따라 움직여도 전환 경로가 현재 자세를 따라간다. 별도 Relative Transform은 저장하거나 보간하지 않는다.

Equip/Unequip 전환은 장비 상태를 바꾸는 영구 전환이다. 일반 `Weapon Socket Transform Transition`은 같은 Action 안의 오른손↔왼손 같은 임시 전환에 사용한다.

---

## 7. Action Pose Scope

`UCWeaponComponent`는 논리적 Action 하나당 Scope 하나를 연다. 콤보에서 Montage가 교체되어도 같은 Scope를 유지한다.

Action 시작 시 캡처하는 값:

```text
현재 WeaponActor
현재 부착 소켓 이름
현재 CommittedPivotRotation
```

Action 종료·취소 시:

```text
진행 중 Socket Transition 무효화
진행 중 Pivot Transition/Override 무효화
시작 소켓으로 재부착
시작 시점의 CommittedPivotRotation 복구
```

따라서 오른손↔왼손 전환 도중 Montage가 끊겨도 Action 시작 소켓으로 돌아온다. Equip/Unequip처럼 영구 상태를 바꾸는 전환은 완료 후 활성 Scope의 소켓 기준선을 새 상태로 재설정하므로 Action 종료가 장착 결과를 되돌리지 않는다.

복구의 진입점은 정식 Complete/Stop/Interrupt 흐름이며 MontageEnd 콜백 자체가 아니다. 장탈착은 Socket NotifyState의 물리 전환 완료 → `CommitEquipWeapon` / `CommitUnequipWeapon`의 논리 상태 확정 → 명시적 Complete 순서로 저작한다. Action과 Reaction의 자연 MontageEnd는 관측용이다. [F08](../04-02_Fix_Pull_Request/F08_UE5_Portfolio_Pull_Request_Fix.md)의 검증 범위와 한계를 함께 참조한다.

---

## 8. 피벗 회전 채널

### 8.1. Transition

`Weapon Pivot Rotation Transition`은 Action 안에서 유지되는 회전 상태를 바꾼다.

```text
Begin    : 현재 Committed 회전을 Source로 캡처
Tick     : Source에서 Target까지 계산
Complete : Target을 Committed 상태로 확정
Cancel   : 기존 Committed 상태 유지
```

### 8.2. Override

`Weapon Pivot Rotation Override`는 현재 Committed 회전에 짧은 회전을 추가했다가 제거한다.

```text
Blend In  : Identity → RotationOffset
Hold      : RotationOffset
Blend Out : RotationOffset → Identity
End       : Temporary Offset 제거
```

Action 중단 시에도 Scope가 Temporary Offset을 제거한다.

### 8.3. 회전 모드

| 모드 | 용도 |
| --- | --- |
| `TargetOrientation` | 특정 최종 방향으로 맞추는 자세 전환 |
| `SignedLocalAxisAngle` | 방향과 회전 수가 중요한 스핀 |

반대 방향으로 돌리려면 `SignedAngleDegrees`의 부호를 바꾼다.

```ini
LocalAxis = (동일)
SignedAngleDegrees = -160  ; 반대 방향은 +160
```

`+360`, `-360`, `+720`도 그대로 지원한다.

---

## 9. 에디터 Authoring 규칙

### 캐릭터 Skeleton

1. 무기별 또는 캐릭터별 전용 Hand/Holster 소켓을 만든다.
2. Animation Editor에서 Preview Asset을 보며 위치·회전·스케일을 최종 조정한다.
3. 해당 값을 장착 자세의 단일 원천으로 취급한다.

### 무기 Skeletal Mesh

1. 실제 회전 중심에 `HandGrip` 소켓을 둔다.
2. `HandGrip` 소켓 Scale은 피벗 연산에 사용하지 않는다. 작성 의도를 명확히 하려면 `(1,1,1)`을 권장하지만 필수 조건은 아니다.
3. Trail이 필요하면 `TrailStart`, `TrailEnd`를 둔다.

### Weapon Blueprint

1. `SocketName_Hand`, `SocketName_Holster`를 캐릭터 전용 소켓 이름과 맞춘다.
2. `PivotSocketName`이 `HandGrip`인지 확인한다.
3. `ActorRootComponent` Relative Transform은 Identity로 둔다.
4. `WeaponMeshComponent` Relative Transform은 Identity를 권장하지만, 필요한 경우 사용할 수 있다. 이 값의 위치·회전·스케일 효과는 피벗 위치·방향 계산에 반영되며 피벗 Scale 자체는 제거된다.
5. Collision과 함께 회전할 콘텐츠를 `ContentRootComponent` 아래에 둔다.

### Montage

- 손↔홀스터: 전용 Equip/Unequip Socket Transition 사용
- Action 내부 손 전환: `Weapon Socket Transform Transition` 사용
- 지속 회전 자세: Pivot Rotation Transition 사용
- 짧은 왕복 회전: Pivot Rotation Override 사용
- 정확한 회전 방향/횟수: `SignedLocalAxisAngle` 사용

---

## 10. 검증 체크리스트

| ID | 검증 항목 |
| --- | --- |
| WPT-01 | 회전 Identity에서 초기화 전후 Mesh·Collision의 월드 자세가 같다. |
| WPT-02 | 회전 중 `HandGrip` 월드 위치가 유지된다. |
| WPT-03 | `WeaponMeshComponent` Relative Transform이 Identity가 아니어도 `HandGrip`의 ContentRoot 기준 위치·방향이 올바르게 계산된다. |
| WPT-04 | `ActorRootComponent`는 안정 장착 상태에서 Identity다. |
| WPT-05 | 캐릭터 소켓의 스케일이 무기 전체에 한 번만 적용된다. |
| WPT-06 | Socket Transition 완료 후 Target 소켓에 붙고 ActorRoot가 Identity다. |
| WPT-07 | Socket Transition 중단 후 Source 소켓에 붙고 ActorRoot가 Identity다. |
| WPT-08 | 오른손↔왼손 콤보가 Action 취소되면 시작 소켓으로 돌아온다. |
| WPT-09 | Equip/Unequip 완료 상태는 Action 종료 후에도 유지된다. |
| WPT-10 | Pivot Override 종료·취소 후 Committed 회전만 남는다. |
| WPT-11 | Collision·Trail·FX가 Mesh와 같은 피벗으로 회전한다. |
| WPT-12 | Blueprint와 Montage에 제거된 Relative Transform 필드가 더 이상 노출되지 않는다. |

---

## 11. 구현 위치

| 책임 | 파일/함수 |
| --- | --- |
| 컴포넌트 계층 및 피벗 초기화 | `Source/Portfolio/Weapon/CWeaponActor.cpp` |
| 안정 소켓 부착 | `ACWeaponActor::AttachToOwnerSocket()` |
| 피벗 기준 계산 | `ACWeaponActor::InitializePivot()` |
| 피벗 회전 적용 | `ACWeaponActor::ApplyPivotRotation()` |
| Action Scope 및 전환 상태 | `Source/Portfolio/Component/CWeaponComponent.cpp` |
| 소켓 전환 Notify | `Source/Portfolio/Notify/CAnimNotifyState_*SocketTransformTransition*` |
| 피벗 회전 Notify | `Source/Portfolio/Notify/CAnimNotifyState_WeaponPivotRotation*` |
| 자동 감사 | `Source/PortfolioEditor/Private/Audit/CWeaponPresentationAuditCommandlet.cpp` |
| 회전 수학 테스트 | `Source/PortfolioEditor/Private/Tests/CWeaponPresentationTypesTests.cpp` |

감사 커맨드:

```powershell
UnrealEditor-Cmd.exe Portfolio.uproject -run=CWeaponPresentationAudit -Roots=/Game -Output="Audits/WeaponPresentationAudit.csv" -unattended -nop4 -nosplash -stdout -FullStdOutLogOutput
```
