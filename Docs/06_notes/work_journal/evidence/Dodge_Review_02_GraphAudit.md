# Review 02: 디졸브 BP 및 이동 VFX 후보 조사

## 증거와 범위

2026-09-22 `CDodgePresentationAudit`를 확장해 BP 그래프의 노드·핀 기본값·연결, Timeline 트랙, Niagara 노출 파라미터를 읽었다. Development 빌드 성공. `Saved/Logs/DodgeGraphAudit05.log`: commandlet exit 0, 0 errors / 0 warnings. 에셋 저장은 하지 않았다. 사용자가 수정한 `M_Fresnel_D`도 보존했다.

조사 대상은 BP_CPlayer_Stellar, BP_CEnemy, BP_CWeaponActor_Sword, BP_CWeaponActor_SciFi_Sword 및 아래 Niagara 세 개다. 모든 파생 BP나 실제 렌더링까지 검증한 결과는 아니다.

## 디졸브와 발광의 차이

- BP_CEnemy는 기본 메시 머티리얼의 동적 인스턴스 `DissolveMID_0/1`을 만든다.
- `On Death Presentation Requested` 경로에서 시작하는 `TL_Dissolve`는 길이 2.5초, 비반복, Float Track `AmountAlpha`를 가진다.
- Timeline의 AmountAlpha 핀은 두 MID의 `Amount`, Niagara 입력, `Set Weapon Dissolve Amount.InAmount`에 연결되어 있다. Update의 Sequence에서 각각 업데이트한다.
- CWeaponComponent의 StartWeaponDissolve / SetWeaponDissolveAmount / FinishWeaponDissolve가 무기 BP의 ReceiveWeaponDissolveStarted / Amount / Finished 이벤트로 전달한다.
- BP_CWeaponActor_Sword는 WeaponMeshComponent의 ElementIndex=0에 DissolveMaterial의 MID를 만들고 초기 `Amount=1`을 기록한다. 시작 이벤트는 별도 Niagara를 Spawn System Attached로 생성한다.
- 신규 프레넬은 기본 머티리얼 슬롯을 교체하지 않고 Overlay MID의 `Alpha`를 변경한다. 따라서 디졸브의 `Amount`를 덮어쓰는 구조가 아니다. 그러나 Overlay 자체에 디졸브 마스크가 없으므로 두 효과를 동시에 유지하면 지워진 표면 위에 Overlay가 남을 수 있다. 사망 시 프레넬을 정리하는 정책을 유지하고, 실제 시각 결과는 PIE 검토 대상으로 둔다. 현재 정리는 피드백 컴포넌트의 생존 검사 Tick에서 이루어지므로 동일 프레임 이벤트 순서까지 보장했다고 주장하지 않는다.

## 기존 무기 BP의 별도 결함 후보

BP_CWeaponActor_Sword의 `K2Node_CallFunction_0` (Set Scalar Parameter Value):

- self는 DissolveMID.
- Value는 Receive Weapon Dissolve Amount.InAmount에 연결.
- **ParameterName은 연결 없이 기본값 None.**

초기화 노드의 ParameterName=Amount와 불일치한다. 이 경로로는 의도한 Amount 갱신이 이루어지지 않는 구성이다. 이번 조사에서 수정하지 않았으며 사용자에게 별도 수정 포함 여부를 질문했다. 이것을 신규 프레넬 코드로 발생한 문제라고 분류하지 않는다.

## Niagara 후보

| 에셋 | 실제 노출 파라미터 | 판단 |
| --- | --- | --- |
| NS_Trail_Tech | User.Skeletal Mesh (SkeletalMesh DI) | 메시 샘플링 의존. 단순 위치 부착만으로 이동 리본으로 쓸 수 있다고 단정할 수 없다. |
| NS_Trail_Water | User.Color, User.Skeletal Mesh | 색 지정 가능하지만 메시 바인딩/이미터 동작 추가 검토 필요. |
| Vfx_SpeedUp | User.AudioOn, Color01, Color03, Color2, ColorBottom, Duration, GlobleSize, LightOn | 부가 음향/광원 비활성 및 색/시간 조절 후보. 이름만으로 러시 속도선에 적합하다고 판단하지 않는다. |

다음 작업은 후보의 이미터/렌더러와 종료 방식을 확인한 뒤 전용 복제본 및 이동 전용 런타임 수명을 구성하는 것이다. 현재 FOV 구현만으로 Niagara 이동 연출이 완료된 것은 아니다. 기존 공격 무기 Trail은 변경하거나 러시 이동 이펙트와 합치지 않는다.

## 이미터 추가 조사

`DodgeEmitterAudit06.log`: 확장한 읽기 전용 도구 실행 성공, 0 errors / 0 warnings, exit 0.

- NS_Trail_Tech: 활성 Trail(Ribbon), Shape/ShapeBig(Sprite), 모두 월드 공간 CPU 구성.
- NS_Trail_Water: 활성 Trail(Ribbon), Water/Spark는 비활성, 모두 월드 공간 CPU 구성.
- Vfx_SpeedUp: 활성 NE_follow_ribbon은 월드 공간 Ribbon. NE_spell_particle_up은 로컬 공간 Sprite. 그 외 활성 바닥 Mesh, Light, Cyclone Mesh가 있어 원본 통째 적용은 부가 장식까지 포함한다.

따라서 SpeedUp의 전용 복제본에서 이동 리본 및 필요한 입자만 남기는 것을 우선 후보로 삼는다. 이미터 목록만으로 속도선 형태, 발생률, 파티클 수명, Deactivate 반응을 증명하지는 못한다. 실제 모듈 설정 및 PIE 검토가 다음 단계다.
