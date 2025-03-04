// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/DecalComponent.h"
#include "WeaponBaseClient.h"
#include "WeaponServer.h"
#include "Bullet.h"
#include "MultiFPSPlayerController.h"
#include "Components/TimelineComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Blueprint/UserWidget.h"
#include <list>
#include "SCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
UCLASS()
class COOPGAME_427_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();
	DECLARE_MULTICAST_DELEGATE_OneParam(DelegateHasWeapon, bool);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	friend class AMultiFPSPlayerController;
#pragma region Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FPArmMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	UAIPerceptionStimuliSourceComponent* AIPerceptionStimuliSourceComp;

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	UAnimInstance* ClientArmsAnimation;

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	UAnimInstance* ServerBodysAnimation;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AMultiFPSPlayerController* FPSPlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentLife, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", ClampMin = 0))//Replicated
	float Health;

	
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AGhost> GhostBlueprint;

	UPROPERTY(EditAnywhere)
	EWeaponType TestWeapon;

	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool IsQuietStep;
	

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerBodysDeathAnimMontage;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> PlayerWidgetBPClass;
	
	UUserWidget* PlayerWidget;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool HasWeapon = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* CapsuleArmComponent;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerBodysSwitchPrimaryWeaponAnimMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerBodysSwitchSecondaryWeaponAnimMontage;
	



#pragma endregion


#pragma region Vehicle
public:
	UPROPERTY(ReplicatedUsing = OnRep_NearVehicle, BlueprintReadOnly)
	bool IsNearVehicle;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool ActivateVehicleUI;
	
	UFUNCTION()
	void OnRep_NearVehicle();
	
	UFUNCTION(Client,Reliable)
    void UpdateClientVehicleUI(bool IsActivate);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateVehicleUIBP(bool IsActivate);
	
	
	
#pragma endregion Vehicle
private:
#pragma region Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> InputMapping = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveForwardAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CarryingVehicles= nullptr;
	
	

