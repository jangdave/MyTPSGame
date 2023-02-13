// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TPSPlayerBaseComponent.h"
#include "TPSPlayerFireComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTPSGAME_API UTPSPlayerFireComponent : public UTPSPlayerBaseComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTPSPlayerFireComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void SetupPlayerInput(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABulletActor> bulletFactory;

	//위젯 공장에서 위젯을 생성
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> crosshairFactory;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> sniperFactory;

	UPROPERTY()
	class UUserWidget* crosshairUI;

	UPROPERTY()
	class UUserWidget* sniperUI;

	UPROPERTY(EditDefaultsOnly)
	class UParticleSystem* bulletImpactFactory;

	UPROPERTY(EditAnywhere)
	class USoundBase* fireSound;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShakeBase> camShakeFactory;

	UPROPERTY(EditAnywhere)
	class UCameraShakeBase* canShakeInstance;

	void OnActionFirePressed();

	void OnActionFireRelesed();

	void DoFire();

	void OnActionGrenade();

	void OnActionSniper();

	void ChooseGun(bool bGrenade);

	//탄창 리로드
	void OnActionReload();

	void ReloadGun();

	void ReloadSniper();

	void OnActionZoomIn();

	void OnActionZoomOut();

	FTimerHandle fireTimerHandle;

	UPROPERTY(EditAnywhere)
	float fireInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bChooseGrenadeGun;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 maxGunAmmo = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 gunAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 maxSniperAmmo = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 sniperAmmo;

};
