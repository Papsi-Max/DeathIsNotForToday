// Copyright Epic Games, Inc. All Rights Reserved.

#include "DINFTCharacter.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

ADINFTCharacter::ADINFTCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADINFTCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADINFTCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();
}

void ADINFTCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
}

void ADINFTCharacter::InitializeAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}
