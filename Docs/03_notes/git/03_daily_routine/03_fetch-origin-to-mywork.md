## remote/origin 내 작업에 반영하기

- `main`이 계속 변하는 상황에서, **내 작업 브랜치를 최신 main 위로 올리고 싶을 때**

#### 01. Main 최신화 후 MyWork에 merge
```bash
# 1) main 최신화
git switch main
git pull origin main

# 2) main 브랜치 쪽의 새로운 커밋들을 MyBranch의 브랜치에 통합
git switch feature/move-system
git merge main

# 3) 충돌 나면 해결
# ...

# 4) 충돌 해결 후 Staging - Commit
git status
git add .
git commit        # 자동 메시지 사용

# 그 다음 계속 작업
```

#### 02. Main 최신화 후 MyWork에 Main을 Rebase
```bash
# 1) main 최신화
git switch main
git pull origin main

# 2) MyBranch의 커밋들을 `origin/main` HEAD 뒤로 옮김
git switch feature/move-system
git fetch origin
git rebase origin/main
# 내부 알고리즘
# 1.`feature/move-system`에서만 있는 커밋들(D, E)을 떼서
# 2. `origin/main`의 최신 커밋 C 뒤에 '다시 적용'해서 새 커밋(D', E')로 만든 다음 
# 3. `feature/move-system`의 HEAD를 E'로 옮김

# 3) 충돌 나면 해결
# ...

# 4) 부분 충돌 해결 후 
git add (FileName)
git rebase --continue

# 5) 그 다음 계속 작업
```

---
