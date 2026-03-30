## 특정 시점 커밋 관찰하기
#### 1. 특정 시점 커밋 관찰 하기
```bash
# 1) 기준 커밋 확인
git log --oneline

# 2) 특정 CommitHash로 HEAD 옮기기
git checkout <CommitHash> 
# 또는 
git switch --detach <CommitHash>   

# 3) 이 상태에서 빌드/실행/비교만  

# 4) 끝난 후 다시 main으로
git switch main
```
- 이 커밋 시점에서 빌드/실행만 해보고 싶고, 작업은 안 할 때
- 여기서는 **새 커밋 만들어도 브랜치에 연결 안 됨**  
	- 해당 명령어는 해당 커밋에 임시 브랜치를 만들어 관찰하는 방식임
    - 필요하면 나중에 `git switch -c <BranchName>` 로 브랜치로 승격 가능

#### 2. 특정 시점 기준으로 임시브랜치를 생성하기
```bash
# 1) 기준 커밋 확인
git log --oneline

# 2) 거기서 새 브랜치 생성
git switch -c <BranchName> <CommitHash>

# 3) 여기서 작업 & 실험
#   ... code ...

git commit -m "CommitMessage"
```

---
