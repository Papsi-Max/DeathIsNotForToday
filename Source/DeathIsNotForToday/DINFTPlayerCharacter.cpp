// Copyright Epic Games, Inc. All Rights Reserved.

#include "DINFTPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GA_MeleeAttack.h"
#include "ComboComponent.h"
#include "DINFTWeapon.h"
#include "GA_Knight_BaseSlash.h"
#include "DINFTGameplayTags.h"

ADINFTPlayerCharacter::ADINFTPlayerCharacter()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 400.f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	ComboComponent = CreateDefaultSubobject<UComboComponent>(TEXT("ComboComponent"));

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ADINFTPlayerCharacter::PossessedBy(AController* NewController)
{
	// Super initialises the ASC via DINFTCharacter::InitializeAbilitySystem.
	Super::PossessedBy(NewController);

	if (MeleeAbilityClass && AbilitySystemComponent)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(MeleeAbilityClass, 1));
	}

	if (ComboComponent && AbilitySystemComponent)
	{
		ComboComponent->GrantAbilities(AbilitySystemComponent);
	}
}

void ADINFTPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		CurrentWeapon = GetWorld()->SpawnActor<ADINFTWeapon>(WeaponClass, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->AttachToSocket(GetMesh(), TEXT("hand_r_socket"));
			CurrentWeapon->OnHitTarget.AddUObject(this, &ADINFTPlayerCharacter::OnWeaponHit);
		}
	}
}

void ADINFTPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADINFTPlayerCharacter::Move);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADINFTPlayerCharacter::Look);
		}
		if (SprintAction)
		{
			EIC->BindAction(SprintAction, ETriggerEvent::Started,   this, &ADINFTPlayerCharacter::StartSprint);
			EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ADINFTPlayerCharacter::StopSprint);
		}
		if (AttackAction && ComboComponent)
		{
			EIC->BindAction(AttackAction, ETriggerEvent::Started, ComboComponent.Get(), &UComboComponent::TryAdvanceCombo);
		}
	}
}

void ADINFTPlayerCharacter::Attack()
{
	if (MeleeAbilityClass && AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilityByClass(MeleeAbilityClass);
	}
}

void ADINFTPlayerCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ADINFTPlayerCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ADINFTPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection,   MovementVector.X);
	}
}

void ADINFTPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
}

void ADINFTPlayerCharacter::OnWeaponHit(AActor* HitActor)
{
	if (!HitActor || !ComboComponent || !AbilitySystemComponent) return;

	const int32 NumAbilities = ComboComponent->ComboAbilities.Num();
	if (NumAbilities == 0) return;

	// TryAdvanceCombo increments ComboIndex after activating the ability, so step
	// back one to identify which combo step is currently in flight.
	const int32 ActiveIndex = (ComboComponent->ComboIndex - 1 + NumAbilities) % NumAbilities;
	TSubclassOf<UGameplayAbility> ActiveClass = ComboComponent->ComboAbilities[ActiveIndex];
	if (!ActiveClass) return;

	const UGA_Knight_BaseSlash* AbilityCDO = ActiveClass->GetDefaultObject<UGA_Knight_BaseSlash>();
	if (!AbilityCDO || !AbilityCDO->DamageEffectClass) return;

	IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(HitActor);
	if (!TargetInterface) return;

	UAbilitySystemComponent* TargetASC = TargetInterface->GetAbilitySystemComponent();
	if (!TargetASC) return;

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
		AbilityCDO->DamageEffectClass, 1, Context);

	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(TAG_Data_Damage, -AbilityCDO->DamageAmount);
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);

		UE_LOG(LogTemp, Log, TEXT("[DINFTPlayerCharacter] Combo step %d hit %s for %.0f damage"),
			ActiveIndex, *GetNameSafe(HitActor), AbilityCDO->DamageAmount);
	}
}
