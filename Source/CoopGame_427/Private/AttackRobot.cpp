// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackRobot.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include <BehaviorTree/BehaviorTreeComponent.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <Perception/AIPerceptionComponent.h>
#include <Perception/AISenseConfig_Sight.h>
#include <Components/SphereComponent.h>
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"

// Sets default values
AAttackRobot::AAttackRobot()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

#pragma region Component
	/*CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	if (CollisionComponent) {
		CollisionComponent->SetupAttachment(RootComponent);
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
		//Mesh->SetSimulatePhysics(true);
		//Mesh->SetGenerateOverlapEvents(true);
		//Mesh->OnComponentBeginOverlap.AddDynamic(this, &ASCharacter::PickEquipment);
	}*/

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	//Mesh->SetSimulatePhysics(true);
	Mesh->SetNotifyRigidBodyCollision(true);//开启simulation generates hit event
	Mesh->SetNotifyRigidBodyCollision(true);
	Mesh->BodyInstance.SetInstanceNotifyRBCollision(true);
	Mesh->OnComponentCollisionSettingsChanged();//缺少导致hit event 失效
	Mesh->SetGenerateOverlapEvents(true);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	if (PointLight)
	{
		PointLight->SetupAttachment(CapsuleComponent);
		PointLight->SetLightColor(FColor(255, 0, 0));
		PointLight->SetAttenuationRadius(15);
	}

	RadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadiaForce"));
	if (RadialForce)
	{
		RadialForce->SetupAttachment(CapsuleComponent);
	}

	//BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoardComp"));

	//BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

	AiPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AiPerception"));
	AiConfigSight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AiConfigSight"));

	if (AiPerception)
	{
		AiConfigSight->SightRadius = 3000.0f;
		AiConfigSight->LoseSightRadius = 4000.0f;
		AiConfigSight->PeripheralVisionAngleDegrees = 90.0f;
		AiConfigSight->DetectionByAffiliation.bDetectNeutrals = true;
		AiConfigSight->DetectionByAffiliation.bDetectEnemies = true;
		AiConfigSight->DetectionByAffiliation.bDetectFriendlies = true;

		AiPerception->ConfigureSense(*AiConfigSight);
		AiPerception->SetDominantSense(UAISenseConfig_Sight::StaticClass());

	}


#pragma endregion
	Aggressivity = 0;
	CollisionAggressivity = 0;

}

// Called when the game starts or when spawned
void AAttackRobot::BeginPlay()
{
	Super::BeginPlay();

	Health = 100.0;

	OnTakePointDamage.AddDynamic(this, &AAttackRobot::OnAttack);
	Mesh->OnComponentHit.AddDynamic(this, &AAttackRobot::OnHit);//Hit事件的注册放到构造函数中会失效

	OnTakeRadialDamage.AddDynamic(this, &AAttackRobot::OnRadialDamage);

	
	
}

// Called every frame
void AAttackRobot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAttackRobot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAttackRobot::UpdateWalkSpeed(float NewSpeed)
{
	UCharacterMovementComponent* Movement = this->GetCharacterMovement();
	if (Movement != nullptr)
	{
		Movement->MaxWalkSpeed = NewSpeed;
	}
}

void AAttackRobot::RobotFire(FVector FireDirection, FVector SpawnLocation, FRotator SpawnRotation)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	//SpawnRotation.Roll += 90;

	UClass* BulletBP = StaticLoadClass(ABullet::StaticClass(), nullptr, TEXT("Blueprint'/Game/BluePrint/Robots/AssaultRobot/BP_AssaultRobotBullet.BP_AssaultRobotBullet_C'"));
	ABullet* Bullet = GetWorld()->SpawnActor<ABullet>(BulletBP, SpawnLocation + 100 * GetActorForwardVector(),
		SpawnRotation,
		SpawnInfo
		);
	if (Bullet)
	{
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("fire!!!!!!!!!!!!!!!!!!!!!")));
		Bullet->SetOwner(this);
		Bullet->SetInstigator(this);
		FVector LaunchDirection = SpawnRotation.Vector();
		Bullet->FireInDirection(LaunchDirection);
	}
}

