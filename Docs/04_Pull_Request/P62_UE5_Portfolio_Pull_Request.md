# UE5 Portfolio Pull Request

## 제목

**P62: Stellar 캐릭터·무기·환경 에셋 통합과 Weapon Presentation 재구성**

## 날짜

**2026.09.17**

## 상태

- [x] Celeste/Stellar 캐릭터·애니메이션 세트 리타게팅 및 프로젝트 연결
- [x] 새 무기 규격에 맞춘 장탈착 전환과 콤보 소켓 이동 도입
- [x] HandGrip 기준 회전과 WeaponActor 표현 계층 분리
- [x] Action Pose Scope 기반 임시 표현 복구와 장착 상태 유지
- [x] Trail 정의의 데이터화와 WeaponActor 런타임 관리 연결
- [x] SciFi 환경 맵 도입과 충돌·조명 환경 구성
- [x] Content 경로·참조 정리, 대체된 에셋과 legacy Notify 제거
- [x] Blueprint 필드 호환성 유지 및 현재 Unreal 에셋 528개 LFS 정규화
- [x] Weapon Presentation 감사 도구·회전 수학 테스트 추가
- [x] Editor 빌드·LFS 검사·에셋 감사 및 사용자 PIE 확인

## 브랜치

- Base: `main`
- Branch: `promotion/portfolio-v1-final`
- Base HEAD: `4d246eb02d42d5d759829e2173668ea55adf64a4`
- Implementation HEAD: `8b23f64241e4f513222da26fe54b9c7fdfb40899`
- GitHub PR: https://github.com/Razria93/Portfolio_UE5.4_verGit/pull/121
- Merge Commit: `2122e331`

이 문서는 PR #121 본문의 교체안이다. 구현 범위는 위 HEAD에 고정하며, 후속 Fix 브랜치의 Reaction 종료 변경·추가 Montage 보완·Execution Montage 감사 도구를 이 PR의 완료 내용에 포함하지 않는다. 이후 제공된 사용자 설명과 화면은 당시 작업 의도를 보강하는 근거로만 사용한다.

## 대표 스크린샷

사용자가 제공한 Retargeter 화면은 Action/Movement 분리와 Spine 회전 전달 조정을, Asset Reference Inspector 화면은 Placeholder_Normal의 참조 조사 과정을 보여준다. 화면별 관찰 내용은 별도 근거 기록에 정리했으며 이미지 원본은 이 문서에 삽입하지 않았다. 서로 다른 애니메이션 프리뷰를 동일 프레임 Before/After로 제시하지 않는다.

## 요약

이번 PR은 새 캐릭터·애니메이션·무기·환경 세트를 기존 전투 프로젝트에 통합한다. Celeste 캐릭터에 맞춰 애니메이션을 리타게팅하고 Player·AnimBP·Montage 연결을 정리했으며, SciFi 무기와 환경을 실제 전투 표현에 사용할 수 있도록 구성했다.

새 무기를 적용하면서 기존 메시 원점과 부착 위치를 전제로 한 표현 방식의 한계가 드러났다. 장탈착 위치 이격은 소켓 전환으로, 콤보의 양손 교체는 임시 부착 전환으로, 처형 중 회전 중심 불일치는 HandGrip 기준 회전으로 처리한다. 이 변환이 충돌체와 FX에도 일관되게 적용되도록 WeaponActor의 계층을 분리했다.

표현을 추가하는 데 그치지 않고 종료 책임도 정리했다. 임시 소켓·회전 변경은 논리적 Action의 수명 안에서 관리하며, 장탈착으로 확정한 상태는 종료 시 되돌리지 않는다. Trail 설정은 데이터로 모으고 무기 Actor가 실행 인스턴스를 관리한다. 대규모 Content 정리에는 참조 확인, Blueprint 호환성 조치, 현재 에셋의 LFS 정규화를 함께 적용했다.

## 핵심 개념

캐릭터 소켓은 무기의 부착 기준을, 무기 내부 Pivot은 회전 기준을, Action Pose Scope는 임시 표현의 수명을 담당한다.

### Socket Transition

