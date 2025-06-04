// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "AttackRobot.h"
#include "Bullet.h"
#include "MissileBaseServer.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "ctime"
#include "UObject/UObjectGlobals.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Delegates/Delegate.h"
#include "Components/CapsuleComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include <string>

#include "AircraftBase.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
using namespace std;

//#include "Components/DecalComponent.h"

// Sets default valuesD:\Epic\Epic Games\UE_4.26\Engine\Source\Runtime\Engine\Classes\GameFramework\Character.h
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

#pragma region Component

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	if (SpringArmComp) {
		SpringArmComp->bUsePawnControlRotation = true;
		SpringArmComp->SetupAttachment(RootComponent);
	}

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	if (CameraComp) {
		CameraComp->SetupAttachment(SpringArmComp);
	}

	FPArmMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPArmMesh"));
	if (FPArmMesh) {
		FPArmMesh->SetupAttachment(CameraComp);
		FPArmMesh->SetOnlyOwnerSee(true);
	}

	CapsuleArmComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleArmComponent"));
	CapsuleArmComponent->SetupAttachment(RootComponent);
	CapsuleArmComponent->SetRelativeRotation(FRotator(90, 0, -90));
	CapsuleArmComponent->SetRelativeLocation(FVector(68, 4, 50));

	CapsuleArmComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	CapsuleArmComponent->SetCollisionObjectType(ECC_Pawn);
	//CapsuleArmComponent->SetNotifyRigidBodyCollision(true);
	//CapsuleArmComponent->BodyInstance.bUseCCD=true;

	Mesh->SetOwnerNoSee(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	Mesh->SetSimulatePhysics(false);
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &ASCharacter::PickEquipment);
	Mesh->SetNotifyRigidBodyCollision(true);
	FPArmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FPArmMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	FPArmMesh->bCastDynamicShadow = false;
	FPArmMesh->CastShadow = false;

	AIPerceptionStimuliSourceComp = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("AIPerceptionStimuliSourceComp"));
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &ASCharacter::OnMeshBeginOverlapDamage);
	ActivateWeapon = EWeaponType::None;
#pragma endregion

}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	bReplicates = true;

	Health = 100;

	IsQuietStep = false;

	IsAiming = false;

	ActivateVehicleUI = false;

	

	UpdateCameraRotationonDelegte.BindUFunction(this, TEXT("UpdateCameraRotation"));

	ReloadMontageEndedDelegate.BindUFunction(this, TEXT("SetReload"));

	//SwitchPrimaryWeaponAnimMontageEndedDelegate.BindUFunction(this, TEXT("SwitchPrimaryWeapon"));

	DeathMontageEndedDelegate.BindUFunction(this, TEXT("DestoryCharacter"));

	OnTakePointDamage.AddDynamic(this, &ASCharacter::OnHit);

	OnTakeRadialDamage.AddDynamic(this, &ASCharacter::RadialDamage);

	ClientArmsAnimation = FPArmMesh->GetAnimInstance();
	ServerBodysAnimation = Mesh->GetAnimInstance();
	FPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());

	if (FPSPlayerController)
	{
		FPSPlayerController->CreatePlayerUI(PlayerWidgetBPClass);
		ClientUpdateHealthUI();
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(FPSPlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
	}
	//StartWithKindOfWeapon();
	

}

#pragma region InputEvent
void ASCharacter::MoveForward(float value)
{
	if (FPSPlayerController && value != 0.0f)
	{
		FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, value);
		//AddMovementInput(GetActorForwardVector(), value);
	}
	
}

void ASCharacter::MoveForwardEvent_Implementation(const FInputActionValue& Value)
{
	if (FPSPlayerController )
	{
		FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Value.Get<float>());
	}
}

void ASCharacter::CarryingVehiclesEvent_Implementation(const FInputActionValue& Value)
{
	
	if (FPSPlayerController!=nullptr)
	{
		ActivateVehicleUI = !ActivateVehicleUI;
		if(ActivateVehicleUI)
		{
			/*FInputModeGameAndUI Mode;
			Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			FPSPlayerController->SetInputMode(Mode);*/
			FPSPlayerController->SetInputMode(FInputModeGameAndUI());
			FPSPlayerController->bShowMouseCursor = true;
		}
		else
		{
			FPSPlayerController->SetInputMode(FInputModeGameOnly());
			FPSPlayerController->bShowMouseCursor = false;
		}
		UpdateClientVehicleUI(ActivateVehicleUI);
		//FPSPlayerController->UpVehicles();
	}
	
}

void ASCharacter::MoveRight(float value)
{
	if (FPSPlayerController && value != 0.0f)
	{
		FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, value);
		//AddMovementInput(GetActorRightVector(), value);
	}
}

void ASCharacter::JumpAction()
{
	Jump();
}

void ASCharacter::StopJumpingAction()
{
	StopJumping();
}

void ASCharacter::LowSpeedWalk_Implementation()
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ClientLowSpeedWalk")));
	CharacterMovement->MaxWalkSpeed = 300;
	//bIsCrouched = true;
	//ServerLowSpeedWalk();
	Crouch();
}

bool ASCharacter::LowSpeedWalk_Validate()
{
	return true;
}

void ASCharacter::NormalSpeedWalk_Implementation()
{
	CharacterMovement->MaxWalkSpeed = 600;
	//ServerSetNormalSpeedWalk();
	//bIsCrouched = false;
	UnCrouch();
}

bool ASCharacter::NormalSpeedWalk_Validate()
{
	return true;
}

void ASCharacter::InputFirePressed()
{
	switch (ActivateWeapon)
	{
	case EWeaponType::AK47:
		FireWeaponPrimary();
		break;
	case EWeaponType::M4A1:
		FireWeaponPrimary();
		break;
	case EWeaponType::Sniper:
		FireWeaponSniper();
		break;
	case EWeaponType::DersertEagle:
		FireWeaponSecondary();
		break;
	default:
		break;
	}
	
}

void ASCharacter::InputFireReleased()
{
	switch (ActivateWeapon)
	{
	case EWeaponType::AK47:
		StopWeaponPrimary();
		break;
	case EWeaponType::M4A1:
		StopWeaponPrimary();
		break;
	case EWeaponType::Sniper:
		StopWeaponSniper();
		break;
	case EWeaponType::DersertEagle:
		break;
	default:
		break;
	}
	
}

void ASCharacter::InputAimingPressed()
{
	ServerSetAiming(true);
	ClientAiming();
}

void ASCharacter::InputAimingReleased()
{
	ServerSetAiming(false);
	ClientEndAiming();
	//this->GetInputAxisValue(TEXT("Aim"));
}

void ASCharacter::SwitchPrimaryWeapon_Implementation()
{
	
	AWeaponServer* ServerWeapon = GetCurrentServerWeapon();
	if (ServerWeapon != nullptr)
	{
		LastWeapon = ServerWeapon;
		switch (ServerWeapon->GradeofWeapon)
		{
		case EWeaponGrade::Secondary:
			{
			if (ServerPrimaryWeapon != nullptr || ReservePrimaryWeapon != nullptr)
			{
				ServerSecondaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				ServerSecondaryWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon2"), EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true);
			}
			break;
			}
		case EWeaponGrade::Melee:
		{
			break;
		}
		case EWeaponGrade::ThrowWeapon:
		{
			break;
		}
		default:
		{

		}
		}
	}

	
	if (ServerPrimaryWeapon != nullptr && ServerPrimaryWeapon->GetAttachParentSocketName() == TEXT("EquipWeapon"))
	{

		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CuttingGun %s"),*ServerPrimaryWeapon->GetName()));
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("CuttingGun")));
		ServerPrimaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		DestoryClientWeapon();

		//delete ServerWeapon;
		EquipPrimary(ServerPrimaryWeapon);
		ClientEquipFPArmsPrimary();
		if (ReservePrimaryWeapon != nullptr)
		{
			ReservePrimaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			ReservePrimaryWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon"), EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
			
		}


	}
	else if (ReservePrimaryWeapon != nullptr && ReservePrimaryWeapon->GetAttachParentSocketName() == TEXT("EquipWeapon"))
	{
		ReservePrimaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ServerPrimaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ServerWeapon = ReservePrimaryWeapon;
		ReservePrimaryWeapon = ServerPrimaryWeapon;
		ServerPrimaryWeapon = ServerWeapon;
		//delete ServerWeapon;
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerWeapon!!!!!!!!!!!!!!!!!! %s"), *ServerWeapon->GetName()));
		DestoryClientWeapon();
		EquipPrimary(ServerPrimaryWeapon);
		//ClientEquipFPArmsPrimary();
		ReservePrimaryWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon"), EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true);
	}
		
	
}

bool ASCharacter::SwitchPrimaryWeapon_Validate()
{
	return true;
}

void ASCharacter::SwitchSecondaryWeapon_Implementation()
{
	AWeaponServer* ServerWeapon = GetCurrentServerWeapon();
	if (ServerWeapon == nullptr && ServerSecondaryWeapon != nullptr)
	{
		ServerSecondaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		DestoryClientWeapon();
		EquipSecondary(ServerSecondaryWeapon);
		ClientEquipFPArmsSecondary();
	}
	if (ServerWeapon && ServerSecondaryWeapon != nullptr)
	{
		//FName AttachSocket = ServerSecondaryWeapon->GetAttachParentSocketName();
		if (ServerWeapon->GradeofWeapon == EWeaponGrade::Primary)
		{
			ServerSecondaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			ServerWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			DestoryClientWeapon();
			
			if (ReservePrimaryWeapon != nullptr)
			{
				ReservePrimaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				ServerWeapon = ReservePrimaryWeapon;
				ReservePrimaryWeapon = ServerPrimaryWeapon;
				ServerPrimaryWeapon = ServerWeapon;
				ServerPrimaryWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon"), EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true);
				ReservePrimaryWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon1"), EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true);
			}
			else
			{
				ServerPrimaryWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon"), EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true);
			}
			EquipSecondary(ServerSecondaryWeapon);
			ClientEquipFPArmsSecondary();
			LastWeapon = ServerPrimaryWeapon;
		}
		
	}


}

