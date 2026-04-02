// Fill out your copyright notice in the Description page of Project Settings.

#include "FishingPuzzleManager.h"
#include "Kismet/GameplayStatics.h"
#include "DayandNight.h"
#include "InventoryComponent.h"

// Sets default values for this component's properties
UFishingPuzzleManager::UFishingPuzzleManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UFishingPuzzleManager::BeginPlay()
{
	Super::BeginPlay();

	CacheDayNightActor();

	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (Inventory)
	{
		UncookedFish = Inventory->GetItemCount("UncookedFish");
	}
}

void UFishingPuzzleManager::CacheDayNightActor()
{
	if (DayNightRef)
	{
		return;
	}

	DayNightRef = Cast<ADayandNight>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADayandNight::StaticClass())
	);

	if (!DayNightRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("FishingPuzzleManager: DayandNight actor not found in level."));
	}
}

int32 UFishingPuzzleManager::GetCurrentDayNumber() const
{
	if (DayNightRef)
	{
		return DayNightRef->DayNumber;
	}

	return 1;
}

int32 UFishingPuzzleManager::GetFishingPuzzleGridSize() const
{
	const int32 Day = GetCurrentDayNumber();

	if (Day >= 1 && Day <= 2)
	{
		return 3;
	}

	if (Day >= 3 && Day <= 4)
	{
		return 4;
	}

	if (Day >= 5 && Day <= 7)
	{
		return 5;
	}

	// Fallback for later days
	return 5;
}

float UFishingPuzzleManager::GetFishingPuzzleTimeLimit() const
{
	const int32 Day = GetCurrentDayNumber();

	if (Day >= 1 && Day <= 2)
	{
		return 30.0f;
	}

	if (Day >= 3 && Day <= 4)
	{
		return 45.0f;
	}

	if (Day >= 5 && Day <= 7)
	{
		return 70.0f;
	}

	return 70.0f;
}

void UFishingPuzzleManager::HandleFishingPuzzleSolved()
{
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("FishingPuzzleManager: InventoryComponent not found on owner."));
		return;
	}

	Inventory->AddItem("UncookedFish", 2);
	UncookedFish = Inventory->GetItemCount("UncookedFish");

	UE_LOG(LogTemp, Log, TEXT("Fishing puzzle solved. UncookedFish is now: %d"), UncookedFish);
}

void UFishingPuzzleManager::AddUncookedFish(int32 Amount)
{
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("FishingPuzzleManager: InventoryComponent not found on owner."));
		return;
	}

	if (Amount > 0)
	{
		Inventory->AddItem("UncookedFish", Amount);
	}
	else if (Amount < 0)
	{
		Inventory->RemoveItem("UncookedFish", FMath::Abs(Amount));
	}

	UncookedFish = Inventory->GetItemCount("UncookedFish");

	UE_LOG(LogTemp, Log, TEXT("Added %d uncooked fish. UncookedFish is now: %d"), Amount, UncookedFish);
}

bool UFishingPuzzleManager::HasUncookedFish() const
{
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return UncookedFish > 0;
	}

	return Inventory->GetItemCount("UncookedFish") > 0;
}