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
		return false;
	}
	*CountPtr -= Amount;
	if (*CountPtr <= 0)
	{
		Items.Remove(ItemID);
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
