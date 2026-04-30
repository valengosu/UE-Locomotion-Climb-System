#include "ClimbComponent.h"
#include "CustomMovementComponent.h"
#include "LocomotionCharacter.h"
#include "GameFramework/PawnMovementComponent.h"

UClimbComponent::UClimbComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UClimbComponent::BeginPlay()
{
	Super::BeginPlay();
	AnimInst = Cast<ULocomotionInstance>(Owner->GetMesh()->GetAnimInstance());
}

bool UClimbComponent::CanClimb()
{
	if (Owner->GetMovementComponent()->IsFalling() == true)
		return false;
	
	if (ClimbSurfaceNormal == FVector::ZeroVector || ClimbSurfaceCenter == FVector::ZeroVector)
		return false;
	
	float Dot = FVector::DotProduct(ClimbSurfaceNormal, FVector::UpVector);
	if (Dot > 0.5f)
		return false;
	
	return true;
}

void UClimbComponent::DetectClimbSurface()
{
	DoCapsuleTraceFromComponent(ClimbSurfaceHits);
	
	ClimbSurfaceNormal = FVector::ZeroVector;
	ClimbSurfaceCenter = FVector::ZeroVector;
	
	for (auto Hit : ClimbSurfaceHits)
	{
		ClimbSurfaceCenter += Hit.ImpactPoint;
		ClimbSurfaceNormal += Hit.ImpactNormal;
	}
	
	if (ClimbSurfaceHits.Num() >= 1)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Weight %d"), ClimbSurfaceHits.Num());
		ClimbSurfaceNormal = ClimbSurfaceNormal.GetSafeNormal();
		ClimbSurfaceCenter = ClimbSurfaceCenter / ClimbSurfaceHits.Num();
	}
	//UE_LOG(LogTemp, Warning, TEXT("Vector: %s"), *ClimbSurfaceNormal.ToString());
}

int test = 0;
void UClimbComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(AnimInst->Montage_IsPlaying(ClimbUpMontage) == true)
	{
		IsClimbingUp = true;
		return;
	}
	
	if (IsClimbingUp == true)
	{
		IsClimbingUp = false;
		FinishClimbUp();
	}
	
	DetectClimbSurface();
	/*for (auto Hit : ClimbSurfaceHits)
	{
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 5.f,12,FColor::Red,false,0);
	}*/
	
	TryUpdateClimbIK(DeltaTime);
	
	if (GetOwnerRole() == ROLE_Authority || GetOwnerRole() == ROLE_AutonomousProxy)
	{
		if (Owner->MoveMode != CLIMB && CanClimb() == true && DoEyeTraceFromComponent().bBlockingHit == true && IsMoveTowardsWall() == true)
		{
			StartClimbing();
			return;
		}	
		
		if (Owner->MoveMode == CLIMB && CanClimb() == false)
		{
			StopClimbing();
			return;
		}
		
		if (Owner->MoveMode == CLIMB && CanClimb() == true)
		{
			if (IsReachLedge(50) == true)
			{
				ClimbUp();
				StopUpdateClimbIK();
				return;
			}
			
			//UE_LOG(LogTemp, Warning, TEXT("Test %d"), test++);
			MovementComp->ClimbSurfaceCenter = ClimbSurfaceCenter;
			MovementComp->ClimbSurfaceNormal = ClimbSurfaceNormal;
			return;
		}
	}
}

bool UClimbComponent::IsMoveTowardsWall()
{
	float CurrentSpeed = Owner->GetVelocity().Size();
	float MaxSpeed = MovementComp->MaxWalkSpeed;
	if (CurrentSpeed < MaxSpeed * 0.8f)
		return false;
	
	FVector VelocityDir = Owner->GetVelocity().GetSafeNormal();
	float Dot = FVector::DotProduct(VelocityDir, -ClimbSurfaceNormal);
	if (Dot < 0.966f)
		return false;
	
	return true;
}

