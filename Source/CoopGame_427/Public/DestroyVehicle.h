// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestroyVehicle.generated.h"

UCLASS()
class COOPGAME_427_API ADestroyVehicle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADestroyVehicle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Skeletal mesh for the Destroyvehicle
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DestroyVehicleMesh;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
