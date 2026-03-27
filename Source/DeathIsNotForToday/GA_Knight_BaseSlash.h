// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Knight_BaseSlash.generated.h"

// Base class for all knight melee combo abilities. Subclasses set DamageAmount.
// Do not use this class directly — it has no damage value of its own.
UCLASS(Abstract)
class DEATHISNOTFORTODAY_API UGA_Knight_BaseSlash : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Knight_BaseSlash();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// Damage dealt to each target. Set by concrete subclasses.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float DamageAmount = 0.f;

	// Radius of the melee hit sphere, in cm.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float AttackRange = 200.f;

	// GE to apply — uses Data.Damage SetByCaller magnitude.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
