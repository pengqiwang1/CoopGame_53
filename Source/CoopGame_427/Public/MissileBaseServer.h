// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MissileBaseServer.generated.h"


class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UNiagaraComponent;


UCLASS()
class COOPGAME_427_API AMissileBaseServer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMissileBaseServer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
#pragma region Component
	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Bullet)
	class USphereComponent* CollisionComponent;*/
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile")
	class UCapsuleComponent* CapsuleCollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missile", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> FX;
#pragma endregion
#pragma region Attack
	UPROPERTY(Replicated, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool IsActivate;

	UPROPERTY(Replicated, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	AActor* AttackTarget;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float Speed;

	UPROPERTY(EditAnywhere)
	float Range;

	UPROPERTY(EditAnywhere)
	float Damage;
	
	UPROPERTY(EditAnywhere)
	float DamageRadius;
	
	UPROPERTY(EditAnywhere, Category = "Particles")
	UParticleSystem* ExplodeParticleEffect;
	
	

#pragma endregion Attack	
public:
	UFUNCTION(Server,BlueprintCallable, Reliable)
	void LockTarget();
	bool LockTarget_Validate();
	
	UFUNCTION(Server, Reliable)
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	void OnHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(Server, Reliable)
	void OnOverLapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void OnOverLapped_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void DestroyMissile();



};
