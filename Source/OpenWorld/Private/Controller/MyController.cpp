// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/MyController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Characters/MyCharacter.h"
#include "GameFramework/Character.h"
#include "Items/Weapons/Weapon.h"

AMyController::AMyController()
{

}

void AMyController::BeginPlay()
{
	Super::BeginPlay();

	check(MyContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(MyContext, 0);
	}
}

void AMyController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyController::Move);
	EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AMyController::Turn);
	EnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &AMyController::LookUp);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMyController::Jump);
	EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AMyController::Equip);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AMyController::Attack);
}

void AMyController::Move(const FInputActionValue& InputActionValue)
{
	ACharacter* ControlledCharacter = GetCharacter();
	if (AMyCharacter* MyCharacter = Cast<AMyCharacter>(ControlledCharacter))
	{
		if(EActionState::EAS_Attacking == MyCharacter->GetActionState())return;;
	}
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

	const FRotator Rotaion = GetControlRotation();
	const FRotator YawRotaion(0.f, Rotaion.Yaw, 0.f);

	const FVector FowardDirection = FRotationMatrix(YawRotaion).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotaion).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(FowardDirection, InputAxisVector.X);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.Y);
	}
}

void AMyController::Turn(const FInputActionValue& InputActionValue)
{
	const float InputAxis = InputActionValue.Get<float>();

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddControllerYawInput(InputAxis);
	}
	
}

void AMyController::LookUp(const FInputActionValue& InputActionValue)
{
	const float InputAxis = InputActionValue.Get<float>();
	
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddControllerPitchInput(InputAxis);
	}
}

void AMyController::Jump(const FInputActionValue& InputActionValue)
{
	if (ACharacter* ControlledCharacter = GetCharacter())
	{
		ControlledCharacter->Jump();
	}
}

void AMyController::Equip(const FInputActionValue& InputActionValue)
{
	ACharacter* ControlledCharacter = GetCharacter();
	if (AMyCharacter* MyCharacter = Cast<AMyCharacter>(ControlledCharacter))
	{
		if(AWeapon* Weapon = Cast<AWeapon>(MyCharacter->GetOverlappingItem()))
		{
			Weapon->Equip(MyCharacter,FName("RightHandSocket"));
		}
		
	}
}

void AMyController::Attack(const FInputActionValue& InputActionValue)
{
	// UAnimInstance* AnimInstance = GetCharacter()->GetMesh()->GetAnimInstance();
	// UAnimMontage* AttackMontage;
	// if(AMyCharacter* MyCharacter = Cast<AMyCharacter>(ControlledCharacter))
	// {
	// 	AttackMontage = MyCharacter->AttackMontage;
	// }
	// if (AnimInstance && AttackMontage)
	// {
	// 	AnimInstance->Montage_Play(AttackMontage);
	// 	int32 Selection = FMath::RandRange(0, 1);
	// 	FName SectionName = FName();
	// 	switch (Selection)
	// 	{
	// 	case 0:
	// 		SectionName = FName("Attack1");
	// 		break;
	// 	case 1:
	// 		SectionName = FName("Attack2");
	// 		break;
	// 	default:
	// 		break;
	// 	}
	// 	AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	// }
}