캐릭터의 두 소켓 사이에서 무기 위치·회전을 전환한다. 오른손↔왼손 교체는 임시 표현이며, Equip/Unequip은 물리 전환 이후 논리 장착 상태까지 확정한다.

### HandGrip Pivot

무기 메시 원점 대신 손잡이의 HandGrip 소켓을 회전 기준으로 사용한다. Pivot 아래 ContentRoot가 원래 배치를 보정하여 메시·충돌체·FX를 함께 변환한다.

### Action Pose Scope

하나의 논리적 Action이 시작할 때 소켓과 회전 상태를 보관하고 종료·중단 시 임시 변환을 정리하는 범위다. 콤보에서 Montage가 바뀌어도 같은 범위를 유지한다. 정상 장탈착은 복구 기준선을 갱신해 확정한 상태를 보존한다.

### Trail Data

Trail의 식별 조건과 Niagara 에셋을 모은 정의다. DataAsset은 설정을, TrailComponent는 Niagara 인스턴스를 나타내며, 생성·캐시·활성화·정리는 WeaponActor가 관리한다.

## 변경 배경

기존 애니메이션을 체구가 다른 캐릭터에 적용하면 흉부가 접혀 내려앉아 보였다. 무기도 기존 검과 크기·원점·손잡이 위치가 달라 단순 메시 교체만으로는 장탈착과 처형 표현이 맞지 않았다.

이를 개별 애니메이션이나 메시 보정으로만 처리하면 손 전환·회전·장착 설정이 서로 간섭하고, 프리뷰와 런타임의 배치도 달라질 수 있었다. 특히 Mesh만 변환하면 무기에 붙은 충돌체와 FX가 다른 기준으로 움직이는 문제가 생긴다.

Trail 역시 식별자·컴포넌트·에셋 설정이 여러 위치에 분산되면 새 무기마다 관리 지점이 늘어난다. 따라서 에셋 규격 차이를 수용하는 표현 구조와 설정 소유권을 함께 정리했다. Content 이동 과정에서는 이름만으로 삭제를 판단하지 않고 실제 참조를 확인해야 했다.

## 주요 변경

### 1. 캐릭터·애니메이션 세트 통합

대상:

```text
Content/01_Character/Asset/StellarGirlCeleste/
Content/01_Character/01_Player/
Content/03_Animation/
Content/04_Montage/
Content/05_BlendSpace/
```

- Action과 Movement용 Retargeter를 분리하여 새 캐릭터에 애니메이션을 적용했다.
- 흉부 접힘을 완화하기 위해 Spine 회전 전달을 조정했다. 사용자 제공 화면의 Spine Rotation Alpha는 두 Retargeter 모두 0.7이다.
- Movement 화면에는 Head 0.0, Action 화면에는 Neck 0.8이 확인된다. 화면에 없는 체인 값까지 동일하다고 가정하지 않는다.
- Additive Layer Track은 사용하지 않았다. 임시 import 프로젝트의 저작 설정과 본 프로젝트에 이주된 애니메이션 결과를 구분한다.
- 캐릭터 메시·애니메이션·Montage 및 Player 연결을 새 세트에 맞췄다. 원본 에셋 제작이 아니라 리타게팅·설정·프로젝트 통합이 본 작업의 기여다.

### 2. 장탈착과 콤보 소켓 전환

대상:

```text
Source/Portfolio/Component/CWeaponComponent.*
Source/Portfolio/Notify/CAnimNotifyState_*SocketTransformTransition.*
Content/04_Montage/Combat/Sword/Default/Equip/
Content/04_Montage/Combat/Sword/Stellar/Equip/
```

- 장탈착 시 무기의 위치·회전을 소켓 사이에서 전환하도록 구성했다.
- 일반 소켓 전환은 오른손↔왼손처럼 콤보 중 부착 위치를 바꾸는 데 사용한다.
- Equip/Unequip은 NotifyState가 물리 전환을 완료한 뒤 `CommitEquipWeapon` / `CommitUnequipWeapon` 명령으로 논리 장착 상태를 확정한다.
- Default/Stellar의 Equip/Unequip Montage 4개를 전환 NotifyState 방식으로 이주하고 retired point Notify를 제거했다.
- 장착 상태 변경과 단순 손 전환을 같은 동작으로 취급하지 않는다.

