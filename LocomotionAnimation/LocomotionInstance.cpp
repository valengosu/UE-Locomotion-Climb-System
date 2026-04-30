#include "LocomotionInstance.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Locomotion/Character/ClimbComponent.h"
#include "Locomotion/Character/LocomotionCharacter.h"

void ULocomotionInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	
	CachedAnimConfig = AnimConfig;
	SetMoveMode(TPS);
}

void ULocomotionInstance::NativeUpdateAnimation(float DeltaSeconds)
{ 
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (GetCharacter() == nullptr)
		return;
	
	if (MoveMode == FPS)
	{	
		IsFPSMode = true;
		AimOffsetSettings.Alpha = 0;
		LeanSettings.Alpha = 0;
		MoveRight = 0;
		MoveForward = 0;
		float TargetAngle = 0;
		MontageBlendWeight = GetUpperBodyMontageWeight(0.25);
		
		CalculateFPSMoveDirection(MoveRight, MoveForward, TargetAngle);
		OrientAngle = FMath::FInterpConstantTo(OrientAngle, TargetAngle, DeltaSeconds, GetCharacter()->GetCharacterMovement()->RotationRate.Yaw);
		
		UpdateGroundAirState(DeltaSeconds);
	}
	
	else if (MoveMode == TPS)
	{
		IsFPSMode = false;
		AimOffsetSettings.Alpha = FMath::FInterpConstantTo(AimOffsetSettings.Alpha, AimOffsetAlpha, DeltaSeconds, 2.f);
		UpdateLeanSetting(DeltaSeconds);
		MoveRight = 0;
		MoveForward = 0;
		MontageBlendWeight = 0;
		OrientAngle = 0;
		
		UpdateGroundAirState(DeltaSeconds);
	}
	
	else if (MoveMode == CLIMB)
	{
		IsFPSMode = false;
		AimOffsetSettings.Alpha = 0;
		LeanSettings.Alpha = 0;
		MoveRight = 0;
		MoveForward = 0;
		MontageBlendWeight = 0;
		OrientAngle = 0;
		
		CalculateClimbMoveDirection(MoveRight, MoveForward);
		
		UpdateClimbState();
	}
}

void ULocomotionInstance::UpdateGroundAirState(float DeltaSeconds)
{
	UpdateSpeed2DRatio(Speed2DRatio);
	UpdatePivotState(DeltaSeconds);
	UpdatePrevVelocityDir(DeltaSeconds);
	
	MoveState.Reset();
	
	if(GetCharacter()->GetMovementComponent()->IsFalling() == true)
	{
		MoveState.InAir = true;
		
		float Z = GetCharacter()->GetVelocity().Z;
		
		MoveState.IsJumpUp = Z > 0.f;
		MoveState.IsJumpAtApex = FMath::Abs(Z) < MoveStateThreshold.ApexThreshold;
		MoveState.IsJumpFall = Z < -MoveStateThreshold.ApexThreshold;
		MoveState.IsLanding = GetCharacter()->IsLanding();
	}
	
	else if (GetCharacter()->GetCharacterMovement()->IsFalling() == false)
	{
		MoveState.IsLanding = true;
		
		MoveState.IsIdle = Speed2DRatio < MoveStateThreshold.IdleThreshold;
		MoveState.IsStartMove = Speed2DRatio > MoveStateThreshold.StartThreshold;
		MoveState.IsMove = Speed2DRatio > MoveStateThreshold.MoveThreshold;
		MoveState.IsStopMove = UpdateStopState(DeltaSeconds) && MoveState.IsPivot == false;
		MoveState.IsTurn = ShouldTurn() && Speed2DRatio < MoveStateThreshold.IdleThreshold;
	}
}

void ULocomotionInstance::UpdateClimbState()
{
	MoveState.IsClimb = true;
	MoveState.IsClimbUp = GetCharacter()->ClimbComponent->IsClimbingUp; //NOT used now
	//UE_LOG(LogTemp, Warning, TEXT("IsClimb %d"), MoveState.IsClimb);
}

void ULocomotionInstance::CalculateClimbMoveDirection(float& MoveRightRef, float& MoveUpRef)
{
	FVector LocalVel = GetCharacter()->GetActorQuat().UnrotateVector(GetCharacter()->GetVelocity());
	float Y = LocalVel.Y;
	float Z = LocalVel.Z;
	
	MoveRightRef = 0.f;
	MoveUpRef = 0.f;
	
	if (FMath::Abs(Y) < 0.01f && FMath::Abs(Z) < 0.01f)
	{
		return;
	}
	
	if (FMath::Abs(Z) >= FMath::Abs(Y))
	{
		MoveUpRef = (Z > 0.f) ? 1.f : -1.f;
	}
	else
	{
		MoveRightRef = (Y > 0.f) ? 1.f : -1.f;
	}
}

