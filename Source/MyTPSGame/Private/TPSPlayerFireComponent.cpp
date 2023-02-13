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

	//1. 태어날때 cui를 보이게 하고 싶다
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
	//총을 쏠때 총알이 남아있는지 검증
	//만약 남아있다면 1발 차감
	//그렇지 않느면 총을 쏘지 않음
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

	//카메라를 흔들고 싶다
	auto cameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	//만약 이미 흔들고 있었다면
	//canShakeInstance가 널이 아니고 흔드는 중이라면
	if (canShakeInstance != nullptr && canShakeInstance->IsFinished() == false)
	{
		cameraManager->StopCameraShake(canShakeInstance);
	}
	//취소하고 다시 흔든다
	canShakeInstance = cameraManager->StartCameraShake(camShakeFactory);

	//총쏘는 애니메이션을 재생
	auto anim = Cast<UTPSPlayerAnim>(me->GetMesh()->GetAnimInstance());

	anim->OnFire(TEXT("Default"));

	//총소리를 낸다
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), fireSound, me->GetActorLocation(), me->GetActorRotation());

	//만약 기본총이라면
	if (bChooseGrenadeGun)
	{
		me->GetWorldTimerManager().SetTimer(fireTimerHandle, this, &UTPSPlayerFireComponent::DoFire, fireInterval, true);

		DoFire();
	}
	//그렇지 않다면
	else
	{
		FHitResult hitInfo;
		FVector start = me->cameraComp->GetComponentLocation();
		FVector end = start + me->cameraComp->GetForwardVector() * 100000.0f;
		FCollisionQueryParams params;
		params.AddIgnoredActor(me);
		//바라보고싶다
		bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, start, end, ECollisionChannel::ECC_Visibility, params);
		//만약에 부딪힌것이 있다면
		if (bHit)
		{
			FTransform trans(hitInfo.ImpactPoint);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletImpactFactory, trans);

			//만약 부딪힌 액터가 enemy라면
			//auto hitActor = hitInfo.GetActor();
			auto enemy = Cast<AEnemy>(hitInfo.GetActor());
			if (enemy != nullptr)
			{
				//enemy에게 데미지를 준다
				UEnemyFSM* fsm = Cast<UEnemyFSM>(enemy->GetDefaultSubobjectByName(TEXT("enemyFSM")));

				fsm->OnDamageProcess(1);
			}

			//상호작용하고 싶다
			auto hitComp = hitInfo.GetComponent();
			//부딪힌 물체가 물리작용을 하고있다면
			if (hitComp != nullptr && hitComp->IsSimulatingPhysics())
			{
				//힘을 가하고 싶다
				FVector forceDir = (hitInfo.TraceEnd - hitInfo.TraceStart).GetSafeNormal();
				//foreceDir.Normalize();
				FVector force = forceDir * 100000.0f * hitComp->GetMass();
				hitComp->AddForce(force);
			}
		}
		//라인말고 박스랑 스피어 종류도 있음
		//UKismetSystemLibrary::Boxtrace

		//오브젝트로 하면 이렇게 채널로 추가할수있다
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
	//만약 바꾸기 전에 스나이퍼건이고 바꾸려는게 유탄이면
	if (bChooseGrenadeGun == false && bGrenade == true)
	{
		//fov를 90, cui o, sui x
		me->cameraComp->SetFieldOfView(90);
		crosshairUI->AddToViewport();
		sniperUI->RemoveFromParent();
	}

	bChooseGrenadeGun = bGrenade;

	me->gunMeshComp->SetVisibility(bGrenade);

	me->sniperMeshComp->SetVisibility(!bGrenade);

	me->OnMyChooseGun(bChooseGrenadeGun);

	//3항 연산자
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
	//fov 확대
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
	//fov 축소
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