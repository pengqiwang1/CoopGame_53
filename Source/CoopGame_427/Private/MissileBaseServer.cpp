// Fill out your copyright notice in the Description page of Project Settings.
#include "MissileBaseServer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Components/SceneCaptureComponent2D.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include <Kismet\GameplayStatics.h>
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AircraftBase.h"


// Sets default values
AMissileBaseServer::AMissileBaseServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
#pragma region Component
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	 
	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
		//RootComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	}
	if(!SkeletalMeshComponent)
	{
		SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
		//SkeletalMeshComponent->SetNotifyRigidBodyCollision(true);//开启simulation generates hit event
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//SkeletalMeshComponent->BodyInstance.bNotifyRigidBodyCollision = true;
		//SkeletalMeshComponent->BodyInstance.SetCollisionProfileName(TEXT("Missile"));
		SkeletalMeshComponent->SetupAttachment(RootComponent);
		SkeletalMeshComponent->SetMobility(EComponentMobility::Movable);
		
	}
	if (!CapsuleCollisionComponent)
	{
		CapsuleCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollisionComponent"));
		CapsuleCollisionComponent->SetNotifyRigidBodyCollision(true);//开启simulation generates hit event
		CapsuleCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CapsuleCollisionComponent->GetBodyInstance()->bNotifyRigidBodyCollision = true;
		CapsuleCollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Missile"));
		CapsuleCollisionComponent->BodyInstance.bUseCCD=true;
		CapsuleCollisionComponent->SetupAttachment(SkeletalMeshComponent);
		CapsuleCollisionComponent->SetMobility(EComponentMobility::Movable);
	}
	

	

	/*SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(SkeletalMeshComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);*/
	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	SceneCaptureComponent2D->SetupAttachment(SkeletalMeshComponent);

	FX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FX"));
	FX->SetupAttachment(SkeletalMeshComponent);
	if (!ProjectileMovementComponent)
	{
		// ???????????????????????
		ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
		ProjectileMovementComponent->SetUpdatedComponent(SkeletalMeshComponent);
		ProjectileMovementComponent->InitialSpeed = 0.0f;
		ProjectileMovementComponent->MaxSpeed = 150000.0f;
		ProjectileMovementComponent->bRotationFollowsVelocity = true;
		ProjectileMovementComponent->bShouldBounce = true;
		ProjectileMovementComponent->bIsHomingProjectile = true;
		ProjectileMovementComponent->Bounciness = 0.3f;
		ProjectileMovementComponent->ProjectileGravityScale = 0.5f;
		ProjectileMovementComponent->bSweepCollision=true;
        
	}
	// X????????????
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(RootComponent);
	RadialForceComp->Radius = 500.0f; // 冲击波半径
	RadialForceComp->ImpulseStrength = 2000.0f; // 冲击力强度
	RadialForceComp->bImpulseVelChange = true; // 是否忽略物体质量，直接改变速度
	RadialForceComp->bAutoActivate = false; // 是否自动激活
	RadialForceComp->bIgnoreOwningActor = true; // 是否忽略自身

	InitialLifeSpan = 100.0f;
	
#pragma endregion

}



// Called when the game starts or when spawned
void AMissileBaseServer::BeginPlay()
{
	Super::BeginPlay();
	CapsuleCollisionComponent->OnComponentBeginOverlap.AddDynamic(this,&AMissileBaseServer::OnOverLapped);
	CapsuleCollisionComponent->OnComponentHit.AddDynamic(this, &AMissileBaseServer::OnHit);//Hit事件的注册放到构造函数中会失效
	OnDestroyed.AddDynamic(this, &AMissileBaseServer::OnMissileDestroyed);
	
}

// Called every frame
void AMissileBaseServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsActivate)
	{
		LockTarget(DeltaTime);
	}

}

void AMissileBaseServer::LockTarget_Implementation(float DeltaTime)
{
	if (AttackTarget!=nullptr)
	{
		if (!FX->IsActive())
		{
			FX->Activate();
		}
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("attacktarget:%f"),*AttackTarget->GetName()));
		/*FVector MissileLocation = GetActorLocation();
		FVector TargetLocation = AttackTarget->GetActorLocation();
		FRotator MissileUpdateRotation = UKismetMathLibrary::FindLookAtRotation(MissileLocation,TargetLocation);*/
		FVector ToTarget = AttackTarget->GetActorLocation() - GetActorLocation();
		float TravelTime = ToTarget.Size() / Speed;
		FVector PredictedPos = AttackTarget->GetActorLocation() + AttackTarget->GetVelocity() * TravelTime;

		// 3. 指定预测后的位置为加速方向
		FRotator MissileUpdateRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(),PredictedPos);

		// 插值旋转，限制转向速度
		MissileUpdateRotation = FMath::RInterpConstantTo(GetActorRotation(), MissileUpdateRotation, DeltaTime, RotationSpeed);
		SetActorRotation(MissileUpdateRotation);
		/*MissileUpdateRotation.Pitch = FMath::Clamp(MissileUpdateRotation.Pitch, -MaxSteeringAngle.Pitch, MaxSteeringAngle.Pitch);
		MissileUpdateRotation.Yaw = FMath::Clamp(MissileUpdateRotation.Yaw, -MaxSteeringAngle.Yaw, MaxSteeringAngle.Yaw);
		MissileUpdateRotation.Roll = FMath::Clamp(MissileUpdateRotation.Roll, -MaxSteeringAngle.Roll, MaxSteeringAngle.Roll);*/
		/*FVector MissileVelocity = MissileUpdateRotation.Vector()*Speed;
		FRotator MissileForwardVector =MissileVelocity.Rotation();*/
		//SkeletalMeshComponent->SetPhysicsLinearVelocity(MissileVelocity);
		FVector NewLoc  = GetActorLocation() + GetActorForwardVector() * Speed * DeltaTime;
		SetActorLocation(NewLoc);
	}

}

