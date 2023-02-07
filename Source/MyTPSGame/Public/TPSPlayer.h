// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSPlayer.generated.h"

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
	class USpringArmComponent* springArmComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UCameraComponent* cameraComp;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABulletActor> bulletFactory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USkeletalMeshComponent* gunMeshComp;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* sniperMeshComp;

	void OnAxisHorizontal(float value);

	void OnAxisVertical(float value);

	void OnAxisLookUp(float value);
	
	void OnAxisTurnRight(float value);
	
	void OnActionJump();

	void OnActionRunPressed();

	void OnActionRunReleased();

	void OnActionCouchPressed();

	void OnActionCouchReleased();

	void OnActionFirePressed();

	void OnActionFireRelesed();

	void DoFire();

	float runSpeed = 600.0f;

	float walkSpeed = 400.0f;

	float crouchSpeed = 200.0f;
	
	FVector direction;
	
	FTimerHandle fireTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bChooseGrenadeGun;

	void ChooseGun(bool bGrenade);

	UPROPERTY(EditAnywhere)
	float fireInterval = 0.5f;

	//위젯 공장에서 위젯을 생성
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> crosshairFactory;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> sniperFactory;

	UPROPERTY()
	class UUserWidget* crosshairUI;

	UPROPERTY()
	class UUserWidget* sniperUI;

	void OnActionGrenade();

	void OnActionSniper();

	void OnActionZoomIn();
	
	void OnActionZoomOut();

	UPROPERTY(EditDefaultsOnly)
	class UParticleSystem* bulletImpactFactory;

	UPROPERTY(EditAnywhere)
	class USoundBase* fireSound;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShakeBase> camShakeFactory;

	UPROPERTY(EditAnywhere)
	class UCameraShakeBase* canShakeInstance;

	//탄창 리로드
	void OnActionReload();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 maxGunAmmo = 20;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 gunAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 maxSniperAmmo = 5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 sniperAmmo;

	void ReloadGun();
	void ReloadSniper();
};