// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "LocomotionController.generated.h"

class ULocomotionInstance;
class ALocomotionCharacter;
/**
 * 
 */
UCLASS()
class LOCOMOTION_API ALocomotionController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void AcknowledgePossession(APawn* InPawn) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> WeaponAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> PrimaryAction;
	
public:
	void EnterAimOffset();
	void ExitAimOffset();
	
	void Primary();
	void HandleActionInput();
	void Look(const FInputActionValue&);
	void Move(const FInputActionValue&);
	void ToggleWeapon();
	
	UFUNCTION(Server, Reliable)
	void Toggle_MoveMode_Server();
	
	void ShowMouseDotAndGetDir();
	FTimerHandle TimerHandle;

private:
	UPROPERTY()
	TObjectPtr<ALocomotionCharacter> ClientCharacter;
	
	UPROPERTY()
	TObjectPtr<ULocomotionInstance> ClientAnimInst;
};
