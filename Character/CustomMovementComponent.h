#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomMovementComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LOCOMOTION_API UCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UCustomMovementComponent();

protected:
	virtual void BeginPlay() override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual float GetMaxSpeed() const override;
	
	FQuat GetComponentQuat(float DeltaTime);
	void SnapMovementToSurface(float DeltaTime);

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	void PhysClimb(float deltaTime, int32 Iterations);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
	float ClimbMaxSpeed = 120.f;
	
	FVector ClimbSurfaceCenter;
	FVector ClimbSurfaceNormal;
};