bool ASCharacter::SwitchSecondaryWeapon_Validate()
{
	return true;
}

void ASCharacter::SwitchWeapon_Implementation()
{
	if (LastWeapon != nullptr)
	{
		switch (LastWeapon->GradeofWeapon)
		{
		case EWeaponGrade::Primary:
		{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SwitchPrimaryWeapon!!!!!")));
				PlayServerSwitchPrimaryWeaponAnimMontage();
			break;
		}
		case EWeaponGrade::Secondary:
		{
			PlayServerSwitchSecondaryWeaponAnimMontage();
			break;
		}
		default:
			PlayServerSwitchPrimaryWeaponAnimMontage();
			break;
		}
	}
}

bool ASCharacter::SwitchWeapon_Validate()
{
	return true;
}

void ASCharacter::DestoryClientWeapon_Implementation()
{
	if (ClientPrimaryWeapon != nullptr)
	{
		ClientPrimaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ClientPrimaryWeapon->Destroy();
		ClientPrimaryWeapon = nullptr;
	}
	if (ClientSecondaryWeapon != nullptr)
	{
		ClientSecondaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ClientSecondaryWeapon->Destroy();
		ClientSecondaryWeapon = nullptr;
	}
	
}

bool ASCharacter::DestoryClientWeapon_Validate()
{
	return true;
}

void ASCharacter::PlayServerSwitchPrimaryWeaponAnimMontage_Implementation()
{
	if (ServerBodysAnimation)
	{
		ServerBodysAnimation->Montage_Play(ServerBodysSwitchPrimaryWeaponAnimMontage);
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("GetPlayLength: %f"), ServerBodysSwitchPrimaryWeaponAnimMontage->GetPlayLength()));
		//ServerBodysAnimation->Montage_SetEndDelegate(SwitchWeaponAnimMontageEndedDelegate);
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		ActionInfo.ExecutionFunction = TEXT("SwitchPrimaryWeapon");
		ActionInfo.UUID = FMath::Rand();
		ActionInfo.Linkage = 0;
		UKismetSystemLibrary::Delay(this, ServerBodysSwitchPrimaryWeaponAnimMontage->GetPlayLength()/2, ActionInfo);
		PlayClientSwitchPrimaryWeaponAnimMontage();
	}
}

void ASCharacter::PlayServerSwitchSecondaryWeaponAnimMontage_Implementation()
{
	if (ServerBodysAnimation)
	{
		ServerBodysAnimation->Montage_Play(ServerBodysSwitchSecondaryWeaponAnimMontage);
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("GetPlayLength: %f"), ServerBodysSwitchPrimaryWeaponAnimMontage->GetPlayLength()));
		//ServerBodysAnimation->Montage_SetEndDelegate(SwitchWeaponAnimMontageEndedDelegate);
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		ActionInfo.ExecutionFunction = TEXT("SwitchSecondaryWeapon");
		ActionInfo.UUID = FMath::Rand();
		ActionInfo.Linkage = 0;
		UKismetSystemLibrary::Delay(this, ServerBodysSwitchSecondaryWeaponAnimMontage->GetPlayLength() / 2, ActionInfo);
		PlayClientSwitchSecondaryWeaponAnimMontage();
	}
}

bool ASCharacter::PlayServerSwitchSecondaryWeaponAnimMontage_Validate()
{
	return true;
}

void ASCharacter::PlayClientSwitchPrimaryWeaponAnimMontage_Implementation()
{
	if (ServerBodysAnimation)
	{
		ServerBodysAnimation->Montage_Play(ServerBodysSwitchPrimaryWeaponAnimMontage);
	}
}

bool ASCharacter::PlayClientSwitchPrimaryWeaponAnimMontage_Validate()
{
	return true;
}

void ASCharacter::PlayClientSwitchSecondaryWeaponAnimMontage_Implementation()
{
	if (ServerBodysAnimation)
	{
		ServerBodysAnimation->Montage_Play(ServerBodysSwitchSecondaryWeaponAnimMontage);
	}
}

bool ASCharacter::PlayClientSwitchSecondaryWeaponAnimMontage_Validate()
{
	return true;
}


#pragma endregion


// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*if (GetLocalRole() == ENetRole::ROLE_AutonomousProxy && ServerPrimaryWeapon && !ClientPrimaryWeapon)
	{
		ClientEquipFPArmsPrimary();
	}*/
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("testfloat: %f"), testfloat));
	if (VerticalRecoilRecoveryTimeLine.IsPlaying())
	{
		VerticalRecoilRecoveryTimeLine.TickTimeline(DeltaTime);
	}
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::JumpAction);
	PlayerInputComponent->BindAction("StopJumping", IE_Released, this, &ASCharacter::StopJumpingAction);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::InputFirePressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::InputFireReleased);

	PlayerInputComponent->BindAction("Aiming", IE_Pressed, this, &ASCharacter::InputAimingPressed);
	PlayerInputComponent->BindAction("Aiming", IE_Released, this, &ASCharacter::InputAimingReleased);

	PlayerInputComponent->BindAction("SwitchPrimaryWeapon", IE_Released, this, &ASCharacter::PlayServerSwitchPrimaryWeaponAnimMontage);
	PlayerInputComponent->BindAction("SwitchSecondaryWeapon", IE_Released, this, &ASCharacter::PlayServerSwitchSecondaryWeaponAnimMontage);
	PlayerInputComponent->BindAction("SwitchWeapon", IE_Released, this, &ASCharacter::SwitchWeapon);
	//PlayerInputComponent->BindAction("LowSpeedWalk", IE_Pressed, this, &ASCharacter::LowSpeedWalk);
	//PlayerInputComponent->BindAction("NormalSpeedWalk", IE_Released, this, &ASCharacter::NormalSpeedWalk);
	PlayerInputComponent->BindAction("LowSpeedWalk", IE_Pressed, this, &ASCharacter::ServerLowSpeedWalk);
	PlayerInputComponent->BindAction("NormalSpeedWalk", IE_Released, this, &ASCharacter::ServerNormalSpeedWalk);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::InputReload);
	
	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//前进事件的绑定
		EnhancedInputComponent->BindAction(MoveForwardAction,ETriggerEvent::Triggered,this,&ASCharacter::MoveForwardEvent);
		//搭乘载具
		EnhancedInputComponent->BindAction(CarryingVehicles,ETriggerEvent::Triggered,this,&ASCharacter::CarryingVehiclesEvent);
	}


}


/*
void ASCharacter::EquipPrimary(AWeaponServer* Weaponserver)
{
}*/
void Delay(int time)
{
	clock_t  now = clock();
	while (clock() - now < time);
}

void ASCharacter::EquipPrimary_Implementation(AWeaponServer* Weaponserver)
{
	//testfloat += 1;
	FName WeaponSocketName = TEXT("WeaponSocket");
	
	ActivateWeapon = Weaponserver->KindofWeapon;
	FName Attach_socket;
	//FName WeaponSocketName = TEXT("WeaponSocket");
	switch (Weaponserver->KindofWeapon)
	{
	case EWeaponType::AK47:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get AK47!"));
		Attach_socket = TEXT("Weapon_Rffle");
		WeaponSocketName = TEXT("WeaponSocket");
		break;
	}
	case EWeaponType::M4A1:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get M4A1!"));
		Attach_socket = TEXT("Weapon_M4A1");
		WeaponSocketName = TEXT("M4A1_Socket");
		break;
	}
	case EWeaponType::Sniper:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get Sniper"));
		Attach_socket = TEXT("Weapon_Sniper");
		WeaponSocketName = TEXT("AWP_Socket");
		break;
	}
	default:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get AK47!"));
		Attach_socket = TEXT("Weapon_Rffle"); 
		WeaponSocketName = TEXT("WeaponSocket");
		break;
	}
	}
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get ServerPrimaryWeapon!"));
	ServerPrimaryWeapon = Weaponserver;
	ServerPrimaryWeapon->SetOwner(this);
	ServerPrimaryWeapon->K2_AttachToComponent(Mesh, Attach_socket, EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		true);
	if (HasAuthority())
	{
		if (ServerPrimaryWeapon)
		{
			if (ClientPrimaryWeapon)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("SERVERCLIENTPrimaryWeapon is exists%%%%%%%%%%%%%%%"));
				ClientPrimaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				ClientPrimaryWeapon->Destroy();
				ClientPrimaryWeapon = nullptr;
			}
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Owner = this;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ClientPrimaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerPrimaryWeapon->ClientWeaponBaseBPClass,
				GetActorTransform(),
				SpawnInfo);
			//不同武器插槽

			/*if (ActivateWeapon == EWeaponType::M4A1)
			{
				WeaponSocketName = TEXT("M4A1_Socket");

			}*/
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex));
			if (ClientPrimaryWeapon)
			{
				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex), true, true, FLinearColor(0.0, 0.66, 1.0, 1.0), 10.0);
				if (ClientArmsAnimation)
				{
					//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex), true, true, FLinearColor(0.0, 0.66, 1.0, 1.0), 10.0);
					UpdateFPArmsBlendPose(ClientPrimaryWeapon->FPArmsBlendPoseIndex);
				}
				ClientPrimaryWeapon->K2_AttachToComponent(FPArmMesh, WeaponSocketName, EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true);
			}
			ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipGunCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
			//ClientEquipFPArmsPrimary();

		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("ServerPrimaryWeapon is not exists$$$$$$$$$$$$$$$$$"));
		};
	}
	else
	{
		
	}
	int32 iter;
	iter = WeaponList.Find(ServerSecondaryWeapon);
	if (iter != INDEX_NONE)
	{
		WeaponList.RemoveAtSwap(iter);
		WeaponList.Emplace(ServerSecondaryWeapon);
	}
}

