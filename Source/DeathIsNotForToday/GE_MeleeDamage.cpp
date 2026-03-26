// Copyright Epic Games, Inc. All Rights Reserved.

#include "GE_MeleeDamage.h"
#include "DINFTTrainingDummyAttributeSet.h"

UGE_MeleeDamage::UGE_MeleeDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute    = UDINFTTrainingDummyAttributeSet::GetCurrentHealthAttribute();
	DamageModifier.ModifierOp   = EGameplayModOp::Additive;
	DamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-20.f));

	Modifiers.Add(DamageModifier);
}
