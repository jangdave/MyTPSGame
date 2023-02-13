// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayerFireComponent.h"
#include "BulletActor.h"
#include "Enemy.h"
#include "EnemyFSM.h"
#include "TPSPlayer.h"
#include "TPSPlayerAnim.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UTPSPlayerFireComponent::UTPSPlayerFireComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ConstructorHelpers::FObjectFinder<USoundBase> tempFireSound(TEXT("/Script/Engine.SoundWave'/Game/SniperGun/Rifle.Rifle'"));
	if (tempFireSound.Succeeded())
	{
		fireSound = tempFireSound.Object;
	}
}


// Called when the game starts
void UTPSPlayerFireComponent::BeginPlay()
{
	Super::BeginPlay();
	
	crosshairUI = CreateWidget(GetWorld(), crosshairFactory);
	sniperUI = CreateWidget(GetWorld(), sniperFactory);

	//1. �¾�� cui�� ���̰� �ϰ� �ʹ�
	crosshairUI->AddToViewport();

	ChooseGun(GRENADE_GUN);

	gunAmmo = maxGunAmmo;

	sniperAmmo = maxSniperAmmo;
}


// Called every frame
void UTPSPlayerFireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTPSPlayerFireComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{

	PlayerInputComponent->BindAction(TEXT("FireBullet"), IE_Pressed, this, &UTPSPlayerFireComponent::OnActionFirePressed);
	PlayerInputComponent->BindAction(TEXT("FireBullet"), IE_Released, this, &UTPSPlayerFireComponent::OnActionFireRelesed);
	PlayerInputComponent->BindAction(TEXT("GrenadeGun"), IE_Pressed, this, &UTPSPlayerFireComponent::OnActionGrenade);
	PlayerInputComponent->BindAction(TEXT("SniperGun"), IE_Pressed, this, &UTPSPlayerFireComponent::OnActionSniper);
	PlayerInputComponent->BindAction(TEXT("SniperZoom"), IE_Pressed, this, &UTPSPlayerFireComponent::OnActionZoomIn);
	PlayerInputComponent->BindAction(TEXT("SniperZoom"), IE_Released, this, &UTPSPlayerFireComponent::OnActionZoomOut);
	PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &UTPSPlayerFireComponent::OnActionReload);
}

void UTPSPlayerFireComponent::OnActionFirePressed()
{
	//���� �� �Ѿ��� �����ִ��� ����
	//���� �����ִٸ� 1�� ����
	//�׷��� �ʴ��� ���� ���� ����
	if (bChooseGrenadeGun)
	{
		if (gunAmmo > 0)
		{
			gunAmmo--;
			me->OnMyGrenadeGunAmmoUpdate(gunAmmo, maxGunAmmo);
		}
		else
		{
			return;
		}
	}
	else
	{
		if (sniperAmmo > 0)
		{
			sniperAmmo--;
			me->OnMySniperGunAmmoUpdate(sniperAmmo, maxSniperAmmo);
		}
		else
		{
			return;
		}
	}

	//ī�޶� ���� �ʹ�
	auto cameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	//���� �̹� ���� �־��ٸ�
	//canShakeInstance�� ���� �ƴϰ� ���� ���̶��
	if (canShakeInstance != nullptr && canShakeInstance->IsFinished() == false)
	{
		cameraManager->StopCameraShake(canShakeInstance);
	}
	//����ϰ� �ٽ� ����
	canShakeInstance = cameraManager->StartCameraShake(camShakeFactory);

	//�ѽ�� �ִϸ��̼��� ���
	auto anim = Cast<UTPSPlayerAnim>(me->GetMesh()->GetAnimInstance());

	anim->OnFire(TEXT("Default"));

	//�ѼҸ��� ����
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), fireSound, me->GetActorLocation(), me->GetActorRotation());

	//���� �⺻���̶��
	if (bChooseGrenadeGun)
	{
		me->GetWorldTimerManager().SetTimer(fireTimerHandle, this, &UTPSPlayerFireComponent::DoFire, fireInterval, true);

		DoFire();
	}
	//�׷��� �ʴٸ�
	else
	{
		FHitResult hitInfo;
		FVector start = me->cameraComp->GetComponentLocation();
		FVector end = start + me->cameraComp->GetForwardVector() * 100000.0f;
		FCollisionQueryParams params;
		params.AddIgnoredActor(me);
		//�ٶ󺸰�ʹ�
		bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, start, end, ECollisionChannel::ECC_Visibility, params);
		//���࿡ �ε������� �ִٸ�
		if (bHit)
		{
			FTransform trans(hitInfo.ImpactPoint);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletImpactFactory, trans);

			//���� �ε��� ���Ͱ� enemy���
			//auto hitActor = hitInfo.GetActor();
			auto enemy = Cast<AEnemy>(hitInfo.GetActor());
			if (enemy != nullptr)
			{
				//enemy���� �������� �ش�
				UEnemyFSM* fsm = Cast<UEnemyFSM>(enemy->GetDefaultSubobjectByName(TEXT("enemyFSM")));

				fsm->OnDamageProcess(1);
			}

			//��ȣ�ۿ��ϰ� �ʹ�
			auto hitComp = hitInfo.GetComponent();
			//�ε��� ��ü�� �����ۿ��� �ϰ��ִٸ�
			if (hitComp != nullptr && hitComp->IsSimulatingPhysics())
			{
				//���� ���ϰ� �ʹ�
				FVector forceDir = (hitInfo.TraceEnd - hitInfo.TraceStart).GetSafeNormal();
				//foreceDir.Normalize();
				FVector force = forceDir * 100000.0f * hitComp->GetMass();
				hitComp->AddForce(force);
			}
		}
		//���θ��� �ڽ��� ���Ǿ� ������ ����
		//UKismetSystemLibrary::Boxtrace

		//������Ʈ�� �ϸ� �̷��� ä�η� �߰��Ҽ��ִ�
		//FCollisionObjectQueryParams objParams;
		//objParams.AddObjectTypesToQuery(ECollisionChannel::ECC_EngineTraceChannel1);
		//GetWorld()->LineTraceSingleByObjectType();
	}

}