void AAttackRobot::DamagePlayer(ASCharacter* AttackPlayer, FVector const& HitFromDirection, FHitResult const& HitInf)
{
	/*UPhysicalMaterial* HitPhysMaterial = HitInf.PhysMaterial.Get();
	float HitDamage = 0;*/
	if (AttackPlayer != nullptr)
	{
		switch (AttackType)
		{
		case EAttackType::Missile:
		{
			
			UGameplayStatics::ApplyPointDamage(HitInf.GetActor(), Aggressivity, HitFromDirection, HitInf,
				GetController(), this, UDamageType::StaticClass());
		}
		break;
		case EAttackType::Shoot:
		{
			UPhysicalMaterial* HitPhysMaterial = HitInf.PhysMaterial.Get();
			float HitDamage = 0;
			if (HitPhysMaterial)
			{
				switch (HitPhysMaterial->SurfaceType)
				{
				case EPhysicalSurface::SurfaceType1:
				{
					//head
					HitDamage = Aggressivity * 4;
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt head")));
				}
				break;
				case EPhysicalSurface::SurfaceType2:
				{
					//body
					HitDamage = Aggressivity * 1;
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt body")));
				}
				break;
				case EPhysicalSurface::SurfaceType3:
				{
					//arm
					HitDamage = Aggressivity * 0.8;
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt arm")));
				}
				break;
				case EPhysicalSurface::SurfaceType4:
				{
					//leg
					HitDamage = Aggressivity * 0.7;
					UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HIt leg")));
				}
				break;
				default:
					break;
				}
			}
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("actor,%s"),*(HitInf.GetActor())->GetName()));
			UGameplayStatics::ApplyPointDamage(AttackPlayer, HitDamage, HitFromDirection, HitInf,
				GetController(), this, UDamageType::StaticClass());
			
		}
		break;
		default:
			break;
		}
	}
}

void AAttackRobot::OnAttack_Implementation(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	Health -= Damage;
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("robot health %f"),Health));
}

bool AAttackRobot::OnAttack_Validate(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser)
{
	return true;
}

void AAttackRobot::OnRadialDamage_Implementation(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	Health = FMath::Min(Health - Damage, Health);
}

bool AAttackRobot::OnRadialDamage_Validate(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	FVector Origin, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	return true;
}

void AAttackRobot::OnHit_Implementation(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	ABullet* HitBullet = Cast<ABullet>(OtherActor);
	ASCharacter* CollisionPlayer = Cast<ASCharacter>(OtherActor);
	if (HitBullet)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HitBullet!!!!!!!!!!!!!!!!!!!!!!!!")));
		ASCharacter* HitPlayer = Cast<ASCharacter>(HitBullet->GetInstigator());
		AWeaponServer* HitWeapon = Cast<AWeaponServer>(HitBullet->GetOwner());
		//AWeaponServer* HitWeapon = Cast<AWeaponServer>(HitPlayer->ServerPrimaryWeapon);
		
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Robot:%s"), *(HitPlayer->ServerPrimaryWeapon->GetOwner())->GetName()));
		
		
		if (HitPlayer)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HitPlayer!!!!!!!!!!!!!!!!!!!!!!!!")));
			if (HitWeapon)
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Hit Robot!!!!!!!!!!!!!!!!!!!!!!!!")));
				UGameplayStatics::ApplyPointDamage(this, HitWeapon->BaseDamage, Hit.Normal, Hit,
				HitPlayer->GetController(), HitPlayer, UDamageType::StaticClass());
				HitBullet->Destroy();
			}
			
		}
		
	}
	else if (CollisionPlayer)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Collision Robot!!!!!!!!!!!!!!!!!!!!!!!!")));
		RadialForce->FireImpulse();
		UGameplayStatics::ApplyPointDamage(CollisionPlayer, CollisionAggressivity, Hit.Normal, Hit,
			GetController(), this, UDamageType::StaticClass());
		

	}
	else
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("UNknow Hit!!!!!!!!!!!!!!!!!!!!!!!!")));
	}

}

bool AAttackRobot::OnHit_Validate(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	return true;
}

