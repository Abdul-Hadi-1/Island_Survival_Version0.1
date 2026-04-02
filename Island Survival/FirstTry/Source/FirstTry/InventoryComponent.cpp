#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryComponent::AddItem(FName ItemID, int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	int32& Count = Items.FindOrAdd(ItemID);
	Count += Amount;

	UE_LOG(LogTemp, Log, TEXT("Added %d of %s. New Count: %d"), Amount, *ItemID.ToString(), Count);
}

bool UInventoryComponent::RemoveItem(FName ItemID, int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	int32* CountPtr = Items.Find(ItemID);
	if (!CountPtr || *CountPtr < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough %s to remove."), *ItemID.ToString());
		return false;
	}

	*CountPtr -= Amount;

	if (*CountPtr <= 0)
	{
		Items.Remove(ItemID);
		UE_LOG(LogTemp, Log, TEXT("%s removed from inventory."), *ItemID.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Removed %d of %s. New Count: %d"), Amount, *ItemID.ToString(), *CountPtr);
	}

	return true;
}

int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
	if (const int32* CountPtr = Items.Find(ItemID))
	{
		return *CountPtr;
	}

	return 0;
}

bool UInventoryComponent::HasItem(FName ItemID, int32 Amount) const
{
	return GetItemCount(ItemID) >= Amount;
}