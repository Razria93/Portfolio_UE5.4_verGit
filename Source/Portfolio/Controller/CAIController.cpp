#include "Controller/CAIController.h"
#include "ProjectGlobal.h"

#include "Perception/AIPerceptionComponent.h"

ACAIController::ACAIController()
{
	// Init AIPerceptionComp
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	check(AIPerceptionComp);
}

void ACAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ACAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!IsValid(InPawn)) return;

	ControlledPawn_Cached = InPawn;
	
	InitializeBlackboardAndTree();
}

void ACAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ACAIController::InitializeBlackboardAndTree()
{
	if (BlackboardAsset)
	{
		UBlackboardComponent* blackboardComp = GetBlackboardComponent();
		UseBlackboard(BlackboardAsset, blackboardComp);
	}

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}
