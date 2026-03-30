## 루틴 패턴
#### 1. Basic
```Bash
# 1) 브랜치 변경
git switch main
git pull --rebase origin main    # 원격까지 최신으로

# 2) 기능단위 작업용 브랜치 생성
# 작업용 브랜치명은 기능에 맞게 (여기선 예시로 move-system 사용)
git switch -c feature/move-system

# ---
# 3) 개발 (여기서 VS/VSCode로 작업)
# 작업 시 사용하는 루틴
# 3-1) 현재 상태 확인
git status

# 3-2) 변경사항 확인
git diff               # 워킹 디렉토리 vs 스테이징
git diff --cached      # 스테이징 vs 최신 커밋

# 3-3) 파일 스테이징 (택1)
git add 파일1 파일2     # 원하는 파일만 스테이징
git add .              # 또는 한번에 스테이징

# 3-4) 커밋
git commit -m "Commit Message"

# 3-5) 히스토리 정리 (자세한 것은 별도 문서 참고)
git rebase -i (BaseCommitHash)
git add <FileName> 
git rebase --continue
git rebase --abort

# 3-6) 원격으로 올리기
git push -u origin feature/move-system         # 최초 push일 경우
git push origin feature/move-system            # 그 외

# 3-7) 브랜치 정리 (선택)
git branch -d feature/move-system              # 로컬 브랜치 삭제 (선택)
git push origin --delete feature/move-system   # 원격 브랜치 삭제 (선택)

# ---
# 4) feature의 작업을 main으로 합쳐서 마무리할 때
git switch main
git pull --rebase origin main
git merge --no-ff feature/move-system 
git push origin main
git branch -d feature/move-system      

```

---
#### 추가 설명
##### git merge --no-ff feature/move-system  
- ff가 가능한 상황에서 
- fast-forward로 포인터만 당기지 않고,
- 무조건 merge commit을 하나 생성하도록 강제함

---
