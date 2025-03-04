// Fill out your copyright notice in the Description page of Project Settings.

#include "RobotController.h"
#include <BehaviorTree/BehaviorTreeComponent.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <Perception/AIPerceptionComponent.h>
#include <Perception/AISenseConfig_Sight.h>
#include "Kismet/KismetSystemLibrary.h"
#include "CoreGlobals.h"
#include "AttackRobot.h"


ARobotController::ARobotController(const class FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer), PreDestinationActor(nullptr), PreTurnToActor(nullptr)
{
    BehaviorTreeComponent = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorTree"));
    BlackboardComponent = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("Blackboard"));
    AiPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AiPerception"));
    AiConfigSight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AiConfigSight"));

    if (AiPerception)
    {
        AiConfigSight->SightRadius = 1000.0f;
        AiConfigSight->LoseSightRadius = 2000.0f;
        AiConfigSight->PeripheralVisionAngleDegrees = 90.0f;
        AiConfigSight->DetectionByAffiliation.bDetectNeutrals = true;
        AiConfigSight->DetectionByAffiliation.bDetectEnemies = true;
        AiConfigSight->DetectionByAffiliation.bDetectFriendlies = true;

        AiPerception->ConfigureSense(*AiConfigSight);
        AiPerception->SetDominantSense(UAISenseConfig_Sight::StaticClass());

    }
}


void ARobotController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (InPawn)
    {
        AAttackRobot* AttackRobot = Cast<AAttackRobot>(InPawn);

        if (AttackRobot)
        {
            if (AttackRobot && AttackRobot->BehaviorTree)
            {
                if (AttackRobot->BehaviorTree)
                {
                    //живЊ
                    BlackboardComponent->InitializeBlackboard(*AttackRobot->BehaviorTree->BlackboardAsset);
                    BehaviorTreeComponent->StartTree(*AttackRobot->BehaviorTree);
                    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("InitializeBlackboard.")));
                }
                else
                {
                    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("No blackboard is assigned to the guide's behavior tree.")));
                    //UE_LOG(GuideLog, Error, TEXT(LOG_HEADER"No blackboard is assigned to the guide's behavior tree."));
                }
            }
            else
            {
                UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("No behavior tree is assigned to guide.")));
                // UE_LOG(GuideLog, Error, TEXT(LOG_HEADER"No behavior tree is assigned to guide."));
            }
        }
        else
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("The pawn possessed is not an instance of AGuide.")));
            //UE_LOG(GuideLog, Error, TEXT(LOG_HEADER"The pawn possessed is not an instance of AGuide."));
        }
    }
}

void ARobotController::OnUnPossess()
{
    Super::OnUnPossess();
    BehaviorTreeComponent->StopTree();
}
