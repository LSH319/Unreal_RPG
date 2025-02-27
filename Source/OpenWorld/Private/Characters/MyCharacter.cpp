// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MyCharacter.h"

#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Components/AttributeComponent.h"
#include "HUD/MyHUD.h"
#include "HUD/MyUserWidget.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Weapons/Weapon.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	Camera->SetupAttachment(SpringArm);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Attributes && MyOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		MyOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void AMyCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && MyOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		MyOverlay->SetSouls(Attributes->GetSouls());
	}
}

void AMyCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && MyOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		MyOverlay->SetGold(Attributes->GetGold());
	}
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	Tags.Add(FName("EngageableTarget"));
	InitializeSlashOverlay();
}

void AMyCharacter::Disarm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void AMyCharacter::Arm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void AMyCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
	{
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else Die();
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
	
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

float AMyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void AMyCharacter::Die()
{
	Super::Die();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

bool AMyCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetAttackCost();
}

bool AMyCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

void AMyCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void AMyCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AMyCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool AMyCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool AMyCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

void AMyCharacter::SetHUDHealth()
{
	if (MyOverlay && Attributes)
	{
		MyOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void AMyCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		AMyHUD* MyHUD = Cast<AMyHUD>(PlayerController->GetHUD());
		if (MyHUD)
		{
			MyOverlay = MyHUD->GetMyOverlay();
			if (MyOverlay && Attributes)
			{
				MyOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				MyOverlay->SetStaminaBarPercent(1.f);
				MyOverlay->SetGold(0);
				MyOverlay->SetSouls(0);
			}
		}
	}
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyCharacter::Attack()
{
	if (IsOccupied() ||!HasEnoughStamina()) return;
	if(CanAttack())
	{
		if (Attributes && MyOverlay)
		{
			Attributes->UseStamina(Attributes->GetAttackCost());
			MyOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
			
			PlayAttackMontage();
			ActionState = EActionState::EAS_Attacking;
		}
	}
}