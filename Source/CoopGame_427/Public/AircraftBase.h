// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "SCharacter.h"
#include "GameFramework/Pawn.h"
#include "AircraftBase.generated.h"

class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UJSBSimMovementComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class COOPGAME_427_API AAircraftBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAircraftBase();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
#pragma region Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UJSBSimMovementComponent> JSBSimMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FPCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* VehicleBoxComponent;

#pragma endregion

public:
	UPROPERTY(EditAnyWhere, Category = "UI")
	TSubclassOf<UUserWidget> WidgetBPClass;
	
	UPROPERTY(BlueprintReadWrite, Category = "UI")
	UUserWidget* Widget = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool Activate;

	void ActivateAircraft();

	void UnActivateAircraft();

	UFUNCTION(BlueprintImplementableEvent)
	void InitialExhaust();

	UFUNCTION(BlueprintImplementableEvent)
	void TurnOffExhaust();

	
	
#pragma region Action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> InputMapping = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ThrottleUpAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ThrottleDownAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RudderAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ElevatorAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AileronAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> FlapAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CenterAileronsRudderAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ParkingBrakeAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> GearAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AirBrakeAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CameraUpDownAction = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CameraLeftRightAction = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CameraSpringArmAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SwitchCameraAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> GetOfAircraftAction = nullptr;

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void ThrottleUp(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ThrottleDown(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void RudderTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ElevatorTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AileronTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void FlapTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void  CenterAileronsRudderTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void  ParkingBrakeTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void  GearTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void  AirBrakeTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void  CameraUpDownTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void  CameraLeftRightTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void  CameraSpringArmTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void  SwitchCameraTriggered(const FInputActionValue& Value);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void  GetOfAircraftTriggered(const FInputActionValue& Value);
	
#pragma endregion
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetExhaustNozzles(float Throttle);

	UFUNCTION(Server, Reliable)
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
						   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						   bool bFromSweep, const FHitResult& SweepResult);

	// Overlap end function
	UFUNCTION(Server, Reliable)
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
						 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);



};


