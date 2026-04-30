#include "LocomotionController.h"

#include "ClimbComponent.h"
#include "CustomMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "LocomotionCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Locomotion/LocomotionAnimation/LocomotionInstance.h"

void ALocomotionController::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(LogTemp, Warning, TEXT("Hello World!"));
}

void ALocomotionController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (IsLocalController() == false)
		return;
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	Subsystem->AddMappingContext(InputMappingContext, 99999);
	
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhancedInputComponent->BindAction(WeaponAction, ETriggerEvent::Started, this, &ALocomotionController::ToggleWeapon);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ALocomotionController::HandleActionInput);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALocomotionController::Look);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALocomotionController::Move);
	EnhancedInputComponent->BindAction(PrimaryAction, ETriggerEvent::Started, this, &ALocomotionController::Primary);
}

void ALocomotionController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);
	
	ClientCharacter = Cast<ALocomotionCharacter>(GetPawn());
	ClientAnimInst = Cast<ULocomotionInstance>(ClientCharacter->GetMesh()->GetAnimInstance());
}

void ALocomotionController::ExitAimOffset()
{
	if (ClientAnimInst != nullptr) ClientAnimInst->AimOffsetAlpha = 0;
	
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void ALocomotionController::EnterAimOffset()
{
	if (ClientAnimInst != nullptr) ClientAnimInst->AimOffsetAlpha = 1;
		
	bShowMouseCursor = true;
	SetInputMode(FInputModeGameAndUI());
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ALocomotionController::ShowMouseDotAndGetDir, 0.02f, true);
}

void ALocomotionController::Primary()
{
	if (IsValid(ClientCharacter) == false || IsValid(ClientAnimInst) == false)
		return;
	
	if (ClientAnimInst->MoveMode != TPS)
		return;
	
	if (ClientAnimInst->AimOffsetAlpha == 1) ExitAimOffset();
	else if (ClientAnimInst->AimOffsetAlpha == 0) EnterAimOffset();
}

void ALocomotionController::HandleActionInput()
{
	if (IsValid(ClientCharacter) == false || IsValid(ClientAnimInst) == false)
		return;
	
	if (ClientCharacter->GetCharacterMovement()->IsFalling() == true)
		return;
	
	if (ClientCharacter->MoveMode == CLIMB)
	{
		ClientCharacter->ClimbComponent->StopClimbing();
		ClientCharacter->ClimbComponent->StopClimb_Server();
	}
	else
	{
		ClientCharacter->Jump();
	}
}

void ALocomotionController::Look(const FInputActionValue& Value)
{
	FVector2D Vector2D = Value.Get<FVector2D>();
	
	AddYawInput(Vector2D.X * 0.2);
	AddPitchInput(Vector2D.Y * -0.2);
}

void ALocomotionController::Move(const FInputActionValue &Value)
{
	if (IsValid(ClientCharacter) == false || IsValid(ClientAnimInst) == false)
		return;
	
	FVector2D MoveVector2D = Value.Get<FVector2D>();
	
	if (ClientCharacter->MoveMode == CLIMB)
	{
		FVector SurfaceNormal = ClientCharacter->ClimbComponent->MovementComp->ClimbSurfaceNormal;
		FVector Tangent = FVector::CrossProduct(FVector::UpVector, SurfaceNormal).GetSafeNormal();
		FVector Bitangent = FVector::CrossProduct(Tangent, -SurfaceNormal).GetSafeNormal();
		
		ClientCharacter->AddMovementInput(Bitangent, MoveVector2D.Y);
		ClientCharacter->AddMovementInput(Tangent, -MoveVector2D.X);
	}
	else
	{
		FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
    	
		FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    	
		ClientCharacter->AddMovementInput(ForwardVector, MoveVector2D.Y);
		ClientCharacter->AddMovementInput(RightVector, MoveVector2D.X);
	}
}

void ALocomotionController::ToggleWeapon()
{
	if (IsValid(ClientCharacter) == false || IsValid(ClientAnimInst) == false)
		return;
	
	Toggle_MoveMode_Server();
}

void ALocomotionController::Toggle_MoveMode_Server_Implementation()
{
	if (ALocomotionCharacter* ServerCharacter = Cast<ALocomotionCharacter>(GetPawn()))
	{
		if (ServerCharacter->MoveMode == CLIMB)
			return;
		
		FString Mode = ServerCharacter->MoveMode == TPS ? FPS : TPS;
		ServerCharacter->ApplyMoveMode(Mode);
	}
}

void ALocomotionController::ShowMouseDotAndGetDir()
{
	if (IsValid(ClientCharacter) == false || IsValid(ClientAnimInst) == false)
		return;
	
	float MouseX, MouseY;
	GetMousePosition(MouseX, MouseY);
	
	FVector WorldLocation, WorldDirection;
	DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);
	
	WorldLocation += WorldDirection * 10;
	DrawDebugPoint(GetWorld(), WorldLocation, 20, FColor::Red, false, 0.02f);
	
	FRotator ActorRot = GetPawn()->GetActorRotation();
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(WorldDirection.Rotation(), ActorRot);

	ClientAnimInst->UpdateAimOffset(Delta.Yaw, Delta.Pitch);
}


