#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CookingComponent.generated.h"

class ADayandNight;
class UFishingPuzzleManager;
class UUserWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FIRSTTRY_API UCookingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCookingComponent();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	ADayandNight* DayNightRef;

	UPROPERTY()
	UFishingPuzzleManager* FishingManagerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cooking Rewards")
	int32 CookedFish = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking State")
	bool bIsCooking = false;

protected:
	void CacheDayNightActor();
	void CacheFishingManager();

public:
	UFUNCTION(BlueprintCallable, Category = "Cooking")
	int32 GetCurrentDayNumber() const;

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	bool CanStartCooking() const;

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	bool HasCookedFish() const;

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	int32 GetCookedFish() const { return CookedFish; }

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	int32 GetCookingPuzzleGridSize() const;

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	float GetCookingPuzzleTimeLimit() const;

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void HandleCookingPuzzleSolved();

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void AddCookedFish(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	bool EatCookedFish();

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	bool IsCooking() const { return bIsCooking; }

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void StartCooking(UUserWidget* FocusWidget);

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void StopCooking();
};