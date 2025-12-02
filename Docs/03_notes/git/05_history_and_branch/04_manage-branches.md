## 브랜치 정리
#### 1. 브랜치 정리
```bash
# 1) 브랜치 목록 확인
git branch

# 2) main에 merge 끝난 브랜치 삭제
git branch -d (baranchName)    # 정상 삭제 (머지돼 있으면)

# 3) 강제 삭제 (실수 위험 있음)
git branch -D (baranchName)

# 4) 원격 브랜치도 삭제
git push origin --delete (baranchName)

# 5) remote에 없어졌는데 로컬에만 잔상 남은 것들 정리
# 모든 remote의 변경사항 가져오고, remote에서 삭제된 브랜치는 로컬에서도 정리
git fetch --prune
```

---
