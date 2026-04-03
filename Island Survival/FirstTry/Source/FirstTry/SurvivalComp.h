// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalComp.generated.h"

class ADayandNight;
class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerDied);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FIRSTTRY_API USurvivalComp : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivalComp();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

protected:
	UPROPERTY()
	ACharacter* PlayerRef;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Survival")
	ADayandNight* DayNightRef;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
	float HoursSinceLastMeal = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
	float HoursSinceLastDrink = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
	float HungerDamageIntervalHours = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
	float ThirstDamageIntervalHours = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
	float HungerDamageAmount = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
	float ThirstDamageAmount = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
	bool bEnableSurvivalSystem = true;

protected:
	void CacheDayNightActor();
	void UpdateSurvival(float DeltaTime);

public:
	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetHungerPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetThirstPercent() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void AdvanceSurvivalByHours(float HoursSkipped);

	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool Drinking();

	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool Eating();

	UPROPERTY(BlueprintAssignable, Category = "Survival")
	FPlayerDied OnPlayerDied;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival")
	bool bGameOver = false;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	bool IsDead() const;

	void CheckForDeath();
};