// Fill out your copyright notice in the Description page of Project Settings.

#include "Bullet.h"
#include "SCharacter.h"
#include "AttackRobot.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
//#include "NiagaraFunctionLibrary.h"
//#include "NiagaraSystem.h"
#include "AircraftBase.h"
#include "VehicleBase.h"
#include "Particles/ParticleSystem.h"
//#include "TimerManager.h"



// Sets default values
ABullet::ABullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	 
    if (!RootComponent)
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSceneComponent"));
    }
    
    if (!ProjectileMovementComponent)
    {
        // ???????????????????????
        ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
        ProjectileMovementComponent->SetUpdatedComponent(RootComponent);
        ProjectileMovementComponent->InitialSpeed = 5500.0f;
        ProjectileMovementComponent->MaxSpeed = 6500.0f;
        ProjectileMovementComponent->bRotationFollowsVelocity = true;
        ProjectileMovementComponent->bShouldBounce = true;
        ProjectileMovementComponent->bIsHomingProjectile = true;
        ProjectileMovementComponent->Bounciness = 0.3f;
        ProjectileMovementComponent->ProjectileGravityScale = 0.5f;
    }

    if (!ProjectileMeshComponent)
    {
        ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
        static ConstructorHelpers::FObjectFinder<UStaticMesh>Mesh(TEXT("StaticMesh'/Game/BluePrint/Weapon/Bullet/Sphere.Sphere'"));
        if (Mesh.Succeeded())
        {
            ProjectileMeshComponent->SetStaticMesh(Mesh.Object);
            //RootComponent = ProjectileMeshComponent;
        }

        /*static ConstructorHelpers::FObjectFinder<UMaterial>Material(TEXT("Material'/Game/BluePrint/Weapon/Bullet/M_BULLET.M_Bullet'"));
        if (Material.Succeeded())
        {
            ProjectileMaterialInstance = UMaterialInstanceDynamic::Create(Material.Object, ProjectileMeshComponent);
        }
        ProjectileMeshComponent->SetMaterial(0, ProjectileMaterialInstance);*/
        ProjectileMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        ProjectileMeshComponent->BodyInstance.SetCollisionProfileName(TEXT("Bullet"));
        //ProjectileMeshComponent->SetRelativeScale3D(FVector(0.09f, 0.09f, 0.09f));
        ProjectileMeshComponent->SetupAttachment(RootComponent);
        //ProjectileMeshComponent->OnComponentHit.AddDynamic(this, &ABullet::OnCompHit);
    }
    
   
    if (!CapsuleCollisionComponent)
    {
        // ????????м?????????
        CapsuleCollisionComponent= CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent2"));
        // ????????????????
        //CollisionComponent->InitSphereRadius(15.0f);
        // ??????????????????????????"Projectile"??
        CapsuleCollisionComponent->SetNotifyRigidBodyCollision(true);//????simulation generates hit event
        CapsuleCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CapsuleCollisionComponent->GetBodyInstance()->bNotifyRigidBodyCollision = true;
        CapsuleCollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Bullet"));
        CapsuleCollisionComponent->SetupAttachment(RootComponent);
        CapsuleCollisionComponent->SetRelativeRotation(FRotator(0, 0, 90));
        // ????????????????????
        //CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        //CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
        //CollisionComponent->SetSimulatePhysics(true);

        
        // ?????????????????????
        CapsuleCollisionComponent->SetGenerateOverlapEvents(true);
        CapsuleCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnOtherBeginOverlap);
        CapsuleCollisionComponent->bReturnMaterialOnMove = true;
        //RootComponent = CollisionComponent;
    }
    // X????????????
    InitialLifeSpan = 1.0f;
}


// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();
    CapsuleCollisionComponent->OnComponentHit.AddDynamic(this, &ABullet::OnCompHit);//Hit?????????????????л??Ч
	
}

// Called every frame
void ABullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABullet::LaunchProjectile_Implementation()
{

	ProjectileMovementComponent->SetVelocityInLocalSpace(FVector::ForwardVector * Speed);	//??????????????????????????????
	ProjectileMovementComponent->Activate();
}

bool ABullet::LaunchProjectile_Validate()
{
	return true;
}

void ABullet::FireInDirection_Implementation(const FVector& ShootDirection)
{
	ProjectileMovementComponent->Velocity = ShootDirection * ProjectileMovementComponent->InitialSpeed;
}

bool ABullet::FireInDirection_Validate(const FVector& ShootDirection)
{
	return true;
}



