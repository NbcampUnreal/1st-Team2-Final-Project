
<div align = left>
  
## 🎮 게임 소개

> 깊은 심해, 자원이 고갈된 지구의 마지막 미개척지. <br><br>
> 플레이어는 'JMT 광물 탐사국'의 계약직 채굴 인턴이 되어, <br>
> 잠수복을 입어 **깊은 바닷 속을 탐사**하고, **광물을 채굴**해 **자원**을  획득한다. <br>
> 그러나, 바닷 속은 단순한 자원지대가 아닌 **의문과 공포가 도사리는 심해 생물의 서식지**. <br><br>
> 게임은 밝고 유쾌한 탐험 분위기로 시작하지만, 점차 **공포와 생존의 무게감**이 플레이어를 조여오는데…. <br><br>
> 숨막히는 **공포**를 이겨내고 **탐사**를 완료하라!

<img width="717" alt="2조 Abyss Diver 프로젝트 이미지(3)" src="https://github.com/user-attachments/assets/30ea249a-2e6a-4c33-afb7-4dadc3d98678" />

---

## 🎮 게임 개요

📌 장르 : 멀티플레이 협동 호러 서바이벌

📌 플랫폼 : PC (Unreal Engine 5 기반)

📌 개발 기간 : 2025.05.07 ~ 2025.07.02

📌 주요 특징 :<br>
    → 심해 탐험 : 제한된 시야, 빛과 산소 등 리소스 관리 요소를 통한 몰입감 높은 탐험 <br>
    → AI 몬스터 : 빛, 소리, 움직임에 반응하는 AI 적, 다양한 상태 전이 및 추적/회피 행동 <br>
    → 멀티플레이 : 네트워크 기반 협동/경쟁 모드 지원 (최대 4인) <br>
    → 환경 상호작용 : 해류, 구조물 탐사, 광석 등 환경과의 상호작용 <br>
    → 몰입감 있는 연출 : 언리얼 엔진을 활용한 리얼타임 심해 연출, 음향과 시각 효과 <br><br> 

📌 개발 목표 :<br>
    → 현실감 있는 심해 생존/공포 경험 제공<br>
    → 동적인 AI와 플레이어의 상호작용, 다양한 플레이 방식 지원<br>
    → 팀원 간의 협력 및 전략적 플레이 유도<br><br>

📌 주요 차별점 :<br>
    → 3D 네비게이션(3D NavMesh) 및 커스텀 AI 설계를 통한 자유로운 이동과 추격<br>
    → 플레이어 조명의 노출, 소리 등 다양한 변수에 반응하는 적 인공지능 구현<br>
    → 심해 특유의 압박감과 긴장감을 살리는 환경(레벨)/사운드 디자인<br><br>

