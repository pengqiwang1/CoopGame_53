// Fill out your copyright notice in the Description page of Project Settings.


#include "VehicleBase.h"
#include "EnhancedInputComponent.h"
#include "AircraftBase.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/MapErrors.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AVehicleBase::AVehicleBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	//bReplicateMovement = true;
	/*CollisionBoxComponent=CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComponent"));
	RootComponent = CollisionBoxComponent;
	
	VehicleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VehicleMesh"));
	VehicleMesh->SetupAttachment(RootComponent);*/
	VehicleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VehicleMesh"));
	if (VehicleMesh)
	{
		VehicleMesh->SetGenerateOverlapEvents(true);
	}
	RootComponent = VehicleMesh;
	/*CollisionBoxComponent=CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComponent"));
	CollisionBoxComponent->SetupAttachment(RootComponent);*/

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(VehicleMesh);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	
	// Create the vehicle movement component
	VehicleMovement = CreateDefaultSubobject<UChaosWheeledVehicleMovementComponent>(TEXT("VehicleMovement"));
	//VehicleMovement = CreateDefaultSubobject<UChaosWheeledVehicleMovementComponent>(TEXT("VehicleMovement"));

	if (VehicleMovement)
	{
		//VehicleMovement->GetSuspensionOffset(35000.f);

		VehicleMovement->SetIsReplicated(true); // Enable replication if needed
	}
}

// Called when the game starts or when spawned
void AVehicleBase::BeginPlay()
{
	CreateUI();
	Super::BeginPlay();
	
	
	
	OnTakeRadialDamage.AddDynamic(this, &AVehicleBase::RadialDamage);
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 1);
		}
	}
	
}

// Called every frame
void AVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//油门事件的绑定
		EnhancedInputComponent->BindAction(SetThrottleAction,ETriggerEvent::Triggered,this,&AVehicleBase::SetThrottle);
		//摄像机Pitch事件的绑定
		EnhancedInputComponent->BindAction(CameraUpDownAction,ETriggerEvent::Triggered,this,&AVehicleBase::CameraUpDownTriggered);
		//摄像机Yaw事件的绑定
		EnhancedInputComponent->BindAction(CameraLeftRightAction,ETriggerEvent::Triggered,this,&AVehicleBase::CameraLeftRightTriggered);
		//SpringArm事件的绑定
		EnhancedInputComponent->BindAction(CameraSpringArmAction,ETriggerEvent::Triggered,this,&AVehicleBase::CameraSpringArmTriggered);
		//转向事件的绑定
		switch (VehicleType)
		{
		case EVehucleType::WheeledVehicle:
			EnhancedInputComponent->BindAction(SetStearAction,ETriggerEvent::Triggered,this,&AVehicleBase::SetStear);
			break;
		default:
			break;
		}
		
		
	}
}

void AVehicleBase::CreateUI_Implementation()
{
	if (IsLocallyControlled())  // 只在服务器上执行
	{
		if (WidgetBPClass)
		{
			Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetBPClass);
			if (Widget)
			{
				Widget->AddToViewport();
			}
		}
	}
}

void AVehicleBase::CreateClientUI_Implementation()
{
	if (WidgetBPClass)
	{
		Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetBPClass);
		if (Widget)
		{
			Widget->AddToViewport();
		}
	}
}

void AVehicleBase::ClientUpdateHealthUI_Implementation()
{
	UpdateHealthUI();
}

void AVehicleBase::DestroyVehicle_Implementation()
{
	//添加生成损毁车辆的逻辑
	FTransform SpawnTransform = this->GetActorTransform();
	ClientUpdateHealthUI();
	if (Widget)
	{
		Widget->RemoveFromParent();  // 从视口移除
		Widget = nullptr;
	}
	this->Destroy();
	
	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = nullptr;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (World)
	{
		World->SpawnActor<ADestroyVehicle>(DestroyVehicleBPClass,
		SpawnTransform,
		SpawnInfo);
	}

}

