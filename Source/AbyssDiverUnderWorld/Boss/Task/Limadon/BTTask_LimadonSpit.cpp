#include "Boss/Task/Limadon/BTTask_LimadonSpit.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Boss/Limadon/Limadon.h"
#include "Character/UnderwaterCharacter.h"

UBTTask_LimadonSpit::UBTTask_LimadonSpit()
{
	NodeName = TEXT("Limadon Spit");
}

EBTNodeResult::Type UBTTask_LimadonSpit::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 캐스팅 실패 시 얼리 리턴
	ALimadon* Limadon = Cast<ALimadon>(OwnerComp.GetAIOwner()->GetPawn());
	if (!IsValid(Limadon)) return EBTNodeResult::Failed;

	// 뱉는 로직 및 애니메이션 재생
	Limadon->M_PlayAnimation(Limadon->SpitAnimation);

	return EBTNodeResult::Succeeded;
}