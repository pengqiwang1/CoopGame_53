// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h" // 如果使用 Chaos Vehicles
#include "DestroyVehicle.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "VehicleBase.generated.h"

UENUM()
enum class EVehucleType :uint8
{
	WheeledVehicle UMETA(DisplayName = "WheeledVehicle"),
	CrawlerVehicle UMETA(DisplayName = "CrawlerVehicle"),
	None UMETA(DisplayName = "None")
	
};

UCLASS()
class COOPGAME_427_API AVehicleBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehicleBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* CollisionBoxComponent;
	
	// Skeletal mesh for the vehicle
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* VehicleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	// Movement component for vehicle physics
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UChaosWheeledVehicleMovementComponent* VehicleMovement;

	UPROPERTY(EditAnywhere)
	EVehucleType VehicleType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class ADestroyVehicle> DestroyVehicleBPClass;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentLife, BlueprintReadOnly, EditAnywhere,meta = (AllowPrivateAccess = "true", ClampMin = 0))//Replicated
	float Health;

	UPROPERTY(EditAnyWhere, Category = "UI")
	TSubclassOf<UUserWidget> WidgetBPClass;
	
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* Widget = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Client, Reliable)
	void CreateClientUI();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI")
	void CreateUI();
	
	UFUNCTION(Client, Reliable)
	void ClientUpdateHealthUI();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UI")
	void UpdateHealthUI();
private:
	UFUNCTION(Server, Reliable)
	void RadialDamage( AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin,
		const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(Server, Reliable)
	void DestroyVehicle();

	

#pragma region Action
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> InputMapping = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SetThrottleAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SetStearAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SetBreakAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SetHnadreakAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SetReverseGearAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SetGearAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CameraUpDownAction = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CameraLeftRightAction = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CameraSpringArmAction = nullptr;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	int CurrentGear;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	int MaxGear;

	float ThrottleValue = 0.0f;  // 当前油门值
	float MaxThrottle = 1.0f;   // 最大油门值
	float ThrottleIncrement = 1.0f; // 油门每次累加值

	float StearValue = 0.0f;  // 当前转向值
	float MaxStear = 1.0f;   // 最大转向值
	float StearIncrement = 1.0f; // 转向每次累加值

	UFUNCTION()
	void OnRep_CurrentLife();

	UFUNCTION()
	void SetThrottle(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent)
	void SetStear(const FInputActionValue& Value);

	UFUNCTION()
	void SetBreak(const FInputActionValue& Value);

	UFUNCTION()
	void SetHandBreak(const FInputActionValue& Value);

	UFUNCTION()
	void SetReverseGear(const FInputActionValue& Value);

	UFUNCTION()
	void SetGear(const FInputActionValue& Value);

	UFUNCTION()
	void  CameraUpDownTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void  CameraLeftRightTriggered(const FInputActionValue& Value);

	UFUNCTION()
	void  CameraSpringArmTriggered(const FInputActionValue& Value);
#pragma endregion 

	
};
