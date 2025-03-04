// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "ChaosVehicleFrontWheelBase.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_427_API UChaosVehicleFrontWheelBase : public UChaosVehicleWheel
{
	GENERATED_BODY()
	UChaosVehicleFrontWheelBase()
	{
		WheelRadius = 35.f;
		WheelWidth = 20.f;
		bAffectedByHandbrake = false;
		MaxSteerAngle = 35.f; // 前轮转向角度
	}
};
