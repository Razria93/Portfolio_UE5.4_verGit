## 제목

🐛 fix(apply-damage): PR #19에서 누락된 파일 추가 (#16)

## 요약

- 이전에 머지된 PR #19에서 **stash에 남아 있던 파일들이 포함되지 않아** 후속 PR로 누락 파일을 추가함
    
- 기능 로직 변경 없이 누락 파일만 포함하여 빌드/구성 완전성을 복구함
    

## 변경 사항

- 추가: `Source/Portfolio/Character/Enemy/CEnemy.h`
    
- 추가: `Source/Portfolio/Character/Enemy/CEnemy.cpp`
    
- 추가: `Source/Portfolio/Component/CApplyDamageComponent.cpp`
    
- 추가: `Source/Portfolio/Type/CWeaponStructure.h`
    

## 테스트 방법

- 에디터/프로젝트 빌드 성공 확인
    
- ApplyDamage 흐름 간단 점검 수행(예: 트리거 로그 확인 / include 컴파일 확인)
    

## 관련

- branch: `fix/apply-damage-omitted-stash-file`
    
- Follow-up: PR #19
    
- Issue: #16
    

---

## 3) PR 본문 (EN)

## Title

🐛 fix(apply-damage): add omitted files from PR #19 (#16)

## Summary

- Added files that were unintentionally left out of the previously merged PR #19 because they were still stashed
    
- Restored build/config completeness by including only the omitted files without changing any feature logic
    

## Changes

- Added: `Source/Portfolio/Character/Enemy/CEnemy.h`
    
- Added: `Source/Portfolio/Character/Enemy/CEnemy.cpp`
    
- Added: `Source/Portfolio/Component/CApplyDamageComponent.cpp`
    
- Added: `Source/Portfolio/Type/CWeaponStructure.h`
    

## Test Plan

- Confirm the editor/project builds successfully
    
- Run a quick sanity check for the ApplyDamage flow (e.g., verify trigger logs / compilation includes)
    

## Related

- branch: `fix/apply-damage-omitted-stash-file`
    
- Follow-up: PR #19
    
- Issue: #16
    

---