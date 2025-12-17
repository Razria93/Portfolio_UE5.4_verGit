## Git Command-03
#### 6. Restore / Reset / Revert
```Bash
# 1. 되돌리기 (작업내역 취소)
git restore (FileName)          # (FileName)의 워킹 디렉토리 변경 취소
git restore .                   # 모든 파일의 워킹 디렉토리 변경 취소
git restore --staged (FileName) # (FileName)의 스테이징 취소
git restore --staged .          # 모든 파일의 스테이징 취소

# 1. 되돌리기 (롤백)
git reset [Opts] (TargetHash)   # (TargetHash)를 HEAD Commit으로 되돌림
								# Opts에 따라 되돌린 내용을 어떻게 처리할지 결정
[Opts]
--soft                          # HEAD만 대상이 되는 Commit으로 변경
--mixed                         # HEAD / Index를 대상이 되는 Commit으로 변경
--hard                          # HEAD / Index / WorkingDirectory를 대상이 되는
                                # Commit으로 변경

[TargetHash]
TargetHash                      # Target이 되는 Commit의 Hash값을 직접 입력
HEAD~(Num)                      # HEAD에서 (Num)만큼 이전의 Commit
HEAD^^                          # HEAD 뒤에 붙인 ^의 갯수만큼 이전의 Commit

# 2. 되돌리기 (역연산 커밋)
git revert (TargetHash)         # 커밋을 "역연산 커밋"으로 되돌리기 (히스토리 보존)
```

#### 7. Statch
```Bash
git stash                       # ★ 현재 작업 내용을 임시저장 + 워킹트리 깨끗하게
git stash list                  # 저장된 stash 목록
git stash apply                 # 가장 최근 stash 적용 (목록은 남김)
git stash pop                   # 적용 + 목록에서 제거
git stash drop                  # 특정 stash 삭제
```

#### 8. Clean
```bash
# 미리보기 (반드시 선행하는 습관 들이기)
git clean -n                    # ★ 삭제될 untracked "파일" 목록 미리 보기(dry-run)
git clean -nd                   # ★ 삭제될 untracked "파일 + 디렉토리" 목록 미리보기

# 실제 삭제 (매우 주의)
git clean -f                    # untracked 파일 삭제
git clean -fd                   # ★ untracked 파일 + 디렉토리 삭제
git clean -fdx                  # ★ .gitignore로 무시되는 것까지 포함해서 싹 다 삭제
```
- **대상:** Git이 모르는 것들 → `Untracked` / `.gitignore` 무시 여부는 `-x`에 따라 달라짐

- **하는 일:**
    - **워킹 디렉토리**의 실제 파일/폴더를 삭제
    - 인덱스(Index)는 원래 untracked를 모름 → **인덱스는 건드리지 않음**
        
- 사용 패턴:
    - `git clean -nd` → 목록 확인
    - `git clean -fd` → 정말 괜찮다고 확신될 때만 실행

#### 9. Remove
```bash
# 기본 삭제 (tracked 파일을 Git에서 제거)
git rm 파일                     # 워킹 디렉토리에서 삭제 + "삭제" 상태로 스테이징
git rm -r 폴더/                 # 폴더 & 내부 파일들 재귀적으로 삭제
git rm -f 파일                  # 변경된 파일도 강제로 삭제 (안전장치 무시)

# 추적만 끊고, 실제 파일은 남기기
git rm --cached 파일            # 로컬 파일은 남기고, Git 추적만 해제
git rm -r --cached 폴더/        # 폴더를 추적 대상에서만 제거 (파일은 그대로 유지)

# 전체 인덱스를 비우고 .gitignore 기준으로 다시 채울 때 자주 쓰는 패턴
git rm -r --cached .            # ★ 인덱스에서 전부 제거(언트래킹 상태로) 후, 
                                # git add . 로 재등록
```
- **대상:** `tracked`(이미 Git이 관리 중인) 파일/폴더
- **동작 요약**
    - `git rm`
        - **Working Directory:** 실제 파일 삭제
        - **Index:** 해당 파일을 “삭제됨” 상태로 기록(스테이징)
        - **결과:** 다음 커밋에서 “이 파일이 삭제되었다”가 기록됨
            
    - `git rm --cached`
        - **Working Directory:** 파일 그대로 유지
        - **Index:** 해당 파일을 인덱스에서 제거 → **“이제부터 이 파일은 Git이 관리 안 함”** 상태로 스테이징
	  
---
