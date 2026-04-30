#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UCharacterAnimConfig.generated.h"

class UBlendSpace1D;
class UAimOffsetBlendSpace;
/**
 * 
 */
UCLASS()
class LOCOMOTION_API UUCharacterAnimConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	//void CopyData(FCharacterAnimConfig& Config);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* Idle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UAnimSequence*> IdleBreaks;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* JumpStart;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* JumpStartLoop;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* JumpApex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* JumpFallLoop;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* JumpLand;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAimOffsetBlendSpace* AimOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBlendSpace1D* Lean;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* EquipWeapon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* AimHipFire;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBlendSpace> ClimbBlendSpace;
};
