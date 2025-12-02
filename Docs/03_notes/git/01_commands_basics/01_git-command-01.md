## Git Command-01
#### 1. Status & Log
```Bash
git status                       # ★ 현재 상태 확인(추가됨/변경됨/커밋됨/브랜치 이름 등)
git diff                         # ★ 워킹디렉터리 ↔ 스테이징 차이
git diff --cached                # 스테이징된 내용 ↔ 마지막 커밋 차이
git log                          # 기본 로그
git log --oneline                # ★ 한 줄로 간단하게 로그 보기
git log --graph --oneline --all  # 브랜치 구조까지 텍스트 그래프로
```

#### 2. File Add & Commit
```Bash
git add .                       # ★ 변경된 파일 전부 스테이징
git add (Path/FileName)         # 특정 파일만 스테이징
git restore --staged (FileName) # 스테이징 취소 (git reset HEAD 파일 과 비슷)
git commit -m "Message"         # ★ 커밋 생성
git commit -am "Message"        # ★ 파일 스테이징 + 커밋 생성 (파일 추가 시 사용 X)
git commit --amend              # 방금 전 커밋 메시지/내용 수정
```

#### 3. Branch
```Bash
git branch                                      # ★ 브랜치 목록 확인
git branch (BranchName)                         # 새 브랜치 생성
git switch (BranchName)                         # ★ 해당 브랜치로 이동
git switch -c (BranchName)                      # 브랜치 생성 + 이동 
git branch -d (BranchName)                      # 로컬 브랜치 삭제(머지된 것만)
git branch -D (BranchName)                      # 강제 삭제
git branch -m (Old_BranchName) (New_BranchName) # 브랜치 이름 변경
```

#### 4. Detached HEAD
```Bash
git switch --detach (CommitHash)            # 해당 커밋의 위치에 생긴
git switch -d (CommitHash)                  # 임시헤드(Detached HEAD)를 구성
git checkout (CommitHash)                   # (위와 동일)

git switch (BranchName)                     # Detached 상태에서 BranchName으로 복귀
git switch -                                # 직전에 있던 브랜치로 돌아가기
git switch -c (New_BranchName) (CommitHash) # CommitHash에 Detached HEAD를 생성 +
                                            # 새 브랜치 생성 + 그 브랜치로 이동
```

#### 5. Remote
```Bash
git remote -v                      # 원격 목록 확인
git remote add (origin) (URL)      # origin 원격 추가
git clone (URL)                    # ★ 원격 저장소를 로컬로 복제

git push -u (origin) (BranchName)  # ★ origin/브랜치에 현재 브랜치 tracking + push
git push                           # 현재 브랜치 push (tracking 설정된 경우)
git pull                           # ★ origin/현재 브랜치의 변경 사항 가져와 merge
git fetch                          # 원격 변경 내용만 가져오고, merge는 안 함
```

---
