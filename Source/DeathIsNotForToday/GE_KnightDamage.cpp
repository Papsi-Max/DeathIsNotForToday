// Copyright Epic Games, Inc. All Rights Reserved.

#include "GE_KnightDamage.h"
#include "DINFTGameplayTags.h"
#include "DINFTTrainingDummyAttributeSet.h"

UGE_KnightDamage::UGE_KnightDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCallerData;
	SetByCallerData.DataTag = TAG_Data_Damage;

	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute        = UDINFTTrainingDummyAttributeSet::GetCurrentHealthAttribute();
	ModifierInfo.ModifierOp       = EGameplayModOp::Additive;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerData);

	Modifiers.Add(ModifierInfo);
}
