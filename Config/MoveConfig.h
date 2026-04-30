// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MoveConfig.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMoveAnimationSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> ForwardStartMove;
    	
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UAnimSequence> BackStartMove;
    	
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UAnimSequence> LeftStartMove;
    	
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UAnimSequence> RightStartMove;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> ForwardStoptMove;
    	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> BackStoptMove;
    	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> LeftStoptMove;
    	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> RightStoptMove;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> ForwardPivot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> BackPivot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> LeftPivot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> RightPivot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBlendSpace> MoveBlendSpace;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> TurnLeft;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> TurnRight;
};

UCLASS()
class LOCOMOTION_API UMoveConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FMoveAnimationSet> MovementMap;
};
