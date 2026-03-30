## ignore 항목 재설정
#### 1. .gitignore 에 규칙 추가
```bash
# .gitignore 수정 
echo "Binaries/" >> .gitignore
```

#### 2️. Tracked 목록 재설정
```bash


# ---
# 1. Tracked목록 부분 재설정

# .gitignore에 추가한 목록들을 수동으로 제거
git rm -r --cached Binaries/ 
git rm -r --cached Saved/ 

git commit -m "CommitMessage" 
git push


# ---
# 2. Tracked목록 전체 재설정

# Tracked 목록 전체 해제
git rm -r --cached .

# Tracked 목록 전체 추가
# 이 때, .gitignore의 규칙이 적용되어 Tracked 목록이 재설정됨
git add .

git commit -m "CommitMessage" 
git push
```
- `.gitignore`을 수정했으므로 Tracked 목록을 재설정 해야 함

---