// Copyright Epic Games, Inc. All Rights Reserved.

#include "GA_Knight_BaseSlash.h"
#include "GE_KnightDamage.h"
#include "DINFTGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Kismet/KismetSystemLibrary.h"

UGA_Knight_BaseSlash::UGA_Knight_BaseSlash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	DamageEffectClass = UGE_KnightDamage::StaticClass();
}

void UGA_Knight_BaseSlash::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (!AvatarActor || !SourceASC || !DamageEffectClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	TArray<AActor*> OverlappedActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = {
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_Pawn)
	};

	UKismetSystemLibrary::SphereOverlapActors(
		AvatarActor,
		AvatarActor->GetActorLocation(),
		AttackRange,
		ObjectTypes,
		nullptr,
		{ AvatarActor },
		OverlappedActors);

	const FVector ForwardVector = AvatarActor->GetActorForwardVector();

	for (AActor* HitActor : OverlappedActors)
	{
		if (!HitActor) continue;

		const FVector ToTarget = (HitActor->GetActorLocation() - AvatarActor->GetActorLocation()).GetSafeNormal();
		if (FVector::DotProduct(ForwardVector, ToTarget) < 0.f) continue;

		IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(HitActor);
		if (!ASCInterface) continue;

		UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent();
		if (!TargetASC) continue;

		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(AvatarActor);

		FGameplayEffectSpecHandle SpecHandle =
			SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);

		if (SpecHandle.IsValid())
		{
			// Write the damage value into the spec so GE_KnightDamage's SetByCaller picks it up.
			SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Damage, -DamageAmount);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

			UE_LOG(LogTemp, Log, TEXT("[%s] Hit %s for %.0f damage"),
				*GetName(), *GetNameSafe(HitActor), DamageAmount);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
