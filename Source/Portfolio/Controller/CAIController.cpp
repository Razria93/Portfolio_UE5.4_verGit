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

	if (!InitializeBlackBoard()) return;
	if (!InitializeBehaviorTree()) return;
}

void ACAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

bool ACAIController::InitializeBlackBoard()
{
	if (!BlackboardAsset) return false;

	UBlackboardComponent* blackboardComp = GetBlackboardComponent();
	UseBlackboard(BlackboardAsset, blackboardComp);
	return true;
}

bool ACAIController::InitializeBehaviorTree()
{
	if (!BehaviorTreeAsset) return false;

	RunBehaviorTree(BehaviorTreeAsset);
	return true;
}