void UClimbComponent::StartClimbing()
{
	Owner->ApplyMoveMode(CLIMB);
	MovementComp->SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::MOVE_Climb);
	
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Owner->PlayAnimMontage(StartClimbMontage);
	}
	else if(GetOwnerRole() == ROLE_Authority)
	{
		Multicast_PlayClimbMontage(StartClimbMontage);
	}
}

void UClimbComponent::ClimbUp()
{
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Owner->PlayAnimMontage(ClimbUpMontage);
	}
	else if(GetOwnerRole() == ROLE_Authority)
	{
		Multicast_PlayClimbMontage(ClimbUpMontage);
	}
}

void UClimbComponent::FinishClimbUp()
{
	Owner->ApplyMoveMode(TPS);
	MovementComp->SetMovementMode(MOVE_Walking);
	MovementComp->StopMovementImmediately();
}

void UClimbComponent::StopClimbing()
{
	Owner->ApplyMoveMode(TPS);
	MovementComp->SetMovementMode(MOVE_Falling);
	MovementComp->StopMovementImmediately();
}

void UClimbComponent::StopClimb_Server_Implementation()
{
	StopClimbing();
}

void UClimbComponent::Multicast_PlayClimbMontage_Implementation(UAnimMontage* Montage)
{
	if (GetOwnerRole() != ROLE_AutonomousProxy)
	{
		Owner->PlayAnimMontage(Montage);	
	}
}

static const FVector IK_Invalid(FLT_MAX);
void UClimbComponent::TryUpdateClimbIK(float DeltaTime)
{
	if (Owner->MoveMode != CLIMB || CanClimb() == false ||
		AnimInst->Montage_IsPlaying(StartClimbMontage) == true || AnimInst->Montage_IsPlaying(ClimbUpMontage) == true)
	{
		StopUpdateClimbIK();
		return;
	}
	
	FHitResult Hit;
	FVector IKPosition = FVector::ZeroVector;
	
	Hit = DoSingleIK("ik_hand_l");
	AnimInst->IsValid_IK_HandL = Hit.bBlockingHit;
	IKPosition = Hit.ImpactPoint + Hit.ImpactNormal * 5.f;
	AnimInst->IK_HandL = AnimInst->IK_HandL == IK_Invalid ? IKPosition : FMath::VInterpTo(AnimInst->IK_HandL, IKPosition, DeltaTime, 10.f);
	
	Hit = DoSingleIK("ik_hand_r");
	AnimInst->IsValid_IK_HandR = Hit.bBlockingHit;
	IKPosition = Hit.ImpactPoint + Hit.ImpactNormal * 5.f;
	AnimInst->IK_HandR = AnimInst->IK_HandR == IK_Invalid ? IKPosition : FMath::VInterpTo(AnimInst->IK_HandR, IKPosition, DeltaTime, 10.f);
	
	Hit = DoSingleIK("ik_foot_l");
	AnimInst->IsValid_IK_FootL = Hit.bBlockingHit;
	IKPosition = Hit.ImpactPoint + Hit.ImpactNormal * 20.f;
	AnimInst->IK_FootL = AnimInst->IK_FootL == IK_Invalid ? IKPosition : FMath::VInterpTo(AnimInst->IK_FootL, IKPosition, DeltaTime, 10.f);
	
	Hit = DoSingleIK("ik_foot_r");
	AnimInst->IsValid_IK_FootR = Hit.bBlockingHit;
	IKPosition = Hit.ImpactPoint + Hit.ImpactNormal * 20.f;
	AnimInst->IK_FootR = AnimInst->IK_FootR == IK_Invalid ? IKPosition : FMath::VInterpTo(AnimInst->IK_FootR, IKPosition, DeltaTime, 10.f);
}

void UClimbComponent::StopUpdateClimbIK()
{
	AnimInst->IsValid_IK_HandL = false;
	AnimInst->IK_HandL = IK_Invalid;
		
	AnimInst->IsValid_IK_HandR = false;
	AnimInst->IK_HandR = IK_Invalid;
		
	AnimInst->IsValid_IK_FootL = false;
	AnimInst->IK_FootL = IK_Invalid;
		
	AnimInst->IsValid_IK_FootR = false;
	AnimInst->IK_FootR = IK_Invalid;
}