void UTPSPlayerFireComponent::OnActionFireRelesed()
{
	me->GetWorldTimerManager().ClearTimer(fireTimerHandle);
}

void UTPSPlayerFireComponent::DoFire()
{
	FTransform t = me->gunMeshComp->GetSocketTransform(TEXT("FirePosition"));

	//t.SetRotation(FQuat(GetControlRotation()));

	GetWorld()->SpawnActor<ABulletActor>(bulletFactory, t);
}

void UTPSPlayerFireComponent::ChooseGun(bool bGrenade)
{
	//���� �ٲٱ� ���� �������۰��̰� �ٲٷ��°� ��ź�̸�
	if (bChooseGrenadeGun == false && bGrenade == true)
	{
		//fov�� 90, cui o, sui x
		me->cameraComp->SetFieldOfView(90);
		crosshairUI->AddToViewport();
		sniperUI->RemoveFromParent();
	}

	bChooseGrenadeGun = bGrenade;

	me->gunMeshComp->SetVisibility(bGrenade);

	me->sniperMeshComp->SetVisibility(!bGrenade);

	me->OnMyChooseGun(bChooseGrenadeGun);

	//3�� ������
	//bool result = bGrenade ? false : true;
}

void UTPSPlayerFireComponent::OnActionGrenade()
{
	ChooseGun(GRENADE_GUN);
	//ChooseGun(true);
}

void UTPSPlayerFireComponent::OnActionSniper()
{
	ChooseGun(SNIPER_GUN);
	//ChooseGun(false);
}

void UTPSPlayerFireComponent::OnActionZoomIn()
{
	//fov Ȯ��
	if (bChooseGrenadeGun != false)
	{
		return;
	}

	me->cameraComp->SetFieldOfView(20);
	crosshairUI->RemoveFromParent();
	sniperUI->AddToViewport();
}

void UTPSPlayerFireComponent::OnActionZoomOut()
{
	//fov ���
	if (bChooseGrenadeGun != false)
	{
		return;
	}

	me->cameraComp->SetFieldOfView(90);
	crosshairUI->AddToViewport();
	sniperUI->RemoveFromParent();
}

void UTPSPlayerFireComponent::OnActionReload()
{
	auto anim = Cast<UTPSPlayerAnim>(me->GetMesh()->GetAnimInstance());

	if (bChooseGrenadeGun)
	{
		anim->OnFire(TEXT("GunReload"));
	}
	else
	{
		anim->OnFire(TEXT("SniperReload"));
	}
}

void UTPSPlayerFireComponent::ReloadGun()
{
	gunAmmo = maxGunAmmo;
	me->OnMyGrenadeGunAmmoUpdate(gunAmmo, maxGunAmmo);
}

void UTPSPlayerFireComponent::ReloadSniper()
{
	sniperAmmo = maxSniperAmmo;
	me->OnMySniperGunAmmoUpdate(sniperAmmo, maxSniperAmmo);
}