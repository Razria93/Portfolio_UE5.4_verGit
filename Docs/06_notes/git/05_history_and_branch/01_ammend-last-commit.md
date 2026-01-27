## 마지막 커밋 수정하기
#### 1. 메시지만 고치기 (Commit 내용은 그대로)
```bash
git commit --amend -m "Commit Message"
```
- 아직 push 안 했을 때만 안전하게 사용 권장

#### 2. 직전 커밋에 파일 추가로 붙이고 싶을 때
```bash
# 1) 파일 수정
git status
git add <FileName>

# 2) 마지막 커밋 덮어쓰기
git commit --amend
# or 메시지까지 한 번에
git commit --amend -m "Commit Message"

# 3) 원격 덮어쓰기
# 안본 feature 브랜치의 초기 작업 정리 / 빠진 파일 추가 / 메세지 오타 정정 등에서만 사용 권장
# [주의] 원격의 구조를 바꾸는 것은 협업 시 바람직하지 않음 (main, develop 등 공유된 브랜치)
# --force-with-lease : 내가 pull 해서 알고 있는 상태와 원격의 상태가 같다면 덮음
git push --force-with-lease
```

---
