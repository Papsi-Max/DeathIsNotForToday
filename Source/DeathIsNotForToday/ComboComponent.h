// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Abilities/GameplayAbility.h"
#include "ComboComponent.generated.h"

class UAbilitySystemComponent;

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class DEATHISNOTFORTODAY_API UComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UComboComponent();

	// Ordered list of abilities that form the combo chain.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TArray<TSubclassOf<UGameplayAbility>> ComboAbilities;

	// Time (seconds) the player has to press again before the combo resets.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float ComboWindowDuration = 1.5f;

	// Current position in the combo chain (0 = first hit).
	UPROPERTY(BlueprintReadOnly, Category = "Combo")
	int32 ComboIndex = 0;

	// Grants all ComboAbilities to the given ASC. Call this from PossessedBy after InitAbilityActorInfo.
	void GrantAbilities(UAbilitySystemComponent* ASC);

	// Activates the current combo ability, advances the index, and restarts the window timer.
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void TryAdvanceCombo();

private:
	void ResetCombo();

	FTimerHandle ComboResetHandle;
};
