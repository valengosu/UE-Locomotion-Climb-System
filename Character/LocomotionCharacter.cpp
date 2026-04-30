#include "LocomotionCharacter.h"

#include "ClimbComponent.h"
#include "CustomMovementComponent.h"
#include "LocomotionController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Locomotion/LocomotionAnimation/LocomotionInstance.h"
#include "Net/UnrealNetwork.h"

ALocomotionCharacter::ALocomotionCharacter(const FObjectInitializer& OI): 
	Super(OI.SetDefaultSubobjectClass<UCustomMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	
	ClimbCheckCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MyExtraCapsule"));
	ClimbCheckCapsule->SetupAttachment(RootComponent);
	
	ClimbComponent = CreateDefaultSubobject<UClimbComponent>(TEXT("ClimbComponent"));
	ClimbComponent->ClimbCheckCapsule = ClimbCheckCapsule;
	ClimbComponent->Owner = this;
	ClimbComponent->MovementComp = Cast<UCustomMovementComponent>(GetCharacterMovement());
	
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	/*
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll  = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;*/
}

void ALocomotionCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALocomotionCharacter, WeaponActor);
	DOREPLIFETIME(ALocomotionCharacter, MoveMode);
}

void ALocomotionCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	GetCharacterMovement()->JumpZVelocity = 600.f;
	ApplyMoveMode(TPS);
	
	if (HasAuthority())
	{
		WeaponActor = GetWorld()->SpawnActor<AStaticMeshActor>(WeaponClass);
		WeaponActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Rifle_Socket"));
		WeaponActor->SetReplicates(true); 
		WeaponActor->SetReplicateMovement(false);
	}
}

void ALocomotionCharacter::OnRep_WeaponActor()
{
	WeaponActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Rifle_Socket"));
	WeaponActor->SetActorHiddenInGame(MoveMode != FPS);
}

void ALocomotionCharacter::ApplyMoveMode(FString Mode)
{
	MoveMode = Mode;
	
	if (ULocomotionInstance* AnimInst = Cast<ULocomotionInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->SetMoveMode(MoveMode);	
	}
	
	if (ALocomotionController* MyController = Cast<ALocomotionController>(GetController()))
	{
		MyController->ExitAimOffset();
	}
	
	if (MoveMode == FPS)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false; 
		GetCharacterMovement()->bUseControllerDesiredRotation = true; 
		bUseControllerRotationYaw = true;
	}
		
	else if (MoveMode == TPS)
	{
		GetCharacterMovement()->bOrientRotationToMovement = true; 
		GetCharacterMovement()->bUseControllerDesiredRotation = false; 
		bUseControllerRotationYaw = false;
	}
	
	else if (MoveMode == CLIMB)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false; 
		GetCharacterMovement()->bUseControllerDesiredRotation = false; 
		bUseControllerRotationYaw = false;
	}
	
	if (WeaponActor != nullptr)
	{
		WeaponActor->SetActorHiddenInGame(MoveMode != FPS);
	}
}

void ALocomotionCharacter::OnRep_MoveMode()
{
	ApplyMoveMode(MoveMode);
	/*
	if (ULocomotionInstance* AnimInst = Cast<ULocomotionInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->SetMoveMode(MoveMode);	
	}
	
	if (ALocomotionController* MyController = Cast<ALocomotionController>(GetController()))
	{
		MyController->ExistAimOffset();
	}*/
}

void ALocomotionCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

TArray<FHitResult> OutHits;
bool ALocomotionCharacter::IsLanding()
{
	UCapsuleComponent* SelfCapsule = GetCapsuleComponent();
	ULocomotionInstance* AnimInst = Cast<ULocomotionInstance>(GetMesh()->GetAnimInstance());
	
	if (SelfCapsule == nullptr || AnimInst == nullptr)
		return false;
	
	if (GetCharacterMovement()->IsFalling() == false)
		return false;
	
	if (GetVelocity().Z > 0)
		return false;
	
	FVector Start = SelfCapsule->GetComponentLocation();
	FVector End = Start - FVector(0, 0, AnimInst->MoveStateThreshold.LandThreshold);

	float Radius = SelfCapsule->GetScaledCapsuleRadius();
	float HalfHeight = SelfCapsule->GetScaledCapsuleHalfHeight();

	FCollisionShape Shape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	OutHits.Empty();
	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		Shape,
		Params
	);
	
	for (const FHitResult& Hit : OutHits)
	{
		if (GetCharacterMovement()->IsWalkable(Hit) == true)
			return true;
	}
	return false;
}