bool ULocomotionInstance::UpdateStopState(float DeltaSeconds)
{
	StopGraceTime = Speed2DRatio >= MoveStateThreshold.StopThreshold ? 0 : StopGraceTime + DeltaSeconds;
	return StopGraceTime >= GraceDuration * 0.5;
}

void ULocomotionInstance::UpdatePivotState(float DeltaSeconds)
{
	if (MoveState.IsPivot == true && PivotGraceTime < GraceDuration)
	{
		//UE_LOG(LogTemp, Warning, TEXT("PivotGraceTime %f"), PivotGraceTime);
		PivotGraceTime += DeltaSeconds;
		return;
	}
	
	FVector CurrentVelocity = GetCharacter()->GetVelocity();
	CurrentVelocity = CurrentVelocity.GetSafeNormal2D();
	
	float Dot = FVector::DotProduct(CurrentVelocity, PrevVelocityDir);
	Dot = FMath::Clamp(Dot, -1.f, 1.f);
	float CrossZ = FVector::CrossProduct(PrevVelocityDir, CurrentVelocity).Z;
	float Angle = FMath::RadiansToDegrees(FMath::Atan2(CrossZ, Dot));
	
	MoveState.IsPivot = FMath::Abs(Angle) > MoveStateThreshold.PivotThreshold;
	PivotGraceTime = MoveState.IsPivot == true ? 0 : GraceDuration; 
}

void ULocomotionInstance::SetMoveMode(const FString& InMoveMode)
{
	this->MoveMode = InMoveMode;
	
	if (auto* Found = MoveConfig->MovementMap.Find(InMoveMode))
	{
		CachedMoveAnimationSet = *Found;
	}
	
	if (InMoveMode == FPS)
	{
		Montage_Play(CachedAnimConfig->EquipWeapon);
	}
}

float ULocomotionInstance::GetUpperBodyMontageWeight(float FadeOutTime)
{
	FAnimMontageInstance* MontageInst = GetActiveMontageInstance();
	if (MontageInst == nullptr)
		return 0;
	
	float Position = MontageInst->GetPosition();
	float Length   = MontageInst->Montage->GetPlayLength();
	float RemainTime = Length - Position;
	
	if (RemainTime > FadeOutTime)
		return 1;
	
	float Weight = RemainTime / FadeOutTime;
	//UE_LOG(LogTemp, Warning, TEXT("Weight %f"), Weight);
	return FMath::Clamp(Weight, 0, 1);
}

void ULocomotionInstance::UpdateAimOffset(float Yaw, float Pitch)
{
	const FBlendParameter& ParamX = AnimConfig->AimOffset->GetBlendParameter(0);
	float MinX = ParamX.Min;
	float MaxX = ParamX.Max;
	AimOffsetSettings.Yaw = FMath::Clamp(Yaw, MinX, MaxX);
	
	const FBlendParameter& ParamY = AnimConfig->AimOffset->GetBlendParameter(1);
	float MinY = ParamY.Min;
	float MaxY = ParamY.Max;
	AimOffsetSettings.Pitch = FMath::Clamp(Pitch, MinY, MaxY);
}

ALocomotionCharacter* ULocomotionInstance::GetCharacter()
{
	return Cast<ALocomotionCharacter>(TryGetPawnOwner());
}

bool ULocomotionInstance::ShouldTurn()
{
	if (MoveMode != FPS)
		return false;
	
	FVector CurrentFwd = GetCharacter()->GetActorForwardVector();
	CurrentFwd = CurrentFwd.GetSafeNormal2D();
	
	float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CurrentFwd, PrevForwardDir)));
	FVector CrossProd = FVector::CrossProduct(PrevForwardDir, CurrentFwd);
	
	PrevForwardDir = CurrentFwd;
	
	if (Angle < MoveStateThreshold.TurnEnterThreshold)
		return false;
	
	if (CrossProd.Z > 0)
		CachedTurnSequence = CachedMoveAnimationSet.TurnRight;
	else 
		CachedTurnSequence = CachedMoveAnimationSet.TurnLeft;
		
	return true;
}

float ULocomotionInstance::GetVelocityToForwardAngle()
{
	FVector CurrentVelocity = GetCharacter()->GetVelocity().GetSafeNormal2D();
	FVector ForwardDir = GetCharacter()->GetActorForwardVector().GetSafeNormal2D();
	
	float Dot = FVector::DotProduct(CurrentVelocity, ForwardDir);
	Dot = FMath::Clamp(Dot, -1.f, 1.f);
	float CrossZ = FVector::CrossProduct(ForwardDir, CurrentVelocity).Z;
	float Angle = FMath::RadiansToDegrees(FMath::Atan2(CrossZ, Dot));
	
	return Angle;
}

