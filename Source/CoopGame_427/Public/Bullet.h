// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "GameFramework/Actor.h"
//#include "SCharacter.h"
#include "Bullet.generated.h"

class  SCharacter;

UENUM()
enum class EBulletType :uint8
{
	GunBullets UMETA(DisplayName = "GunBullets"),
	ArtilleryShells UMETA(DisplayName = "ArtilleryShells"),
	None UMETA(DisplayName = "None")
};
UCLASS()
class COOPGAME_427_API ABullet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, Category = Bullet)
	UStaticMeshComponent* ProjectileMeshComponent;

	UPROPERTY(EditAnywhere, Category = Bullet)
	UMaterialInstanceDynamic* ProjectileMaterialInstance;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* BulletDecalMaterial;//»÷ÖÐÌù»­

	UPROPERTY(EditAnywhere, Category = Bullet)
	class UCapsuleComponent* CapsuleCollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Bullet)
	class UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Bullet)
	class UParticleSystemComponent* ParticleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Bullet)
	class URadialForceComponent* RadialForceComp;

	UPROPERTY(EditAnywhere, Category = Bullet)
	float Speed;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = Bullet)
	UParticleSystem* ExplodeFlash;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = Bullet)
	UNiagaraSystem* ExplodeFX;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = Bullet)
	float Damage;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = Bullet)
	float DamageRadius;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = Bullet)
	EBulletType BulletType;

public:
	UFUNCTION(Server, Reliable, WithValidation,BlueprintCallable)
	void LaunchProjectile();
	void LaunchProjectile_Implementation();
	bool LaunchProjectile_Validate();

	UFUNCTION(Server, Reliable, WithValidation,BlueprintCallable)
	void FireInDirection(const FVector& ShootDirection);
	void FireInDirection_Implementation(const FVector& ShootDirection);
	bool FireInDirection_Validate(const FVector& ShootDirection);

	UFUNCTION(Server, Reliable, WithValidation)
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
	void OnHit_Implementation(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
	bool OnHit_Validate(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION(Server, Reliable)
	void OnCompHit(UPrimitiveComponent* HitComp,AActor* OtherActor,UPrimitiveComponent* OtherComp,FVector NormalImpulse,const FHitResult& Hit);
	void OnCompHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(Server, Reliable, WithValidation)
	void OnOtherBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	void OnOtherBeginOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	bool OnOtherBeginOverlap_Validate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void BulletDecal(FVector Location, FRotator Rotation);
	void BulletDecal_Implementation(FVector Location, FRotator Rotation);
	bool BulletDecal_Validate(FVector Location, FRotator Rotation);

	UFUNCTION(NetMulticast,Unreliable)
	void SpawnBulletFX(FVector Location, FRotator Rotation);

};
