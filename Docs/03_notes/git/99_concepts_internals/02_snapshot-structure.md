#### B1. Working Directory (워킹 디렉토리)
##### 원본 키워드
- 실제 파일이 있는 OS 상의 폴더
- 수정 중인 내용, 미저장 변경 포함
- 실제로 사용자가 작업하는 영역
##### 확장 설명
- 에디터에서 수정하는 파일
- Git이 관리하는 것은 “`WorkingDirectory` ↔ `Index` ↔ `HEAD`” 사이의 차이

---
#### B2. Index (Staging Area)
##### 원본 키워드
- 다음 커밋에 들어갈 스냅샷
- 경로를 기반으로 한 `Blob`의 해시 테이블 (+stat 정보)
- `git add`로 수정하는 대상

##### 확장 설명
- Index는 “다음 커밋이 어떤 파일/버전들을 포함할지”를 모아두는 작업장
- 아래 시점에 Index가 특정 커밋의 스냅샷으로 채워짐
	- git checkout (commit/branch)
	- git switch (branch)
	- git reset --mixed/--hard (commit)
	- 위 명령어가 호출될 경우, 변경의 대상이 된 `commit`혹은 `branch`의 SnapShot의 구성으로 Index의 데이터가 갱신됨

----
#### B3. HEAD Snapshot (Last Commit)

##### 원본 키워드
- 마지막으로 커밋된 스냅샷
- HEAD가 가리키는 Commit의 Tree
	- 해당 Commit의 전체 파일 구조 스냅샷 (트래킹 중인 것만)

##### 확장 설명
- HEAD Commit의 Tree가 **현재 브랜치 기준 마지막 커밋 상태**
- Index/Working Dir이 해당 Tree와 얼마나 다른지가 staged / unstaged / clean 상태를 결정
- `git checkout`이나 `git switch --detach`으로 HEAD의 위치를 옮기는 것 또한 해당 Commit을 시작으로 하는 새로운 임시 Branch를 만드는 것임
  
---
#### B4. 세 단계 관계
##### 기본 설명
- `HEAD` (커밋의 기준)
	- `HEAD` : 지금 내가 서있는 `Commit`
	- `HEAD`를 바꾸면 : 
		- 이제부터 이 `Commit`을 기준으로 작업/비교/커밋을 이어가겠음
		- 즉, 이 `Commit`뒤에다가 History를 쌓겠다는 의미
- `Index` (다음 커밋 후보)
	- `Index` : `HEAD`뒤에 생성될 예비 `Commit`
	- `Index`를 바꾸면 :
		- 다음에 생성될 `Commit`의 내용을 바꾸겠다는 것임
		- `git add`를 호출할 경우 
			1. 파일의 `hash`를 비교하여 갱신여부를 확인
			2. 존재하는 `Blob`이라면 해당 `Blob의 hash`를 재사용하여 `Index`를 갱신함
			3. 존재하지 않는다면 새로운 `Blob`을 생성하고 해당 `hash`값을 `Index`에 갱신함
- `Working Dir` (실제 파일)
	- `Working Dir` : 실제로 작업하고 수정이 이루어지는 파일
	- `Working Dir`를 바꾸면 :
		- 현재 작업하고 있는 내용을 바꾸겠다는 것임
		- 말 그대로 작업하고 있는 내용을 바꾸는 것이기 때문에 별도의 백업을 하지 않고 reset을 할 경우 작업 내용이 그대로 `override`되어 복구가 불가능해질 수 있음

##### 확장 설명
- 명령어에 따라서 `HEAD만 바꾸거나`, `HEAD + Index만 바꾸거나`, `HEAD + Index + WorkingDir을 다 바꾸거나` 하는 등 작업 내역이 다름
- 같은 명령어에 추가 조건을 어떻게 다느냐에 따라서 결과값이 매우 달라질 수 있으며, 돌이킬 수 없거나 매우 작업이 힘들어질 수 있으니 정확히 알고 사용할 것

---