// Fill out your copyright notice in the Description page of Project Settings.


#include "AircraftBase.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "ImaginaryBlueprintData.h"
#include "JSBSimMovementComponent.h"
#include "SCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/OutputDeviceNull.h"

// Sets default values
AAircraftBase::AAircraftBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Activate=false;
#pragma region Component
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(this->RootComponent);

	VehicleBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleBox"));
	VehicleBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VehicleBoxComponent->BodyInstance.SetCollisionProfileName(TEXT("VehicleBox"));
	VehicleBoxComponent->SetGenerateOverlapEvents(true);
	//VehicleBoxComponent->BodyInstance.bUseCCD=true;
	VehicleBoxComponent->SetupAttachment(SkeletalMeshComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(SkeletalMeshComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	FPCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FPCameraComponent"));
	FPCameraComponent->SetupAttachment(SkeletalMeshComponent);
	
	JSBSimMovementComponent = CreateDefaultSubobject<UJSBSimMovementComponent>(TEXT("JSBSimMovementComponent"));

	VehicleBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AAircraftBase::OnBoxBeginOverlap);
	VehicleBoxComponent->OnComponentEndOverlap.AddDynamic(this, &AAircraftBase::OnBoxEndOverlap);
#pragma endregion


}

// Called when the game starts or when spawned
void AAircraftBase::BeginPlay()
{
	Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetBPClass);

	Widget->SetOwningPlayer(UGameplayStatics::GetPlayerController(GWorld, 0));
	
	Widget->SetVisibility(ESlateVisibility::Hidden);
	Widget->AddToViewport(0);
	Super::BeginPlay();
    JSBSimMovementComponent->Commands.ParkingBrake=1;
	for(int Engine=0; Engine<JSBSimMovementComponent->EngineCommands.Num();++Engine)
	{
		
			JSBSimMovementComponent->EngineCommands[Engine].Throttle=0.0f;
	}
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
	}
	
}

void AAircraftBase::ActivateAircraft()
{
	
	CameraComponent->SetActive(true);
	FPCameraComponent->SetActive(true);
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACTIVATE!!!!!!!!!!!"));
		EnableInput(PlayerController);  // 启用输入，确保控制权生效
			
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings(); 
			Subsystem->AddMappingContext(InputMapping,0);
		}
	}
	InitialExhaust();
}

void AAircraftBase::UnActivateAircraft()
{
	CameraComponent->SetActive(false);
	FPCameraComponent->SetActive(false);
	JSBSimMovementComponent->Commands.ParkingBrake=1;
	for(int Engine=0; Engine<JSBSimMovementComponent->EngineCommands.Num();++Engine)
	{
		
		JSBSimMovementComponent->EngineCommands[Engine].Throttle=0.0f;
	}
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		Widget->SetVisibility(ESlateVisibility::Hidden);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}
	TurnOffExhaust();
}


void AAircraftBase::GetOfAircraftTriggered_Implementation(const FInputActionValue& Value)
{
	AMultiFPSPlayerController* VehicleController = Cast<AMultiFPSPlayerController>(GetController());
	if (VehicleController!=nullptr)
	{
		VehicleController->DownVehicles();
	}
}

