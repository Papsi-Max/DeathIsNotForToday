// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DINFTWeapon.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHitTarget, AActor*);

UCLASS()
class DEATHISNOTFORTODAY_API ADINFTWeapon : public AActor
{
	GENERATED_BODY()

public:
	ADINFTWeapon();

	// Enable hitbox overlap detection and clear the per-swing hit list.
	// Call this at the start of an attack animation notify.
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartTrace();

	// Disable hitbox overlap detection and clear the per-swing hit list.
	// Call this at the end of an attack animation notify.
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopTrace();

	// Broadcasts the hit AActor once per target per swing.
	// Bind from an ability or character to apply a Gameplay Effect to the target.
	FOnHitTarget OnHitTarget;

	// Attach this weapon to a named socket on the owner's skeletal mesh.
	// Example: AttachToSocket(GetMesh(), TEXT("hand_r_socket"))
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AttachToSocket(USceneComponent* InParentComponent, FName SocketName);

protected:
	// The weapon visual. Also serves as the root component so socket attachment is
	// applied directly to the mesh origin.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// Collision hitbox positioned along the blade/impact area in the Blueprint.
	// Collision is disabled by default and only active between StartTrace/StopTrace.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UBoxComponent> HitBox;

private:
	UFUNCTION()
	void OnHitBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// Tracks actors already hit this swing to prevent double-hits.
	TArray<TObjectPtr<AActor>> HitActors;
};
