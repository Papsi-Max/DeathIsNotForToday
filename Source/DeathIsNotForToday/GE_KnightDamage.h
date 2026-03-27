// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_KnightDamage.generated.h"

// Instant damage effect whose magnitude is set at runtime via SetByCaller (tag: Data.Damage).
UCLASS()
class DEATHISNOTFORTODAY_API UGE_KnightDamage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_KnightDamage();
};
