# UE5 Portfolio Pull Request Fix

## 제목

**F08: Stellar 에셋 통합 후 Montage·종료 계약·검증 보완**

## 날짜

**2026.09.17**

## 상태

- [x] Montage의 실행 역할·Complete 설정과 통합 에셋 보완
- [x] Reaction 자연 MontageEnd를 관측용으로 변경
- [x] Complete Trigger 감사 및 검사 범위 명확화
- [x] 빌드·감사 결과와 사용자 PIE 정상 확인
- [x] 작업 내역·사례·설정 화면 근거 및 PR #121 본문 교체안 정리
- [ ] 원격 게시 및 리뷰

## 브랜치

- Branch: `fix/stellar-asset-integration`
- Base branch: `main`
- Base HEAD: `2122e331` (최종 에셋 이주 PR 병합)
- 선행 에셋/도구 커밋: `af6778aa`, `a68bd30d`, `62378b3f`
- 종료 계약·감사 보완: `051a5884`; 계약 문서 마감: `93c4ba22`
- 관련 작업 문서화: `e7ec7a15`, `84f3a3de`. 문서 전용 PR로 분리하지 않고 본 Fix에 포함한다.
- PR 문서 정리: `1ee5236a`.
- 기존 작업명 `fix/execution-montage-terminal-contract`와 임시 문서 분리 계획은 현재 브랜치 하나로 통합했다. 새 PR은 본 문서를 사용하며 별도 문서 전용 PR은 만들지 않는다.

---

## 요약

새 에셋 통합 후 Montage 종료를 명시적 Complete로 통일하고, 실제 에셋의 종료 설정과 검사 도구를 보완한다. Source Execution은 Action, Target Execution은 Reaction으로 각각의 종료 Notify를 사용한다. 무기 표현·Montage·Player 에셋의 검증된 변경도 본 Fix 브랜치에 통합했다.

이 보완의 판단 근거와 원래 에셋 통합 과정을 함께 문서화했다. W07 작업 내역, I03~I08 사례, 사용자 Retargeter·Inspector 화면 기록을 연결한다. 문서화는 별도 기능 PR이 아니라 실제 Montage 정리·검증에 수반되는 작업이다. P62는 기존 PR #121의 본문 교체안으로 별도 유지한다.

## 원인

Action의 자연 MontageEnd 자동 완료를 제거한 뒤 Stellar Equip의 Complete Trigger가 Unequip으로 설정된 문제가 드러났다. 사용자가 해당 설정을 수정하고 정상 종료를 확인했다. Reaction에는 자연 종료 fallback이 남아 있어 같은 명시적 종료 원칙이 적용되지 않았다.

기존 감사는 Complete 클래스 존재를 확인했지만 Trigger Type/Index 불일치를 충분히 판별하지 못했다. 클래스가 맞는 것과 런타임 실행에 전달되는 것은 다른 조건이다.

## 변경 사항

### 1. Montage 종료 설정과 관련 에셋 정리

Montage가 끝나는 것만으로 실행 완료를 추정하지 않도록, 실행 역할에 맞는 Complete 클래스와 Trigger를 정리했다. Execution의 Source는 Action, Target은 Reaction으로 처리한다. Equip/Unequip은 물리 소켓 전환과 논리 장착 확정 뒤 명시적으로 Action을 완료하는 기존 설계에 맞춘다.

promotion 작업본과 대조한 Montage·무기·Trail·Player 변경을 현재 프로젝트에 통합했다. `a68bd30d`의 42개 Content는 Montage 34개와 Player BP, 무기 Skeleton/BP, Niagara 및 Trail DataAsset 변경을 포함한다. Stellar Player BP는 `62378b3f`로 별도 반영했다. 이 파일 수는 앞선 커밋과 합산한 고유 변경 수가 아니다. 새 기능을 추가하기보다 실제 검증한 에셋 설정이 주 프로젝트에 반영되도록 정리한 변경이다.

### 2. Action과 Reaction의 명시적 종료 원칙 통일

Action에 이미 적용된 관측 정책을 Reaction에도 적용했다. `UCReaction::OnMontageEnd`는 자연 종료 시 `Complete()`를 호출하지 않고 `NaturalMontageEndObserved`를 기록한다. 정상 완료는 명시적 Complete가, 중단·복구는 정식 Stop/Interrupt 흐름이 담당한다.

따라서 잘못된 Complete 설정을 자연 종료 fallback이 숨기지 않는다. 기존 stale callback 검증과 time-zero Notify 방어는 유지하며, MontageEnd에 Action Pose Scope 해제나 Trail 정리 책임을 새로 부여하지 않는다.

### 3. Complete 클래스뿐 아니라 실행 조건을 검사

