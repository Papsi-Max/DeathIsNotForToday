// Copyright Epic Games, Inc. All Rights Reserved.

#include "DINFTTrainingDummy.h"
#include "DINFTTrainingDummyAttributeSet.h"
#include "AbilitySystemComponent.h"

ADINFTTrainingDummy::ADINFTTrainingDummy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UDINFTTrainingDummyAttributeSet>(TEXT("AttributeSet"));
}

void ADINFTTrainingDummy::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AttributeSet->OnHealthDecreased.AddUObject(this, &ADINFTTrainingDummy::OnDamageTaken);
}

UAbilitySystemComponent* ADINFTTrainingDummy::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float ADINFTTrainingDummy::GetCurrentHealth() const
{
	return AttributeSet ? AttributeSet->GetCurrentHealth() : 0.f;
}

float ADINFTTrainingDummy::GetMaxHealth() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

void ADINFTTrainingDummy::LogHealth() const
{
	UE_LOG(LogTemp, Log, TEXT("[DINFTTrainingDummy] %s — CurrentHealth: %.1f / %.1f"),
		*GetName(), GetCurrentHealth(), GetMaxHealth());
}

void ADINFTTrainingDummy::OnDamageTaken()
{
	// Interrupt any ongoing regen and restart the delay countdown.
	GetWorldTimerManager().ClearTimer(RegenTickHandle);
	GetWorldTimerManager().SetTimer(RegenDelayHandle, this, &ADINFTTrainingDummy::StartRegen, RegenDelay, false);
}

void ADINFTTrainingDummy::StartRegen()
{
	GetWorldTimerManager().SetTimer(RegenTickHandle, this, &ADINFTTrainingDummy::RegenTick, RegenInterval, true);
}

void ADINFTTrainingDummy::RegenTick()
{
	const float Max        = AttributeSet->GetMaxHealth();
	const float HealthGain = (Max / RegenDuration) * RegenInterval;
	const float NewHealth  = FMath::Min(AttributeSet->GetCurrentHealth() + HealthGain, Max);

	// Set directly via ASC base value — bypasses PostGameplayEffectExecute intentionally
	// so regen does not re-trigger the damage delegate.
	AbilitySystemComponent->SetNumericAttributeBase(
		UDINFTTrainingDummyAttributeSet::GetCurrentHealthAttribute(), NewHealth);

	if (NewHealth >= Max)
	{
		GetWorldTimerManager().ClearTimer(RegenTickHandle);
	}
}
