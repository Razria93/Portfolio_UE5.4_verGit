## 제목

🐛 fix(apply-damage): PR #19에서 누락된 파일 추가 (#16)

## 요약

- 이전에 머지된 PR에서 실수로 누락된 헤더 파일 `Source/Portfolio/Type/DamageEventId.h`를 추가함
    
- 기능 로직 변경 없이 누락 파일만 포함하여 빌드/구성 완전성을 복구함
    

## 변경 사항

- 추가: `Source/Portfolio/Type/DamageEventId.h`
    

## 테스트 방법

-  에디터/프로젝트 빌드 성공 확인
    
-  ApplyDamage 흐름 간단 점검 수행(예: 트리거 로그 확인 / include 컴파일 확인)
    

## 관련

- branch: fix/apply-damage-omitted-untracted-file

- Follow-up: PR #19
    
- Issue: #16
    

---