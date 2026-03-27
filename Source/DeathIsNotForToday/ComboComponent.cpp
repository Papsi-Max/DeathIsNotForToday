// Copyright Epic Games, Inc. All Rights Reserved.

#include "ComboComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UComboComponent::UComboComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UComboComponent::GrantAbilities(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	for (TSubclassOf<UGameplayAbility> AbilityClass : ComboAbilities)
	{
		if (AbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		}
	}
}

void UComboComponent::TryAdvanceCombo()
{
	if (ComboAbilities.IsEmpty()) return;

	IAbilitySystemInterface* ASCOwner = Cast<IAbilitySystemInterface>(GetOwner());
	if (!ASCOwner) return;

	UAbilitySystemComponent* ASC = ASCOwner->GetAbilitySystemComponent();
	if (!ASC) return;

	if (!ComboAbilities.IsValidIndex(ComboIndex) || !ComboAbilities[ComboIndex]) return;

	const bool bActivated = ASC->TryActivateAbilityByClass(ComboAbilities[ComboIndex]);
	if (!bActivated) return;

	UE_LOG(LogTemp, Log, TEXT("[ComboComponent] Combo step %d activated (%s)"),
		ComboIndex, *GetNameSafe(ComboAbilities[ComboIndex]));

	ComboIndex = (ComboIndex + 1) % ComboAbilities.Num();

	// Restart the window — if this timer fires, the combo resets to 0.
	GetWorld()->GetTimerManager().ClearTimer(ComboResetHandle);
	GetWorld()->GetTimerManager().SetTimer(
		ComboResetHandle, this, &UComboComponent::ResetCombo, ComboWindowDuration, false);
}

void UComboComponent::ResetCombo()
{
	UE_LOG(LogTemp, Log, TEXT("[ComboComponent] Combo reset (window expired)"));
	ComboIndex = 0;
}
