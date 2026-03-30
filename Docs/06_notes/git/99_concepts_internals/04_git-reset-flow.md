
#### D0. Reset 개요
##### 원본 키워드
- `어느 커밋을 기준으로 HEAD / Index / WorkingDirectory를 맞출 것인가`를 결정
- `Blob/Tree/Commit` 객체 자체를 삭제/생성하지는 않음 (ref/스냅샷만 이동)

---
##### D1. reset --soft
##### 원본 키워드
- `Branch ref / HEAD`만 이동
	- `HEAD`는 `Branch ref(파일)`를 가리키고, 그 `Branch ref`가 `Commit Hash`를 가리킵
	- 실질적으로 `Branch` 내에서 `Commit`을 가리키는 것은 `ref`
	- 즉, HEAD를 변경한다는 것은 `작업의 기준점을 변경한다는 것`임
- `Index / WorkingDirectory`는 그대로
- 결과적으로 “커밋만 되돌리고, 변경 내용은 그대로 staged 상태로 남음”
##### 확장 설명  
- 만약 막 `git commit`을 한 직후라고 가정하면 :
	1. `reset --soft` 전후로 `Index / WorkingDirectory` 자체는 바뀌지 않음
	2. 커밋 직후에는 `HEAD Commit`의 Tree와 `Index` 내용이 같았는데,
	  `reset --soft <과거 커밋>`으로 인해 **HEAD 기준점이 과거 커밋으로 이동**하면서  
	  이제는 `HEAD Commit`과 `Index` 사이에 차이가 생기게 됨
	3. Git은 이 차이를 “staged 변경사항”으로 인식하므로,  
	  **Index 내용이 다시 한 번 커밋을 기다리는 상태**가 된 것처럼 보이게 되는 것

그 내용이 아직도 staging 되어 있는 것처럼 보이게 하는 용도.

---
#### D2. reset --mixed (기본값)
##### 원본 키워드
- `Branch ref / HEAD` 이동
- `Index`는 **지정한 커밋의 Tree 스냅샷으로 교체**
- `WorkingDirectory`는 그대로

###### 결과적으로 :
- 브랜치 기준은 과거/지정 커밋으로 되돌리고, 
  Index도 그 커밋 상태로 맞춘 뒤, 
  WorkingDir와 변경된 Index 사이의 차이를 전부 unstaged 변경사항으로 만듦

##### 확장 설명
- 예를 들어, 커밋을 하나 한 직후 상황을 가정하면:
	1. 커밋 직후에는
		- `HEAD Commit의 Tree`와 `Index 내용`이 같고
		- `WorkingDirectory`도 `Index`와 동일 (클린 상태)
		- 
	2. 여기서 `git reset --mixed <과거_커밋>`을 실행하면:
		- `HEAD / Branch ref`가 `<과거_커밋>`을 가리키도록 이동
		- `Index`는 `<과거_커밋>의 Tree 스냅샷`으로 덮어쓰여서,
		- 이제 `Index = <과거_커밋>` 상태가 됨
		- WorkingDirectory는 그대로라서, 방금 만들었던 커밋의 내용이 워킹 디렉토리에는 남아 있음
###### 결과적으로 :
- `HEAD vs Index`는 둘 다 `<과거_커밋>` 기준이므로 차이가 없음
- 하지만 `Index vs WorkingDirectory` 사이에는 **방금 커밋에서 추가/수정했던 내용 전체**가 차이로 나타남
- Git은 이 차이를 **“unstaged 변경사항”**으로 인식하므로,
	git status 기준으로는 “작업 내용이 전부 staging 되지 않은 변경사항” 상태가 됨
- 사실상 reset --mixed HEAD~1는
	**마지막 커밋을 없애고, 그 내용을 전부 unstage 상태로 워킹 디렉토리에 남기는 것**과 같은 효과

---
#### D3. reset --hard
##### 원본 키워드
- `Branch ref / HEAD` **이동**
- `Index`는 **지정한 커밋의 Tree 스냅샷으로 교체**
- `WorkingDirectory`도 **지정한 커밋의 Tree 스냅샷으로 덮어쓰기**

###### 결과적으로 :
- 브랜치 기준, Index, 실제 파일 상태 모두를 특정 커밋 상태로 강제 동기화
- 추적 중이던 파일의 변경 내용은 전부 사라지고, 해당 커밋 시점의 내용으로 롤백됨

#### 확장 설명
- `reset --hard <타깃_커밋>`이 하는 일은 단계별로 보면 :
	1. `Branch ref / HEAD` 이동
		- 브랜치 ref가 <타깃_커밋> 해시를 가리키도록 변경
		- HEAD는 그 브랜치 ref를 계속 가리킴
		- 즉, “이제부터 이 커밋을 기준으로 작업하겠다”로 기준점 변경
	2. `Index를 <타깃_커밋>의 Tree로 교체`
		- Index에 들어 있던 기존 스냅샷은 버리고,
		- <타깃_커밋>의 디렉터리/파일 구조 + Blob 해시 정보로 다시 채움
	3. WorkingDirectory를 <타깃_커밋> 기준으로 덮어쓰기
		- Git이 추적하고 있던 파일들에 한해서, 
		  워킹 디렉토리의 실제 파일 내용을 <타깃_커밋>의 Blob 내용으로 교체
###### 결과적으로 :
- 수정했던 내용, 새로 만들었지만 추적 중이던 파일의 변경사항 등은 
	  - 해당 커밋 상태로 완전히 되돌아감
- HEAD Commit의 Tree, Index, WorkingDirectory
	- 세 레이어가 모두 같은 스냅샷을 가리키는 완전 클린 상태가 됨
---
###### 주의 :
- 이건 단순히 “기준점만 옮기는 것(soft)”이나
	“Index만 롤백하고 변경사항을 unstage로 남기는 것(mixed)”과 달리,
	실제 파일 내용까지 과거 커밋 기준으로 되돌리기 때문에,
	작업 내용이 날아갈 수 있는 위험한 명령임

---
##### 요약하면:

**reset --soft** : 기준점(HEAD)만 과거로 → 변경사항은 그대로 staged
**reset --mixed** : 기준점 + Index를 과거로 → 변경사항은 unstaged로
**reset --hard** : 기준점 + Index + WorkingDir 전부 과거로 → 변경사항 자체가 삭제 (해당 커밋 상태로 강제 동기화)

---
