#include "LocomotionInstance.h"

void ULocomotionInstance::SelectPivotSequence()
{
	UE_LOG(LogTemp, Warning, TEXT("SelectPivotSequence"));	
	
	if (MoveMode == "TPS")
	{
		CachedPivotSequence = CachedMoveAnimationSet.ForwardPivot;
		return;
	}
	
	float MoveAngle = GetVelocityToForwardAngle();
	
	if (FMath::Abs(MoveAngle) <= 45)
	{
		CachedPivotSequence = CachedMoveAnimationSet.BackPivot;
		return;
	}
	
	if (FMath::Abs(MoveAngle) >= 135)
	{
		CachedPivotSequence = CachedMoveAnimationSet.ForwardPivot;
		return;
	}
	
	if (MoveAngle < 0.f)	
	{
		CachedPivotSequence = CachedMoveAnimationSet.RightPivot;
		return;
	}
	
	CachedPivotSequence = CachedMoveAnimationSet.LeftPivot;
}

void ULocomotionInstance::SelectStartMoveSequence()
{
	if (MoveMode == "TPS")
	{
		CachedStartMoveSequence = CachedMoveAnimationSet.ForwardStartMove;
		return;
	}
	
	float MoveAngle = GetVelocityToForwardAngle();
	
	if (FMath::Abs(MoveAngle) <= 45)
	{
		CachedStartMoveSequence = CachedMoveAnimationSet.ForwardStartMove;
		return;
	}
	
	if (FMath::Abs(MoveAngle) >= 135)
	{
		CachedStartMoveSequence= CachedMoveAnimationSet.BackStartMove;
		return;
	}
	
	if (MoveAngle > 0.f)
	{
		CachedStartMoveSequence= CachedMoveAnimationSet.RightStartMove;
		return;
	}
	
	CachedStartMoveSequence = CachedMoveAnimationSet.LeftStartMove; 
}

void ULocomotionInstance::SelectStopMoveSequence()
{
	if (MoveMode == "TPS")
	{
		CachedStopMoveSequence = CachedMoveAnimationSet.ForwardStoptMove;
		return;
	}
	
	float MoveAngle = GetVelocityToForwardAngle();
	
	if (FMath::Abs(MoveAngle) <= 45)
	{
		CachedStopMoveSequence = CachedMoveAnimationSet.ForwardStoptMove;
		return;
	}
	
	if (FMath::Abs(MoveAngle) >= 135)
	{
		CachedStopMoveSequence= CachedMoveAnimationSet.BackStoptMove;
		return;
	}
	
	if (MoveAngle > 0.f)
	{
		CachedStopMoveSequence= CachedMoveAnimationSet.RightStoptMove;
		return;
	}
	
	CachedStopMoveSequence = CachedMoveAnimationSet.LeftStoptMove;
}

UAnimSequence* ULocomotionInstance::GetRandomIdleBreak()
{
	if (CachedAnimConfig->IdleBreaks.Num() == 0) return nullptr;
	int32 Index = FMath::RandRange(0, CachedAnimConfig->IdleBreaks.Num() - 1);
	return CachedAnimConfig->IdleBreaks[Index];
}