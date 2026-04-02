// Fill out your copyright notice in the Description page of Project Settings.

#include "SwimComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Water (UE5.7)
#include "WaterBodyActor.h"
#include "WaterBodyComponent.h"
#include "WaterBodyRiverActor.h"
#include "WaterBodyOceanActor.h"

// Sets default values for this component's properties
USwimComponent::USwimComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void USwimComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("SwimComponent BeginPlay: Owner=%s"), *GetNameSafe(GetOwner()));

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	MoveComp = OwnerCharacter->GetCharacterMovement();
	CapsuleComp = OwnerCharacter->GetCapsuleComponent();

	if (!MoveComp || !CapsuleComp) return;

	// Bind overlap events so we can detect entering/leaving water
	CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &USwimComponent::OnCapsuleBeginOverlap);
	CapsuleComp->OnComponentEndOverlap.AddDynamic(this, &USwimComponent::OnCapsuleEndOverlap);
}

void USwimComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CapsuleComp)
	{
		CapsuleComp->OnComponentBeginOverlap.RemoveAll(this);
		CapsuleComp->OnComponentEndOverlap.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void USwimComponent::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap fired with Actor=%s Comp=%s"),
		*GetNameSafe(OtherActor), *GetNameSafe(OtherComp));

	if (!OtherActor) return;

	AWaterBody* WaterBody = Cast<AWaterBody>(OtherActor);
	if (!WaterBody) return;

	EnterWater(WaterBody, OtherComp);
}

void USwimComponent::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	AWaterBody* WaterBody = Cast<AWaterBody>(OtherActor);
	if (!WaterBody) return;

	if (WaterBody == CurrentWaterBody)
	{
		ExitWater();
	}
}

void USwimComponent::EnterWater(AWaterBody* WaterBody)
{
	EnterWater(WaterBody, nullptr);
}

void USwimComponent::EnterWater(AWaterBody* WaterBody, UPrimitiveComponent* WaterOverlapComp)
{
	UE_LOG(LogTemp, Warning, TEXT("ENTER WATER TRIGGERED"));

	if (!OwnerCharacter || !MoveComp) return;

	bInWater = true;
	CurrentWaterBody = WaterBody;
	CurrentWaterOverlapComp = WaterOverlapComp;

	const float Depth = GetWaterDepthAtLocation(OwnerCharacter->GetActorLocation());
	UE_LOG(LogTemp, Warning, TEXT("Depth=%.2f Threshold=%.2f"), Depth, ShallowDepthThreshold);

	// IMPORTANT CHANGE:
	// Use FLYING while in water so floor/ground does not "win" (your logs show MovementMode=1 which is Walking).
	// You can still drive swim animations from your own state.
	MoveComp->SetMovementMode(MOVE_Flying);

	// Base swim settings (kept as-is)
	MoveComp->Buoyancy = Buoyancy;
	MoveComp->GravityScale = GravityScaleInWater;
	MoveComp->BrakingDecelerationSwimming = 1024.f;

	// Shallow vs deep: just adjust speed (kept as-is)
	if (Depth <= ShallowDepthThreshold)
	{
		MoveComp->MaxSwimSpeed = WalkSpeedInShallow;
	}
	else
	{
		MoveComp->MaxSwimSpeed = SwimSpeed;
	}

	UE_LOG(LogTemp, Warning, TEXT("SWIMMING: MovementMode=%d MaxSwimSpeed=%.1f Buoyancy=%.2f GravityScale=%.2f"),
		(int32)MoveComp->MovementMode, MoveComp->MaxSwimSpeed, MoveComp->Buoyancy, MoveComp->GravityScale);

	// Apply immediate surface hold so you do not get the "delay"
	ApplySurfaceHold(0.f);
}

void USwimComponent::ExitWater()
{
	if (!OwnerCharacter || !MoveComp) return;

	bInWater = false;
	CurrentWaterBody = nullptr;
	CurrentWaterOverlapComp = nullptr;

	MoveComp->SetMovementMode(MOVE_Walking);

	MoveComp->MaxWalkSpeed = 500.f;
	MoveComp->GravityScale = 1.0f;
}

// Called every frame
void USwimComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInWater || !OwnerCharacter || !MoveComp || !CurrentWaterBody) return;

	// IMPORTANT CHANGE:
	// Run surface hold whenever we are in water
	ApplySurfaceHold(DeltaTime);

	// Apply current only when in flying/swimming-like state (kept similar behavior)
	if (MoveComp->MovementMode == MOVE_Flying || MoveComp->MovementMode == MOVE_Swimming)
	{
		ApplyCurrent(DeltaTime);
	}

	// If anything flips us out of Flying while still in water, force it back
	if (bInWater && MoveComp->MovementMode != MOVE_Flying)
	{
		MoveComp->SetMovementMode(MOVE_Flying);
	}
}

float USwimComponent::GetWaterSurfaceZAtLocation(const FVector& WorldLocation) const
{
	if (!CurrentWaterBody) return WorldLocation.Z;

	UWaterBodyComponent* BodyComp = CurrentWaterBody->GetWaterBodyComponent();
	if (!BodyComp) return WorldLocation.Z;

	FVector SurfaceLoc = FVector::ZeroVector;
	FVector SurfaceNormal = FVector::UpVector;
	FVector WaterVelocity = FVector::ZeroVector;
	float WaterDepth = 0.f;

	const bool bOk = BodyComp->GetWaterSurfaceInfoAtLocation(
		WorldLocation,
		SurfaceLoc,
		SurfaceNormal,
		WaterVelocity,
		WaterDepth,
		true
	);

	return bOk ? SurfaceLoc.Z : WorldLocation.Z;
}

