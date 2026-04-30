#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Locomotion/Config/MoveConfig.h"
#include "Locomotion/Config/UCharacterAnimConfig.h"
#include "LocomotionInstance.generated.h"

const FString FPS = "FPS";
const FString TPS = "TPS";
const FString CLIMB = "Climb";

class ALocomotionCharacter;
class UUCharacterAnimConfig;
struct FCharacterAnimConfig;

USTRUCT(BlueprintType)
struct FMoveState
{
	GENERATED_BODY()

    void Reset()
    {
		IsIdle = false;
    	IsMove = false;
		IsJumpAtApex = false;
		IsJumpFall = false;
		IsJumpUp = false;
		IsLanding = false;
		InAir = false;
		//IsPivot = false;
		IsTurn = false;
		IsClimb = false;
		IsClimbUp = false;
    }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsIdle;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsStartMove;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsStopMove;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsMove;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsJumpUp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsJumpFall;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsJumpAtApex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsLanding;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool InAir;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsPivot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsTurn;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsClimb;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsClimbUp;
};

USTRUCT(BlueprintType)
struct FMoveStateThreshold
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdleThreshold  = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartThreshold = 0.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveThreshold  = 0.5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StopThreshold  = 0.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ApexThreshold = 25.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LandThreshold = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PivotThreshold = 120.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TurnEnterThreshold = 2.f;
};

USTRUCT(BlueprintType)
struct FAimOffsetSettings
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Yaw = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Pitch = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Alpha = 0.0f;
};

USTRUCT(BlueprintType)
struct FLeanSettings
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float LeanAngle = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Alpha = 0.0f;
};

UCLASS()
class LOCOMOTION_API ULocomotionInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
public:
	void SetMoveMode(const FString& InMoveMode);
	
	void UpdateAimOffset(float Yaw, float Pitch);
	void UpdateLeanSetting(float DeltaSeconds);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FMoveState MoveState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMoveStateThreshold MoveStateThreshold;

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	UAnimSequence* GetRandomIdleBreak();
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SelectPivotSequence();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAnimSequence* CachedPivotSequence;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SelectStartMoveSequence();
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequence* CachedStartMoveSequence;
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintThreadSafe))
	void SelectStopMoveSequence();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAnimSequence* CachedStopMoveSequence;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAnimSequence* CachedTurnSequence;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UUCharacterAnimConfig* AnimConfig;	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UUCharacterAnimConfig> CachedAnimConfig;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FAimOffsetSettings AimOffsetSettings; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FLeanSettings LeanSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMoveConfig* MoveConfig;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FMoveAnimationSet CachedMoveAnimationSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoveRight = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MoveForward = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float GlobalPlayRate = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MontageBlendWeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsFPSMode = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float OrientAngle = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector IK_HandL;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsValid_IK_HandL;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector IK_HandR;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsValid_IK_HandR;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector IK_FootL;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsValid_IK_FootL;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector IK_FootR;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsValid_IK_FootR;
	
	FString MoveMode;
	float AimOffsetAlpha = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsTest = false;
	
private:
	ALocomotionCharacter *GetCharacter();
	
	float StopTimer = 0.f;
	float MaxStopTime = 0.1;
	
	bool ShouldTurn();
	void CalculateFPSMoveDirection(float& MoveRight, float& MoveForward, float& OutWarpAngle);
	void CalculateClimbMoveDirection(float& MoveRight, float& MoveUp);
	float GetUpperBodyMontageWeight(float FadeOutTime);
	void UpdateGroundAirState(float DeltaSeconds);
	void UpdateClimbState();

private:
	void UpdateSpeed2DRatio(float& Ratio);
	float Speed2DRatio = 0.f;
	
	float GetPathCurvature();
	float GetVelocityToForwardAngle();
	
	bool UpdateStopState(float DeltaSeconds);
	float StopGraceTime = 0.f;
	
	void UpdatePivotState(float DeltaSeconds);
	float PivotGraceTime = 0.f;
	
	void UpdatePrevVelocityDir(float DeltaSeconds);
	float VelocityGraceTime = 0;
	float GraceDuration = 0.15f;
	
	FVector PrevPrevLocation = FVector::ZeroVector;
	FVector PrevLocation = FVector::ZeroVector;
	FVector PrevVelocityDir = FVector::ZeroVector;
	FVector PrevForwardDir = FVector::ZeroVector;
};