bool ASCharacter::EquipPrimary_Validate(AWeaponServer* Weaponserver)
{
	return true;
}




#pragma region NetWork
void ASCharacter::ServerLowSpeedWalk_Implementation()
{
	//CharacterMovement->MaxWalkSpeed = 300;
	//bIsCrouched = true;
	//Crouch();
	CharacterMovement->MaxWalkSpeed = 300;
	Crouch();
	LowSpeedWalk();
	IsQuietStep = true;
}

bool ASCharacter::ServerLowSpeedWalk_Validate()
{
	return true;
}

void ASCharacter::ServerNormalSpeedWalk_Implementation()
{
	CharacterMovement->MaxWalkSpeed = 600;
	UnCrouch();
	NormalSpeedWalk();
	IsQuietStep = false;
}

bool ASCharacter::ServerNormalSpeedWalk_Validate()
{
	return true;
}

void ASCharacter::ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	if (ServerPrimaryWeapon)
	{
		
		
		//后坐力
		SetRecoil();

		//枪口火焰
		ServerPrimaryWeapon->MultiShootingEffect();

		ServerPrimaryWeapon->ClipGunCurrentAmmo -= 1;
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipGunCurrentAmmo: %d"), ServerPrimaryWeapon->ClipGunCurrentAmmo));

		//客户端更新子弹UI
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipGunCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);

		IsFiring = true;

		//第三人称动画
		MultiShooting();

		//射线检测
		//RifleLinerTrace(CameraLocation, CameraRotation, IsMoving);

		//发射子弹
		ABullet* Bullet = SpawnBullet(IsMoving);
		Bullet->SetOwner(ServerPrimaryWeapon);
	}
}

bool ASCharacter::ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void ASCharacter::ServerFireSecondaryWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	if (ServerSecondaryWeapon)
	{


		//后坐力
		SetRecoil();

		//枪口火焰
		ServerSecondaryWeapon->MultiShootingEffect();

		ServerSecondaryWeapon->ClipGunCurrentAmmo -= 1;
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipGunCurrentAmmo: %d"), ServerPrimaryWeapon->ClipGunCurrentAmmo));

		//客户端更新子弹UI
		ClientUpdateAmmoUI(ServerSecondaryWeapon->ClipGunCurrentAmmo, ServerSecondaryWeapon->GunCurrentAmmo);

		IsFiring = true;

		//第三人称动画
		MultiShooting();

		//射线检测
		//RifleLinerTrace(CameraLocation, CameraRotation, IsMoving);

		//发射子弹
		ABullet* Bullet = SpawnBullet(IsMoving);
		Bullet->SetOwner(ServerSecondaryWeapon);

		if (ClientSecondaryWeapon)
		{
			FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			ActionInfo.ExecutionFunction = TEXT("DelaySpreadSecondaryWeaponShootCallback");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			UKismetSystemLibrary::Delay(this, ClientSecondaryWeapon->ClientArmsFireAnimMontage->GetPlayLength(), ActionInfo);
		}
	}
}

bool ASCharacter::ServerFireSecondaryWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void ASCharacter::ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	if (ServerPrimaryWeapon)
	{
		//后坐力
		SetRecoil();

		ServerPrimaryWeapon->MultiShootingEffect();

		ServerPrimaryWeapon->ClipGunCurrentAmmo -= 1;
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipGunCurrentAmmo: %d"), ServerPrimaryWeapon->ClipGunCurrentAmmo));

		//客户端更新子弹UI
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipGunCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);

		if (ClientPrimaryWeapon)
		{
			FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			ActionInfo.ExecutionFunction = TEXT("DelaySniperShootCallBack");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			UKismetSystemLibrary::Delay(this, ClientPrimaryWeapon->ClientArmsFireAnimMontage->GetPlayLength(), ActionInfo);

		}
		IsFiring = true;
		//第三人称动画
		MultiShooting();

		//射线检测
		//RifleLinerTrace(CameraLocation, CameraRotation, IsMoving);

		//发射子弹
		ABullet* Bullet = SpawnBullet(!IsAiming);
		Bullet->SetOwner(ServerPrimaryWeapon);
		if (IsAiming)
		{
			ClientEndAiming();
		}
	}
}

bool ASCharacter::ServerFireSniperWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void ASCharacter::ClientUpdateAmmoUI_Implementation(int32 ClipGunCurrentAmmo, int32 GunCurrentAmmo)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateAmmoUI(ClipGunCurrentAmmo, GunCurrentAmmo);
	}
}

bool ASCharacter::ClientUpdateAmmoUI_Validate(int32 ClipGunCurrentAmmo, int32 GunCurrentAmmo)
{
	return true;
}

void ASCharacter::ClientUpdateHealthUI_Implementation()
{
	if (FPSPlayerController)
	{
		ASCharacter* PawnHealth = Cast<ASCharacter>(FPSPlayerController->GetPawn());
		if (PawnHealth)
		{
			FPSPlayerController->UpdateHealthUI(PawnHealth->Health);
		}
	}
}

bool ASCharacter::ClientUpdateHealthUI_Validate()
{
	return true;
}

void ASCharacter::OnRep_CurrentLife()
{
	ClientUpdateHealthUI();
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnRep_CurrentLife!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!,%f"),Health));
	if (FPSPlayerController)
	{
		ASCharacter* PawnHealth = Cast<ASCharacter>(FPSPlayerController->GetPawn());
		if (PawnHealth)
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("health:%d"), PawnHealth->Health));
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
	}
	
	
}

void ASCharacter::OnRep_NearVehicle()
{
	if(FPSPlayerController)
	{
		
		for(AActor* Actor:FPSPlayerController->VehicleList)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("NearVehicle:%s"),*Actor->GetName()));
		}
	}
}

void ASCharacter::UpdateClientVehicleUI_Implementation(bool IsActivate)
{
	UpdateVehicleUIBP(IsActivate);
}
//当武器的mesh和碰撞体均有碰撞时该方法会触发两次，导致主副武器混乱
void ASCharacter::PickEquipment_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                               bool bFromSweep, const FHitResult& SweepResult)
{
	
	AWeaponServer* ServerWeapon = Cast<AWeaponServer>(OtherActor);
	AWeaponServer* CurrentServerWeapon = GetCurrentServerWeapon();
	if (ServerWeapon && ServerWeapon->GetOwner() == nullptr)
	{
		if (ServerWeapon->GradeofWeapon == EWeaponGrade::Primary)
		{
			if (ServerPrimaryWeapon == nullptr)
			{
				LastWeapon = ServerWeapon;
				ServerWeapon->EquipOrFall_Weapon(true);
				WeaponList.Emplace(ServerWeapon);
				ServerWeapon->SetOwner(this);
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("EquipWeapon")));
				ServerWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon"), EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true);
				
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon owner:%s"),*(ServerWeapon->GetOwner())->GetName()));
				ServerPrimaryWeapon = ServerWeapon;
			}
			else if (ReservePrimaryWeapon == nullptr)
			{
				LastWeapon = ServerWeapon;
				ServerWeapon->EquipOrFall_Weapon(true);
				WeaponList.Emplace(ServerWeapon);
				ServerWeapon->SetOwner(this);
				if (CurrentServerWeapon==ServerPrimaryWeapon)
				{
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("EquipWeapon")));
					ServerWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon"), EAttachmentRule::SnapToTarget,
						EAttachmentRule::SnapToTarget,
						EAttachmentRule::SnapToTarget,
						true);	
				}
				else
				{
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("EquipWeapon1")));
					ServerWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon1"), EAttachmentRule::SnapToTarget,
						EAttachmentRule::SnapToTarget,
						EAttachmentRule::SnapToTarget,
						true);
				}
				
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon owner:%s"),*(ServerWeapon->GetOwner())->GetName()));
				ReservePrimaryWeapon = ServerWeapon;
			}
			else
			{
				
			}
		}
		else if (ServerWeapon->GradeofWeapon == EWeaponGrade::Secondary)
		{
			ServerWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon2"), EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
			ServerWeapon->SetOwner(this);
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("PickEquipment")));
			ServerSecondaryWeapon = ServerWeapon;
		}
		else
		{

		}
	}
	/*else
	{
		if (ReservePrimaryWeapon == nullptr)
		{
			PrimaryWeapon->K2_AttachToComponent(Mesh, TEXT("EquipWeapon"), EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
			PrimaryWeapon->SetOwner(this);
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("PickEquipment")));
			ReservePrimaryWeapon = PrimaryWeapon;
		}
	}*/
}