void ULocomotionInstance::UpdatePrevVelocityDir(float DeltaSeconds)
{
	FVector CurrentVelocity = GetCharacter()->GetVelocity();
	CurrentVelocity.Z = 0;
	
	if (CurrentVelocity.Size2D() < 0.1f && VelocityGraceTime < GraceDuration)
	{
		VelocityGraceTime += DeltaSeconds;
		return;
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("VelocityGraceTime %f"), VelocityGraceTime);
	VelocityGraceTime = 0;
	PrevVelocityDir = CurrentVelocity.GetSafeNormal2D();
}

void ULocomotionInstance::CalculateFPSMoveDirection(float& MoveRightRef, float& MoveForwardRef, float& OutWarpAngle)
{
	float MoveAngle = GetVelocityToForwardAngle();
	MoveAngle = FMath::RoundToFloat(MoveAngle / 45.f) * 45.f;
	
	float Rad = FMath::DegreesToRadians(MoveAngle);
	float RawY = FMath::Cos(Rad);
	float RawX = FMath::Sin(Rad);
	float BaseAngle = 0;
	
	MoveRightRef = 0.f;
	MoveForwardRef = 0.f;
	
	if (FMath::Abs(RawY) >= FMath::Abs(RawX))
	{
		MoveForwardRef = (RawY >= 0.f) ? 1.f : -1.f;
		BaseAngle = (RawY >= 0.f) ? 0.f : -180.f;
	}
	else
	{
		MoveRightRef = (RawX >= 0.f) ? 1.f : -1.f;
		BaseAngle = (RawX >= 0.f) ? 90.f : -90.f;
	}
	
	OutWarpAngle = FMath::UnwindDegrees(MoveAngle - BaseAngle);
	/*
	IsTest = false;
	OutWarpAngle = FMath::UnwindDegrees(ControlToInputAngle - BaseAngle);
	
	//for test...
	if (FMath::Abs(ControlToInputAngle - LastControlToInputAngle) > 30)
	{
		//UE_LOG(LogTemp, Warning, TEXT("OutWarpAngle %f"), OutWarpAngle);
		//RequestSlotGroupInertialization(FName("DefaultGroup.FullBody"), 1.25);
		IsTest = true;
	}
	LastControlToInputAngle = ControlToInputAngle;
	*/
}

void ULocomotionInstance::UpdateSpeed2DRatio(float& Ratio)
{
	float Speed2D = GetCharacter()->GetVelocity().Size2D();
	float MaxSpeed2D = GetCharacter()->GetCharacterMovement()->MaxWalkSpeed;
	Ratio = Speed2D / MaxSpeed2D;
}

float ULocomotionInstance::GetPathCurvature()
{
	FVector CurrentLocation = GetCharacter()->GetActorLocation();
	
	FVector CurrMoveDir = (CurrentLocation - PrevLocation).GetSafeNormal2D();
	FVector PrevMoveDir = (PrevLocation - PrevPrevLocation).GetSafeNormal2D();
	
	PrevPrevLocation = PrevLocation;
	PrevLocation = CurrentLocation;
	
	float Dot = FVector::DotProduct(PrevMoveDir, CurrMoveDir);
	Dot = FMath::Clamp(Dot, -1.f, 1.f);
	float Angle = FMath::Acos(Dot);
	
	return FMath::RadiansToDegrees(Angle);
}

void ULocomotionInstance::UpdateLeanSetting(float DeltaSeconds)
{
	float TargetAlpha = 1.f;
	
	if (MoveState.IsMove == false)
	{
		TargetAlpha = 0.f;
	}	
	
	else if (FMath::Abs(GetPathCurvature()) < 1.2f)
	{
		TargetAlpha = 0.f;
	}
	
	LeanSettings.Alpha = FMath::FInterpConstantTo(LeanSettings.Alpha, TargetAlpha,DeltaSeconds,5.f);
	
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(GetCharacter()->GetVelocity().Rotation(),GetCharacter()->GetActorRotation());
	float LeanYaw = GetCharacter()->GetVelocity().Size2D() < 5 ? 0 : -Delta.Yaw;
	
	const FBlendParameter& ParamX = AnimConfig->Lean->GetBlendParameter(0);
	float MinX = ParamX.Min;
	float MaxX = ParamX.Max;
	LeanSettings.LeanAngle = FMath::Clamp(LeanYaw, MinX, MaxX);
}