// Called every frame
void AAircraftBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAircraftBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//增加油门事件的绑定
		EnhancedInputComponent->BindAction(ThrottleUpAction,ETriggerEvent::Triggered,this,&AAircraftBase::ThrottleUp);
		//减少油门事件的绑定
		EnhancedInputComponent->BindAction(ThrottleDownAction,ETriggerEvent::Triggered,this,&AAircraftBase::ThrottleDown);
		//方向舵事件的绑定
		EnhancedInputComponent->BindAction(RudderAction,ETriggerEvent::Triggered,this,&AAircraftBase::RudderTriggered);
		//升降舵事件的绑定
		EnhancedInputComponent->BindAction(ElevatorAction,ETriggerEvent::Triggered,this,&AAircraftBase::ElevatorTriggered);
		//副翼事件的绑定
		EnhancedInputComponent->BindAction(AileronAction,ETriggerEvent::Triggered,this,&AAircraftBase::AileronTriggered);
		//襟翼事件的绑定
		EnhancedInputComponent->BindAction(FlapAction,ETriggerEvent::Triggered,this,&AAircraftBase::FlapTriggered);
		//平衡飞机事件的绑定
		EnhancedInputComponent->BindAction(CenterAileronsRudderAction,ETriggerEvent::Triggered,this,&AAircraftBase::CenterAileronsRudderTriggered);
		//刹车事件的绑定
		EnhancedInputComponent->BindAction(ParkingBrakeAction,ETriggerEvent::Triggered,this,&AAircraftBase::ParkingBrakeTriggered);
		//刹车事件的绑定
		EnhancedInputComponent->BindAction(GearAction,ETriggerEvent::Triggered,this,&AAircraftBase::GearTriggered);
		//空中刹车事件的绑定
		EnhancedInputComponent->BindAction(AirBrakeAction,ETriggerEvent::Triggered,this,&AAircraftBase::AirBrakeTriggered);
		//摄像机Pitch事件的绑定
		EnhancedInputComponent->BindAction(CameraUpDownAction,ETriggerEvent::Triggered,this,&AAircraftBase::CameraUpDownTriggered);
		//摄像机Yaw事件的绑定
		EnhancedInputComponent->BindAction(CameraLeftRightAction,ETriggerEvent::Triggered,this,&AAircraftBase::CameraLeftRightTriggered);
		//SpringArm事件的绑定
		EnhancedInputComponent->BindAction(CameraSpringArmAction,ETriggerEvent::Triggered,this,&AAircraftBase::CameraSpringArmTriggered);
		//SwitchCamera事件的绑定
		EnhancedInputComponent->BindAction(SwitchCameraAction,ETriggerEvent::Triggered,this,&AAircraftBase::SwitchCameraTriggered);
		//下飞机事件的绑定
		EnhancedInputComponent->BindAction(GetOfAircraftAction,ETriggerEvent::Triggered,this,&AAircraftBase::GetOfAircraftTriggered);
	}

	
}


void AAircraftBase::OnBoxEndOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASCharacter* Player = Cast<ASCharacter>(OtherActor);
	AMultiFPSPlayerController* VehicleController = Cast<AMultiFPSPlayerController>(Player->GetController());
	if (Player != nullptr && VehicleController != nullptr)
	{
		Player->IsNearVehicle=false;
		if (VehicleController->VehicleList.Contains(this) && !Activate)
		{
			VehicleController->VehicleList.Remove(this);
		}
	}
	if (Player->GetLocalRole() == ROLE_Authority)
	{
		Player->OnRep_NearVehicle();
	}
}

void AAircraftBase::OnBoxBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASCharacter* Player = Cast<ASCharacter>(OtherActor);
	AMultiFPSPlayerController* VehicleController = Cast<AMultiFPSPlayerController>(Player->GetController());
	if (Player != nullptr)
	{
		Player->IsNearVehicle=true;
		if (!VehicleController->VehicleList.Contains(this))
		{
			VehicleController->VehicleList.Add(this);
		}
		
	}
	if (Player->GetLocalRole() == ROLE_Authority)
	{
		Player->OnRep_NearVehicle();
	}
}

void AAircraftBase::ThrottleUp_Implementation(const FInputActionValue& Value)
{
	for(int Engine=0; Engine<JSBSimMovementComponent->EngineCommands.Num();++Engine)
	{
		if (JSBSimMovementComponent->EngineCommands[Engine].Throttle<1)
		{
			JSBSimMovementComponent->EngineCommands[Engine].Throttle+=0.1f;
		}
	}
	SetExhaustNozzles(JSBSimMovementComponent->EngineCommands[0].Throttle);
}
void AAircraftBase::ThrottleDown_Implementation(const FInputActionValue& Value)
{
	for(int Engine=0;Engine<JSBSimMovementComponent->EngineCommands.Num();++Engine)
	{
		if (this->JSBSimMovementComponent->EngineCommands[Engine].Throttle>0)
		{
			this->JSBSimMovementComponent->EngineCommands[Engine].Throttle-=0.1f;
		}
	}
	SetExhaustNozzles(JSBSimMovementComponent->EngineCommands[0].Throttle);
	/*FOutputDeviceNull device;
	this->CallFunctionByNameWithArguments(
	TEXT("SetExhaustNozzles"),
	device, 
	NULL, 
	true
	);*/
}

void AAircraftBase::RudderTriggered_Implementation(const FInputActionValue& Value)
{
	const float Rudder = Value.Get<float>();
	/*UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Rudder:%f"),JSBSimMovementComponent->Commands.Rudder));
	if ((Rudder<0&&JSBSimMovementComponent->Commands.Rudder<=-1)||((Rudder>0&&JSBSimMovementComponent->Commands.Rudder>=1)))
	{
		
	}
	else
	{
		JSBSimMovementComponent->Commands.Rudder += Rudder*0.1;
	}*/
	JSBSimMovementComponent->Commands.Rudder += Rudder*0.1;
	JSBSimMovementComponent->Commands.Rudder = FMath::Clamp(JSBSimMovementComponent->Commands.Rudder,-1,1);
	//JSBSimMovementComponent->Commands.Rudder = Rudder;
}

