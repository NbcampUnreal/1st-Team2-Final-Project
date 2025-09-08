#include "Monster/EyeStalker/EyeStalkerAIController.h"

#include "Monster/Effect/PostProcessSettingComponent.h"
#include "Container/BlackboardKeys.h"

#include "Character/UnderwaterCharacter.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"

void AEyeStalkerAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AEyeStalkerAIController::InitTargetPlayer()
{
	AUnderwaterCharacter* Player = Cast<AUnderwaterCharacter>(GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::TargetPlayerKey));
	if (!IsValid(Player)) return;

	UPostProcessSettingComponent* PostProcessSettingComponent = Player->GetPostProcessSettingComponent();
	if (!IsValid(PostProcessSettingComponent)) return;

	Player->SetIsAttackedByEyeStalker(false);
	PostProcessSettingComponent->C_DeactivateVignetteEffect();
	
	GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::TargetPlayerKey, nullptr);
	GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::EyeStalker::bIsAttackingKey, false);
	GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::EyeStalker::bHasDetectedKey, false);
}