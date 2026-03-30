## 특정 커밋 가져오기
#### 1. '특정 커밋만' 가져오기
```bash
# 1) 원하는 커밋 해시 확인
git log --oneline other-branch

# 2) 현재 브랜치로 이동
git switch main

# 3) 커밋 하나 가져오기
git cherry-pick <CommitHash>
```

#### 2. 여러 개일 경우
```bash
# 1) 여러 개의 커밋해시 (명시)
git cherry-pick <CommitHash1> <CommitHash2> <CommitHash3>

# 2) 여러 개의 커밋해시 (구간)
git cherry-pick <OldCommitHash>^..<NewCommitHash>
```

---
