// Fill out your copyright notice in the Description page of Project Settings.


#include "DestroyVehicle.h"

// Sets default values
ADestroyVehicle::ADestroyVehicle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DestroyVehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestroyVehicleMesh"));
	DestroyVehicleMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	DestroyVehicleMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	DestroyVehicleMesh->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void ADestroyVehicle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADestroyVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