감사 도구가 Blueprint CDO의 Action/Reaction 데이터와 Montage의 직접 Notify를 대조하도록 구성했다. Complete 클래스가 맞아도 Type 또는 Action Index가 실행 데이터와 맞지 않으면 오류로 보고한다. None/Max는 거부하고 All 및 Action Index의 와일드카드는 런타임 조건에 맞춰 처리한다.

검색한 데이터에서 참조를 찾지 못한 경우는 `NoScannedComponentDataReference`로 표시한다. 이를 미사용 판정으로 오해하지 않도록 범위를 명시하고, 직접 Notify가 없는 Montage 행과 실제 Notify 이벤트 수를 분리했다.

### 4. 구현 계약과 문제 해결 기록 연결

S26/S31/S39의 현재 종료 계약을 동기화하고 W07·I03~I08·근거 목록을 연결했다. 리타게팅은 Action/Movement 분리와 Additive 미사용, 참조 정리는 자체 Inspector를 이용한 Placeholder 보존 판단을 사용자 설명·화면에 맞춰 기록했다. 충돌·조명은 간단한 작업 내역으로만 남긴다.

기존 PR #121의 교체안 P62는 해당 PR의 `8b23f642` 시점에 고정했다. Reaction fallback 제거 등 이번 Fix를 과거 완료 내용으로 소급하지 않는다. 문서 작업은 별도 PR로 분리하지 않는다.

## 주요 처리 흐름

```text
Montage의 Complete Notify
-> 실행 역할과 Trigger 조건 일치 확인
-> 해당 Action / Reaction의 Complete
-> Component 실행 상태 정리

정식 Stop / Interrupt
-> 실행 중단 및 관련 runtime 정리
-> Action의 임시 소켓·회전 복구와 Trail 정리

자연 MontageEnd
-> Montage / Play Serial 유효성 확인
-> 관측 기록만 남김 (완료 대체 없음)
```

## 변경하지 않은 것

- 정식 Stop/Interrupt, time-zero Notify 방어, Action Pose Scope와 Trail 종료 책임.
- Execution Source/Target 역할 및 장탈착의 물리 전환 후 논리 commit 순서.
- `DeathPresentationFallbackDelay`: 사망 표현 실패에 대한 별개 정책.
- enum 추가 재배열, 기존 Git 이력, LFS 과거 이력, 원격 브랜치.

## 테스트 방법

1. Montage에 배치된 Complete 클래스와 Type/Action Index를 실행 데이터와 대조한다.
2. 장탈착의 물리 전환·논리 commit 이후 Action 완료 순서를 확인한다.
3. Source Execution Action과 Target Reaction의 정상 종료, 중단 시 정리를 PIE에서 확인한다.
4. `PortfolioEditor Win64 Development` 빌드와 `CExecutionMontageAudit`를 실행해 컴파일·에셋 검사 결과를 확인한다.
5. 문서의 당시/현재 계약, 상대 링크와 `git diff --check`를 검사한다.

다음은 기존에 수행·확인한 결과다. PR 본문 정리를 위해 빌드나 PIE를 새로 실행한 것으로 기록하지 않는다.

## 검증 결과

- `git diff --check` 통과 (2026-09-17).
- 이전 구현 단계의 Editor Development 빌드 성공을 `UnrealBuildTool/Log.txt`에서 재확인함: 두 수정 cpp 컴파일, DLL 링크, WriteMetadata 8/8 완료. 이번 마감은 코드 diff와 기존 실행 로그를 대조했으며 새 빌드 실행으로 기록하지 않음.
- 보존된 `Portfolio-backup-2026.09.17-08.49.08.log`의 감사 결과: 발견/로드 1,399 packages, 실패 0, 데이터 행 93, 감사 Errors 0 / Warnings 0, Montage 49개, 직접 Notify 이벤트 151개.
- 감사 출력: `Saved/Audit/ReactionTerminalPrePIE`, `Saved/Audit/ReactionInventoryPrePIE` (로컬 생성물).
- 사용자가 최종 변경 후 PIE에서 모두 정상 동작한다고 확인함. 자동 테스트 결과와 구분한 사용자 검증임.
- 감사 0건은 전체 엔진 로그 무경고를 의미하지 않음. 기존 SK_Mannequin material import 경고는 별도 잔여 사항임.

## 검증 한계

### PR #122 리뷰 후 로컬 보완

