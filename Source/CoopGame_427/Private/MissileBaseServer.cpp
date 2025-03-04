// Fill out your copyright notice in the Description page of Project Settings.
#include "MissileBaseServer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include <Kismet\GameplayStatics.h>
#include "Components/CapsuleComponent.h"
#include "AircraftBase.h"





// Sets default values
AMissileBaseServer::AMissileBaseServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
#pragma region Component
	/*CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	// 设置球体的碰撞半径。
	CollisionComponent->InitSphereRadius(15.0f);
	// 将球体的碰撞配置文件名称设置为"Projectile"。
	CollisionComponent->SetNotifyRigidBodyCollision(true);//开启simulation generates hit event
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->GetBodyInstance()->bNotifyRigidBodyCollision = true;
	CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Bullet"));
	CollisionComponent->SetupAttachment(this->RootComponent);*/
	
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	//SkeletalMeshComponent->SetNotifyRigidBodyCollision(true);//开启simulation generates hit event
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//SkeletalMeshComponent->BodyInstance.bNotifyRigidBodyCollision = true;
	//SkeletalMeshComponent->BodyInstance.SetCollisionProfileName(TEXT("Missile"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);

	

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(SkeletalMeshComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	FX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FX"));
	FX->SetupAttachment(SkeletalMeshComponent);

	CapsuleCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollisionComponent"));
	CapsuleCollisionComponent->SetNotifyRigidBodyCollision(true);//开启simulation generates hit event
	CapsuleCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CapsuleCollisionComponent->GetBodyInstance()->bNotifyRigidBodyCollision = true;
	CapsuleCollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Missile"));
	CapsuleCollisionComponent->BodyInstance.bUseCCD=true;
	CapsuleCollisionComponent->SetupAttachment(SkeletalMeshComponent);
#pragma endregion

}



// Called when the game starts or when spawned
void AMissileBaseServer::BeginPlay()
{
	Super::BeginPlay();
	CapsuleCollisionComponent->OnComponentBeginOverlap.AddDynamic(this,&AMissileBaseServer::OnOverLapped);
	CapsuleCollisionComponent->OnComponentHit.AddDynamic(this, &AMissileBaseServer::OnHit);//Hit事件的注册放到构造函数中会失效
	
}

// Called every frame
void AMissileBaseServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsActivate)
	{
		LockTarget();
	}

}

void AMissileBaseServer::LockTarget_Implementation()
{
	if (AttackTarget!=nullptr)
	{
		if (!FX->IsActive())
		{
			FX->Activate();
		}
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("attacktarget:%f"),*AttackTarget->GetName()));
		FVector MissileLocation = GetActorLocation();
		FVector TargetLocation = AttackTarget->GetActorLocation();
		FRotator MissileUpdateRotation = UKismetMathLibrary::FindLookAtRotation(MissileLocation,TargetLocation);
		
		SetActorRotation(MissileUpdateRotation);
		//->SetActorRotation(MissileUpdateRotation);

		FVector MissileForwardVector = MissileUpdateRotation.Vector(); 
		FVector MissileVelocity = MissileForwardVector*Speed;
		SkeletalMeshComponent->SetPhysicsLinearVelocity(MissileVelocity);
		SetActorRotation(MissileUpdateRotation);
	}

}

bool AMissileBaseServer::LockTarget_Validate()
{
	return true;
}

void AMissileBaseServer::OnHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor!=nullptr && Cast<AAircraftBase>(OtherActor) == nullptr)
	{ 
	TArray<AActor*> IngoreActors;
	IngoreActors.Add(this);
	IngoreActors.Add(GetOwner());
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("DamageByMissile:%s"),*OtherActor->GetName()));
	UGameplayStatics::ApplyRadialDamage(this, Damage, Hit.Location, DamageRadius, UDamageType::StaticClass(), IngoreActors,
		this, this->GetInstigatorController(), true, ECC_Visibility);
	if (OtherActor!=this)
	{
		Destroy();
	}
	 
	}
	
}

void AMissileBaseServer::OnOverLapped_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor!=nullptr && OtherActor!=this->GetOwner())
	{
		TArray<AActor*> IngoreActors;
		IngoreActors.Add(this);
		IngoreActors.Add(GetOwner());
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OVERLAPDamageByMissile:%s"),*OtherActor->GetName()));
		/*static ConstructorHelpers::FClassFinder<UDamageType> MissileDamage(TEXT("/Game/Vehicles/Aircraft/F16C/Weapon/DT_Missile.DT_Missile_C"));
		if (MissileDamage.Succeeded())
		{
			UGameplayStatics::ApplyRadialDamage(this, Damage, SweepResult.Location, DamageRadius, MissileDamage.Class, IngoreActors,
						this, this->GetInstigatorController(), true, ECC_Visibility);
			
		}*/
		UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(),
		Damage,
		0, 
        GetActorLocation(),
        DamageRadius,
        DamageRadius*2,
		1, 
		nullptr, 
		IngoreActors,
		this,
		this->GetInstigatorController(),
		ECC_Visibility
	    );
		
		if (OtherActor!=this)
		{
			
			UGameplayStatics::SpawnEmitterAttached(ExplodeParticleEffect, SkeletalMeshComponent, TEXT("cap_jnt"), FVector::ZeroVector,
		FRotator::ZeroRotator, FVector(5,5,5), EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::None, true);
			FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			ActionInfo.ExecutionFunction = TEXT("DestroyMissile");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			UKismetSystemLibrary::Delay(this, 1, ActionInfo);
		
		}
	 
	}
}

void AMissileBaseServer::DestroyMissile()
{
	Destroy();
}


void AMissileBaseServer:: GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); 

	DOREPLIFETIME(AMissileBaseServer, IsActivate);
}
