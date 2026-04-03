// Fill out your copyright notice in the Description page of Project Settings.
#include "SurvivalComp.h"
#include "DayandNight.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "InventoryComponent.h"
USurvivalComp::USurvivalComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void USurvivalComp::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = Cast<ACharacter>(GetOwner());

	if (!PlayerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: PlayerRef NOT found"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: BeginPlay running on %s"), *PlayerRef->GetName());

	CacheDayNightActor();

	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
}

void USurvivalComp::CacheDayNightActor()
{
	if (DayNightRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: Using assigned DayNightRef"));
		return;
	}

	DayNightRef = Cast<ADayandNight>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADayandNight::StaticClass())
	);

	if (DayNightRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: Found DayNightRef automatically"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: DayNightRef NOT found"));
	}
}

void USurvivalComp::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableSurvivalSystem) return;

	UpdateSurvival(DeltaTime);
}

void USurvivalComp::UpdateSurvival(float DeltaTime)
{
	if (!PlayerRef) return;

	if (!DayNightRef)
	{
		CacheDayNightActor();
		if (!DayNightRef) return;
	}

	if (DayNightRef->DayLengthSeconds <= 0.0f) return;
	if (CurrentHealth <= 0.0f) return;

	const float GameHoursDelta = DeltaTime * (24.0f / DayNightRef->DayLengthSeconds);

	HoursSinceLastMeal += GameHoursDelta;
	HoursSinceLastDrink += GameHoursDelta;

	while (HoursSinceLastMeal >= HungerDamageIntervalHours)
	{
		CurrentHealth -= HungerDamageAmount;
		HoursSinceLastMeal -= HungerDamageIntervalHours;
		
	}

	while (HoursSinceLastDrink >= ThirstDamageIntervalHours)
	{
		CurrentHealth -= ThirstDamageAmount;
		HoursSinceLastDrink -= ThirstDamageIntervalHours;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	CheckForDeath();
}


float USurvivalComp::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f) return 0.0f;
	return CurrentHealth / MaxHealth;
}

float USurvivalComp::GetHungerPercent() const
{
	if (HungerDamageIntervalHours <= 0.0f)
	{
		return 1.0f;
	}

	return FMath::Clamp(
		1.0f - (HoursSinceLastMeal / HungerDamageIntervalHours),
		0.0f,
		1.0f
	);
}

float USurvivalComp::GetThirstPercent() const
{
	if (ThirstDamageIntervalHours <= 0.0f)
	{
		return 1.0f;
	}

	return FMath::Clamp(
		1.0f - (HoursSinceLastDrink / ThirstDamageIntervalHours),
		0.0f,
		1.0f
	);
}

void USurvivalComp::AdvanceSurvivalByHours(float HoursSkipped)
{
	if (!bEnableSurvivalSystem) return;
	if (CurrentHealth <= 0.0f) return;

	HoursSinceLastMeal += HoursSkipped;
	HoursSinceLastDrink += HoursSkipped;

	while (HoursSinceLastMeal >= HungerDamageIntervalHours)
	{
		CurrentHealth -= HungerDamageAmount;
		HoursSinceLastMeal -= HungerDamageIntervalHours;
	}

	while (HoursSinceLastDrink >= ThirstDamageIntervalHours)
	{
		CurrentHealth -= ThirstDamageAmount;
		HoursSinceLastDrink -= ThirstDamageIntervalHours;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	CheckForDeath();

	UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: Advanced survival by %f hours during sleep."), HoursSkipped);
}
bool USurvivalComp::Drinking()
{
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: InventoryComponent not found."));
		return false;
	}
	if (!Inventory->HasItem("CleanWater", 1))
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: No Clean Water in your inventory."));
		return false;
	}
	const bool Removed = Inventory->RemoveItem("CleanWater", 1);
	if (!Removed)
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: Failed to remove CleanWater from inventory."));
		return false;
	}

	HoursSinceLastDrink = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("SurvivalComp: Player drank water. Thirst timer reset."));
	return true;

}
bool USurvivalComp::Eating()
{
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: InventoryComponent not found."));
		return false;
	}
	if (!Inventory->HasItem("CookedFish", 1))
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: No Cooked Fish in your inventory."));
		return false;
	}
	const bool Removed = Inventory->RemoveItem("CookedFish", 1);
	if (!Removed)
	{
		UE_LOG(LogTemp, Warning, TEXT("SurvivalComp: Failed to remove CookedFish from inventory."));
		return false;
	}

	HoursSinceLastMeal = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("SurvivalComp: Player ate Fish. Hunger timer reset."));
	return true;

}
bool USurvivalComp::IsDead() const
{
	return CurrentHealth <= 0.0f;
}

void USurvivalComp::CheckForDeath()
{
	if (bGameOver)
	{
		return;
	}

	if (CurrentHealth <= 0.0f)
	{
		bGameOver = true;
		bEnableSurvivalSystem = false;
		OnPlayerDied.Broadcast();
	}
}