void AVehicleBase::RadialDamage_Implementation(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                               FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	Health = FMath::Min(Health - Damage, Health);
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("RadialDamage:%f"), Health));
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_CurrentLife();
	}
	/*if (Health<=0)
	{
		ServerDeath();
	}*/
}

void AVehicleBase::OnRep_CurrentLife()
{
	ClientUpdateHealthUI();
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnRep_CurrentLife!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!,%f"),Health));
	/*if (FPSPlayerController)
	{
		ASCharacter* PawnHealth = Cast<ASCharacter>(FPSPlayerController->GetPawn());
		if (PawnHealth)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("health:%d"), PawnHealth->Health));
			if (PawnHealth->Health <= 0)
			{
				ServerDeath();
			}
		}
	}
	else
	{
		if(Health<=0)
		{
			this->Destroy();
		}
	}*/
	if(Health<=0)
	{
		DestroyVehicle();
	}
	
}

void AVehicleBase::SetThrottle(const FInputActionValue& Value)
{
	if(VehicleMovement)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Throttle!!!!!!!!!!!!!!!!!!!!!")));
		const float Throttle = Value.Get<float>();
		// ThrottleValue = FMath::Clamp(ThrottleValue + Throttle*ThrottleIncrement ,  -1*MaxThrottle, MaxThrottle);//* GetWorld()->DeltaTimeSeconds
		// VehicleMovement->SetThrottleInput(ThrottleValue);
		VehicleMovement->SetThrottleInput(Throttle);
		if (Throttle > 0)
           {
              VehicleMovement->SetBrakeInput(0.f);
           }
           else
           {
              VehicleMovement->SetBrakeInput(-Throttle);
           }
	}
}

void AVehicleBase::SetStear_Implementation(const FInputActionValue& Value)
{
	const float Stear = Value.Get<float>();
	if(VehicleMovement)
	{
		switch (VehicleType)
		{
		case(EVehucleType::WheeledVehicle):
			StearValue = FMath::Clamp(StearValue + Stear * StearIncrement, -MaxStear, MaxStear);
			VehicleMovement->SetSteeringInput(StearValue);
			break;
		case(EVehucleType::CrawlerVehicle):
			break;
		default:
			StearValue = FMath::Clamp(StearValue + Stear * StearIncrement, -MaxStear, MaxStear);
			VehicleMovement->SetSteeringInput(StearValue);
			break;
		}
	}
}


void AVehicleBase::SetBreak(const FInputActionValue& Value)
{
}

void AVehicleBase::SetHandBreak(const FInputActionValue& Value)
{
}

void AVehicleBase::SetReverseGear(const FInputActionValue& Value)
{
}

void AVehicleBase::SetGear(const FInputActionValue& Value)
{
}

void AVehicleBase::CameraUpDownTriggered(const FInputActionValue& Value)
{
	const float LookPitch = Value.Get<float>();
	/*FRotator CameraRot = CameraComponent->GetRelativeRotation();
	FRotator CameraNewRot = FRotator(CameraRot.Pitch+LookPitch, CameraRot.Yaw, CameraRot.Roll);
	CameraComponent->SetRelativeRotation(CameraNewRot);*/
	AddControllerPitchInput(LookPitch);
}

void AVehicleBase::CameraLeftRightTriggered(const FInputActionValue& Value)
{
	const float LookYaw = Value.Get<float>();
	/*FRotator CameraRot = CameraComponent->GetRelativeRotation();
	FRotator CameraNewRot = FRotator(CameraRot.Pitch, CameraRot.Yaw+LookYaw, CameraRot.Roll);
	CameraComponent->SetRelativeRotation(CameraNewRot);*/
	AddControllerYawInput(LookYaw);
}

void AVehicleBase::CameraSpringArmTriggered(const FInputActionValue& Value)
{
	const float SpringArm = Value.Get<float>();
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("springarm:%f"),SpringArm));
	SpringArmComponent->TargetArmLength += SpringArm*50;
}

void AVehicleBase:: GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); 

	DOREPLIFETIME(AVehicleBase, Health);
	
}
