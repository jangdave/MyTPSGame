// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPSPlayerBaseComponent.h"
#include "TPSPlayerMoveComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTPSGAME_API UTPSPlayerMoveComponent : public UTPSPlayerBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTPSPlayerMoveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SetupPlayerInput(class UInputComponent* PlayerInputComponent) override;

	void OnAxisHorizontal(float value);

	void OnAxisVertical(float value);

	void OnAxisLookUp(float value);

	void OnAxisTurnRight(float value);

	void OnActionJump();

	void OnActionRunPressed();

	void OnActionRunReleased();

	void OnActionCouchPressed();

	void OnActionCouchReleased();

	float runSpeed = 600.0f;

	float walkSpeed = 400.0f;

	float crouchSpeed = 300.0f;

	FVector direction;
};
