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

}

void UWaterFilter::CacheInventory()
{
	if (InventoryRef)
	{
		return;
	}
	InventoryRef = GetOwner()->FindComponentByClass<UInventoryComponent>();
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
	const bool bHasDirtyWater = InventoryRef->HasItem("DirtyWater", 1);
	const bool bHasCharcoal = InventoryRef->HasItem("Charcoal", 1);
	const bool bHasSand = InventoryRef->HasItem("Sand", 1);
	const bool bHasGravel = InventoryRef->HasItem("Gravel", 1);
	const int32 LayerCount = GetPurificationPuzzleLayerCount();

	return bHasDirtyWater && bHasCharcoal && bHasSand && bHasGravel && !bIsPurifying;
}

void UWaterFilter::StartPurifying(UUserWidget* FocusWidget)
{
	if (bIsPurifying)
	{
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
		return;
	}
	if (bIsRevealPhase)
	{
		return;
	}
	const int32 CurrentIndex = PlayerOrder.Num();
	if (!CorrectOrder.IsValidIndex(CurrentIndex))
	{
		bPuzzleFailed = true;
		HandlePurificationFailed();
		return;
	}
	PlayerOrder.Add(Material);
	if (Material != CorrectOrder[CurrentIndex])
	{
		bPuzzleFailed = true;
		HandlePurificationFailed();
		return;
	}
	if (PlayerOrder.Num() == CorrectOrder.Num())
	{
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
		return;
	}
	InventoryRef->RemoveItem("DirtyWater", 1);
	InventoryRef->RemoveItem("Charcoal", 1);
	InventoryRef->RemoveItem("Sand", 1);
	InventoryRef->RemoveItem("Gravel", 1);

	InventoryRef->AddItem("CleanWater", 1);
}

void UWaterFilter::HandlePurificationFailed()
{
	if (!InventoryRef)
	{
		return;
	}
	InventoryRef->RemoveItem("DirtyWater", 1);
	InventoryRef->RemoveItem("Charcoal", 1);
	InventoryRef->RemoveItem("Sand", 1);
    InventoryRef->RemoveItem("Gravel", 1);
}