void ASCharacter::SetUnActivate_Implementation()
{
		// 禁用根组件的碰撞,并隐藏
	if (RootComponent)
	{
				
		// 隐藏根组件
		RootComponent->SetVisibility(false, true);  // true 表示递归隐藏所有子组件
	}
	FPArmMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FPArmMesh->SetEnableGravity(false);
	CapsuleArmComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetEnableGravity(false);
	//SetActorEnableCollision(false);
	/*TArray<UActorComponent*> Components;
	this->GetComponents(Components);
	if (Components.Num()>0)
	{
		
		for (UActorComponent* Component : Components)
		{
			UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component);
			if (IsValid(PrimitiveComponent))  // 确保组件指针有效
			{
				PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				UE_LOG(LogTemp, Log, TEXT("Collision disabled for component %s in Actor %s"), *PrimitiveComponent->GetName(), *GetName());
			}
		}
	}*/
	FPSPlayerController=nullptr;
	if (WeaponList.Num()>0)
	{
		for (AWeaponServer* ServerWeapon:WeaponList)
		{
			if(ServerWeapon)
			{
				ServerWeapon->SetActorEnableCollision(false);
			}
		}
	}
	
}

void ASCharacter::SetActivate_Implementation()
{
	// 设置角色移动模式
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	// 确保根组件可见
	if (RootComponent)
	{
		RootComponent->SetVisibility(true, true);
	}

	// 配置 CapsuleComponent
	CapsuleComponent->SetSimulatePhysics(false);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionObjectType(ECC_Pawn);
	CapsuleComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CapsuleComponent->BodyInstance.bUseCCD = true;

	// 配置 CapsuleArmComponent
	CapsuleArmComponent->SetSimulatePhysics(false);
	CapsuleArmComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleArmComponent->SetCollisionObjectType(ECC_Pawn);
	CapsuleArmComponent->BodyInstance.bUseCCD = true;

	// 配置 Mesh
	Mesh->SetSimulatePhysics(false);
	Mesh->SetOwnerNoSee(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_Pawn);
	Mesh->SetEnableGravity(true);

	// 配置 FPArmMesh
	FPArmMesh->SetSimulatePhysics(false); // 检查是否需要物理模拟
	FPArmMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FPArmMesh->SetCollisionObjectType(ECC_Pawn);
	FPArmMesh->bCastDynamicShadow = false;
	FPArmMesh->CastShadow = false;
	FPArmMesh->SetEnableGravity(true);

	FPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());
	
	if (WeaponList.Num()>0)
	{
		for (AWeaponServer* ServerWeapon:WeaponList)
		{
			if(ServerWeapon)
			{
				ServerWeapon->SetActorEnableCollision(true);
			}
		}
	}
	if (FPSPlayerController)
	{
		EnableInput(FPSPlayerController);  // 启用输入，确保控制权生效
			
		FPSPlayerController->SetInputMode(FInputModeGameOnly());
		FPSPlayerController->bShowMouseCursor = false;
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(FPSPlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings(); // 可选，清除其他映射上下文
			Subsystem->AddMappingContext(InputMapping, 1); // 添加当前映射上下文
		}
	}

	
}
/*bool ASCharacter::PickEquipment_Validate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	return true;
}*/


void ASCharacter::EquipSecondary_Implementation(AWeaponServer* Weaponserver)
{
	FName WeaponSocketName = TEXT("WeaponSocket");
	
	ActivateWeapon = Weaponserver->KindofWeapon;
	FName Attach_socket;
	//FName WeaponSocketName = TEXT("WeaponSocket");
	switch (Weaponserver->KindofWeapon)
	{
	case EWeaponType::DersertEagle:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get DersertEagle!"));
		Attach_socket = TEXT("Weapon_Rffle");
		WeaponSocketName = TEXT("WeaponSocket");
		break;
	}
	default:
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get AK47!"));
		Attach_socket = TEXT("Weapon_Rffle");
		WeaponSocketName = TEXT("WeaponSocket");
		break;
	}
	}
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerSecondaryWeapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%s"), *ServerSecondaryWeapon->GetName()));
	ServerSecondaryWeapon = Weaponserver;
	ServerSecondaryWeapon->SetOwner(this);
	ServerSecondaryWeapon->K2_AttachToComponent(Mesh, Attach_socket, EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		true);
	if (HasAuthority())
	{
		if (ServerSecondaryWeapon)
		{
			if (ClientSecondaryWeapon)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Server&&Client SecondaryWeapon is exists%%%%%%%%%%%%%%%"));
				ClientSecondaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				ClientSecondaryWeapon->Destroy();
				ClientSecondaryWeapon = nullptr;
			}
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.Owner = this;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ClientSecondaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerSecondaryWeapon->ClientWeaponBaseBPClass,
				GetActorTransform(),
				SpawnInfo);
			//不同武器插槽

			/*if (ActivateWeapon == EWeaponType::M4A1)
			{
				WeaponSocketName = TEXT("M4A1_Socket");

			}*/
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex));
			if (ClientSecondaryWeapon)
			{
				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex), true, true, FLinearColor(0.0, 0.66, 1.0, 1.0), 10.0);
				if (ClientArmsAnimation)
				{
					//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex), true, true, FLinearColor(0.0, 0.66, 1.0, 1.0), 10.0);
					UpdateFPArmsBlendPose(ClientSecondaryWeapon->FPArmsBlendPoseIndex);
				}
				ClientSecondaryWeapon->K2_AttachToComponent(FPArmMesh, WeaponSocketName, EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true);
			}
			ClientUpdateAmmoUI(ServerSecondaryWeapon->ClipGunCurrentAmmo, ServerSecondaryWeapon->GunCurrentAmmo);

		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("ServerSecondaryWeapon is not exists$$$$$$$$$$$$$$$$$"));
		};
	}
	int32 iter;
	iter = WeaponList.Find(ServerSecondaryWeapon);
	if (iter != INDEX_NONE)
	{
		WeaponList.RemoveAtSwap(iter);
		WeaponList.Emplace(ServerSecondaryWeapon);
	}

}

bool ASCharacter::EquipSecondary_Validate(AWeaponServer* Weaponserver)
{
	return true;
}

void ASCharacter::ClientEquipFPArmsSecondary_Implementation()
{
	if (ServerSecondaryWeapon)
	{
		if (ClientSecondaryWeapon)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("ClientSecondaryWeapon is exists%%%%%%%%%%%%%%%"));
			DestoryClientWeapon();
		}
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ClientSecondaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerSecondaryWeapon->ClientWeaponBaseBPClass,
			GetActorTransform(),
			SpawnInfo);
		//不同武器插槽
		FName WeaponSocketName = TEXT("WeaponSocket");
		/*if (ActivateWeapon == EWeaponType::M4A1)
		{
			WeaponSocketName = TEXT("M4A1_Socket");
		}*/
		//ActivateWeapon = ServerSecondaryWeapon->KindofWeapon;
		switch (ActivateWeapon)
		{
		case EWeaponType::DersertEagle:
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get DersertEagle!"));
			WeaponSocketName = TEXT("WeaponSocket");
			break;
		}
		default:
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get AK47!"));
			WeaponSocketName = TEXT("WeaponSocket");
			break;
		}
		}
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex));
		if (ClientSecondaryWeapon)
		{
			if (ClientArmsAnimation)
			{
				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weaponArmsAnimation!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex), true, true, FLinearColor(0.0, 0.66, 1.0, 1.0), 10.0);
				UpdateFPArmsBlendPose(ClientSecondaryWeapon->FPArmsBlendPoseIndex);
			}
			ClientSecondaryWeapon->K2_AttachToComponent(FPArmMesh, WeaponSocketName, EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
		}
		ClientUpdateAmmoUI(ServerSecondaryWeapon->ClipGunCurrentAmmo, ServerSecondaryWeapon->GunCurrentAmmo);

	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("ServerSecondaryWeapon is not exists$$$$$$$$$$$$$$$$$"));
	}
}

bool ASCharacter::ClientEquipFPArmsSecondary_Validate()
{
	return true;
}

void ASCharacter::ClientEquipFPArmsPrimary_Implementation()
{	
	
	if (ServerPrimaryWeapon)
	{
		if (ClientPrimaryWeapon)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("ClientPrimaryWeapon is exists%%%%%%%%%%%%%%%"));
			DestoryClientWeapon();
		}
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ClientPrimaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerPrimaryWeapon->ClientWeaponBaseBPClass,
			GetActorTransform(),
			SpawnInfo);
		//不同武器插槽
		FName WeaponSocketName = TEXT("WeaponSocket");
		/*if (ActivateWeapon == EWeaponType::M4A1)
		{
			WeaponSocketName = TEXT("M4A1_Socket");
		}*/
		ActivateWeapon=ServerPrimaryWeapon->KindofWeapon;
		switch (ActivateWeapon)
		{
		case EWeaponType::AK47:
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get AK47!"));
			WeaponSocketName = TEXT("WeaponSocket");
			break;
		}
		case EWeaponType::M4A1:
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get M4A1!"));
			WeaponSocketName = TEXT("M4A1_Socket");
			break;
		}
		case EWeaponType::Sniper:
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get Sniper"));
			WeaponSocketName = TEXT("AWP_Socket");
			break;
		}
		default:
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("Get AK47!"));
			WeaponSocketName = TEXT("WeaponSocket");
			break;
		}
		}
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex));
		if (ClientPrimaryWeapon)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapon!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex), true, true, FLinearColor(0.0, 0.66, 1.0, 1.0), 10.0);
			if (ClientArmsAnimation)
			{
				//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weaponArmsAnimation!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:%d"), ClientPrimaryWeapon->FPArmsBlendPoseIndex), true, true, FLinearColor(0.0, 0.66, 1.0, 1.0), 10.0);
				UpdateFPArmsBlendPose(ClientPrimaryWeapon->FPArmsBlendPoseIndex);
			}
			ClientPrimaryWeapon->K2_AttachToComponent(FPArmMesh, WeaponSocketName, EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
		}
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipGunCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
		
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("ServerPrimaryWeapon is not exists$$$$$$$$$$$$$$$$$"));
	}
	
}

