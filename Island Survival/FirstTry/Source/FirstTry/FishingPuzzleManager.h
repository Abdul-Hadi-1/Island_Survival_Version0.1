// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FishingPuzzleManager.generated.h"

class ADayandNight;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FIRSTTRY_API UFishingPuzzleManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFishingPuzzleManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing Rewards")
	int32 UncookedFish = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fishing")
	ADayandNight* DayNightRef = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void CacheDayNightActor();

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	int32 GetCurrentDayNumber() const;

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	int32 GetFishingPuzzleGridSize() const;

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void HandleFishingPuzzleSolved();

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void AddUncookedFish(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	float GetFishingPuzzleTimeLimit() const;

	UFUNCTION(BlueprintCallable, Category = "Fishing Rewards")
	bool HasUncookedFish() const;
};