---
## 🎥 Video (트레일러)
![스크린샷 2025-07-01 155352](https://github.com/user-attachments/assets/493eb057-b5f5-4069-8862-eb47b7c2ee97)
[https://youtu.be/JVUk93A31gM](https://youtu.be/nGzMUnRa-vc)

---
## 🛠️ 기술 스택
[![My Skills](https://skillicons.dev/icons?i=cpp,visualstudio,git,github,rider,unreal,notion&theme=light)](https://skillicons.dev)

---

## Work Flow


---
### Desctiption Part / Email or Blog Link <br>
- MultiPlayGame like HyperActionFight 
- Direction, Network, UI, Gameflow, Optimization : 지성현 /  <br>
- Character, Animation, SkillSystem : 정우영  /  [개발블로그](https://velog.io/@buu1147/2025-04-18-KPT%ED%9A%8C%EA%B3%A0)<br>
- Character, Animation, SkillSystem : 정혜창 / [개발블로그](https://velog.io/@hch9097/posts)<br>
- GameMode, Network, GameSession, Gameflow  : 최원석 /  [개발블로그](https://onestone13.tistory.com/)<br>
- UI, Weapon, Character, Movie, SkillSystem  : 한가윤 / [개발블로그](https://yoosorang.tistory.com) [Git](https://github.com/swehio))<br>
- UI, ShopSystem, EconomySystem  : 최세훈 /  <br>

---
## Project Part Directory And Main Features <br>
<details>
  <summary> Network GameSession </summary>
  
    | -- Source
      | -- GameMode
        | -- MainLobyGameMode // 클라이언트가 닉네임과, 사용할 케릭터를 선택할 수 있다.
        | -- GFBaseGameMode // 모든 플레이어가 선택을 완료 할 경우 다음 레벨로 전환한다.
      | -- Actor
- C++ 기반의 플레이어 로직 설계
  - Enhanced Input System을 사용한 입력 액션
  - CharacterMovement Component을 사용한 캐릭터 로직 설계
  - Tick을 사용하지 않은 이벤트 기반의 플레이어 로직 구현
  - GameInstance와 연동한 레벨 전환시 플레이어 정보 저장/불러오기 기능
</details>

<details>
  <summary> Character </summary>
  
        | -- Character
          | -- BP_MainCharacter // 케릭터가 멀티플레이어를 하기 위한 구조를 갖춘 기본 클래스
            | -- CBP_Default // 케릭터가 멀티플레이어 이후 애니메이션을 적용하기 위한 자료를 가지고 있는 클래스
              | -- ABP_Default // Locomotion과 타격 애니메이션이 들어있는 애니메이션 블루프린트
              | -- CBP_Night  //  리타겟한 스켈레탈이 보이는 케릭터
              | -- CBP_Archer  //  리타겟한 스켈레탈이 보이는 케릭터
              | -- CBP_Berserker //  리타겟한 스켈레탈이 보이는 케릭터
              | -- CBP_Magiction //  리타겟한 스켈레탈이 보이는 케릭터
</details>

<details>
  <summary> GameMode </summary> 

        | -- GameMode  
          | -- GFBaseGameMode  
            | -- GF_FFAGameMode // 개인전 (FFA) 모드  
            | -- GFStoreGameMode // 상점에서 아이템을 구매하는 스토어 모드  
            | -- BP_GM_Multiplayer (Blueprint) // 멀티플레이어용 기본 블루프린트 GameMode  
              | -- BP_GF_DOM_Multiplayer (Blueprint) // 점령전 (DOM, Domination) 모드  
              | -- BP_GF_TDM_Multiplayer (Blueprint) // 팀 데스매치 (TDM, Team Deathmatch) 모드  

- 팀전 모드를 템플릿을 기준으로 팀전 로직을 제거한 개인전 모드(FFA)를 별도로 제작하여 재사용성을 높임.
- 게임 모드를 개인전과 팀전 모드로 나누고, 추가 모드를 쉽게 확장할 수 있도록 설계함.
- 최상위에는 GFBaseGameMode를 두고, 레벨 전환과 플레이어 준비 완료 체크 등 공통 로직을 구현하여 중복을 제거.
</details>

<details>
  <summary> Weapon </summary>
  
          | -- WeaponBase // 근접 무기에 메쉬와 로직을 담는 클래스     
              | -- TowHandedSword // 양손검
              | -- SwordAndShield // 검과 방패
          | -- RangeWeaponBase // 원거리 무기 메쉬와 로직을 담는 클래스
              | -- MagicBook // 마법책
              | -- Bow // 활
          | -- SkillBase // 콜리전과 이펙트를 가진 클래스
              | -- VFX // 이펙트만 소유한 클래스 (제거 및 변경을 원하랗게 하기 위해서)     
</details>

<details>
  <summary> Animation </summary>
### Animation
</details>

<details>
  <summary> Shop </summary>
### Shop
</details>

<details>
  <summary> UI </summary>
### UI
</details>

<details>
  <summary> SkillCoolTime </summary>
### SkillCoolTime
</details>

---
## 개발 환경 / 개발 도구
- C++ 17 (v14.38)
- MSVC v143
- Unreal Engine 5.5
- Visual Studio 2022
- Git LFS
- Rider
</div>

---
## Network GameSession
<details>
<summary>Network GameSession UML Diagram</summary>    

</details>

## Character
<details>
<summary>Character UML Class Diagram</summary>
</details>

## GameMode
<details>
<summary>GameMode UML Class Diagram</summary>
</details>

## Weapon
<details>
<summary>Weapon UML Class Diagram</summary>
</details>

## Animation
<details>
<summary>Animation UML Class Diagram</summary>
</details>

## Shop
<details>
<summary>Shop UML Class Diagram</summary>
</details>

## UI
<details>
<summary>UI UML Class Diagram</summary>
</details>