bool ASCharacter::ClientEquipFPArmsPrimary_Validate()
{
	return true;
}

void ASCharacter::ClientFire_Implementation()
{
	

	//播放开火枪体动画
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();
	if (CurrentClientWeapon)
	{
		CurrentClientWeapon->PlayShootAnimation();
		//手臂动画蒙太奇播放
		UAnimMontage* ClientArmsFireAnimMontage = CurrentClientWeapon->ClientArmsFireAnimMontage;
		ClientArmsAnimation->Montage_SetPlayRate(ClientArmsFireAnimMontage, 1);
		ClientArmsAnimation->Montage_Play(ClientArmsFireAnimMontage);

		//播放设计声音
		CurrentClientWeapon->DisplayClientWeaponSound();

		//播放开火粒子效果
		CurrentClientWeapon->DisplayClientWeaponEffect();

		//镜头抖动
		FPSPlayerController->PlayCameraShake(CurrentClientWeapon->CameraShakeClass);

		//十字准星扩散动画
		FPSPlayerController->DoCrossHairRecoll();
	}
}

bool ASCharacter::ClientFire_Validate()
{
	return true;
}


void ASCharacter::StartWithKindOfWeapon()
{
	if (HasAuthority())
	{
		PurchaseWeapon(TestWeapon);
	}
}

void ASCharacter::PurchaseWeapon(EWeaponType WeaponType)
{
	switch (WeaponType)
	{
	case EWeaponType::AK47:
	{
		ActivateWeapon = EWeaponType::AK47;
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UClass* BluePrintWeapon = StaticLoadClass(AWeaponServer::StaticClass(), nullptr, TEXT("Blueprint'/Game/BluePrint/Weapon/AK47/BP_AK47Server.BP_AK47Server_C'"));
		AWeaponServer* StartPrimaryWeapon = GetWorld()->SpawnActor<AWeaponServer>(BluePrintWeapon,
			GetActorTransform(),
			SpawnInfo);
		StartPrimaryWeapon->EquipOrFall_Weapon(true);
		EquipPrimary(StartPrimaryWeapon);
		

	}
	break;
	case EWeaponType::M4A1:
	{
		ActivateWeapon = EWeaponType::M4A1;
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UClass* BluePrintWeapon = StaticLoadClass(AWeaponServer::StaticClass(), nullptr, TEXT("Blueprint'/Game/BluePrint/Weapon/M4A1/ServerBP_M4A1.ServerBP_M4A1_C'"));
		AWeaponServer* StartPrimaryWeapon = GetWorld()->SpawnActor<AWeaponServer>(BluePrintWeapon,
			GetActorTransform(),
			SpawnInfo);
		StartPrimaryWeapon->EquipOrFall_Weapon(true);
		EquipPrimary(StartPrimaryWeapon);
		

	}
	break;
	case EWeaponType::Sniper:
	{
		ActivateWeapon = EWeaponType::Sniper;
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UClass* BluePrintWeapon = StaticLoadClass(AWeaponServer::StaticClass(), nullptr, TEXT("Blueprint'/Game/BluePrint/Weapon/Sniper/ServerBP_Sniper.ServerBP_Sniper_C'"));
		AWeaponServer* StartPrimaryWeapon = GetWorld()->SpawnActor<AWeaponServer>(BluePrintWeapon,
			GetActorTransform(),
			SpawnInfo);
		StartPrimaryWeapon->EquipOrFall_Weapon(true);
		EquipPrimary(StartPrimaryWeapon);


	}
	break;
	case EWeaponType::DersertEagle:
	{
		ActivateWeapon = EWeaponType::DersertEagle;
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		UClass* BluePrintWeapon = StaticLoadClass(AWeaponServer::StaticClass(), nullptr, TEXT("Blueprint'/Game/BluePrint/Weapon/DersertEagle/ServerBP_DersertEagle.ServerBP_DersertEagle_C'"));
		AWeaponServer* StartSecondaryWeapon = GetWorld()->SpawnActor<AWeaponServer>(BluePrintWeapon,
			GetActorTransform(),
			SpawnInfo);
		StartSecondaryWeapon->EquipOrFall_Weapon(true);
		EquipSecondary(StartSecondaryWeapon);
	}
	break;
	}
	if (ClientPrimaryWeapon && ServerPrimaryWeapon)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ammo!!!!!!!!!!!!!!!!!!!!!:%d"), ServerPrimaryWeapon->ClipGunCurrentAmmo));
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipGunCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
	}

}

AWeaponBaseClient* ASCharacter::GetCurrentClientWeapon()
{
	switch (ActivateWeapon) 
	{
	case EWeaponType::AK47:
	{
		return ClientPrimaryWeapon;
	}
	break;
	case EWeaponType::M4A1:
	{
		return ClientPrimaryWeapon;
	}
	break;
	case EWeaponType::Sniper:
	{
		return ClientPrimaryWeapon;
	}
	break;
	case EWeaponType::DersertEagle:
	{
		return ClientSecondaryWeapon;
	}
	break;
	default:
	{
		return nullptr;
	}
	}
	
}

AWeaponServer* ASCharacter::GetCurrentServerWeapon()
{
	switch (ActivateWeapon)
	{
	case EWeaponType::AK47:
	{
		return ServerPrimaryWeapon;
	}
	break;
	case EWeaponType::M4A1:
	{
		return ServerPrimaryWeapon;
	}
	break;
	case EWeaponType::Sniper:
	{
		return ServerPrimaryWeapon;
	}
	break;
	case EWeaponType::DersertEagle:
	{
		return ServerSecondaryWeapon;
	}
	break;
	default:
	{
		return nullptr;
	}
	}
}

void ASCharacter::MultiShooting_Implementation()
{
	AWeaponServer* ServerWeapon = GetCurrentServerWeapon();
	if (ServerBodysAnimation)
	{
		ServerBodysAnimation->Montage_Play(ServerWeapon->ServerBodysShootAnimMontage);
	}
	if (ServerWeapon)
	{
		ServerWeapon->PlayFireAnimation();
	}
}

bool ASCharacter::MultiShooting_Validate()
{
	return true;
}

void ASCharacter::MultiBulletDecal_Implementation(FVector Location, FRotator Rotation)
{
	if (ServerPrimaryWeapon)
	{
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ServerPrimaryWeapon->BulletDecalMaterial, FVector(8, 8, 8),
			Location, Rotation, 10);

		if (Decal)
		{
			Decal->SetFadeScreenSize(0.001);
		}
	}
}

bool ASCharacter::MultiBulletDecal_Validate(FVector Location, FRotator Rotation)
{
	return true;
}

void ASCharacter::OnRep_UpdateClientPrimaryWeapon_Implementation()
{
	AWeaponServer* ServerWeapon = GetCurrentServerWeapon();
	if (ServerPrimaryWeapon != nullptr && ServerPrimaryWeapon->KindofWeapon == ActivateWeapon)
	{
		ClientEquipFPArmsPrimary();
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnReClientPrimaryweapon!!!!!!!!!!!!!!!!!!!!!%s"), *ServerWeapon->GetAttachParentSocketName().ToString()));
	}
}

bool ASCharacter::OnRep_UpdateClientPrimaryWeapon_Validate()
{
	return true;
}

void ASCharacter::OnRep_UpdateClientSecondaryWeapon_Implementation()
{
	//AWeaponServer* ServerWeapon = GetCurrentServerWeapon();
	if (ServerSecondaryWeapon != nullptr && ServerSecondaryWeapon->KindofWeapon == ActivateWeapon)
	{
		ClientEquipFPArmsSecondary();
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnReClientSecondaryweapon!!!!!!!!!!!!!!!!!!!!!%s"), *ServerSecondaryWeapon->GetName()));
	}
	
}

bool ASCharacter::OnRep_UpdateClientSecondaryWeapon_Validate()
{
	return true;
}

void ASCharacter::OnRep_UpdateReserveWeapon()
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnRep_UpdateReserveWeapon!!!!!!!!!!!!!!!!!!!!!")));
}

void ASCharacter::OnRep_Updatetestfloat()
{
}




void ASCharacter::AutoFire()
{
	if (ServerPrimaryWeapon->ClipGunCurrentAmmo > 0 && !IsReload)
	{
		//服务器开火逻辑
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(CameraComp->GetComponentLocation(), CameraComp->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(CameraComp->GetComponentLocation(), CameraComp->GetComponentRotation(), false);
		}
		

		//客户端开火逻辑
		ClientFire();
	}
	else
	{
		StopWeaponPrimary();
	}
}

