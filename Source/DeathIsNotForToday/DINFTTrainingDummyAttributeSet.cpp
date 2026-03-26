// Copyright Epic Games, Inc. All Rights Reserved.

#include "DINFTTrainingDummyAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UDINFTTrainingDummyAttributeSet::UDINFTTrainingDummyAttributeSet()
{
	InitMaxHealth(100.f);
	InitCurrentHealth(100.f);
}

void UDINFTTrainingDummyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		const float Clamped = FMath::Clamp(GetCurrentHealth(), 0.f, GetMaxHealth());
		SetCurrentHealth(Clamped);

		// A negative magnitude means health was reduced — broadcast so the actor can reset its regen timer.
		if (Data.EvaluatedData.Magnitude < 0.f)
		{
			OnHealthDecreased.Broadcast();
		}
	}
}

void UDINFTTrainingDummyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDINFTTrainingDummyAttributeSet, MaxHealth,     COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDINFTTrainingDummyAttributeSet, CurrentHealth, COND_None, REPNOTIFY_Always);
}

void UDINFTTrainingDummyAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDINFTTrainingDummyAttributeSet, MaxHealth, OldMaxHealth);
}

void UDINFTTrainingDummyAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDINFTTrainingDummyAttributeSet, CurrentHealth, OldCurrentHealth);
}
