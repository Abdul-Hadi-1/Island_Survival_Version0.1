// Fill out your copyright notice in the Description page of Project Settings.

#include "DayandNight.h"
#include "Engine/World.h"
#include "TimerManager.h"

#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"

//  ADD these (needed for SetIntensity + math)
#include "Components/DirectionalLightComponent.h"
#include "Math/UnrealMathUtility.h"

// Sets default values
ADayandNight::ADayandNight()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADayandNight::BeginPlay()
{
	Super::BeginPlay();
    
	/*if (DayNumber <= 1)
	{
		TimeOfDay01 = 0.25f; // Sunrise
	}*/

	// Skylight recapture on a timer (not every frame)
	if (SkyLight && SkyRecaptureInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			SkyRecaptureTimer,
			this,
			&ADayandNight::RecaptureSky,
			SkyRecaptureInterval,
			true
		);
	}

	ApplySunRotation();
}

// Called every frame
void ADayandNight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bRunSystem) return;
	if (DayLengthSeconds <= 0.f) return;

	// Advance time
	AccumulatedSeconds += DeltaTime;

	const float Delta01 = DeltaTime / DayLengthSeconds;
	TimeOfDay01 += Delta01;

	// Handle day rollover
	while (TimeOfDay01 >= 1.0f)
	{
		TimeOfDay01 -= 1.0f;
		DayNumber++;
		// If you want: UE_LOG(LogTemp, Warning, TEXT("New Day: %d"), DayNumber);
	}

	ApplySunRotation();
}

void ADayandNight::ApplySunRotation()
{
	if (!SunLight) return;

	// Correct mapping:
	// 0.00 = midnight
	// 0.25 = sunrise
	// 0.50 = noon
	// 0.75 = sunset
	// 1.00 = midnight

	const float TimeAngle = (TimeOfDay01 - 0.25f) * 2.0f * PI;

	// Sun height:
	// -1 = midnight
	//  0 = sunrise/sunset
	//  1 = noon
	const float SunHeight = FMath::Sin(TimeAngle);

	// Convert height to pitch
	const float SunPitch = -SunHeight * 90.0f;

	FRotator NewRot = SunLight->GetActorRotation();
	NewRot.Pitch = SunPitch;
	SunLight->SetActorRotation(NewRot);

	// ===== Light intensity =====
	const float DayFactor = FMath::Clamp((SunHeight + 0.1f) / 0.2f, 0.0f, 1.0f);

	if (UDirectionalLightComponent* SunComp = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent()))
	{
		SunComp->SetIntensity(FMath::Lerp(0.0f, SunIntensity, DayFactor));
	}

	const float NightFactor = 1.0f - DayFactor;

	// Moon rotation + intensity (only if MoonLight is set)
	if (MoonLight)
	{
		const float MoonPitch = SunPitch + 180.0f;

		FRotator MoonRot = MoonLight->GetActorRotation();
		MoonRot.Pitch = MoonPitch;
		MoonLight->SetActorRotation(MoonRot);

		if (UDirectionalLightComponent* MoonComp = Cast<UDirectionalLightComponent>(MoonLight->GetLightComponent()))
		{
			MoonComp->SetIntensity(MoonIntensity * NightFactor);
		}
	}
}
bool ADayandNight::IsNightTime() const
{
	return (TimeOfDay01 < 0.20f || TimeOfDay01 > 0.75f);
}

float ADayandNight::GetHoursUntilMorning(float MorningTime01) const
{
	float FractionUntilMorning = 0.0f;

	if (TimeOfDay01 <= MorningTime01)
	{
		FractionUntilMorning = MorningTime01 - TimeOfDay01;
	}
	else
	{
		FractionUntilMorning = (1.0f - TimeOfDay01) + MorningTime01;
	}

	return FractionUntilMorning * 24.0f;
}

void ADayandNight::SkipToMorning(float MorningTime01)
{
	if (TimeOfDay01 > MorningTime01)
	{
		DayNumber++;
	}

	TimeOfDay01 = MorningTime01;

	ApplySunRotation();
	RecaptureSky();
}

void ADayandNight::RecaptureSky()
{
	if (!SkyLight) return;

	if (USkyLightComponent* SkyComp = Cast<USkyLightComponent>(SkyLight->GetLightComponent()))
	{
		SkyComp->RecaptureSky();
	}
}