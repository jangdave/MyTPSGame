// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPSPlayerAnim.generated.h"

/**
 * 
 */
UCLASS()
class MYTPSGAME_API UTPSPlayerAnim : public UAnimInstance
{
	GENERATED_BODY()

public:

	void NativeUpdateAnimation(float DeltaSeconds) override;

	//forwordVelocity, rightVelocity, isInAir
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float forwordVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float rightVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isInAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isCrouching;

	UPROPERTY(EditAnywhere)
	class UAnimMontage* fireMontageFactory;

	UFUNCTION(BlueprintCallable)
	void OnFire(FName sectionName);

	UFUNCTION(BlueprintCallable)
	void OnGunReload();

	UFUNCTION(BlueprintCallable)
	void OnSniperReload();
};
