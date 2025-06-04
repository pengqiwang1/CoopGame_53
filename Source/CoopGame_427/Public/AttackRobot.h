// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Bullet.h"
#include "SCharacter.h"
#include "WeaponServer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AttackRobot.generated.h"

UENUM()
enum class EAttackType :uint8
{
	Shoot UMETA(DisplayName = "Shoot"),
	Suicide UMETA(DisplayName = "Suicide"),
	Missile UMETA(DisplayName = "Missile"),

};

UCLASS()
class COOPGAME_427_API AAttackRobot : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAttackRobot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
#pragma region Component
public:


	/*UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Robot, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CollisionComponent;*/

	

	// 行为树与黑板类
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Blackboard", meta = (AllowPrivateAccess = "true"))
	//class UBlackboardComponent* BlackboardComp;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Behavior", meta = (AllowPrivateAccess = "true"))
	//class UBehaviorTreeComponent* BehaviorComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBehaviorTree* BehaviorTree;
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Robot)
	class UPointLightComponent* PointLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Robot)
	class URadialForceComponent* RadialForce;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|PawSensing")
	class UAIPerceptionComponent* AiPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|PawSensing")
	class UAISenseConfig_Sight* AiConfigSight;


#pragma endregion

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn|BehaviorTree")
	UBehaviorTree* DefaultBehaviorTreeAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn|BehaviorTree")
	UBlackboardData* DefaultBlackboardAsset;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn|Weapon")
	//bool HasWeapon;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	EAttackType AttackType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))//ReplicatedReplicatedUsing = OnRep_UpdateWeapon
	AWeaponServer* ServerPrimaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn|Weapon")
	float Aggressivity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn|Weapon")
	float CollisionAggressivity;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn|Weapon")
	float CollisionAggressivityRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn|Weapon")
	float Health;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pawn|Weapon")
	UClass* BulletBP;
	

public:
	UFUNCTION(BlueprintCallable)
	void UpdateWalkSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable)
	void RobotFire(FVector FireDirection, FVector SpawnLocation, FRotator SpawnRotation);

	void DamagePlayer(ASCharacter* AttackPlayer, FVector const& HitFromDirection, FHitResult const& HitInf);

	UFUNCTION(Server, Reliable, WithValidation)
	void OnAttack(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent*
			FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
	void OnAttack_Implementation(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent*
		FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
	bool OnAttack_Validate(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent*
		FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void OnRadialDamage( AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(Server, Reliable, WithValidation)
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
	void OnHit_Implementation(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
	bool OnHit_Validate(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);



};
