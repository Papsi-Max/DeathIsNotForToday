// Copyright Epic Games, Inc. All Rights Reserved.

#include "DINFTWeapon.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemInterface.h"

ADINFTWeapon::ADINFTWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetGenerateOverlapEvents(false);

	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	HitBox->SetupAttachment(MeshComponent);

	// Starts fully disabled — collision is only active during a swing.
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitBox->SetCollisionObjectType(ECC_WorldDynamic);
	HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_Pawn,         ECR_Overlap);
	HitBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	HitBox->OnComponentBeginOverlap.AddDynamic(this, &ADINFTWeapon::OnHitBoxOverlap);
}

void ADINFTWeapon::StartTrace()
{
	HitActors.Reset();
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	UE_LOG(LogTemp, Log, TEXT("[DINFTWeapon] %s — trace started"), *GetName());
}

void ADINFTWeapon::StopTrace()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitActors.Reset();

	UE_LOG(LogTemp, Log, TEXT("[DINFTWeapon] %s — trace stopped"), *GetName());
}

void ADINFTWeapon::AttachToSocket(USceneComponent* InParentComponent, FName SocketName)
{
	if (!InParentComponent) return;

	AttachToComponent(InParentComponent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName);

	UE_LOG(LogTemp, Log, TEXT("[DINFTWeapon] %s attached to socket '%s'"),
		*GetName(), *SocketName.ToString());
}

void ADINFTWeapon::OnHitBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == GetOwner()) return;

	// Skip actors that don't participate in GAS (no point broadcasting them).
	if (!OtherActor->Implements<UAbilitySystemInterface>()) return;

	// Deduplicate: only hit each target once per swing.
	if (HitActors.Contains(OtherActor)) return;

	HitActors.Add(OtherActor);
	OnHitTarget.Broadcast(OtherActor);

	UE_LOG(LogTemp, Log, TEXT("[DINFTWeapon] %s hit %s"), *GetName(), *GetNameSafe(OtherActor));
}
