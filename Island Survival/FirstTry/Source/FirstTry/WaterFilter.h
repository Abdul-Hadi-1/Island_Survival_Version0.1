#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WaterFilter.generated.h"

class ADayandNight;
class UInventoryComponent;
class UUserWidget;

UENUM(BlueprintType)
enum class EFilterMaterial : uint8
{
	None      UMETA(DisplayName = "None"),
	Cloth     UMETA(DisplayName = "Cloth"),
	Charcoal  UMETA(DisplayName = "Charcoal"),
	Sand      UMETA(DisplayName = "Sand"),
	Gravel    UMETA(DisplayName = "Gravel")
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FIRSTTRY_API UWaterFilter : public UActorComponent
{
	GENERATED_BODY()

public:
	UWaterFilter();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

protected:
	UPROPERTY()
	ADayandNight* DayNightRef;

	UPROPERTY()
	UInventoryComponent* InventoryRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Puzzle")
	bool bIsPurifying = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Puzzle")
	bool bIsRevealPhase = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Puzzle")
	bool bPuzzleSolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Puzzle")
	bool bPuzzleFailed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Puzzle")
	float PuzzleTimeRemaining = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Puzzle")
	float RevealTimeRemaining = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Puzzle")
	TArray<EFilterMaterial> CorrectOrder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Puzzle")
	TArray<EFilterMaterial> PlayerOrder;

protected:
	void CacheDayNightActor();
	void CacheInventory();

public:
	UFUNCTION(BlueprintCallable, Category = "Water")
	int32 GetCurrentDayNumber() const;

	UFUNCTION(BlueprintCallable, Category = "Water")
	bool CanStartPurifying() const;

	UFUNCTION(BlueprintCallable, Category = "Water")
	void StartPurifying(UUserWidget* FocusWidget);

	UFUNCTION(BlueprintCallable, Category = "Water")
	void StopPurifying();

	UFUNCTION(BlueprintCallable, Category = "Water")
	void ResetPurificationPuzzle();

	UFUNCTION(BlueprintCallable, Category = "Water")
	void BuildCorrectOrder();

	UFUNCTION(BlueprintCallable, Category = "Water")
	void SubmitFilterMaterial(EFilterMaterial Material);

	UFUNCTION(BlueprintCallable, Category = "Water")
	bool IsPurificationSolved() const { return bPuzzleSolved; }

	UFUNCTION(BlueprintCallable, Category = "Water")
	bool HasPurificationFailed() const { return bPuzzleFailed; }

	UFUNCTION(BlueprintCallable, Category = "Water")
	bool IsPurifying() const { return bIsPurifying; }

	UFUNCTION(BlueprintCallable, Category = "Water")
	bool IsInRevealPhase() const { return bIsRevealPhase; }

	UFUNCTION(BlueprintCallable, Category = "Water")
	float GetPuzzleTimeRemaining() const { return PuzzleTimeRemaining; }

	UFUNCTION(BlueprintCallable, Category = "Water")
	float GetRevealTimeRemaining() const { return RevealTimeRemaining; }

	UFUNCTION(BlueprintCallable, Category = "Water")
	int32 GetPurificationPuzzleLayerCount() const;

	UFUNCTION(BlueprintCallable, Category = "Water")
	float GetPurificationPuzzleTimeLimit() const;

	UFUNCTION(BlueprintCallable, Category = "Water")
	float GetOrderRevealTime() const;

	UFUNCTION(BlueprintCallable, Category = "Water")
	TArray<EFilterMaterial> GetCorrectOrder() const { return CorrectOrder; }

	UFUNCTION(BlueprintCallable, Category = "Water")
	TArray<EFilterMaterial> GetPlayerOrder() const { return PlayerOrder; }

	UFUNCTION(BlueprintCallable, Category = "Water")
	void HandleWaterPurified();

	UFUNCTION(BlueprintCallable, Category = "Water")
	void HandlePurificationFailed();
};