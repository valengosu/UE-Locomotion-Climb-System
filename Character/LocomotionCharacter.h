#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Locomotion/LocomotionAnimation/LocomotionInstance.h"
#include "LocomotionCharacter.generated.h"

class UClimbComponent;

UCLASS()
class LOCOMOTION_API ALocomotionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALocomotionCharacter(const FObjectInitializer& OI);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	bool IsLanding();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCapsuleComponent* ClimbCheckCapsule;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UClimbComponent* ClimbComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	TSubclassOf<AStaticMeshActor> WeaponClass;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponActor)
	TObjectPtr<AStaticMeshActor> WeaponActor;
	UFUNCTION()
	void OnRep_WeaponActor();
	
	UPROPERTY(ReplicatedUsing = OnRep_MoveMode)
	FString MoveMode = TPS;
	UFUNCTION()
	void OnRep_MoveMode();
	
	void ApplyMoveMode(FString Mode);
};

namespace Debug
{
	static void Print(const FString& Message, FColor Color = FColor::MakeRandomColor(), int Order = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(Order, 6.f, Color, Message);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		}
	}
};