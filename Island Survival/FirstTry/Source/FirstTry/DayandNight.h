// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DayandNight.generated.h"

class ADirectionalLight;
class ASkyLight;


UCLASS()
class FIRSTTRY_API ADayandNight : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADayandNight();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:	
	// Called every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Refs")
	ADirectionalLight* SunLight = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Refs")
	ASkyLight* SkyLight = nullptr;

	// How long is a full day (24h) in REAL seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Settings", meta = (ClampMin = "1.0"))
	float DayLengthSeconds = 600.0f; // 10 minutes per day by default

	// Time of day normalized [0..1). 0 = midnight, 0.25 = sunrise-ish, 0.5 = noon, 0.75 = sunset-ish
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|State", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TimeOfDay01 = 0.35f;

	// Starts at day 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|State", meta = (ClampMin = "1"))
	int32 DayNumber = 1;

	// Optional: pause the system for debugging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Settings")
	bool bRunSystem = true;

	// Skylight recapture interval (seconds). Keep this > 0.5 to avoid heavy updates.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight|Settings", meta = (ClampMin = "0.1"))
	float SkyRecaptureInterval = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Refs")
	ADirectionalLight* MoonLight = nullptr;

	UPROPERTY(EditAnywhere, Category = "Night")
	float MoonIntensity = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Day")
	float SunIntensity = 10.0f;

	UFUNCTION(BlueprintCallable, Category = "DayNight")
	bool IsNightTime() const;

	UFUNCTION(BlueprintCallable, Category = "DayNight")
	float GetHoursUntilMorning(float MorningTime01 = 0.25f) const;

	UFUNCTION(BlueprintCallable, Category = "DayNight")
	void SkipToMorning(float MorningTime01 = 0.25f);	

private:
	float AccumulatedSeconds = 0.0f;

	FTimerHandle SkyRecaptureTimer;

	void ApplySunRotation();
	void RecaptureSky();

};