void AAircraftBase::ElevatorTriggered_Implementation(const FInputActionValue& Value)
{
	
	const float Elevator = Value.Get<float>();
	/*if ((Elevator<0&&JSBSimMovementComponent->Commands.Elevator<=-1)||((Elevator>0&&JSBSimMovementComponent->Commands.Elevator>=1)))
	{
		
	}
	else
	{*/
		JSBSimMovementComponent->Commands.Elevator += Elevator*0.1;
		JSBSimMovementComponent->Commands.Elevator = FMath::Clamp(JSBSimMovementComponent->Commands.Elevator,-1,1);
	//}
}

void AAircraftBase::AileronTriggered_Implementation(const FInputActionValue& Value)
{
	
	const float Aileron = Value.Get<float>();
	/*if ((Aileron<0&&JSBSimMovementComponent->Commands.Aileron<=-1)||((Aileron>0&&JSBSimMovementComponent->Commands.Aileron>=1)))
	{
		
	}
	else
	{
		JSBSimMovementComponent->Commands.Aileron += Aileron*0.1;
	}*/
	JSBSimMovementComponent->Commands.Aileron += Aileron*0.1;
	JSBSimMovementComponent->Commands.Aileron = FMath::Clamp(JSBSimMovementComponent->Commands.Aileron,-1,1);
	
}

void AAircraftBase::FlapTriggered_Implementation(const FInputActionValue& Value)
{
	const float Flap = Value.Get<float>();
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Flap:%f"),JSBSimMovementComponent->Commands.Flap));
	/*if ((Flap<0&&JSBSimMovementComponent->Commands.Flap<=-1)||((Flap>0&&JSBSimMovementComponent->Commands.Flap>=1)))
	{
		
	}
	else
	{
		JSBSimMovementComponent->Commands.Flap += Flap*0.1;
	}*/
	JSBSimMovementComponent->Commands.Flap += Flap*0.1;
	JSBSimMovementComponent->Commands.Flap = FMath::Clamp(JSBSimMovementComponent->Commands.Flap,-1,1);
}

void AAircraftBase::CenterAileronsRudderTriggered_Implementation(const FInputActionValue& Value)
{
	JSBSimMovementComponent->Commands.Rudder=0;
	JSBSimMovementComponent->Commands.Aileron=0;
}

void AAircraftBase::ParkingBrakeTriggered_Implementation(const FInputActionValue& Value)
{
	JSBSimMovementComponent->Commands.ParkingBrake = 1-JSBSimMovementComponent->Commands.ParkingBrake;
}

void AAircraftBase::GearTriggered_Implementation(const FInputActionValue& Value)
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Gear:%f"),JSBSimMovementComponent->Commands.GearDown));
	JSBSimMovementComponent->Commands.GearDown = 1 - JSBSimMovementComponent->Commands.GearDown;
}

void AAircraftBase::AirBrakeTriggered_Implementation(const FInputActionValue& Value)
{
	JSBSimMovementComponent->Commands.SpeedBrake = 1-JSBSimMovementComponent->Commands.SpeedBrake;
}

void AAircraftBase::CameraUpDownTriggered_Implementation(const FInputActionValue& Value)
{
	const float LookPitch = Value.Get<float>();
	AddControllerPitchInput(LookPitch);
}  

void AAircraftBase::CameraLeftRightTriggered_Implementation(const FInputActionValue& Value)
{
	const float LookYaw = Value.Get<float>();
	AddControllerYawInput(LookYaw);
}

void AAircraftBase::CameraSpringArmTriggered_Implementation(const FInputActionValue& Value)
{
	const float SpringArm = Value.Get<float>();
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("springarm:%f"),SpringArm));
	SpringArmComponent->TargetArmLength += SpringArm*50;
}

void AAircraftBase::SwitchCameraTriggered_Implementation(const FInputActionValue& Value)
{
	if (CameraComponent->IsActive())
	{
		CameraComponent->SetActive(false);
		FPCameraComponent->SetActive(true);
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
        if (PlayerController)
        {
          Widget->SetVisibility(ESlateVisibility::Visible);
          PlayerController->SetInputMode(FInputModeGameAndUI());
          PlayerController->bShowMouseCursor = true;
        }
		
	}
	else
	{
		CameraComponent->SetActive(true);
		FPCameraComponent->SetActive(false);
		
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController)
        {
          Widget->SetVisibility(ESlateVisibility::Hidden);
          PlayerController->SetInputMode(FInputModeGameOnly());
          PlayerController->bShowMouseCursor = false;
        }
	}
}