// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.h"
#include "WeaponServer.generated.h"

UENUM()
enum class EWeaponType:uint8
{
	AK47 UMETA(DisplayName="AK47"),
	M4A1 UMETA(DisplayName = "M4A1"),
	DersertEagle UMETA(DisplayName = "DersertEagle"),
	Sniper UMETA(DisplayName = "Sniper"),
	None UMETA(DisplayName = "None")
};

UENUM()
enum class EWeaponGrade :uint8
{
	Primary UMETA(DisplayName = "Primary"),
	Secondary UMETA(DisplayName = "Secondary"),
	Melee UMETA(DisplayName = "Melee"),
	ThrowWeapon UMETA(DisplayName = "ThrowWeapon"),
	None UMETA(DisplayName = "None")
	
};



UCLASS()
class COOPGAME_427_API AWeaponServer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponServer();
	UPROPERTY(EditAnywhere)
	EWeaponType KindofWeapon;

	UPROPERTY(EditAnywhere)
	bool HasSight;

	UPROPERTY(EditAnywhere)
	EWeaponGrade GradeofWeapon;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	USkeletalMeshComponent* WeaponMesh;
	//UPROPERTY(EditAnywhere)
	//class USphereComponent* SphereComponent;
	UPROPERTY(EditAnywhere)
	class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AWeaponBaseClient> ClientWeaponBaseBPClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABullet> BulletBP;

	UFUNCTION()
	void OnOtherBeginOverlap_(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EquipOrFall_Weapon(bool sign);

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	UPROPERTY(Replicated, EditAnywhere)
	int32 GunCurrentAmmo;//剩余子弹数

	UPROPERTY(Replicated, EditAnywhere)
	int32 ClipGunCurrentAmmo;//弹夹剩余子弹数

	UPROPERTY(EditAnywhere)
	int32 MaxClipAmmo;//弹夹容量

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerBodysShootAnimMontage;//开火动画

	UPROPERTY(EditAnywhere)
	float BulletDistance;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* BulletDecalMaterial;//击中贴画

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseDamage;

	UPROPERTY(EditAnywhere)
	float AutoFireRate;

	UPROPERTY(EditAnywhere)
	bool IsAutoFire;

	UPROPERTY(EditAnywhere)
	UCurveFloat* VerticalRecoilCurve;

	UPROPERTY(EditAnywhere)
	UCurveFloat* VerticalRecoilRecoveryCurve;

	UPROPERTY(EditAnywhere)
	UCurveFloat* HorizontalRecoilCurve;

	UPROPERTY(EditAnywhere)
	float MovingFireXRandomRange;

	UPROPERTY(EditAnywhere)
	float MovingFireYRandomRange;

	UPROPERTY(EditAnywhere)
	float MovingFireZRandomRange;

	//手枪后坐力递增幅度
	UPROPERTY(EditAnywhere)
	float SecondaryWeaponSpreadMinIndex = 0;

	UPROPERTY(EditAnywhere)
	float SecondaryWeaponSpreadMaxIndex = 0;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerBodysReloadAnimMontage;

	UFUNCTION(BlueprintImplementableEvent, Category = "FPGunAnimation")
	void PlayReloadAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "FPGunAnimation")
	void PlayFireAnimation();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShootingEffect();
	void MultiShootingEffect_Implementation();
	bool MultiShootingEffect_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiWeaponsDropped();
	void MultiWeaponsDropped_Implementation();
	bool MultiWeaponsDropped_Validate();

};