### 3. HandGrip 회전과 무기 계층 분리

대상:

```text
Source/Portfolio/Weapon/CWeaponActor.*
Source/Portfolio/Type/CWeaponPresentationTypes.h
Source/Portfolio/Notify/ (Pivot 회전 NotifyState)
Config/DefaultEngine.ini
```

```text
RootScene
-> PresentationPivot
   -> PresentationContentRoot
      -> WeaponMesh / Collision / FX
```

- 캐릭터 부착 기준과 무기 내부 회전 기준을 분리했다.
- HandGrip 위치·방향으로 회전 기준을 정하고 ContentRoot에 역보정을 적용하여 회전이 없을 때 기존 배치를 유지한다.
- 메시뿐 아니라 무기 자식 요소가 같은 변환을 받도록 구성했다.
- 애니메이션 프리뷰에서도 무기 배치를 확인할 수 있도록 캐릭터 소켓의 저작 기준을 유지했다.
- C++ 필드 명칭을 정리하면서 Native Component 내부 이름은 보존하고 PropertyRedirect를 추가했다.

### 4. Action 수명에 맞춘 표현 복구와 Trail 관리

대상:

```text
Source/Portfolio/Action/CAction.*
Source/Portfolio/Component/CActionComponent.*
Source/Portfolio/Component/CActionFeedbackComponent.*
Source/Portfolio/Component/CWeaponComponent.*
Source/Portfolio/Component/CWeaponTrailComponent.*
Source/Portfolio/DataAsset/CWeaponTrailDataAsset.*
```

- Action Pose Scope가 시작 소켓과 회전을 보관하고 정식 종료·중단 흐름에서 임시 표현을 복구한다.
- 정상 장탈착 후에는 기준선을 새 장착 상태로 갱신하므로 Action 종료가 장착 결과를 되돌리지 않는다.
- Action의 자연 MontageEnd는 관측만 담당하며 정상 완료는 명시적 Complete, 중단은 정식 Stop/Interrupt가 처리한다. time-zero Notify 대응 방어는 유지한다.
- Trail의 식별 조건과 Niagara 에셋은 DataAsset으로 집중하고 WeaponActor가 인스턴스를 관리한다. Action 종료 시 Weapon runtime 및 Feedback 정리 경로에서 Trail을 비활성화한다.
- Trail 소스를 Weapon 경로에서 Component/DataAsset 경로로 역할에 맞게 이동했다.
- 이 PR 시점의 Reaction은 자연 MontageEnd에서 Complete fallback을 유지한다. 후속 Fix의 대칭 계약을 소급하지 않는다.

### 5. 환경·Content·저장 정책 정리

대상:

```text
Content/SciFi/SciFiMap.umap
Content/01_Character/Asset/
Content/06_Weapon/
Content/07_FX/
.gitattributes
```

- SciFiMap을 도입하고 충돌 문제를 해결한 뒤 조명 환경을 구성했다. 세부 옵션·원인·성능 수치는 본 PR의 주장 범위에 넣지 않는다.
- 에셋을 역할별 경로로 이주하고 참조를 갱신했으며 대체된 legacy 리소스를 정리했다.
- Placeholder_Normal은 자체 Asset Reference Inspector의 Referencers 조회와 Material 내부 사용 확인을 거쳐 삭제하지 않고 `T_CELESTE_FlatNormal`로 보존·이주했다.
- 현재 Unreal 에셋 528개를 LFS 포인터로 정규화했다. 과거 전체 Git 이력을 재작성한 작업은 아니다.
- enum의 legacy 명령 제거와 이름 정리에는 숫자 이동이 포함된다. PropertyRedirect가 enum 값 이동까지 보호한다고 보지 않는다.

### 6. 검증 도구와 부수 변경

대상:

```text
Source/PortfolioEditor/Private/Audit/CWeaponPresentationAuditCommandlet.cpp
Source/PortfolioEditor/Private/Tests/CWeaponPresentationTypesTests.cpp
Docs/05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md
```

