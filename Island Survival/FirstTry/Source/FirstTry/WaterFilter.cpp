#include "WaterFilter.h"

#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "DayandNight.h"
#include "InventoryComponent.h"

UWaterFilter::UWaterFilter()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWaterFilter::BeginPlay()
{
	Super::BeginPlay();

	CacheDayNightActor();
	CacheInventory();
}

void UWaterFilter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsPurifying)
	{
		return;
	}

	if (bPuzzleSolved || bPuzzleFailed)
	{
		return;
	}

	if (bIsRevealPhase)
	{
		RevealTimeRemaining -= DeltaTime;

		if (RevealTimeRemaining <= 0.0f)
		{
			RevealTimeRemaining = 0.0f;
			bIsRevealPhase = false;
		}

		return;
	}

	PuzzleTimeRemaining -= DeltaTime;

	if (PuzzleTimeRemaining <= 0.0f)
	{
		PuzzleTimeRemaining = 0.0f;
		bPuzzleFailed = true;
		HandlePurificationFailed();
	}
}

void UWaterFilter::CacheDayNightActor()
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
		UE_LOG(LogTemp, Warning, TEXT("WaterFilter: DayandNight actor not found in level."));
	}
}

void UWaterFilter::CacheInventory()
{
	if (InventoryRef)
	{
		return;
	}

	InventoryRef = GetOwner()->FindComponentByClass<UInventoryComponent>();

	if (!InventoryRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaterFilter: InventoryComponent not found on owner."));
	}
}

int32 UWaterFilter::GetCurrentDayNumber() const
{
	if (DayNightRef)
	{
		return DayNightRef->DayNumber;
	}

	return 1;
}

bool UWaterFilter::CanStartPurifying() const
{
	if (!InventoryRef)
	{
		return false;
	}

	// Cloth is always available, so only check inventory items that matter
	const bool bHasDirtyWater = InventoryRef->HasItem("DirtyWater", 1);
	const bool bHasCharcoal = InventoryRef->HasItem("Charcoal", 1);
	const bool bHasSand = InventoryRef->HasItem("Sand", 1);

	const int32 LayerCount = GetPurificationPuzzleLayerCount();
	const bool bNeedsGravel = (LayerCount >= 4);
	const bool bHasGravel = !bNeedsGravel || InventoryRef->HasItem("Gravel", 1);

	return bHasDirtyWater && bHasCharcoal && bHasSand && bHasGravel && !bIsPurifying;
}

void UWaterFilter::StartPurifying(UUserWidget* FocusWidget)
{
	if (bIsPurifying)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaterFilter: already purifying."));
		return;
	}

	ResetPurificationPuzzle();
	BuildCorrectOrder();

	bIsPurifying = true;
	bIsRevealPhase = true;
	RevealTimeRemaining = GetOrderRevealTime();
	PuzzleTimeRemaining = GetPurificationPuzzleTimeLimit();

	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (CharacterOwner && CharacterOwner->GetCharacterMovement())
	{
		CharacterOwner->GetCharacterMovement()->StopMovementImmediately();
		CharacterOwner->GetCharacterMovement()->DisableMovement();
	}

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	APlayerController* PC = PawnOwner ? Cast<APlayerController>(PawnOwner->GetController()) : nullptr;

	if (PC)
	{
		PC->bShowMouseCursor = true;
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(
			PC,
			FocusWidget,
			EMouseLockMode::DoNotLock,
			false
		);
	}

	UE_LOG(LogTemp, Log, TEXT("WaterFilter: purification started."));
}

