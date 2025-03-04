// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "ChaosVehicleRearWheelBase.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_427_API UChaosVehicleRearWheelBase : public UChaosVehicleWheel
{
	GENERATED_BODY()
public:
	UChaosVehicleRearWheelBase()
	{
		WheelRadius = 35.f;
		WheelWidth = 20.f;
		bAffectedByHandbrake = true;
		MaxSteerAngle = 0.f; // 后轮不转向
	}
};