ABullet* ASCharacter::SpawnBullet(bool IsMoving)
{
	AWeaponServer* ServerWeapon = GetCurrentServerWeapon();
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = ServerPrimaryWeapon;
	SpawnInfo.Instigator = this;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FVector CameraLocation = CameraComp->GetComponentLocation();
	FRotator CameraRotation = CameraComp->GetComponentRotation();

	//UClass* BulletBP = StaticLoadClass(ABullet::StaticClass(), nullptr, TEXT("Blueprint'/Game/BluePrint/Weapon/Bullet/BP_Bullet.BP_Bullet_C'"));
	ABullet* Bullet = GetWorld()->SpawnActor<ABullet>(ServerPrimaryWeapon->BulletBP, CameraLocation+200*CameraRotation.Vector(),
		CameraRotation,
		SpawnInfo
		);
	if (Bullet && ServerWeapon)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("fire!!!!!!!!!!!!!!!!!!!!!")));
		Bullet->SetOwner(ServerWeapon);
		Bullet->SetInstigator(this);
		FVector LaunchStartDirection = CameraRotation.Vector();
		if (IsMoving)
		{
			FVector SpreadRot = CalSpreadRot(ServerWeapon);
			FVector LaunchEndDirection = FVector(LaunchStartDirection.X + SpreadRot.X, LaunchStartDirection.Y + SpreadRot.Y, LaunchStartDirection.Z + SpreadRot.Z);
			Bullet->FireInDirection(LaunchEndDirection);
		}
		else
		{
			switch (ServerWeapon->GradeofWeapon)
			{
			case EWeaponGrade::Primary:
			{
				Bullet->FireInDirection(LaunchStartDirection);
			}
			break;
			case EWeaponGrade::Secondary:
			{
				FVector PistolSpreadRot = CalPistolSpreadRot();
				FVector LaunchEndDirection = FVector(LaunchStartDirection.X + PistolSpreadRot.X, LaunchStartDirection.Y + PistolSpreadRot.Y, LaunchStartDirection.Z + PistolSpreadRot.Z);
				Bullet->FireInDirection(LaunchEndDirection);
				SecondaryWeaponSpreadMin -= ServerWeapon->SecondaryWeaponSpreadMinIndex;
				SecondaryWeaponSpreadMax += ServerWeapon->SecondaryWeaponSpreadMaxIndex;

			}
			break;
			default:
				Bullet->FireInDirection(LaunchStartDirection);
				break;
			}
			
		}
	}
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("bullet owner:%s"),*(Bullet->GetOwner())->GetName()));

	return Bullet;
}

void ASCharacter::UpdateCameraRotation()
{

	if (FPSPlayerController)
	{
		float CurrentControlPitch = ServerPrimaryWeapon->VerticalRecoilCurve->GetFloatValue(RecoilXCrood);
		FRotator ControlerRotater = FPSPlayerController->GetControlRotation();
		FPSPlayerController->SetControlRotation(FRotator(
			ControlerRotater.Pitch - (CurrentControlPitch/RecoilXCrood),
			ControlerRotater.Yaw, ControlerRotater.Roll));
	}
}

void ASCharacter::SetRecoil()
{
	if (ServerPrimaryWeapon)
	{
		UCurveFloat* VerticalRecoilCurve = ServerPrimaryWeapon->VerticalRecoilCurve;
		UCurveFloat* HorizontalRecoilCurve = ServerPrimaryWeapon->HorizontalRecoilCurve;

		if (FPSPlayerController && VerticalRecoilCurve && HorizontalRecoilCurve)
		{
			FRotator ControlerRotater = FPSPlayerController->GetControlRotation();
			RecoilXCrood += 0.1;
			FPSPlayerController->SetControlRotation(FRotator(
				ControlerRotater.Pitch + VerticalRecoilCurve->GetFloatValue(RecoilXCrood) - VerticalRecoilCurve->GetFloatValue(RecoilXCrood-0.1),
				ControlerRotater.Yaw + HorizontalRecoilCurve->GetFloatValue(fmodf(RecoilXCrood,2.9)) - HorizontalRecoilCurve->GetFloatValue(fmodf(RecoilXCrood-0.1,2.9)),
				ControlerRotater.Roll));
		}
	}
}


void ASCharacter::FireWeaponPrimary()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("CLient Fire!"));
	VerticalRecoilRecoveryTimeLine.Stop();
	//AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();

	if (ServerPrimaryWeapon != nullptr && ServerPrimaryWeapon->ClipGunCurrentAmmo > 0 && !IsReload)
	{
		//服务器开火逻辑
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireRifleWeapon(CameraComp->GetComponentLocation(), CameraComp->GetComponentRotation(), true);
		}
		else
		{
			ServerFireRifleWeapon(CameraComp->GetComponentLocation(), CameraComp->GetComponentRotation(), false);
		}

		//客户端开火逻辑
		ClientFire();

		//后坐力
		SetRecoil();

		if (ServerPrimaryWeapon->IsAutoFire)
		{
			GetWorldTimerManager().SetTimer(AutoFireTimeHandle, this, &ASCharacter::AutoFire, ServerPrimaryWeapon->AutoFireRate, true);
		}
	}
	else
	{
		StopWeaponPrimary();
		if (IsReload)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("isreload")));
		}
		
	}

}

void ASCharacter::StopWeaponPrimary()
{
	GetWorldTimerManager().ClearTimer(AutoFireTimeHandle);
	/*if (ServerPrimaryWeapon)
	{
		VerticalRecoilRecoveryTimeLine.AddInterpFloat(ServerPrimaryWeapon->VerticalRecoilCurve, UpdateCameraRotationonDelegte);
		VerticalRecoilRecoveryTimeLine.SetNewTime(VerticalRecoilRecoveryTimeLine.GetTimelineLength()-RecoilXCrood);
		VerticalRecoilRecoveryTimeLine.Play();
	}*/
	IsFiring = false;

	RecoilXCrood = 0.0;
	
}
//射线检测
void ASCharacter::RifleLinerTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	FVector EndLocation;
	FVector CameraForwardVecter = UKismetMathLibrary::GetForwardVector(CameraRotation);
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	FHitResult HitResult;

	if (IsMoving)
	{
		FVector Vector = CameraLocation + CameraForwardVecter * ServerPrimaryWeapon->BulletDistance;
		float RandomRangeX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireXRandomRange, ServerPrimaryWeapon->MovingFireXRandomRange);
		float RandomRangeY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireYRandomRange, ServerPrimaryWeapon->MovingFireYRandomRange);
		float RandomRangeZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireZRandomRange, ServerPrimaryWeapon->MovingFireZRandomRange);
		EndLocation = FVector(Vector.X + RandomRangeX, Vector.Y + RandomRangeY, Vector.Z + RandomRangeZ);
	}
	else
	{
		EndLocation = CameraLocation + CameraForwardVecter * ServerPrimaryWeapon->BulletDistance;
	}

	bool HItSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(), CameraLocation, EndLocation, ETraceTypeQuery::TraceTypeQuery1,
		false, IgnoreActors, EDrawDebugTrace::Persistent, HitResult, true, FLinearColor::Red, FLinearColor::Green, 3.0f);
	
	if (HItSuccess)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt Component: %s"), HitResult.Component.Get()));
		ASCharacter* FPSCharacter = Cast<ASCharacter>(HitResult.GetActor());
		if (FPSCharacter)
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt Actor: %s"), *FPSCharacter->GetName()));
			DamagePLayer(ServerPrimaryWeapon, CameraLocation,HitResult);
		}
		else
		{
			FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			MultiBulletDecal(HitResult.Location,XRotator);
		}
	}



}

FVector ASCharacter::CalSpreadRot(AWeaponServer* ServerWeapon)
{

	float RandomRangeX = UKismetMathLibrary::RandomFloatInRange(-ServerWeapon->MovingFireXRandomRange, ServerWeapon->MovingFireXRandomRange);
	float RandomRangeY = UKismetMathLibrary::RandomFloatInRange(-ServerWeapon->MovingFireYRandomRange, ServerWeapon->MovingFireYRandomRange);
	float RandomRangeZ = UKismetMathLibrary::RandomFloatInRange(-ServerWeapon->MovingFireZRandomRange, ServerWeapon->MovingFireZRandomRange);
	FVector SpreadRot = FVector(RandomRangeX, RandomRangeY, RandomRangeZ);
	return SpreadRot;
}

void ASCharacter::FireWeaponSecondary()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("CLient Fire!"));
	VerticalRecoilRecoveryTimeLine.Stop();
	//AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();

	if (ServerSecondaryWeapon != nullptr && ServerSecondaryWeapon->ClipGunCurrentAmmo > 0 && !IsReload)
	{
		//服务器开火逻辑
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)
		{
			ServerFireSecondaryWeapon(CameraComp->GetComponentLocation(), CameraComp->GetComponentRotation(), true);
		}
		else
		{
			ServerFireSecondaryWeapon(CameraComp->GetComponentLocation(), CameraComp->GetComponentRotation(), false);
		}

		//客户端开火逻辑
		ClientFire();


		if (ServerSecondaryWeapon->IsAutoFire)
		{
			GetWorldTimerManager().SetTimer(AutoFireTimeHandle, this, &ASCharacter::AutoFire, ServerSecondaryWeapon->AutoFireRate, true);
		}
	}
	else
	{
		StopWeaponSecondary();
		if (IsReload)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("isreload")));
		}

	}
}

void ASCharacter::StopWeaponSecondary()
{
	IsFiring = false;
}

FVector ASCharacter::CalPistolSpreadRot()
{

	float RandomRangeX = UKismetMathLibrary::RandomFloatInRange(SecondaryWeaponSpreadMin, SecondaryWeaponSpreadMax);
	float RandomRangeY = UKismetMathLibrary::RandomFloatInRange(SecondaryWeaponSpreadMin, SecondaryWeaponSpreadMax);
	float RandomRangeZ = UKismetMathLibrary::RandomFloatInRange(SecondaryWeaponSpreadMin, SecondaryWeaponSpreadMax);
	FVector SpreadRot = FVector(RandomRangeX, RandomRangeY, RandomRangeZ);
	return SpreadRot;
}

void ASCharacter::DelaySpreadSecondaryWeaponShootCallback()
{
	SecondaryWeaponSpreadMin = 0;

	SecondaryWeaponSpreadMax = 0;
}

