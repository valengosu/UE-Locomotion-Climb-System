#include "CustomMovementComponent.h"
#include "ClimbComponent.h"
#include "LocomotionCharacter.h"

UCustomMovementComponent::UCustomMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCustomMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCustomMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::MOVE_Climb)
	{
		PhysClimb(deltaTime, Iterations);
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("How I come up here???"));
	Super::PhysCustom(deltaTime, Iterations);
}

float UCustomMovementComponent::GetMaxSpeed() const
{
	if (MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::MOVE_Climb)
		return ClimbMaxSpeed;
	return Super::GetMaxSpeed();
}

FQuat UCustomMovementComponent::GetComponentQuat(float DeltaTime)
{
	FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();
	
	if(HasAnimRootMotion() && CurrentRootMotion.HasOverrideVelocity())
		return CurrentQuat;
	
	FQuat TargetQuat = FRotationMatrix::MakeFromX(-ClimbSurfaceNormal).ToQuat();
	CurrentQuat = FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, 5);
	CurrentQuat.Normalize();  
	
	return CurrentQuat;
}

void UCustomMovementComponent::SnapMovementToSurface(float DeltaTime)
{
	//FVector Forward = UpdatedComponent->GetForwardVector();
	FVector Location = UpdatedComponent->GetComponentLocation();
	
	float SnapLength = (ClimbSurfaceCenter - Location).ProjectOnTo(-ClimbSurfaceNormal).Length();
	FVector SnapVector = -ClimbSurfaceNormal * SnapLength;
	//UE_LOG(LogTemp, Warning, TEXT("V = %s"), *SnapVector.ToString());	
	UpdatedComponent->MoveComponent(SnapVector * DeltaTime * ClimbMaxSpeed, UpdatedComponent->GetComponentQuat(), true);
}

void UCustomMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCustomMovementComponent::PhysClimb(float deltaTime, int32 Iterations)
{
	if (GetOwnerRole() != ROLE_Authority && GetOwnerRole() != ROLE_AutonomousProxy)
		return;
	
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	
	RestorePreAdditiveRootMotionVelocity();

	if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() )
	{
		CalcVelocity(deltaTime, 0, true, 999);
	}

	ApplyRootMotionToVelocity(deltaTime);
	
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	
	if(!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		FQuat NewQuat = GetComponentQuat(deltaTime);
		UpdatedComponent->SetWorldRotation(NewQuat);
	}
	
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		HandleImpact(Hit, deltaTime, Adjusted);
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}

	if(!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
		SnapMovementToSurface(deltaTime);
	}
}

