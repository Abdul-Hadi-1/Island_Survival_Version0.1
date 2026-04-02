#include "CookingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DayandNight.h"
#include "FishingPuzzleManager.h"
#include "InventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UCookingComponent::UCookingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCookingComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheDayNightActor();
	CacheFishingManager();

	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (Inventory)
	{
		CookedFish = Inventory->GetItemCount("CookedFish");
	}
}

void UCookingComponent::CacheDayNightActor()
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
		UE_LOG(LogTemp, Warning, TEXT("CookingComponent: DayandNight actor not found in level."));
	}
}

void UCookingComponent::CacheFishingManager()
{
	if (FishingManagerRef)
	{
		return;
	}

	FishingManagerRef = GetOwner()->FindComponentByClass<UFishingPuzzleManager>();

	if (!FishingManagerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("CookingComponent: FishingPuzzleManager component not found on owner."));
	}
}

int32 UCookingComponent::GetCurrentDayNumber() const
{
	if (DayNightRef)
	{
		return DayNightRef->DayNumber;
	}

	return 1;
}

bool UCookingComponent::CanStartCooking() const
{
	if (!FishingManagerRef)
	{
		return false;
	}

	return FishingManagerRef->HasUncookedFish() && !bIsCooking;
}

bool UCookingComponent::HasCookedFish() const
{
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		return CookedFish > 0;
	}

	return Inventory->GetItemCount("CookedFish") > 0;
}

int32 UCookingComponent::GetCookingPuzzleGridSize() const
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

	return 5;
}

float UCookingComponent::GetCookingPuzzleTimeLimit() const
{
	const int32 Day = GetCurrentDayNumber();

	if (Day >= 1 && Day <= 2)
	{
		return 20.0f;
	}

	if (Day >= 3 && Day <= 4)
	{
		return 30.0f;
	}

	if (Day >= 5 && Day <= 7)
	{
		return 45.0f;
	}

	return 45.0f;
}

void UCookingComponent::HandleCookingPuzzleSolved()
{
	if (!FishingManagerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("CookingComponent: FishingManagerRef is null."));
		return;
	}

	if (!FishingManagerRef->HasUncookedFish())
	{
		UE_LOG(LogTemp, Warning, TEXT("CookingComponent: No uncooked fish available to cook."));
		return;
	}

	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("CookingComponent: InventoryComponent is null."));
		return;
	}

	Inventory->RemoveItem("UncookedFish", 2);
	Inventory->AddItem("CookedFish", 2);

	if (FishingManagerRef)
	{
		FishingManagerRef->UncookedFish = Inventory->GetItemCount("UncookedFish");
	}

	CookedFish = Inventory->GetItemCount("CookedFish");

	UE_LOG(LogTemp, Log, TEXT("Cooking puzzle solved. CookedFish is now: %d"), CookedFish);
}

void UCookingComponent::AddCookedFish(int32 Amount)
{
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("CookingComponent: InventoryComponent is null."));
		return;
	}

	if (Amount > 0)
	{
		Inventory->AddItem("CookedFish", Amount);
	}
	else if (Amount < 0)
	{
		Inventory->RemoveItem("CookedFish", FMath::Abs(Amount));
	}

	CookedFish = Inventory->GetItemCount("CookedFish");

	UE_LOG(LogTemp, Log, TEXT("Added %d cooked fish. CookedFish is now: %d"), Amount, CookedFish);
}

bool UCookingComponent::EatCookedFish()
{
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("CookingComponent: InventoryComponent is null."));
		return false;
	}

	if (Inventory->GetItemCount("CookedFish") <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CookingComponent: No cooked fish available to eat."));
		return false;
	}

	Inventory->RemoveItem("CookedFish", 1);
	CookedFish = Inventory->GetItemCount("CookedFish");

	UE_LOG(LogTemp, Log, TEXT("Cooked fish eaten. CookedFish is now: %d"), CookedFish);
	return true;
}

void UCookingComponent::StartCooking(UUserWidget* FocusWidget)
{
	if (bIsCooking)
	{
		UE_LOG(LogTemp, Warning, TEXT("CookingComponent: StartCooking called but player is already cooking."));
		return;
	}

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

	bIsCooking = true;

	UE_LOG(LogTemp, Log, TEXT("CookingComponent: Cooking started."));
}

void UCookingComponent::StopCooking()
{
	ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (CharacterOwner && CharacterOwner->GetCharacterMovement())
	{
		UCharacterMovementComponent* MoveComp = CharacterOwner->GetCharacterMovement();
		MoveComp->SetMovementMode(MOVE_Walking);
		MoveComp->Velocity = FVector::ZeroVector;
	}
	UE_LOG(LogTemp, Warning, TEXT("CookingComponent: Movement mode restored to Walking"));

	APawn* PawnOwner = Cast<APawn>(GetOwner());
	APlayerController* PC = PawnOwner ? Cast<APlayerController>(PawnOwner->GetController()) : nullptr;

	if (PC)
	{
		PC->bShowMouseCursor = false;
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC);
	}

	bIsCooking = false;

	UE_LOG(LogTemp, Log, TEXT("CookingComponent: Cooking stopped."));
}