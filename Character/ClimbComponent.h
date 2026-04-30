#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Locomotion/LocomotionAnimation/LocomotionInstance.h"
#include "ClimbComponent.generated.h"

class UCustomMovementComponent;
class ALocomotionCharacter;

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	MOVE_Climb UMETA(DisplayName = "Climb Mode")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LOCOMOTION_API UClimbComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UClimbComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	bool CanClimb();
	void DetectClimbSurface();
	
	void StartClimbing();
	void ClimbUp();
	void FinishClimbUp();
	
	void StopClimbing();
	UFUNCTION(Server, Reliable)
	void StopClimb_Server();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayClimbMontage(UAnimMontage* Montage);
	
	bool DoCapsuleTraceFromComponent(TArray<FHitResult>& Hits);
	FHitResult DoEyeTraceFromComponent(float UpOffset = 0);
	FHitResult DoLineTrace(FVector Start, FVector End);
	
	bool IsReachLedge(float UpOffset);
	bool IsMoveTowardsWall();
	
	void StopUpdateClimbIK();
	void TryUpdateClimbIK(float DeltaTime);
	FHitResult DoSingleIK(FName BoneName);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climb")
	UAnimMontage* StartClimbMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climb")
	UAnimMontage* ClimbUpMontage;
	
public:
	UCustomMovementComponent* MovementComp;
	UCapsuleComponent* ClimbCheckCapsule;
	ALocomotionCharacter* Owner;
	ULocomotionInstance* AnimInst;

	FVector ClimbSurfaceCenter;
	FVector ClimbSurfaceNormal;
	TArray<FHitResult> ClimbSurfaceHits;
	float Offset = 80.f;
	bool IsClimbingUp = false;
};
