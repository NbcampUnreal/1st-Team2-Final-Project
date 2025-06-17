#include "Gimmic/Volume/BattleFieldVolume.h"
#include "Character/UnderwaterCharacter.h"

void ABattleFieldVolume::BeginPlay()
{
	Super::BeginPlay();

	// 필드에 플레이어가 남아있는 지 확인하기 위해 함수를 호출할 때마다 Overlap을 하여 확인하는 방법도 있다.
	// Volume의 크기가 커짐에 따라 비용이 증가할 우려가 있기 때문에
	// 플레이어가 오버랩 될 때마다 목록에 추가하는 방식으로 관리한다.
	OnActorBeginOverlap.AddDynamic(this, &ABattleFieldVolume::OnComponentBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &ABattleFieldVolume::OnComponentEndOverlap);
}

AUnderwaterCharacter* ABattleFieldVolume::GetBattleFieldPlayer()
{
	// 배열이 비어있는지 확인
	if (BattleFieldPlayers.Num() == 0) return nullptr;
	
	// 랜덤 인덱스 생성 (0부터 배열 크기-1까지)
	const uint8 RandomIndex = FMath::RandRange(0, BattleFieldPlayers.Num() - 1);
    
	// 랜덤으로 선택된 플레이어 반환
	return BattleFieldPlayers[RandomIndex].Get();
}

void ABattleFieldVolume::OnComponentBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;

	if (BattleFieldPlayers.Contains(Player)) return;
	
	BattleFieldPlayers.Add(Player);

	if (OnBattleFieldBeginOverlapDelegate.IsBound())
	{
		LOG(TEXT(" Broadcast Begin Overlap: %s"), *Player->GetName());
		OnBattleFieldBeginOverlapDelegate.Broadcast();
	}
}

void ABattleFieldVolume::OnComponentEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(OtherActor);
	if (!IsValid(Player)) return;

	if (!BattleFieldPlayers.Contains(Player)) return;
	
	BattleFieldPlayers.Remove(Player);

	if (OnBattleFieldEndOverlapDelegate.IsBound())
	{
		LOG(TEXT("Broadcast End Overlap: %s"), *Player->GetName());
		OnBattleFieldEndOverlapDelegate.Broadcast(BattleFieldPlayers.Num());
	}
	
}
