// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SwimComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class UCapsuleComponent;
class AWaterBody;
class UPrimitiveComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FIRSTTRY_API USwimComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USwimComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY() ACharacter* OwnerCharacter = nullptr;
	UPROPERTY() UCharacterMovementComponent* MoveComp = nullptr;
	UPROPERTY() UCapsuleComponent* CapsuleComp = nullptr;

	UPROPERTY() AWaterBody* CurrentWaterBody = nullptr;
	UPROPERTY() UPrimitiveComponent* CurrentWaterOverlapComp = nullptr;
	bool bInWater = false;

	// TO TUNE for later just for refrence//
	UPROPERTY(EditAnywhere, Category = "Swimming|Speeds") float SwimSpeed = 450.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|Speeds") float WalkSpeedInShallow = 250.f;

	UPROPERTY(EditAnywhere, Category = "Swimming|Buoyancy") float Buoyancy = 1.1f;
	UPROPERTY(EditAnywhere, Category = "Swimming|Buoyancy") float GravityScaleInWater = 0.2f;

	// If water depth is smaller than this, we treat it like “wading” not full swimming
	UPROPERTY(EditAnywhere, Category = "Swimming|Depth") float ShallowDepthThreshold = 80.f;

	// River current
	UPROPERTY(EditAnywhere, Category = "Swimming|Current") float RiverCurrentStrength = 600.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|Current") float OceanCurrentStrength = 150.f;

	// Surface hold (keeps neck above water and prevents sinking)
	UPROPERTY(EditAnywhere, Category = "Swimming|SurfaceHold") float NeckDownFromSurface = 10.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|SurfaceHold") float NeckDownFromTop = 25.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|SurfaceHold") float SurfaceHoldKp = 3500.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|SurfaceHold") float SurfaceHoldKd = 250.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|SurfaceHold") float MaxUpAccel = 12000.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|SurfaceHold") float MaxDownAccel = 12000.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|SurfaceHold") float MaxSinkSpeed = 30.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|SurfaceHold") float MaxRiseSpeed = 250.f;
	UPROPERTY(EditAnywhere, Category = "Swimming|SurfaceHold") float PseudoMass = 150.f;

	// Tune variables end here

private:
	// Overlap callbacks
	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Helpers
	void EnterWater(AWaterBody* WaterBody);
	void EnterWater(AWaterBody* WaterBody, UPrimitiveComponent* WaterOverlapComp);
	void ExitWater();

	float GetWaterSurfaceZAtLocation(const FVector& WorldLocation) const;
	float GetWaterDepthAtLocation(const FVector& WorldLocation) const;

	FVector GetCurrentDirectionAtLocation(AWaterBody* WaterBody, const FVector& WorldLocation) const;
	void ApplyCurrent(float DeltaTime);

	void ApplySurfaceHold(float DeltaTime);
};