- Weapon Presentation 감사 도구로 에셋 로드와 legacy Notify 잔존 여부 등을 확인할 수 있게 했다.
- 회전 계산의 방향·축 처리 등을 검증하는 수학 테스트를 추가했다. 테스트 코드 존재와 개별 실행 성공은 구분한다.
- 표현 계층과 저작 기준을 S39에 정리하고 Targeting/HUD 관련 문서도 갱신했다.
- Walk 입력을 Tab에 연결했다. 에셋 통합의 핵심 변경과 구분한 부수 작업이다.

## 주요 처리 흐름

### 장탈착

```text
Equip / Unequip Action 시작
-> Action Pose Scope 시작
-> Socket NotifyState에서 물리 전환
-> 전환 완료 후 논리 장착 command
-> 장착 상태 확정 및 Scope 기준선 갱신
-> 명시적 Action Complete
-> 임시 표현 정리, 확정된 장착 상태 유지
```

### 콤보 표현과 중단

```text
Action 시작 상태 보관
-> 콤보 Montage에서 손 소켓·Pivot 회전·Trail 표현
-> 같은 Action의 다음 Montage에서도 Scope 유지
-> 정식 Complete 또는 Stop/Interrupt
-> 진행 중 전환 무효화, 시작 소켓·회전 복구, Trail 정리
```

## 트러블슈팅과 설계 판단

### 메시 원점 회전만으로 해결하지 않은 이유

새 검의 원점은 원하는 손잡이 회전 기준과 일치하지 않았다. 메시 위치를 개별 보정하는 대신 HandGrip 기준의 부모 회전 계층을 두어 충돌체와 FX까지 같은 기준으로 처리했다.

### 임시 손 전환과 장착 확정을 분리한 이유

콤보의 손 전환은 Action 종료 시 복구해야 하지만 Equip/Unequip의 결과는 유지해야 한다. 동일한 복구 규칙으로 처리하지 않고 논리 장착 확정 시 기준선을 갱신한다.

### Placeholder 에셋을 남긴 이유

Demo 경로와 Placeholder라는 이름은 미사용 근거가 아니었다. 참조 조회와 실제 Material 사용을 확인한 뒤 기능적으로 필요한 flat normal을 보존하고 이름·경로를 정리했다.

## 변경 파일 범위

- Source: WeaponActor, Weapon/Action/Feedback Component, Socket/Pivot Notify, Trail DataAsset/Component, 관련 Type.
- PortfolioEditor: Weapon Presentation 감사 도구와 회전 수학 테스트.
- Config: Blueprint 필드 호환성 redirects.
- Content: 캐릭터·애니메이션·Montage·무기·Trail·환경과 경로 이동·참조 갱신.
- Docs: Weapon Presentation 및 Targeting/HUD 문서.

## 테스트 방법

### 정적 검사·빌드

- `4d246eb0..8b23f642` 차이에서 표현 책임, legacy Notify 제거, redirect 대상과 enum 변경을 확인한다.
- `git diff --check 4d246eb0 8b23f642`로 whitespace를 검사한다.
- `PortfolioEditor Win64 Development`를 빌드한다.
- Git LFS 무결성과 Weapon Presentation 감사 결과를 확인한다.

### 에디터·PIE

1. 새 캐릭터의 이동·전투 애니메이션 연결과 흉부 보정 결과를 확인한다.
2. Equip/Unequip의 물리 전환 뒤 논리 장착 상태가 바뀌는지 확인한다.
3. 콤보의 손 전환과 처형의 HandGrip 기준 회전을 확인한다.
4. Action 종료·중단 시 임시 소켓·회전·Trail이 남지 않는지 확인한다.
5. 확정된 장착 상태는 Action 종료 뒤 유지되는지 확인한다.

## 검증 결과

| 항목 | 결과 | 근거·범위 |
| --- | --- | --- |
| Editor Development 빌드 | 당시 통과 기록 | 기존 PR #121 검증 기록. 본문 재작성에서 새 빌드로 주장하지 않음 |
| Git LFS 무결성 | 당시 통과 기록 | 기존 PR #121의 fsck 결과. 모든 과거 이력·원격 객체 보장 아님 |
| Weapon Presentation 감사 | 1,398 packages 로드, 실패 0 | 기존 PR #121 기록; legacy Equip/Unequip Notify 참조 미검출 |
| 전투 표현 PIE | 사용자 확인 기록 | 당시 동작 확인. 후속 Fix 결과를 당시 완전성 증명으로 대체하지 않음 |
| Retargeter 설정·참조 조회 | 후속 설명·화면으로 작업 근거 보강 | Additive 미사용, Action/Movement 분리, 자체 Inspector 활용 |
| 전체 자동 회귀 검사 | 주장하지 않음 | 감사·수학 테스트와 모든 runtime 경로 검증은 별개 |

