// Fill out your copyright notice in the Description page of Project Settings.
#include "MultiFPSPlayerController.h"

#include "AircraftBase.h"
#include "Net/UnrealNetwork.h"
#include "Ghost.h"
#include "SCharacter.h"
#include "VehicleBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


//AMultiFPSPlayerController::AMultiFPSPlayerController() 
//{
//	
//	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
//	InputMode.SetHideCursorDuringCapture(true);
//	SetInputMode(InputMode);
//}
void AMultiFPSPlayerController::BeginPlay()
{
	ControlPlayer=GetPawn();
	Super::BeginPlay();
	
}

void AMultiFPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMultiFPSPlayerController, VehicleList);
	DOREPLIFETIME(AMultiFPSPlayerController, CurrentVehicle);
}


void AMultiFPSPlayerController::PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShake)
{
	ClientStartCameraShake(CameraShake, 1, ECameraShakePlaySpace::CameraLocal, FRotator::ZeroRotator);
}

void AMultiFPSPlayerController::DownVehicles_Implementation()
{
	if (CurrentVehicle != nullptr)
	{
		//¥Ó≥À‘ÿæﬂ
		if(ControlPlayer)
		{
			ASCharacter* SCharacter = Cast<ASCharacter>(ControlPlayer);
			AAircraftBase* Aircraft = Cast<AAircraftBase>(CurrentVehicle);
			AVehicleBase* Vehicle = Cast<AVehicleBase>(CurrentVehicle);
			if (Aircraft)
			{
				Aircraft->Activate=false;
				Aircraft->UnActivateAircraft();
			}
			if (Vehicle)
			{
				Vehicle->Activate=false;
				Vehicle->UnActivateVehicle();
			}
			if (SCharacter)
			{
				UnPossess();
				Possess(SCharacter);
				CreatePlayerUI(SCharacter->PlayerWidgetBPClass);
				SCharacter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				FTransform VehicleTransform = CurrentVehicle->GetActorTransform();
				//TArray<UBoxComponent*> FoundVehicleBoxComponents;
				//FoundVehicleBoxComponents = CurrentVehicle->GetComponentsByTag(UBoxComponent::StaticClass(), FName(TEXT("Vechicle")));
				FVector DownLocation = FVector(0,0,0);
				UBoxComponent* BoxComponent = CurrentVehicle->FindComponentByClass<UBoxComponent>();
				if (BoxComponent)
				{
					// ªÒ»°BoxComponentµƒ≥ﬂ¥Á£®∞Î≥ﬂ¥Á£©
					FVector BoxExtent = BoxComponent->GetScaledBoxExtent();
					DownLocation = UKismetMathLibrary::TransformLocation(VehicleTransform,
					FVector(0,BoxExtent[1]-10,300));//317.000000,260.000000,300
				}
				else
				{
					DownLocation = UKismetMathLibrary::TransformLocation(VehicleTransform,
						FVector(317.000000,260.000000,300));
				}
				SCharacter->SetActorLocation(DownLocation);
				SCharacter->SetActorRotation(VehicleTransform.Rotator());
				SCharacter->SetActivate();
				SCharacter->ClientUpdateHealthUI();
				AWeaponServer* CurrentWeapon = SCharacter->GetCurrentServerWeapon();
				if (CurrentWeapon)
				{
					SCharacter->ClientUpdateAmmoUI(CurrentWeapon->ClipGunCurrentAmmo, CurrentWeapon->GunCurrentAmmo);
				}
				
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT(" Pawn is nullptr."));
			}
		}
	}
	CurrentVehicle = nullptr;
}

void AMultiFPSPlayerController::UpVehicles_Implementation(APawn* Vehicle)
{
	
	if (CurrentVehicle == nullptr)
	{
		//¥Ó≥À‘ÿæﬂ
		
		if(Vehicle)
		{
			
			ASCharacter* VehiclePlayer = Cast<ASCharacter>(GetPawn());
			ControlPlayer=VehiclePlayer;
			if (Player)
			{
				AAircraftBase* Aircraft = Cast<AAircraftBase>(Vehicle);
				AVehicleBase* TargetVehicle = Cast<AVehicleBase>(Vehicle);
				if (Aircraft)
				{
					CurrentVehicle = Aircraft;
					Aircraft->Activate=true;
					VehiclePlayer->SetUnActivate();
					UnPossess();
					Possess(CurrentVehicle);
					Aircraft->ActivateAircraft();
				}
				if (TargetVehicle)
				{
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("TargetVehicle")));
					CurrentVehicle = TargetVehicle;
					TargetVehicle->Activate=true;
					VehiclePlayer->SetUnActivate();
					UnPossess();
					Possess(CurrentVehicle);
					TargetVehicle->ActivateVehicle();
				}
				if (CurrentVehicle)
				{
					DestoryPlayerUI();
					USkeletalMeshComponent* SkeletalMeshComponent = Vehicle->FindComponentByClass<USkeletalMeshComponent>();
					if (SkeletalMeshComponent)
					{
						VehiclePlayer->K2_AttachToComponent(SkeletalMeshComponent, TEXT("VehicleSocket"), EAttachmentRule::SnapToTarget,
						EAttachmentRule::SnapToTarget,
						EAttachmentRule::SnapToTarget,
						true);
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("FPSPlayerController or Pawn is nullptr."));
			}
			
			
		}
	}
	
}
