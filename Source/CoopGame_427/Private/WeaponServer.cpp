// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponServer.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "WeaponBaseClient.h"
#include "SCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AWeaponServer::AWeaponServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	WeaponMesh->SetOwnerNoSee(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//WeaponMesh->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh->SetCollisionProfileName(TEXT("Weapon"));
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetSimulatePhysics(true);
	

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetupAttachment(RootComponent);
	CapsuleComponent->SetRelativeRotation(FRotator(0, 0, 90));

	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionProfileName(TEXT("Weapon"));
	//CapsuleComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CapsuleComponent->SetGenerateOverlapEvents(true);
	HasSight = false;
	//CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeaponServer::OnOtherBeginOverlap);注释后需要重新启动项目
}

void AWeaponServer::OnOtherBeginOverlap_(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASCharacter* FPSCharacter = Cast<ASCharacter>(OtherActor);
	//if (FPSCharacter) {
	//	EquipOrFall_Weapon(true);
	//	switch (GradeofWeapon)
	//	{
	//	case EWeaponGrade::Primary:
	//		FPSCharacter->EquipPrimary(this);
	//		break;
	//	case EWeaponGrade::Secondary:
	//		FPSCharacter->EquipSecondary(this);
	//		break;
	//	case EWeaponGrade::Melee:
	//	case EWeaponGrade::Melee:
	//		break;
	//	case EWeaponGrade::ThrowWeapon:
	//		break;
	//	default:
	//		break;
	//	}
	//	
	//	//FPSCharacter->ClientEquipFPArmsPrimary();
	//	
	//}

}
//Equipment or falling weapon. change weapon attribute
void AWeaponServer::EquipOrFall_Weapon(bool sign)
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("EquipOrFall_Weapon")));
	if (sign) {
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetSimulatePhysics(false);
		CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else {
		this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetSimulatePhysics(true);
		CapsuleComponent->SetCollisionProfileName(TEXT("Weapon"));
	}
}

// Called when the game starts or when spawned
void AWeaponServer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AWeaponServer::MultiShootingEffect_Implementation()
{
	if (GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(),0))
	{
		//多播开火粒子效果
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, TEXT("Fire_FX_Slot"), FVector::ZeroVector,
			FRotator::ZeroRotator, FVector::OneVector, EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::None, true);
		
		//多播开火音效
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
	}
}

bool AWeaponServer::MultiShootingEffect_Validate()
{
	return true;
}

void AWeaponServer::MultiWeaponsDropped_Implementation()
{
	this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetSimulatePhysics(true);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	/*this->SetOwner(nullptr);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionProfileName(TEXT("Weapon"));
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("weapondrop %s"), *(WeaponMesh->GetCollisionProfileName().ToString())));
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleComponent->SetCollisionProfileName(TEXT("Weapon"));
	CapsuleComponent->SetSimulatePhysics(true);
	CapsuleComponent->SetEnableGravity(true);*/
}

bool AWeaponServer::MultiWeaponsDropped_Validate()
{
	return true;
}

void AWeaponServer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponServer, GunCurrentAmmo);
	DOREPLIFETIME(AWeaponServer, ClipGunCurrentAmmo);
}