## 설계 판단 기준

- 캐릭터 소켓, 무기 Pivot, Action 수명은 서로 다른 책임이다.
- 물리 전환과 논리 장착 확정은 순서가 있는 별개 단계다.
- Trail 정의는 데이터에, 런타임 인스턴스 관리는 WeaponActor에 둔다.
- 이름이나 폴더 위치만으로 에셋 삭제를 결정하지 않는다.
- 원래 작업 커밋과 최종 재구성 커밋을 서로 다른 성과로 중복 집계하지 않는다.

## Scope Guard

이번 PR에서 하지 않은 것:

- Reaction 자연 MontageEnd fallback 제거: 후속 Fix 범위.
- `CExecutionMontageAuditCommandlet` 도입과 Complete Trigger 감사 강화: 후속 Fix 범위.
- 후속 Montage·무기·Player 에셋 재통합 커밋의 변경.
- 과거 Git 전체 이력의 LFS 재작성.
- 원본 상용 에셋 자체의 제작, 기존 Root Motion 도구의 신규 개발.
- 충돌·조명 수치의 독립 검증과 측정 없는 성능 개선 주장.

## 리스크 / 리뷰 포인트

- enum 숫자 이동의 영향을 받는 직렬화 데이터는 사용 위치별 확인이 필요하다.
- Action의 명시적 Complete가 없거나 Trigger가 맞지 않으면 자연 MontageEnd가 완료를 대신하지 않는다.
- `/Game/01_Character/Regacy/Mesh/SK_Mannequin`의 M_Man_Body / M_Man_ChestLogo import 경고는 기존 PR에 기록된 잔여 legacy 항목이다. 감사 오류 0과 전체 엔진 로그 무경고는 다르다.
- 참조 조회 결과는 동적 문자열 참조를 포함한 모든 사용 경로의 완전한 보장이 아니다.

## 후속 작업

1. Montage 명시적 종료 설정과 Action/Reaction 종료 정책 보완은 후속 Fix 브랜치에서 구분해 기록한다.
2. legacy material import 경고는 별도 Content 정리 대상으로 둔다.
3. 작업 내역·문제 해결 보고서·화면 근거는 후속 문서 브랜치에서 유지한다. 본 PR의 구현 범위를 늘리지 않는다.

## 관련 문서

- System Architecture: `S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md` — 해당 PR의 설계는 Implementation HEAD 기준으로 조회.
- 후속 Work List: `W07_UE5_Portfolio_Work_List.md`.
- 후속 Issue Analysis: `I03_UE5_Portfolio_Issue_Analysis_Report.md` ~ `I08_UE5_Portfolio_Issue_Analysis_Report.md`.
- 후속 화면 기록: `Docs/98_Evidence/Stellar_Asset_Integration/Capture_Record.md`.

## 대표 커밋

```text
ace28d4c chore(lfs): migrate current Unreal assets to Git LFS
16c54976 refactor(weapon): add presentation compatibility bridge
13299da5 chore(content): migrate portfolio assets and update references
16e00a4b refactor(weapon): migrate equip socket transition notifies
67228d9f refactor(weapon): remove retired presentation notifies
b4dfcf8f feat(input): bind walk action to Tab
eb824904 docs(portfolio): update presentation and targeting documentation
8b23f642 refactor(weapon): clarify completion commands and montage lifecycle
```

## 정리

이번 PR은 새 캐릭터·무기·환경을 기존 전투에 연결하면서 드러난 규격 차이를 표현 계층과 수명 계약으로 흡수했다. 리타게팅, 소켓 이동, 손잡이 기준 회전, Trail 데이터화와 참조·저장 정책 정리를 함께 수행하여 에셋 교체가 개별 보정의 누적으로 끝나지 않도록 구성했다.
