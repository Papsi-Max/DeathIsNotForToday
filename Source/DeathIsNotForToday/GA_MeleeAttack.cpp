// Copyright Epic Games, Inc. All Rights Reserved.

#include "GA_MeleeAttack.h"
#include "GE_MeleeDamage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Kismet/KismetSystemLibrary.h"

UGA_MeleeAttack::UGA_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	DamageEffectClass = UGE_MeleeDamage::StaticClass();
}

void UGA_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("[GA_MeleeAttack] Ability activated by %s"),
		*GetNameSafe(GetAvatarActorFromActorInfo()));

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

	// Sphere overlap centred on the player, covering actors within AttackRange.
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

	UE_LOG(LogTemp, Log, TEXT("[GA_MeleeAttack] Overlap found %d actor(s) within %.0f cm"),
		OverlappedActors.Num(), AttackRange);

	const FVector ForwardVector = AvatarActor->GetActorForwardVector();

	for (AActor* HitActor : OverlappedActors)
	{
		if (!HitActor) continue;

		// Discard actors behind the player (forward half-sphere).
		const FVector ToTarget = (HitActor->GetActorLocation() - AvatarActor->GetActorLocation()).GetSafeNormal();
		if (FVector::DotProduct(ForwardVector, ToTarget) < 0.f) continue;

		IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(HitActor);
		if (!ASCInterface) continue;

		UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent();
		if (!TargetASC) continue;

		FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
		ContextHandle.AddSourceObject(AvatarActor);

		const FGameplayEffectSpecHandle SpecHandle =
			SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), ContextHandle);

		if (SpecHandle.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			UE_LOG(LogTemp, Log, TEXT("[GA_MeleeAttack] Damage applied to %s"), *GetNameSafe(HitActor));
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
