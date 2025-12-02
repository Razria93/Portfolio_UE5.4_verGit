## 작업 내역 임시 저장
#### 1. 현재 작업내역 임시 저장
```bash
# 1) 현재 변경사항 임시 보관 
git status git stash push -m "StashMessage"  

# 2) 다른 브랜치 가서 작업 
git switch main 

# ... 급한 수정, commit, push ...  

# 3) 다시 돌아와서 
git switch feature/move-system  

# 4) stash 꺼내오기 
git stash list               # 저장된 목록 확인 
git stash apply stash@{0}    # 방금꺼 적용 (stash는 남아있음) 
# 또는 
git stash pop                # 적용 + 목록에서 제거
```

- 지금 하던 거 마저 못 끝냈는데,  **다른 브랜치로 잠깐 넘어가서 급한 버그를 봐야 할 때**”
- stash도 충돌 날 수 있음 → 그때는 일반 충돌처럼 해결

---
