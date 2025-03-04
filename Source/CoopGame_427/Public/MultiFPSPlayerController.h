// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Ghost.h"
#include "MultiFPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_427_API AMultiFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:

	//AMultiFPSPlayerController();
	 
	UPROPERTY(Replicated, BlueprintReadOnly)
	TArray<APawn*> VehicleList;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	APawn* CurrentVehicle;

	APawn* ControlPlayer; 

	void PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShake);

	FInputModeGameAndUI InputMode;

	UFUNCTION(BlueprintImplementableEvent,Category="PlayerUI")
	void CreatePlayerUI(TSubclassOf<UUserWidget> PlayerWidgetBPClass);

	UFUNCTION(BlueprintImplementableEvent,Category="PlayerUI")
	void DestoryPlayerUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerUI")
	void DoCrossHairRecoll();

	UFUNCTION(BlueprintImplementableEvent, Category = "PlayerUI")
	void UpdateAmmoUI(int32 ClipGunCurrentAmmo,int32 GunCurrentAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "PlayerUI")
	void UpdateHealthUI(float Health);

	UFUNCTION(Server, Reliable,BlueprintCallable)
	void UpVehicles(APawn* Vehicle);

	UFUNCTION(Server, Reliable)
	void DownVehicles();
};
