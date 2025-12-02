## 히스토리 조회
#### 1. History 조회
```bash
# 최근 커밋 로그 간단히 
git log --oneline --graph --decorate --all  

# 특정 파일에 대해, 누가 언제 어떻게 바꿨는지
git log -- <FileName>  

# 특정 커밋에서 어떤 변경이 있었는지 
git show <CommitHash>  

# 두 커밋 사이 차이 
git diff <CommitHash1> <CommitHash2>
```
- 습관 추천:
	- commit 하기 전에 항상 `git diff` 한 번
	- 작업 방향 헷갈리면 `git log --oneline --graph` 한 번

---