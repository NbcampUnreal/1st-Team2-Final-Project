#include "Monster/EyeStalker/EyeStalkerAIController.h"

#include "Monster/Effect/PostProcessSettingComponent.h"
#include "Container/BlackboardKeys.h"

#include "Character/UnderwaterCharacter.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Framework/ADPlayerController.h"

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
	Player->OnUntargeted(GetPawn());

	PostProcessSettingComponent->C_DeactivateVignetteEffect();
	if (AADPlayerController* PC = Player->GetController<AADPlayerController>())
	{
		PC->C_SetRadialBlurEffect(false);
	}
	
	GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::TargetPlayerKey, nullptr);
	GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::EyeStalker::bIsAttackingKey, false);
	GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::EyeStalker::bHasDetectedKey, false);
}