void ASCharacter::FireWeaponSniper()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::White, TEXT("CLient Fire!"));
	VerticalRecoilRecoveryTimeLine.Stop();
	//AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();

	if (ServerPrimaryWeapon != nullptr && ServerPrimaryWeapon->ClipGunCurrentAmmo > 0 && !IsReload && !IsFiring)
	{
		//服务器开火逻辑
		//是否开镜
		ServerFireSniperWeapon(CameraComp->GetComponentLocation(), CameraComp->GetComponentRotation(), IsAiming);

		//客户端开火逻辑
		ClientFire();

		//后坐力
		SetRecoil();

		/*if (ServerPrimaryWeapon->IsAutoFire)
		{
			GetWorldTimerManager().SetTimer(AutoFireTimeHandle, this, &ASCharacter::AutoFire, ServerPrimaryWeapon->AutoFireRate, true);
		}*/
	}
	else
	{
		StopWeaponPrimary();
		if (IsReload)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("isreload")));
		}

	}
}

void ASCharacter::StopWeaponSniper()
{

}

void ASCharacter::ServerSetAiming_Implementation(bool AimingState)
{
	IsAiming = AimingState;
}

bool ASCharacter::ServerSetAiming_Validate(bool AimingState)
{
	return true;
}

void ASCharacter::ClientAiming_Implementation()
{
	AWeaponServer* PrimaryWeapon = GetCurrentServerWeapon();
	if (PrimaryWeapon!=nullptr && PrimaryWeapon->HasSight)
	{
		if (FPArmMesh)
		{
			FPArmMesh->SetHiddenInGame(true);
		}
		ClientPrimaryWeapon->SetActorHiddenInGame(true);
		if (CameraComp)
		{
			CameraComp->SetFieldOfView(ClientPrimaryWeapon->FieldOfAimimgView);
		}

		if (ClientPrimaryWeapon->SniperScopeBPClass)
		{
			//使用c++创建UMG需要在Build.cs文件中引入"UMG"模块，否则报错无法解析的外部符号
			WidgetScope = CreateWidget<UUserWidget>(GetWorld(), ClientPrimaryWeapon->SniperScopeBPClass);
			WidgetScope->AddToViewport();
		}
	}

}

bool ASCharacter::ClientAiming_Validate()
{
	return true;
}

void ASCharacter::ClientEndAiming_Implementation()
{
	if (FPArmMesh)
	{
		FPArmMesh->SetHiddenInGame(false);
	}
	if (ClientPrimaryWeapon)
	{
		ClientPrimaryWeapon->SetActorHiddenInGame(false);
		if (CameraComp)
		{
			CameraComp->SetFieldOfView(90);
		}
	}
	if (WidgetScope)
	{
		WidgetScope->RemoveFromParent();
	}
}

bool ASCharacter::ClientEndAiming_Validate()
{
	return true;
}

void ASCharacter::DelaySniperShootCallBack()
{
	IsFiring = false;
}

void ASCharacter::DamagePLayer(AWeaponServer* AttackWeapon, FVector const& HitFromDirection, FHitResult const& HitInf)
{
	UPhysicalMaterial* HitPhysMaterial = HitInf.PhysMaterial.Get();
	float HitDamage = 0;
	ASCharacter* AttackPlayer = Cast<ASCharacter>(AttackWeapon->GetOwner());
	if (HitPhysMaterial && AttackWeapon != nullptr)
	{
		switch (HitPhysMaterial->SurfaceType)
		{
		case EPhysicalSurface::SurfaceType1:
		{
			//head
			HitDamage = AttackWeapon->BaseDamage * 4;
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt head")));
		}
		break;
		case EPhysicalSurface::SurfaceType2:
		{
			//body
			HitDamage = AttackWeapon->BaseDamage * 1;
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt body")));
		}
		break;
		case EPhysicalSurface::SurfaceType3:
		{
			//arm
			HitDamage = AttackWeapon->BaseDamage * 0.8;
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt arm")));
		}
		break;
		case EPhysicalSurface::SurfaceType4:
		{
			//leg
			HitDamage = AttackWeapon->BaseDamage * 0.7;
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt leg")));
		}
		break;
		default:
			break;
		}
		UGameplayStatics::ApplyPointDamage(HitInf.GetActor(), HitDamage, HitFromDirection, HitInf,
			AttackPlayer->GetController(), AttackPlayer, UDamageType::StaticClass());
	}
	
}

void ASCharacter::OnHit_Implementation(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	Health -= Damage;
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_CurrentLife();
	}
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Damage Actor: %s, Damage: %f, health: %f"),*GetName(),Damage,Health));
	//ClientUpdateHealthUI();
	/*if (Health <= 0)
	{
		//ServerDeath();
	}*/

}

bool ASCharacter::OnHit_Validate(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	return true;
}

//FRotator ASCharacter::GetAimOffsets() const
//{
//	const FVector AimDirWS = GetBaseAimRotation().Vector();
//	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
//	const FRotator AimRotLS = AimDirLS.Rotation();
//
//	return AimRotLS;
//}

void ASCharacter::GetAimOffsets_Implementation()
{
	//AMultiFPSPlayerController* PlayerController =Cast<AMultiFPSPlayerController>(this->GetController());
	
	FQuat AimDirWS = this->GetControlRotation().Quaternion();
	AimOffset = (GetActorTransform().InverseTransformRotation(AimDirWS)).Rotator();
	/*FRotator CalAimOffset = (GetActorTransform().InverseTransformRotation(AimDirWS)).Rotator();
	float Yaw = CalAimOffset.Yaw;
	float Pitch = CalAimOffset.Pitch;
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("pitch=%f"), CalAimOffset.Pitch));
	if (CalAimOffset.Yaw > 180)
	{
		Yaw = CalAimOffset.Yaw - 360;
	}
	if (CalAimOffset.Pitch > 180)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("pitch<0!!!!!!!!!!!!!!!!!!!!!!!")));
		Pitch = CalAimOffset.Pitch - 360;
	}
	FRotator NomAimOffset;
	NomAimOffset.Yaw = Yaw;
	NomAimOffset.Pitch = Pitch;

	AimOffset = NomAimOffset;*/
	/*FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	AimOffset = AimDirLS.Rotation();*/
	//if (ServerBodysAnimation) {
	//	for (TFieldIterator<FFloatProperty> StrProperty(ServerBodysAnimation->GetClass()); StrProperty; ++StrProperty)
	//	{
	//		// 属性名称为SizeX 
	//		if (StrProperty->GetName().Contains(TEXT("yaw")))
	//		{
	//			StrProperty->SetPropertyValue_InContainer(ServerBodysAnimation, AimDirLS.Rotation().Yaw);
	//			break;
	//		}
	//	}
	//}

}

void ASCharacter::RadialDamage_Implementation(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser)
{
	
	Health = FMath::Min(Health - Damage, Health);
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("RadialDamage:%f"), Damage));
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_CurrentLife();
	}
	/*if (Health<=0)
	{
		ServerDeath();
	}*/
}

/*float ASCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
/*float ASCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health > DamageAmount) {
		Health -= DamageAmount;
	}
	else {
		Health = 0;
	}
	if (GetLocalRole() == ROLE_Authority)
	{
		OnRep_CurrentLife();
	}
	return DamageAmount;
}
*/

void ASCharacter::InputReload()
{
	AWeaponServer* ServerWeapon = GetCurrentServerWeapon();
	if (ServerWeapon && !IsReload && ServerWeapon->GunCurrentAmmo > 0 && ServerWeapon->ClipGunCurrentAmmo < ServerWeapon->MaxClipAmmo)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("inputreload!!!!!!!!!!!!!!!!!!!!!!!")));
		IsReload = true;
		switch (ActivateWeapon)
		{
		case EWeaponType::AK47:
			ServerPrimaryReload();
			break;
		case EWeaponType::M4A1:
			ServerPrimaryReload();
			break;
		case EWeaponType::Sniper:
			ServerPrimaryReload();
			break;
		case EWeaponType::DersertEagle:
			ServerSecondaryReload();
			break;
		default:
			break;
		}
	}
}
void ASCharacter::ReloadDelayCallback()
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Delay!!!!!!!!!!!!!!!!")));
	/*int32 ClipGunCurrentAmmo = ServerPrimaryWeapon->ClipGunCurrentAmmo;
	int32 GunCurrentAmmo = ServerPrimaryWeapon->GunCurrentAmmo;
	int32 const MaxClipAmmo = ServerPrimaryWeapon->MaxClipAmmo;
	if (MaxClipAmmo - ClipGunCurrentAmmo >= GunCurrentAmmo)
	{
		ClipGunCurrentAmmo += GunCurrentAmmo;
		GunCurrentAmmo = 0;
	}
	else
	{
		GunCurrentAmmo -= MaxClipAmmo - ClipGunCurrentAmmo;
		ClipGunCurrentAmmo = MaxClipAmmo;
	}
	ServerPrimaryWeapon->ClipGunCurrentAmmo = ClipGunCurrentAmmo;
	ServerPrimaryWeapon->GunCurrentAmmo = GunCurrentAmmo;
	ClientUpdateAmmoUI(ClipGunCurrentAmmo, GunCurrentAmmo);*/

}
void ASCharacter::SetReload_Implementation()
{
	IsReload = false;
	AWeaponServer* ServerWeapon = GetCurrentServerWeapon();
	if (ServerWeapon)
	{
		int32 ClipGunCurrentAmmo = ServerWeapon->ClipGunCurrentAmmo;
		int32 GunCurrentAmmo = ServerWeapon->GunCurrentAmmo;
		int32 const MaxClipAmmo = ServerWeapon->MaxClipAmmo;
		if (MaxClipAmmo - ClipGunCurrentAmmo >= GunCurrentAmmo)
		{
			ClipGunCurrentAmmo += GunCurrentAmmo;
			GunCurrentAmmo = 0;
		}
		else
		{
			GunCurrentAmmo -= MaxClipAmmo - ClipGunCurrentAmmo;
			ClipGunCurrentAmmo = MaxClipAmmo;
		}
		ServerWeapon->ClipGunCurrentAmmo = ClipGunCurrentAmmo;
		ServerWeapon->GunCurrentAmmo = GunCurrentAmmo;
		ClientUpdateAmmoUI(ClipGunCurrentAmmo, GunCurrentAmmo);
	}
	
}
bool ASCharacter::SetReload_Validate()
{
	return true;
}
void ASCharacter::ServerPrimaryReload_Implementation()
{
	if (ServerPrimaryWeapon)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Reload111111111111111111111")));
		if (ServerPrimaryWeapon->GunCurrentAmmo > 0 && ServerPrimaryWeapon->ClipGunCurrentAmmo < ServerPrimaryWeapon->MaxClipAmmo)
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Reload222222222222222222222")));
			ClientReload();
			MultiReloadAnimation();
			ServerBodysAnimation->Montage_SetEndDelegate(ReloadMontageEndedDelegate);

			FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			ActionInfo.ExecutionFunction = TEXT("ReloadDelayCallback");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			UKismetSystemLibrary::Delay(this, ClientPrimaryWeapon->ClientArmsReloadAnimMontage->GetPlayLength(), ActionInfo);
		}
	}
	
}
bool ASCharacter::ServerPrimaryReload_Validate()
{
	return true;
}

