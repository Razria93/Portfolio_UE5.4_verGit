# UE5 Portfolio Pull Request Fix

## 제목

**F08: Montage 명시적 종료 계약과 통합 에셋 보완**

## 날짜

**2026.09.17**

## 상태

- 로컬 구현·사용자 PIE 확인 완료. Push / PR 생성은 수행하지 않음.

## 브랜치

- `fix/execution-montage-terminal-contract`
- Base: `2122e331` (최종 에셋 이주 PR 병합)
- 선행 에셋/도구 커밋: `af6778aa`, `a68bd30d`, `62378b3f`

---

## 요약

새 에셋 통합 후 Montage 종료를 명시적 Complete로 통일하고, 실제 에셋의 종료 설정과 검사 도구를 보완한다. Source Execution은 Action, Target Execution은 Reaction으로 각각의 종료 Notify를 사용한다. 무기 표현·Montage·Player 에셋의 검증된 변경도 본 Fix 브랜치에 통합했다.

## 원인

Action의 자연 MontageEnd 자동 완료를 제거한 뒤 Stellar Equip의 Complete Trigger가 Unequip으로 설정된 문제가 드러났다. 사용자가 해당 설정을 수정하고 정상 종료를 확인했다. Reaction에는 자연 종료 fallback이 남아 있어 같은 명시적 종료 원칙이 적용되지 않았다.

기존 감사는 Complete 클래스 존재를 확인했지만 Trigger Type/Index 불일치를 충분히 판별하지 못했다. 클래스가 맞는 것과 런타임 실행에 전달되는 것은 다른 조건이다.

## 변경 사항

- `af6778aa`: Montage 설정 보완과 Execution Montage 감사 도구 도입.
- `a68bd30d`: promotion 작업본과 대조한 Montage·무기·Trail 등 Content 42개 통합.
- `62378b3f`: 검증한 Stellar Player Blueprint 포함.
- `UCReaction::OnMontageEnd`: 자연 종료 시 `Complete()` 대신 `NaturalMontageEndObserved`만 기록.
- 감사: Complete 클래스 외 Type과 Action Index를 검사. None/Max 및 불일치를 오류로 판정.
- 감사 결과의 미참조 표현을 `NoScannedComponentDataReference`로 한정. Notify 없는 Montage 행과 실제 이벤트 수를 분리.
- S26/S31/S39의 현행 계약 동기화. 과거 PR 기록은 덮어쓰지 않음.

## 변경하지 않은 것

- 정식 Stop/Interrupt, time-zero Notify 방어, Action Pose Scope와 Trail 종료 책임.
- Execution Source/Target 역할 및 장탈착의 물리 전환 후 논리 commit 순서.
- `DeathPresentationFallbackDelay`: 사망 표현 실패에 대한 별개 정책.
- enum 추가 재배열, 기존 Git 이력, LFS 과거 이력, 원격 브랜치.

## 검증 결과

- `git diff --check` 통과 (2026-09-17).
- 이전 구현 단계의 Editor Development 빌드 성공을 `UnrealBuildTool/Log.txt`에서 재확인함: 두 수정 cpp 컴파일, DLL 링크, WriteMetadata 8/8 완료. 이번 마감은 코드 diff와 기존 실행 로그를 대조했으며 새 빌드 실행으로 기록하지 않음.
- 보존된 `Portfolio-backup-2026.09.17-08.49.08.log`의 감사 결과: 발견/로드 1,399 packages, 실패 0, 데이터 행 93, 감사 Errors 0 / Warnings 0, Montage 49개, 직접 Notify 이벤트 151개.
- 감사 출력: `Saved/Audit/ReactionTerminalPrePIE`, `Saved/Audit/ReactionInventoryPrePIE` (로컬 생성물).
- 사용자가 최종 변경 후 PIE에서 모두 정상 동작한다고 확인함. 자동 테스트 결과와 구분한 사용자 검증임.
- 감사 0건은 전체 엔진 로그 무경고를 의미하지 않음. 기존 SK_Mannequin material import 경고는 별도 잔여 사항임.

## 검증 한계

감사는 Blueprint CDO component 데이터와 Montage에 직접 배치된 Notify를 대상으로 한다. Sequence 내부 Notify, 레벨 인스턴스 override, 동적 참조, 실행 중 Notify 전달은 보장하지 않는다. 검색된 component 데이터에서 미참조라는 결과는 미사용·삭제 가능 판정이 아니다. 수학 테스트 구현의 존재와 이번 실행 성공도 구분한다.

## 관련 문서

- [S26](../05_System_Architecture/S26_UE5_Portfolio_System_Architecture.md)
- [S31](../05_System_Architecture/S31_UE5_Portfolio_System_Architecture.md)
- [S39](../05_System_Architecture/S39_UE5_Portfolio_Weapon_Presentation_Pivot_Architecture.md)

---