/*void AMissileBaseServer::LockTarget_Implementation()
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("attacktarget:")));
	if(AttackTarget!=nullptr)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("attacktarget:%f"),*AttackTarget->GetName()));
		USkeletalMeshComponent* TargetMesh = AttackTarget->FindComponentByClass<USkeletalMeshComponent>();
		if (TargetMesh && ProjectileMovementComponent)
		{
			ProjectileMovementComponent->Activate();
			ProjectileMovementComponent->HomingTargetComponent = TargetMesh;
			ProjectileMovementComponent->bIsHomingProjectile = true;
			ProjectileMovementComponent->HomingAccelerationMagnitude = 10000.f; // 根据需要调整
			FVector ToTarget = AttackTarget->GetActorLocation() - GetActorLocation();
			float TravelTime = ToTarget.Size() / ProjectileMovementComponent->MaxSpeed;
			FVector PredictedPos = AttackTarget->GetActorLocation() + AttackTarget->GetVelocity() * TravelTime;

			// 3. 指定预测后的位置为加速方向
			ProjectileMovementComponent->Velocity = (PredictedPos - GetActorLocation()).GetSafeNormal()
										   * ProjectileMovementComponent->MaxSpeed;
			
		}
	}
}*/

bool AMissileBaseServer::LockTarget_Validate(float DeltaTime)
{
	return true;
}

void AMissileBaseServer::OnHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("HitDamageByMissile:%s"),*OtherActor->GetName()));
	if (OtherActor!=nullptr && Cast<AAircraftBase>(OtherActor) == nullptr)
	{ 
		
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
		/*TArray<AActor*> IngoreActors;
		IngoreActors.Add(this);
		IngoreActors.Add(GetOwner());
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OVERLAPDamageByMissile:%s"),*OtherActor->GetName()));
		UGameplayStatics::ApplyRadialDamageWithFalloff(
		GetWorld(),
		Damage,
		Damage/2, 
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
	    */
		
		if (OtherActor!=this)
		{
			
			/*UGameplayStatics::SpawnEmitterAttached(ExplodeParticleEffect, SkeletalMeshComponent, TEXT("cap_jnt"), FVector::ZeroVector,
		FRotator::ZeroRotator, FVector(5,5,5), EAttachLocation::KeepRelativeOffset, true, EPSCPoolMethod::None, true);
			*/
			/*FLatentActionInfo ActionInfo;
			ActionInfo.CallbackTarget = this;
			ActionInfo.ExecutionFunction = TEXT("DestroyMissile");
			ActionInfo.UUID = FMath::Rand();
			ActionInfo.Linkage = 0;
			UKismetSystemLibrary::Delay(this, 1, ActionInfo);*/
		
		}
		Destroy();
	}
}

void AMissileBaseServer::DestroyMissile()
{
	Destroy();
}

void AMissileBaseServer::OnMissileDestroyed_Implementation(AActor* DestroyedActor)
{
	UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OnMissileDestroyed")));
	RadialForceComp->FireImpulse();
	UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),                     // WorldContextObject
			ExplodeParticleEffect,               // EmitterTemplate
			DestroyedActor->GetActorLocation(),            // FVector Location
			FRotator::ZeroRotator,         // FRotator Rotation
			FVector(10.f,10.f,10.f),
			true,                          // bAutoDestroy
			EPSCPoolMethod::AutoRelease,   // PoolingMethod
			true                           // bAutoActivateSystem
		);
	TArray<AActor*> IngoreActors;
	IngoreActors.Add(this);
	IngoreActors.Add(GetOwner());
	UGameplayStatics::ApplyRadialDamage(this, Damage, GetActorLocation(), DamageRadius, UDamageType::StaticClass(), IngoreActors,
		this, this->GetInstigatorController(), true, ECC_Visibility);
	UKismetRenderingLibrary::ClearRenderTarget2D(
	this,                   // UObject* WorldContextObject
	SceneCaptureComponent2D->TextureTarget,         // UTextureRenderTarget2D* TextureRenderTarget
	FLinearColor::Black     // FLinearColor ClearColor
	);
}

void AMissileBaseServer:: GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); 

	DOREPLIFETIME(AMissileBaseServer, IsActivate);
}