TArray<FVector> Dirs;
FHitResult UClimbComponent::DoSingleIK(FName BoneName)
{
	FVector BoneWorldPos = Owner->GetMesh()->GetSocketLocation(BoneName);
	
	Dirs.Empty();
	Dirs.Add(ClimbSurfaceNormal);
	Dirs.Add(ClimbSurfaceNormal.RotateAngleAxis(90.f, FVector::UpVector));
	Dirs.Add(ClimbSurfaceNormal.RotateAngleAxis(-90.f, FVector::UpVector));
	Dirs.Add(ClimbSurfaceNormal.RotateAngleAxis(60.f, FVector::UpVector));
	Dirs.Add(ClimbSurfaceNormal.RotateAngleAxis(-60.f, FVector::UpVector));
	Dirs.Add(ClimbSurfaceNormal.RotateAngleAxis(45.f, FVector::UpVector));
	Dirs.Add(ClimbSurfaceNormal.RotateAngleAxis(-45.f, FVector::UpVector));
	Dirs.Add(ClimbSurfaceNormal.RotateAngleAxis(30.f, FVector::UpVector));
	Dirs.Add(ClimbSurfaceNormal.RotateAngleAxis(-30.f, FVector::UpVector));
	
	for (int i = 0; i < Dirs.Num(); i++)
	{
		FVector Start = BoneWorldPos + Dirs[i] * 20.f; 
		FVector End = BoneWorldPos - Dirs[i] * (50.f + 60 * (i != 0 ? 1 : 0));
		
		FHitResult Hit = DoLineTrace(Start, End);
		if (Hit.bBlockingHit == false)
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 0.f, 0, 1.f);
		}
		if (Hit.bBlockingHit)
		{
			DrawDebugSphere(GetWorld(),Hit.ImpactPoint,5.f,8,FColor::Red,false,0.f);
			return Hit;	
		}
	}
	return FHitResult();
}

bool UClimbComponent::DoCapsuleTraceFromComponent(TArray<FHitResult>& Hits)
{
	const FVector Start = ClimbCheckCapsule->GetComponentLocation();
	const FVector End = Start + ClimbCheckCapsule->GetForwardVector() * Offset;

	const float Radius = ClimbCheckCapsule->GetScaledCapsuleRadius();
	const float HalfH = ClimbCheckCapsule->GetScaledCapsuleHalfHeight();
	const FQuat Rotation = ClimbCheckCapsule->GetComponentQuat();

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	
	Hits.Empty();
	bool bo = GetWorld()->SweepMultiByObjectType(Hits,Start, End, Rotation, ObjParams, FCollisionShape::MakeCapsule(Radius, HalfH), Params);
	DrawDebugCapsule(GetWorld(), End,HalfH, Radius, Rotation, FColor::Red, false,0);
	
	return bo;
}

FHitResult UClimbComponent::DoEyeTraceFromComponent(float UpOffset)
{
	FVector EyeLocation;
	FRotator EyeRotation;
	GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector Start = EyeLocation + UpOffset * Owner->GetActorUpVector();
	FVector End = Start + GetOwner()->GetActorForwardVector() * Offset;
	
	FHitResult Hit = DoLineTrace(Start, End);
	DrawDebugLine(GetWorld(), Start, End, FColor::Green,false,0,0,1.f);
	return Hit;
}

FHitResult UClimbComponent::DoLineTrace(FVector Start, FVector End)
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FHitResult Hit;
	GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, ObjectParams, Params);
	return Hit;
}

bool UClimbComponent::IsReachLedge(float UpOffset)
{
	FHitResult Hit;
	Hit = DoEyeTraceFromComponent(UpOffset);
	if (Hit.bBlockingHit == true)
		return false;
	
	FVector DownStart = Hit.TraceEnd;
	FVector DownEnd = DownStart + -Owner->GetActorUpVector() * 100.f;
	Hit = DoLineTrace(DownStart, DownEnd);
	
	if (Hit.bBlockingHit)
	{
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint,5.f,12, FColor::Red,false,1.0f);
	}
	
	return Hit.bBlockingHit;
}


