## 기능 단위 작업
##### 01. Branch에서 작업
```bash
# 1) main branch 이동 (택1)
git checkout main
git switch main

# 2) branch 최신화
git pull origin main

# 3) 기능 브랜치 생성 & 전환 (택1)
# feature/move-system은 예시용 branchName (구분을 위해 폴더트리와 같은 형태로 작명)
git checkout -b feature/move-system
git switch -c feature/move-system

# 3) 개발 (여기서 VS/VSCode로 작업)
#   ... 작업 ...

git status
git add .
git commit -m "feat: 기본 이동 시스템 구현"

# 필요하면 여러 커밋 더
git commit -m "refactor: 입력 처리 구조 정리"

# 4) 원격에 브랜치도 올리기(백업 + PR용)
git push -u origin feature/move-system
```

##### 02. Branch를 main에 Merge
```bash
# 1) main branch 이동 (택1)
git checkout main
git switch main

# 2) branch 최신화
git pull origin main

# 3) 기능 브랜치 내용을 main에 병합
git merge feature/move-system

# 충돌 발생하면 충돌 해결

# 4) 충돌 없으면/충돌 해결하면
git push origin main

# 5) 브랜치 정리 (선택)
git branch -d feature/move-system
git push origin --delete feature/move-system   # 원격 브랜치 삭제(선택)
```

---