void ASCharacter::ServerSecondaryReload_Implementation()
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SecondaryReload111111111111111111111")));
	if (ServerSecondaryWeapon)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("SecondaryReload111111111111111111111")));
		if (ServerSecondaryWeapon->GunCurrentAmmo > 0 && ServerSecondaryWeapon->ClipGunCurrentAmmo < ServerSecondaryWeapon->MaxClipAmmo)
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Reload222222222222222222222")));
			ClientReload();
			MultiReloadAnimation();
			ServerBodysAnimation->Montage_SetEndDelegate(ReloadMontageEndedDelegate);

			FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			ActionInfo.ExecutionFunction = TEXT("ReloadDelayCallback");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			UKismetSystemLibrary::Delay(this, ClientSecondaryWeapon->ClientArmsReloadAnimMontage->GetPlayLength(), ActionInfo);
		}
	}
}

bool ASCharacter::ServerSecondaryReload_Validate()
{
	return true;
}

void ASCharacter::ClientReload_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientWeapon();
	if (CurrentClientWeapon)
	{
		UAnimMontage* ClientReloadMontage = CurrentClientWeapon->ClientArmsReloadAnimMontage;
		ClientArmsAnimation->Montage_Play(ClientReloadMontage);
		CurrentClientWeapon->PlayReloadAnimation();
		//ClientArmsAnimation->Montage_SetEndDelegate(ReloadMontageEndedDelegate);
		
		
	}
}

bool ASCharacter::ClientReload_Validate()
{
	return true;
}

void ASCharacter::MultiReloadAnimation_Implementation()
{
	AWeaponServer* ServerWeapon = GetCurrentServerWeapon();
	if (ServerBodysAnimation)
	{
		if (ServerWeapon)
		{
			ServerBodysAnimation->Montage_Play(ServerWeapon->ServerBodysReloadAnimMontage);
			ServerWeapon->PlayReloadAnimation();
		}
	}
	
	
}

bool ASCharacter::MultiReloadAnimation_Validate()
{
	return true;
}

void ASCharacter::OnMeshBeginOverlapDamage_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Damage222222222222222222222")));
	ABullet* Bullet = Cast<ABullet>(OtherActor);
	AMissileBaseServer* Missile = Cast<AMissileBaseServer>(OtherActor);
	//ASCharacter* HitPlayer = Cast<ASCharacter>(SweepResult.Actor);
	if (Bullet)
	{
		AWeaponServer* AttackerWeapon = Cast<AWeaponServer>(Bullet->GetOwner());
		AAttackRobot* AttackRobote = Cast<AAttackRobot>(Bullet->GetInstigator());
		/*if (Bullet->IsPendingKill())
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Bullet is destory!!!!!!!!!!!!!!!!!!")));
		}*/
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("BulletOwner:%s"), *(Bullet->GetOwner())->GetName()));//actor如果被销毁，则无法获取Owner!!!!!
		if (AttackerWeapon != nullptr)
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("DamageByPlayer")));
			DamagePLayer(AttackerWeapon, Bullet->GetActorForwardVector(), SweepResult);
		}
		else if(AttackRobote != nullptr)
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("DamageByRobote")));
			AttackRobote->DamagePlayer(this, Bullet->GetActorForwardVector(), SweepResult);
		}
		else
		{
			//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("DamageByOther")));
			//AttackRobote->DamagePlayer(this, Bullet->GetActorForwardVector(), SweepResult);
		}
		Bullet->Destroy();
	}
	//else if(Missile)
	//{
	//	/*TArray<AActor*> IngoreActors;
	//	IngoreActors.Add(Missile);
	//	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("DamageByMissile")));
	//	UGameplayStatics::ApplyRadialDamage(this, Missile->Damage, SweepResult.Location, Missile->DamageRadius,UDamageType::StaticClass(),IngoreActors,
	//		Missile, Cast<APawn>(Missile->GetOwner())->GetController(), true, ECC_Visibility);
	//	Missile->Destroy();*/
	//}
}

bool ASCharacter::OnMeshBeginOverlapDamage_Validate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	return true;
}

//float ASCharacter::GetHealth()
//{
//	return Health;
//}
//
//void ASCharacter::SetHealth(float NewHealth)
//{
//	Health = NewHealth;
//}

void ASCharacter::ServerDeath_Implementation()
{
	
	Mesh->SetSimulatePhysics(true);
	FPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());
	if (FPSPlayerController)
	{
		ClientUpdateAmmoUI(0, 0); 
		FVector Location = this->GetActorLocation();
		FRotator Rotation = this->GetActorRotation();
		Location -= 50 * this->GetActorForwardVector();
		SpawnGhost(Location, Rotation);
		
	}
	this->DetachFromControllerPendingDestroy();
	DestoryFPArm();
	DestoryFPBody();
	//ServerPrimaryWeapon->MultiWeaponsDropped();
	
	
}

bool ASCharacter::ServerDeath_Validate()
{
	return true;
}

void ASCharacter::DestoryFPBody_Implementation()
{
	/*if (ServerBodysAnimation)
	{
		ServerBodysAnimation->Montage_Play(ServerBodysDeathAnimMontage);
		ServerBodysAnimation->Montage_SetEndDelegate(DeathMontageEndedDelegate);
	}*/
	DeathEffect();
	FLatentActionInfo ActionInfo;
	ActionInfo.CallbackTarget = this;
	ActionInfo.ExecutionFunction = TEXT("DestoryCharacter");
	ActionInfo.UUID = FMath::Rand();
	ActionInfo.Linkage = 0;
	UKismetSystemLibrary::Delay(this, 10, ActionInfo);

}

bool ASCharacter::DestoryFPBody_Validate()
{
	return true;
}

void ASCharacter::DestoryFPArm_Implementation()
{
	this->DetachFromControllerPendingDestroy();
	if (ClientPrimaryWeapon)
	{
		ClientPrimaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ClientPrimaryWeapon->Destroy();
	}
	FPArmMesh->DestroyComponent();
	Mesh->SetOwnerNoSee(false);
	SpringArmComp->TargetArmLength = DeadTargetArmLength;
	
}

bool ASCharacter::DestoryFPArm_Validate()
{
	return true;
}

void ASCharacter::SpawnGhost_Implementation(FVector const& Location, FRotator const& Rotation)
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("spawn11111111111")));
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = GetController();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGhost* Ghost = GetWorld()->SpawnActor<AGhost>(GhostBlueprint,
		Location,
		Rotation,
		SpawnInfo);
	if (GetController() && Ghost)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("spawn222222222222222222222%s"),*GetController()->GetName()));
		FPSPlayerController->Possess(Ghost);//该命令必须HasAuthority
	}
	else
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("spawn33333333")));
	}
}

bool ASCharacter::SpawnGhost_Validate(FVector const& Location, FRotator const& Rotation)
{
	return true;
}

void ASCharacter::DestoryCharacter()
{
	//ServerPrimaryWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//ServerPrimaryWeapon->MultiWeaponsDropped();
	if (ServerPrimaryWeapon)
	{
		ServerPrimaryWeapon->EquipOrFall_Weapon(false);
	}
	this->Destroy();
	//this->DetachFromControllerPendingDestroy();
}


void ASCharacter:: GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); 

	DOREPLIFETIME(ASCharacter, ServerPrimaryWeapon);
	DOREPLIFETIME(ASCharacter, testfloat);
	DOREPLIFETIME(ASCharacter, ReservePrimaryWeapon);
	DOREPLIFETIME(ASCharacter, ActivateWeapon);
	DOREPLIFETIME(ASCharacter, Health);
	DOREPLIFETIME(ASCharacter, IsReload);
	DOREPLIFETIME(ASCharacter, AimOffset);
	DOREPLIFETIME(ASCharacter, IsAiming);
	DOREPLIFETIME(ASCharacter, ServerSecondaryWeapon);
	DOREPLIFETIME(ASCharacter, IsQuietStep);
	DOREPLIFETIME(ASCharacter, IsNearVehicle);
	DOREPLIFETIME(ASCharacter, ActivateVehicleUI);
}

#pragma endregion