float USwimComponent::GetWaterDepthAtLocation(const FVector& WorldLocation) const
{
	if (CapsuleComp)
	{
		const float SurfaceZ = GetWaterSurfaceZAtLocation(WorldLocation);
		const float Half = CapsuleComp->GetScaledCapsuleHalfHeight();
		const float BottomZ = WorldLocation.Z - Half;
		return FMath::Max(0.f, SurfaceZ - BottomZ);
	}

	if (!CurrentWaterBody) return 0.f;

	UWaterBodyComponent* BodyComp = CurrentWaterBody->GetWaterBodyComponent();
	if (!BodyComp) return 0.f;

	FVector SurfaceLoc = FVector::ZeroVector;
	FVector SurfaceNormal = FVector::UpVector;
	FVector WaterVelocity = FVector::ZeroVector;
	float WaterDepth = 0.f;

	const bool bOk = BodyComp->GetWaterSurfaceInfoAtLocation(
		WorldLocation,
		SurfaceLoc,
		SurfaceNormal,
		WaterVelocity,
		WaterDepth,
		true
	);

	return bOk ? WaterDepth : 0.f;
}

FVector USwimComponent::GetCurrentDirectionAtLocation(AWaterBody* WaterBody, const FVector& WorldLocation) const
{
	if (!WaterBody) return FVector::ZeroVector;

	UWaterBodyComponent* BodyComp = WaterBody->GetWaterBodyComponent();
	if (!BodyComp) return FVector::ZeroVector;

	FVector SurfaceLoc = FVector::ZeroVector;
	FVector SurfaceNormal = FVector::UpVector;
	FVector WaterVelocity = FVector::ZeroVector;
	float WaterDepth = 0.f;

	const bool bOk = BodyComp->GetWaterSurfaceInfoAtLocation(
		WorldLocation,
		SurfaceLoc,
		SurfaceNormal,
		WaterVelocity,
		WaterDepth,
		false
	);

	if (!bOk) return FVector::ZeroVector;

	return WaterVelocity.GetSafeNormal();
}

void USwimComponent::ApplyCurrent(float DeltaTime)
{
	if (!OwnerCharacter || !MoveComp || !CurrentWaterBody) return;

	const FVector Location = OwnerCharacter->GetActorLocation();
	const FVector Dir = GetCurrentDirectionAtLocation(CurrentWaterBody, Location);

	if (Dir.IsNearlyZero()) return;

	const float Strength =
		(CurrentWaterBody->IsA(AWaterBodyRiver::StaticClass())) ? RiverCurrentStrength : OceanCurrentStrength;

	MoveComp->AddForce(Dir * Strength);
}

void USwimComponent::ApplySurfaceHold(float DeltaTime)
{
	if (!OwnerCharacter || !MoveComp || !CapsuleComp) return;

	const FVector Loc = OwnerCharacter->GetActorLocation();
	const float LocZ = Loc.Z;

	const float SurfaceZ = GetWaterSurfaceZAtLocation(Loc);

	// IMPORTANT CHANGE:
	// Use a real head/neck socket if possible. Fallback to your old capsule-based neck estimate.
	float NeckZ = 0.f;
	bool bUsedSocket = false;

	if (USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh())
	{
		// Try common socket/bone names (works for Manny/Quinn + many rigs)
		static const FName HeadSocket(TEXT("head"));
		static const FName NeckSocket(TEXT("neck_01"));

		if (MeshComp->DoesSocketExist(HeadSocket))
		{
			NeckZ = MeshComp->GetSocketLocation(HeadSocket).Z;
			bUsedSocket = true;
		}
		else if (MeshComp->DoesSocketExist(NeckSocket))
		{
			NeckZ = MeshComp->GetSocketLocation(NeckSocket).Z;
			bUsedSocket = true;
		}
	}

	// Fallback (your original method)
	const float Half = CapsuleComp->GetScaledCapsuleHalfHeight();
	const float TopZ = LocZ + Half;

	if (!bUsedSocket)
	{
		NeckZ = TopZ - NeckDownFromTop;
	}

	// Desired neck relative to surface
	const float DesiredNeckZ = SurfaceZ - NeckDownFromSurface;

	// Compute desired actor Z from neck position
	// (If using socket, this becomes simply Err = DesiredNeckZ - NeckZ, but keep full form for clarity)
	const float NeckOffsetFromActor = NeckZ - LocZ;
	const float DesiredActorZ = DesiredNeckZ - NeckOffsetFromActor;

	const float Err = DesiredActorZ - LocZ;
	const float VelZ = MoveComp->Velocity.Z;

	float AccelZ = (SurfaceHoldKp * Err) - (SurfaceHoldKd * VelZ);
	AccelZ = FMath::Clamp(AccelZ, -MaxDownAccel, MaxUpAccel);

	MoveComp->AddForce(FVector(0.f, 0.f, AccelZ * PseudoMass));

	FVector V = MoveComp->Velocity;
	V.Z = FMath::Clamp(V.Z, -MaxSinkSpeed, MaxRiseSpeed);
	MoveComp->Velocity = V;

	UE_LOG(LogTemp, Warning, TEXT("SwimDebug: LocZ=%.2f SurfaceZ=%.2f Half=%.2f NeckZ=%.2f DesiredNeckZ=%.2f DesiredActorZ=%.2f Err=%.2f VelZ=%.2f AccelZ=%.2f Mode=%d"),
		LocZ, SurfaceZ, Half, NeckZ, DesiredNeckZ, DesiredActorZ, Err, VelZ, AccelZ, (int32)MoveComp->MovementMode);
}