void UWaterFilter::StopPurifying()
{
	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (CharacterOwner && CharacterOwner->GetCharacterMovement())
	{
		CharacterOwner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	APlayerController* PC = PawnOwner ? Cast<APlayerController>(PawnOwner->GetController()) : nullptr;

	if (PC)
	{
		PC->bShowMouseCursor = false;
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
	}

	bIsPurifying = false;
	bIsRevealPhase = false;

	UE_LOG(LogTemp, Log, TEXT("WaterFilter: purification stopped."));
}

void UWaterFilter::ResetPurificationPuzzle()
{
	CorrectOrder.Empty();
	PlayerOrder.Empty();

	bPuzzleSolved = false;
	bPuzzleFailed = false;
	bIsRevealPhase = false;

	PuzzleTimeRemaining = 0.0f;
	RevealTimeRemaining = 0.0f;
}

void UWaterFilter::BuildCorrectOrder()
{
	CorrectOrder.Empty();

	const int32 LayerCount = GetPurificationPuzzleLayerCount();

	const TArray<EFilterMaterial> BasePattern =
	{
		EFilterMaterial::Cloth,
		EFilterMaterial::Charcoal,
		EFilterMaterial::Gravel,
		EFilterMaterial::Sand
	};

	for (int32 i = 0; i < LayerCount; i++)
	{
		CorrectOrder.Add(BasePattern[i % BasePattern.Num()]);
	}
}

void UWaterFilter::SubmitFilterMaterial(EFilterMaterial Material)
{
	if (!bIsPurifying || bPuzzleSolved || bPuzzleFailed)
	{
		UE_LOG(LogTemp, Warning, TEXT("SubmitFilterMaterial ignored. Purifying=%d Solved=%d Failed=%d"),
			bIsPurifying, bPuzzleSolved, bPuzzleFailed);
		return;
	}

	if (bIsRevealPhase)
	{
		UE_LOG(LogTemp, Warning, TEXT("SubmitFilterMaterial ignored because still in reveal phase."));
		return;
	}

	const int32 CurrentIndex = PlayerOrder.Num();

	UE_LOG(LogTemp, Warning, TEXT("SubmitFilterMaterial called. CurrentIndex=%d CorrectOrderNum=%d"),
		CurrentIndex, CorrectOrder.Num());

	if (!CorrectOrder.IsValidIndex(CurrentIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("CorrectOrder index invalid. Puzzle failing immediately."));
		bPuzzleFailed = true;
		HandlePurificationFailed();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Clicked Material=%d Expected Material=%d"),
		(int32)Material, (int32)CorrectOrder[CurrentIndex]);

	PlayerOrder.Add(Material);

	if (Material != CorrectOrder[CurrentIndex])
	{
		UE_LOG(LogTemp, Error, TEXT("Wrong material selected. Puzzle failed."));
		bPuzzleFailed = true;
		HandlePurificationFailed();
		return;
	}

	if (PlayerOrder.Num() == CorrectOrder.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Correct order completed. Puzzle solved."));
		bPuzzleSolved = true;
		HandleWaterPurified();
	}
}

int32 UWaterFilter::GetPurificationPuzzleLayerCount() const
{
	const int32 Day = GetCurrentDayNumber();

	if (Day >= 1 && Day <= 2)
	{
		return 4;
	}

	if (Day >= 3 && Day <= 5)
	{
		return 6;
	}

	if (Day >= 6 && Day <= 7)
	{
		return 8;
	}

	return 8;
}

float UWaterFilter::GetPurificationPuzzleTimeLimit() const
{
	const int32 Day = GetCurrentDayNumber();

	if (Day >= 1 && Day <= 2)
	{
		return 40.0f;
	}

	if (Day >= 3 && Day <= 5)
	{
		return 30.0f;
	}

	if (Day >= 6 && Day <= 7)
	{
		return 20.0f;
	}

	return 10.0f;
}

float UWaterFilter::GetOrderRevealTime() const
{
	const int32 Day = GetCurrentDayNumber();

	if (Day >= 1 && Day <= 2)
	{
		return 5.0f;
	}

	if (Day >= 3 && Day <= 5)
	{
		return 4.0f;
	}

	if (Day >= 6 && Day <= 7)
	{
		return 3.0f;
	}

	return 3.0f;
}

void UWaterFilter::HandleWaterPurified()
{
	if (!InventoryRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("WaterFilter: InventoryRef is null."));
		return;
	}

	InventoryRef->RemoveItem("DirtyWater", 1);
	InventoryRef->RemoveItem("Charcoal", 1);
	InventoryRef->RemoveItem("Sand", 1);

	if (GetPurificationPuzzleLayerCount() >= 4)
	{
		InventoryRef->RemoveItem("Gravel", 1);
	}

	InventoryRef->AddItem("CleanWater", 1);

	UE_LOG(LogTemp, Log, TEXT("WaterFilter: water purified successfully."));
}

void UWaterFilter::HandlePurificationFailed()
{
	UE_LOG(LogTemp, Warning, TEXT("WaterFilter: purification failed."));
}