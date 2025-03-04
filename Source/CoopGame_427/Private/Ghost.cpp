// Fill out your copyright notice in the Description page of Project Settings.
#include "Ghost.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Camera/CameraComponent.h"



// Sets default values
AGhost::AGhost()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

#pragma region Component
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	if (CameraComp && SpringArmComp) {
		CameraComp->SetupAttachment(SpringArmComp);
	}

#pragma endregion

}

// Called when the game starts or when spawned
void AGhost::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGhost::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#pragma region InputEvent
void AGhost::MoveForward(float value)
{
	FVector Location = this->GetActorLocation();
	FVector NewLocation = Location + value * MoveSpeed * this->GetActorForwardVector();
	this->SetActorLocation(NewLocation);
}

void AGhost::MoveRight(float value)
{
	FVector Location = this->GetActorLocation();
	FVector NewLocation = Location + value * MoveSpeed * this->GetActorRightVector();
	this->SetActorLocation(NewLocation);
}

void AGhost::MoveUp(float value)
{
	FVector Location = this->GetActorLocation();
	FVector NewLocation = Location + value * MoveSpeed * this->GetActorUpVector();
	this->SetActorLocation(NewLocation);
}

#pragma endregion

// Called to bind functionality to input
void AGhost::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AGhost::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGhost::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &AGhost::MoveUp);

	PlayerInputComponent->BindAxis("LookUp", this, &AGhost::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &AGhost::AddControllerYawInput);
}

