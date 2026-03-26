// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "DINFTTrainingDummy.generated.h"

class UAbilitySystemComponent;
class UDINFTTrainingDummyAttributeSet;

UCLASS()
class DEATHISNOTFORTODAY_API ADINFTTrainingDummy : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADINFTTrainingDummy();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void LogHealth() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UDINFTTrainingDummyAttributeSet> AttributeSet;

private:
	void OnDamageTaken();
	void StartRegen();
	void RegenTick();

	FTimerHandle RegenDelayHandle;
	FTimerHandle RegenTickHandle;

	static constexpr float RegenDelay    = 3.f;
	static constexpr float RegenDuration = 2.f;
	static constexpr float RegenInterval = 0.05f;
};
