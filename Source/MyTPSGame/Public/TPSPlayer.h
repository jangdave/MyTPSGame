// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSPlayer.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FSetupInputDelegate, class UInputComponent*);

class USpringArmComponent; //전방선언

#define GRENADE_GUN true
#define SNIPER_GUN false

UCLASS()
class MYTPSGAME_API ATPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	const bool AGRENADE_GUN = true; //const 상수

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
	class UTPSPlayerMoveComponent* moveComp;

	UPROPERTY(EditAnywhere)
	class UTPSPlayerFireComponent* fireComp;

	//-----------------------------------------------------------

	UPROPERTY(EditAnywhere)
	class USpringArmComponent* springArmComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UCameraComponent* cameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USkeletalMeshComponent* gunMeshComp;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* sniperMeshComp;

	FSetupInputDelegate setupInputDelegate;

	void OnHit(int damage);

	UPROPERTY()
	int HP;

	UPROPERTY()
	int MaxHP = 2;

	UFUNCTION(BlueprintImplementableEvent)
	void OnMyGameOver();

	UFUNCTION(BlueprintImplementableEvent)
	void OnMyChooseGun(bool isGrenadeGun);

	UFUNCTION(BlueprintImplementableEvent)
	void OnMyGrenadeGunAmmoUpdate(int current, int max);

	UFUNCTION(BlueprintImplementableEvent)
	void OnMySniperGunAmmoUpdate(int current, int max);

};