- P1: UE5.4의 실제 `ACharacter::PlayAnimMontage`와 Section 점프를 임시 Montage/테스트 AnimInstance로 검사했다. 0초(양의 trigger offset 포함) Queued/BranchingPoint Notify 모두 두 호출 안에서는 미전달, 이후 엔진 Montage 업데이트에서 각각 1회 전달됐다. 이 테스트 경로에서는 동기 Complete 누락 전제가 재현되지 않아 Action/Reaction 런타임은 변경하지 않았다. Blueprint 사용자 콜백·기존 Montage 종료 재진입·실제 Reaction/Balance 통합 전체를 검증한 결과는 아니다.
- P2: 시작 Section과 authored NextSection 연결로 도달 가능한 직접 Notify만 검사한다. 없는 시작/다음 Section과 비양수 유효 재생 속도는 오류로 보고한다. 다중 Section·루프의 NotifyState 순서는 수동 검토 경고로 남기며 동적 연결 변경은 보장하지 않는다.
- Editor Development 빌드 및 `Portfolio.Animation.Audit.SectionPath`, `Portfolio.Animation.Runtime.StartNotifyDelivery` 자동 테스트 통과. 초기 테스트 환경 중복 초기화와 양성 대조 실패는 테스트 코드 수정 후 재실행했으며 최종 결과와 구분한다.
- 강화된 현재 감사: 1,399 packages, 로드 실패 0, DataRows 93, Errors 4 / Warnings 3. 이전의 0/0은 강화 전 기록이다. `FailOnIssues=true` 실행은 의도대로 종료 코드 1이다.
- 오류 4행은 Default/Stellar `M_Parry_Sword_Up_L` 두 에셋의 존재하지 않는 다음 Section `Start` 참조다. 경고 3행은 Default `M_HitReact` 한 에셋의 다중 Section/루프 순서 검토다. 중복 데이터 참조 수와 고유 에셋 수를 구분한다. 에셋은 자동 수정하지 않았으며, 실제 종료 누락이 재현됐다는 의미도 아니다.
- 로컬 로그: `Saved/Logs/PR122AnimationFinal.log`, `Saved/Logs/PR122SectionFinal.log`; 상세 결과: `Saved/Audit/PR122SectionAudit.csv`.

### Section 검토 후 마감 결과

- Parry 두 에셋은 `Default` 한 Section(0~0.233333초)만 있으며 `Next=Start`가 남아 있었다. 엔진은 없는 이름을 `INDEX_NONE`으로 해석하므로 `None`으로 정리해 기존 종료 경로를 유지했다. Complete는 0.216667초로 그대로다. 정확한 두 경로와 기존 값을 검사한 임시 저장 코드는 작업 후 제거했다.
- Default HitReact는 `Default(0~0.166667) → Start(0.166667~1.133333) → None`의 연속 경로다. Intervention NotifyState는 0.2~0.533333초, Complete는 1.1초이므로 에셋 변경 없이 정상으로 판정했다. 감사는 연속 순방향 Section에 기존 시각 비교를 적용하고, 건너뛰기·역방향·루프는 계속 검토 경고로 남긴다.
- 최종 재로드 감사: 1,399 packages / 실패 0 / DataRows 93 / Errors 0 / Warnings 0, `FailOnIssues=true` 종료 코드 0. 기존 legacy material import 경고 2개는 별개로 남는다.
- Editor Development 빌드와 Animation 자동 테스트 2개 통과. 수정 전후 전체 Notify inventory가 동일함을 비교했다. 실제 PIE를 새로 수행한 결과는 아니다.
- 최종 로컬 근거: `Saved/Logs/PR122ClosedAudit.log`, `Saved/Logs/PR122ClosedTests.log`, `Saved/Audit/PR122Closed.csv`, `Saved/Audit/PR122ClosedInventory.csv`.
- 리뷰 답변 예정: P1은 검사한 실제 엔진 경로에서 미재현이며 사용자 Blueprint 재진입 전반까지 보장하지 않는다고 설명한다. P2는 시작 경로 검사·회귀 테스트·위 에셋 정리와 최종 감사 결과를 제시한다. 원격 답변 및 Push는 별도 승인 전까지 수행하지 않는다.

감사는 Blueprint CDO component 데이터와 Montage에 직접 배치된 Notify를 대상으로 한다. Sequence 내부 Notify, 레벨 인스턴스 override, 동적 참조, 실행 중 Notify 전달은 보장하지 않는다. 검색된 component 데이터에서 미참조라는 결과는 미사용·삭제 가능 판정이 아니다. 수학 테스트 구현의 존재와 이번 실행 성공도 구분한다.

## 관련 문서

- Work List: `W07_UE5_Portfolio_Work_List.md`.
- Issue Analysis: `I03_UE5_Portfolio_Issue_Analysis_Report.md` ~ `I08_UE5_Portfolio_Issue_Analysis_Report.md`.
- Evidence: `Docs/98_Evidence/Stellar_Asset_Integration/README.md`, `Capture_Record.md`.

- [S26](../05_System_Architecture/S26_UE5_Portfolio_System_Architecture.md)
- [S31](../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)
- [S39](../05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md)

## 정리

이번 Fix는 에셋 통합 이후의 Montage 설정과 실행 종료 책임을 맞추고, 이를 검출하는 감사와 설명 문서를 함께 보완한다. 명시적 완료·정식 중단·엔진 관측을 구분하면서 검증한 에셋 결과를 현재 프로젝트에 일관되게 반영한다.

---