void ABullet::OnCompHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    AWeaponServer* Weapon = Cast<AWeaponServer>(OtherActor);
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Bullet Hit!!!")));
    if (OtherActor != this)
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Bullet Hit")));
       
        FRotator XRotator = UKismetMathLibrary::MakeRotFromX(Hit.Normal);
        //BulletDecal(Hit.Location, XRotator);
        AWeaponServer* ServerPrimaryWeapon = Cast<AWeaponServer>(GetOwner());
        if (ServerPrimaryWeapon)
        {
            /*UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ServerPrimaryWeapon->BulletDecalMaterial, FVector(8, 8, 8),
                Hit.Location, XRotator, 10);*/
            UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(),  BulletDecalMaterial, FVector(8, 8, 8),
                            Hit.Location, XRotator, 10);
            if (Decal)
            {
                //UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Bullet Hit")));
                Decal->SetFadeScreenSize(0);
            }
            
        //UGameplayStatics::ApplyPointDamage(Hit.Actor.Get(), ServerPrimaryWeapon->BaseDamage, this->GetActorForwardVector(), Hit, (this->GetOwner())->GetInstigatorController(), this, UDamageType::StaticClass());
        }
        SpawnBulletFX(Hit.Location,XRotator);
        OtherComp->AddImpulseAtLocation(ProjectileMovementComponent->Velocity * 50.0f, Hit.ImpactPoint);
    }

    //Destroy();
}

void ABullet::OnHit_Implementation(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    
    AAttackRobot* Robot = Cast<AAttackRobot>(OtherActor);
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Bullet Hit!!!!!!!!!!!!!!!!!!!!!!!!!!!")));
    if (OtherActor != this && (Robot || Cast<UStaticMesh>(OtherActor)))
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("Bullet Hit")));
        FRotator XRotator = UKismetMathLibrary::MakeRotFromX(Hit.Normal);
        BulletDecal(Hit.Location, XRotator);
        OtherComponent->AddImpulseAtLocation(ProjectileMovementComponent->Velocity * 100.0f, Hit.ImpactPoint);
        Destroy();
        
    }
   
    
    
}

bool ABullet::OnHit_Validate(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    return true;
}

void ABullet::OnOtherBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OVERLAP!!!!!")));
    ASCharacter* HitFPSCharacter = Cast<ASCharacter>(OtherActor);
    AVehicleBase* HitVehicle = Cast<AVehicleBase>(OtherActor);
    AAircraftBase* HitAircraft = Cast<AAircraftBase>(OtherActor);
    if (OtherActor != this)
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OVERLAP!!!!!,%s"), *(OtherActor->GetName())));
        SpawnBulletFX(OtherComp->GetComponentLocation(),UKismetMathLibrary::MakeRotFromX(SweepResult.Normal));
        OtherComp->AddImpulseAtLocation(ProjectileMovementComponent->Velocity * 50.0f, SweepResult.ImpactPoint);
        if (HitFPSCharacter)
        {
            
            //UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("OVERLAP!!!!!,%d"), (ProjectileMovementComponent->Velocity * 100.0f).Size()));
            //SweepResult.GetComponent()->AddImpulseAtLocation(ProjectileMovementComponent->Velocity * 100.0f, SweepResult.ImpactPoint, SweepResult.BoneName);
            //OverlapActor->DamagePLayer(this->GetActorForwardVector(), SweepResult);
            
        }
        else if(HitVehicle)
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("DamageVehicle!!!!!")));
            switch (BulletType)
            {
            case EBulletType::GunBullets:
                UGameplayStatics::ApplyPointDamage(SweepResult.GetActor(), Damage, this->GetActorForwardVector(), SweepResult,
            GetInstigator()->GetController(), GetOwner(), UDamageType::StaticClass());
                break;
            case EBulletType::ArtilleryShells:
                UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ArtilleryShells!!!!!")));
                TArray<AActor*> IgnoredActors = TArray<AActor*>();
                IgnoredActors.Add(this->GetOwner());
                /*UGameplayStatics::ApplyRadialDamage(OtherActor, Damage, OtherComp->GetComponentLocation(), DamageRadius, UDamageType::StaticClass(),
                    IgnoredActors,GetInstigator(), GetInstigatorController(), true);*/
                UGameplayStatics::ApplyRadialDamageWithFalloff(
                                                                GetWorld(),
                                                                Damage,
                                                                0, 
                                                                GetActorLocation(),
                                                                DamageRadius,
                                                                DamageRadius*2,
                                                                1, 
                                                                nullptr, 
                                                                IgnoredActors,
                                                                this,
                                                                this->GetInstigatorController(),
                                                                ECC_Visibility
                                                                );
                break;
            }
            
            Destroy();
        }
        
        
        
    }
    //Destroy();
    
}

bool ABullet::OnOtherBeginOverlap_Validate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    return true;
}

void ABullet::BulletDecal_Implementation(FVector Location, FRotator Rotation)
{
    AWeaponServer* ServerWeapon = Cast<AWeaponServer>(GetOwner());
    if (ServerWeapon)
    {
        /*UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ServerWeapon->BulletDecalMaterial, FVector(8, 8, 8),
            Location, Rotation, 10);*/
        UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BulletDecalMaterial, FVector(8, 8, 8),
                   Location, Rotation, 10);
        if (Decal)
        {
            Decal->SetFadeScreenSize(0.001);
        }
    } 
}

bool ABullet::BulletDecal_Validate(FVector Location, FRotator Rotation)
{
    return true;
}

void ABullet::SpawnBulletFX_Implementation(FVector Location, FRotator Rotation)
{
    if(ExplodeFlash)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ExplodeFlash,Location,Rotation,
            FVector(10,10,10),true,EPSCPoolMethod::None,
            true);
    }
    else
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ExplodeFlash is null")));
    }
    //UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),);
}

