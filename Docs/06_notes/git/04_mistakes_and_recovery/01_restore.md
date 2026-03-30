## 작업을 버리고 싶은 경우

#### 1. 워킹 디렉토리 수정만 버리기 (아직 add 안 한 것)
```bash
# 특정 파일만 원래 커밋 상태로 되돌리기 
git restore FileName.cpp  

# 폴더 전체 
git restore .
```

#### 2. 이미 `git add`까지 한 상태를 되돌리고 싶을 때
```bash
# 스테이징에 올린 것 다시 내리기 
git restore --staged FileName.cpp  

# 그리고 필요하면 워킹 디렉토리도 되돌리기 
git restore FileName.cpp
```

---
## 작업을 복원하고 싶은 경우
#### 01. 과거 커밋에서 특정 파일만 다시 가져오기
```bash
# 1) 원하는 시점의 커밋 해시 확인
git log --oneline

# 2) 해당 커밋의 파일 내용을 현재 브랜치로 가져오기
git checkout <CommitHash> -- <FileName>
# 또는
git restore --source=<CommitHash> -- <FileName> # 현대식 문법

# 3) 내용 확인
git diff

# 4) 스테이징 후 커밋
git add <FileName>
git commit -m "CommitMessage"

```

#### 02. main에 있는 최신 버전을 내 브랜치로만 끌어오기
```bash
# 1) main의 파일 내용을 현재 브랜치로 가져오기
# 현재 브랜치: feature/move-system라고 가정
git checkout main -- <FileName>
# 또는
git restore --source=main -- <FileName> # 현대식 문법

# 2) 내용 확인
git diff

# 3) 스테이징 후 커밋
git add <FileName>
git commit -m "CommitMessage"

```
- 여전히 `feature/move-system` 브랜치에 있음
- 다만 `main`에 있던 `FileName` 내용이 현재 브랜치 파일로 덮어씌워짐

---
