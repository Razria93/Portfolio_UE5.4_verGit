#include "AI/BehaviorTree/Task/CBTTask_SetMaxWalkSpeed.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCBTTask_SetMaxWalkSpeed::UCBTTask_SetMaxWalkSpeed()
{
	NodeName = TEXT("Set MaxWalkSpeed");
}

EBTNodeResult::Type UCBTTask_SetMaxWalkSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACharacter* character = Cast<ACharacter>(aiController->GetPawn());
	if (!IsValid(character)) return EBTNodeResult::Failed;

	UCharacterMovementComponent* movementComp = character->GetCharacterMovement();
	if (!IsValid(movementComp)) return EBTNodeResult::Failed;

	if (FMath::IsNearlyEqual(movementComp->MaxWalkSpeed, SetMaxWalkSpeed)) return EBTNodeResult::Succeeded;

	FLog::Log(FString::Printf(TEXT("%-20s | %s: %.3f | %s: %.3f"), TEXT("MaxWalkSpeed"), TEXT("Before"), movementComp->MaxWalkSpeed, TEXT("After"), SetMaxWalkSpeed));

	movementComp->MaxWalkSpeed = SetMaxWalkSpeed;
	return EBTNodeResult::Succeeded;
}
