// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include <BehaviorTree/BehaviorTreeComponent.h>
#include <BehaviorTree/BlackboardComponent.h>
#include "BehaviorTree\Blackboard\BlackboardKeyAllTypes.h"
#include "RobotController.generated.h"


/**
 * 
 */
UCLASS()
class COOPGAME_427_API ARobotController : public AAIController
{
	GENERATED_BODY()
public:
	/// 设置基本属性的构造函数
	ARobotController(const class FObjectInitializer& ObjectInitializer);


	virtual void OnPossess(class APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UBehaviorTreeComponent* BehaviorTreeComponent;

	UBlackboardComponent* BlackboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|PawSensing")
	class UAIPerceptionComponent* AiPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|PawSensing")
	class UAISenseConfig_Sight* AiConfigSight;

private:
	AActor* PreDestinationActor;

	AActor* PreTurnToActor;


};