#pragma endregion 
protected:
#pragma region InputEvent
	void MoveForward(float value);

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void MoveForwardEvent(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void CarryingVehiclesEvent(const FInputActionValue& Value);

	

	
	
	void MoveRight(float value);

	void JumpAction();

	void StopJumpingAction();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowSpeedWalk();
	void ServerLowSpeedWalk_Implementation();
	bool ServerLowSpeedWalk_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerNormalSpeedWalk();
	void ServerNormalSpeedWalk_Implementation();
	bool ServerNormalSpeedWalk_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void LowSpeedWalk();
	void LowSpeedWalk_Implementation();
	bool LowSpeedWalk_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void NormalSpeedWalk();
	void NormalSpeedWalk_Implementation();
	bool NormalSpeedWalk_Validate();

	void InputFirePressed();

	void InputFireReleased();

	void InputAimingPressed();

	void InputAimingReleased();


#pragma endregion

#pragma region NetWork
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireSecondaryWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireSecondaryWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireSecondaryWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireSniperWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireSniperWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	UFUNCTION(Client, Reliable, WithValidation)
	void ClientUpdateAmmoUI(int32 ClipGunCurrentAmmo,int32 GunCurrentAmmo);
	void ClientUpdateAmmoUI_Implementation(int32 ClipGunCurrentAmmo, int32 GunCurrentAmmo);
	bool ClientUpdateAmmoUI_Validate(int32 ClipGunCurrentAmmo, int32 GunCurrentAmmo);

	UFUNCTION(Client, Reliable, WithValidation)
	void ClientUpdateHealthUI();
	void ClientUpdateHealthUI_Implementation();
	bool ClientUpdateHealthUI_Validate();

	UFUNCTION()
	void OnRep_CurrentLife();//c++实现，Health改变时无法自动在服务器回调，需要手动调用NearVehicle

public:
	UFUNCTION(Server, Reliable)
	void PickEquipment(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	void PickEquipment_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	/*bool PickEquipment_Validate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);*/
	
	UFUNCTION(Server, Reliable)
	void SetUnActivate();
	
	UFUNCTION(Server, Reliable)
	void SetActivate();

#pragma endregion
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Weapon
//using ServerWeaponList = list<AWeaponServer*>;
public:
	UPROPERTY(Replicated, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<AWeaponServer*> WeaponList;

	UPROPERTY(Replicated, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	EWeaponType ActivateWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateClientPrimaryWeapon, BlueprintReadOnly, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	AWeaponServer* ServerPrimaryWeapon; 

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))//ReplicatedReplicatedUsing = OnRep_UpdateWeapon
	AWeaponServer* LastWeapon;

	//UPROPERTY(ReplicatedUsing = OnRep_UpdateClientSecondaryWeapon, BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))//ReplicatedReplicatedUsing = OnRep_UpdateWeapon
	//AWeaponServer* ServerSecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_UpdateClientSecondaryWeapon, BlueprintReadOnly, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	AWeaponServer* ServerSecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Updatetestfloat, BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))//ReplicatedReplicatedUsing = OnRep_UpdateWeapon
	float testfloat=100.0;

	UPROPERTY(Replicated, meta = (AllowPrivateAccess = "true"))//ReplicatedUsing = OnRep_UpdateReserveWeapon
	AWeaponServer* ReservePrimaryWeapon;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientPrimaryWeapon;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientSecondaryWeapon;

	EWeaponType Weapon;
public:
	UFUNCTION(Server, Reliable)//, WithValidation, Reliable
	void EquipPrimary(AWeaponServer* Weaponserver);
	void EquipPrimary_Implementation(AWeaponServer* Weaponserver);
	bool EquipPrimary_Validate(AWeaponServer* Weaponserver);

	UFUNCTION(Server, Reliable)//, WithValidation, Reliable
	void EquipSecondary(AWeaponServer* Weaponserver);
	void EquipSecondary_Implementation(AWeaponServer* Weaponserver);
	bool EquipSecondary_Validate(AWeaponServer* Weaponserver);
	
	UFUNCTION(Client,Reliable,BlueprintCallable)
	void ClientEquipFPArmsSecondary();
	void ClientEquipFPArmsSecondary_Implementation();
	bool ClientEquipFPArmsSecondary_Validate();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientEquipFPArmsPrimary();
	void ClientEquipFPArmsPrimary_Implementation();
	bool ClientEquipFPArmsPrimary_Validate();

	UFUNCTION(Client,Reliable,WithValidation)
	void ClientFire();
	void ClientFire_Implementation();
	bool ClientFire_Validate();

	/*UFUNCTION(Client, Reliable, BlueprintCallable, WithValidation)
	void ClientFire();
	void ClientFire_Implementation();
	bool ClientFrie_Validate();*/

	void StartWithKindOfWeapon();

	void PurchaseWeapon(EWeaponType WeaponType);

	AWeaponBaseClient* GetCurrentClientWeapon();

	AWeaponServer* GetCurrentServerWeapon();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShooting();
	void MultiShooting_Implementation();
	bool MultiShooting_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiBulletDecal(FVector Location,FRotator Rotation);
	void MultiBulletDecal_Implementation(FVector Location, FRotator Rotation);
	bool MultiBulletDecal_Validate(FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateFPArmsBlendPose(int NewBlendPoseIndex);

	UFUNCTION(Client, Reliable)
	void OnRep_UpdateClientPrimaryWeapon();
	void OnRep_UpdateClientPrimaryWeapon_Implementation();
	bool OnRep_UpdateClientPrimaryWeapon_Validate();

	UFUNCTION(Client, Reliable)
	void OnRep_UpdateClientSecondaryWeapon();
	void OnRep_UpdateClientSecondaryWeapon_Implementation();
	bool OnRep_UpdateClientSecondaryWeapon_Validate();

	UFUNCTION()
	void OnRep_UpdateReserveWeapon();

	UFUNCTION()
	void OnRep_Updatetestfloat();


	

#pragma endregion

#pragma region Fire
public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FRotator AimOffset;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool IsAiming;
	//计时器
	FTimerHandle AutoFireTimeHandle;

	void AutoFire();

	//发射子弹
	ABullet* SpawnBullet(bool IsMoving);

	//后坐力
	float RecoilXCrood = 0.0;
	FTimeline VerticalRecoilRecoveryTimeLine;
	FOnTimelineFloatStatic UpdateCameraRotationonDelegte;
	UFUNCTION()
	void UpdateCameraRotation();

	void SetRecoil();

	//步枪射击
	void FireWeaponPrimary();

	void StopWeaponPrimary();

	void RifleLinerTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	FVector CalSpreadRot(AWeaponServer* ServerWeapon);

	//手枪射击
	void FireWeaponSecondary();

	void StopWeaponSecondary();

	float SecondaryWeaponSpreadMin = 0;

	float SecondaryWeaponSpreadMax = 0;

	FVector CalPistolSpreadRot();

	UFUNCTION()
	void DelaySpreadSecondaryWeaponShootCallback();

	//狙击枪射击
	void FireWeaponSniper();

	void StopWeaponSniper();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerSetAiming(bool AimingState);
	void ServerSetAiming_Implementation(bool AimingState);
	bool ServerSetAiming_Validate(bool AimingState);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientAiming();
	void ClientAiming_Implementation();
	bool ClientAiming_Validate();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientEndAiming();
	void ClientEndAiming_Implementation();
	bool ClientEndAiming_Validate();


	UPROPERTY(VisibleAnyWhere, Category = "SniperUI")
	UUserWidget* WidgetScope;

	UFUNCTION()
	void DelaySniperShootCallBack();

	void DamagePLayer(AWeaponServer* AttackWeapon, FVector const& HitFromDirection, FHitResult const& HitInf);

	UFUNCTION(Server, Reliable, WithValidation)//NetMulticast
	void OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent*
			FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
	void OnHit_Implementation(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent*
		FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);
	bool OnHit_Validate(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent*
		FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	//由于SimulatedProxy无法获取controllerrotation，在服务器端计算player瞄准方向，并同步到客户端，但是客户端需要将相对角度范围变为0-360，需要手动转换为-180-180，服务器端不需要转换
	UFUNCTION(Server, UnReliable, BlueprintCallable)
	void GetAimOffsets();
	void GetAimOffsets_Implementation();
	
	UFUNCTION(Server, Reliable)
	void RadialDamage( AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser);
	void RadialDamage_Implementation(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin, const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser);


	//换弹
	UPROPERTY(Replicated)
	bool IsFiring;

	UPROPERTY(Replicated)
	bool IsReload;

	FOnMontageEnded ReloadMontageEndedDelegate;

	void InputReload();

	UFUNCTION()
	void ReloadDelayCallback();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void SetReload();
	void SetReload_Implementation();
	bool SetReload_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPrimaryReload();
	void ServerPrimaryReload_Implementation();
	bool ServerPrimaryReload_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSecondaryReload();
	void ServerSecondaryReload_Implementation();
	bool ServerSecondaryReload_Validate();

	UFUNCTION(Client, Reliable, WithValidation)
	void ClientReload();
	void ClientReload_Implementation();
	bool ClientReload_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiReloadAnimation();
	void MultiReloadAnimation_Implementation();
	bool MultiReloadAnimation_Validate();

	//伤害
	UFUNCTION(Server, Reliable, WithValidation)
	void OnMeshBeginOverlapDamage(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	void OnMeshBeginOverlapDamage_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	bool OnMeshBeginOverlapDamage_Validate(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	/*UFUNCTION(BlueprintCallable)
	float GetHealth();

	UFUNCTION(BlueprintCallable)
	void SetHealth(float NewHealth);*/

	//死亡
	float DeadTargetArmLength = 7000.0;
	FOnMontageEnded DeathMontageEndedDelegate;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDeath();
	void ServerDeath_Implementation();
	bool ServerDeath_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void DestoryFPBody();
	void DestoryFPBody_Implementation();
	bool DestoryFPBody_Validate();

	UFUNCTION()
	void DestoryCharacter();
	//void DestoryCharacter_Implementation();
	//bool DestoryCharacter_Validate();

	UFUNCTION(Client, Reliable, WithValidation)
	void DestoryFPArm();
	void DestoryFPArm_Implementation();
	bool DestoryFPArm_Validate();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SpawnGhost(FVector const& Location, FRotator const& Rotation);
	void SpawnGhost_Implementation(FVector const& Location, FRotator const& Rotation);
	bool SpawnGhost_Validate(FVector const& Location, FRotator const& Rotation);

	UFUNCTION(BlueprintImplementableEvent)
	void DeathEffect();
	

#pragma endregion
#pragma region SwitchWeapon
	FOnMontageEnded SwitchPrimaryWeaponAnimMontageEndedDelegate;

	UFUNCTION(Server, Reliable, WithValidation)
	void SwitchPrimaryWeapon();
	void SwitchPrimaryWeapon_Implementation();
	bool SwitchPrimaryWeapon_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void SwitchSecondaryWeapon();
	void SwitchSecondaryWeapon_Implementation();
	bool SwitchSecondaryWeapon_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void SwitchWeapon();
	void SwitchWeapon_Implementation();
	bool SwitchWeapon_Validate();

	UFUNCTION(Client, Reliable, WithValidation)
	void DestoryClientWeapon();
	void DestoryClientWeapon_Implementation();
	bool DestoryClientWeapon_Validate();

	UFUNCTION(Server, Reliable)
	void PlayServerSwitchPrimaryWeaponAnimMontage();
	void PlayServerSwitchPrimaryWeaponAnimMontage_Implementation();

	UFUNCTION(Server, Reliable)
	void PlayServerSwitchSecondaryWeaponAnimMontage();
	void PlayServerSwitchSecondaryWeaponAnimMontage_Implementation();
	bool PlayServerSwitchSecondaryWeaponAnimMontage_Validate();

	UFUNCTION(NetMulticast, Reliable)
	void PlayClientSwitchPrimaryWeaponAnimMontage();
	void PlayClientSwitchPrimaryWeaponAnimMontage_Implementation();
	bool PlayClientSwitchPrimaryWeaponAnimMontage_Validate();

	UFUNCTION(NetMulticast, Reliable)
	void PlayClientSwitchSecondaryWeaponAnimMontage();
	void PlayClientSwitchSecondaryWeaponAnimMontage_Implementation();
	bool PlayClientSwitchSecondaryWeaponAnimMontage_Validate();

#pragma endregion
};
