// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "BulletActor.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy.h"
#include "EnemyFSM.h"
#include "TPSPlayerAnim.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//sudo code �ǻ��ڵ� -> �˰���
	//1. �ܰ��� �ش��ϴ� ������ �о�´� FClassFinder -> �������Ʈ ���� ������
	ConstructorHelpers::FObjectFinder<USkeletalMesh> tempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn'"));
	//2. �о���°� �����ߴٸ�
	if (tempMesh.Succeeded())
	{
		//3. mesh�� �����ϰ� �ʹ�
		GetMesh()->SetSkeletalMesh(tempMesh.Object);
		//4. transform�� �����ϰ� �ʹ�
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90.0f), FRotator(0, -90.0f, 0));
	}
	//��������, ī�޶� ������Ʈ ����
	//�������� root, ī�޶� �������Ͽ� ���
	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("springArmComp"));
	springArmComp->SetupAttachment(RootComponent);
	cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("cameraComp"));
	cameraComp->SetupAttachment(springArmComp);

	springArmComp->SetRelativeLocation(FVector(0, 40.0f, 100.0f));
	springArmComp->TargetArmLength = 250.0f;

	//�Է°��� ȸ���� �ݿ��ϰ� �ʹ�
	bUseControllerRotationYaw = true;
	springArmComp->bUsePawnControlRotation = true;
	cameraComp->bUsePawnControlRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	gunMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("gunMeshComp"));
	gunMeshComp->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	ConstructorHelpers::FObjectFinder<USkeletalMesh> tempGunMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/FPWeapon/Mesh/SK_FPGun.SK_FPGun'"));
	if (tempGunMesh.Succeeded())
	{
		gunMeshComp->SetSkeletalMesh(tempGunMesh.Object);

		gunMeshComp->SetRelativeLocationAndRotation(FVector(-7.94f, 3.54f, -0.80f), FRotator(1.72f, 109.85f,-9.85f));
	}

	sniperMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("sniperMeshComp"));
	sniperMeshComp->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));
	ConstructorHelpers::FObjectFinder<UStaticMesh> tempSniperMesh(TEXT("/Script/Engine.StaticMesh'/Game/SniperGun/sniper1.sniper1'"));
	if (tempSniperMesh.Succeeded())
	{
		sniperMeshComp->SetStaticMesh(tempSniperMesh.Object);

		sniperMeshComp->SetRelativeLocationAndRotation(FVector(-34.58f, -6.86f, 4.85f), FRotator(0.17f, 105.63f, -5.52f));
	
		sniperMeshComp->SetRelativeScale3D(FVector(0.15f));
	}

	ConstructorHelpers::FObjectFinder<USoundBase> tempFireSound(TEXT("/Script/Engine.SoundWave'/Game/SniperGun/Rifle.Rifle'"));
	if(tempFireSound.Succeeded())
	{
		fireSound = tempFireSound.Object;
	}
}
// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	//UI�� �����ϰ� �ʹ�
	crosshairUI = CreateWidget(GetWorld(), crosshairFactory);
	sniperUI = CreateWidget(GetWorld(), sniperFactory);
	
	//1. �¾�� cui�� ���̰� �ϰ� �ʹ�
	crosshairUI->AddToViewport();

	ChooseGun(GRENADE_GUN);

	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//direction �������� �̵�
	FTransform trans(GetControlRotation());
	FVector resultDirection = trans.TransformVector(direction);
	resultDirection.Z = 0;
	resultDirection.Normalize();

	AddMovementInput(resultDirection);
	//���� �ʱ�ȭ
	direction = FVector::ZeroVector;
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Horizontal"), this, &ATPSPlayer::OnAxisHorizontal);
	PlayerInputComponent->BindAxis(TEXT("Vertical"), this, &ATPSPlayer::OnAxisVertical);
	PlayerInputComponent->BindAxis(TEXT("Turn Right"), this, &ATPSPlayer::OnAxisTurnRight);
	PlayerInputComponent->BindAxis(TEXT("Look Up"), this, &ATPSPlayer::OnAxisLookUp);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ATPSPlayer::OnActionJump);
	PlayerInputComponent->BindAction(TEXT("FireBullet"), IE_Pressed, this, &ATPSPlayer::OnActionFirePressed);
	PlayerInputComponent->BindAction(TEXT("FireBullet"), IE_Released, this, &ATPSPlayer::OnActionFireRelesed);
	PlayerInputComponent->BindAction(TEXT("GrenadeGun"), IE_Pressed, this, &ATPSPlayer::OnActionGrenade);
	PlayerInputComponent->BindAction(TEXT("SniperGun"), IE_Pressed, this, &ATPSPlayer::OnActionSniper);
	PlayerInputComponent->BindAction(TEXT("SniperZoom"), IE_Pressed, this, &ATPSPlayer::OnActionZoomIn);
	PlayerInputComponent->BindAction(TEXT("SniperZoom"), IE_Released, this, &ATPSPlayer::OnActionZoomOut);
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Pressed, this, &ATPSPlayer::OnActionRunPressed);
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Released, this, &ATPSPlayer::OnActionRunReleased);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &ATPSPlayer::OnActionCouchPressed);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &ATPSPlayer::OnActionCouchReleased);
}

void ATPSPlayer::OnAxisHorizontal(float value)
{
	direction.Y = value;
}

void ATPSPlayer::OnAxisVertical(float value)
{
	direction.X = value;
}

void ATPSPlayer::OnAxisLookUp(float value)
{
	//pitch
	AddControllerPitchInput(value);
}

void ATPSPlayer::OnAxisTurnRight(float value)
{
	//yaw
	AddControllerYawInput(value);
}

void ATPSPlayer::OnActionJump()
{
	Jump();
}

void ATPSPlayer::OnActionRunPressed()
{
	GetCharacterMovement()->MaxWalkSpeed = runSpeed;
}

void ATPSPlayer::OnActionRunReleased()
{
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

void ATPSPlayer::OnActionCouchPressed()
{
	GetCharacterMovement()->MaxWalkSpeed = crouchSpeed;
}

void ATPSPlayer::OnActionCouchReleased()
{
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

void ATPSPlayer::OnActionFirePressed()
{
	//ī�޶� ���� �ʹ�
	auto cameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	//���� �̹� ���� �־��ٸ�
	//canShakeInstance�� ���� �ƴϰ� ���� ���̶��
	if(canShakeInstance != nullptr && canShakeInstance->IsFinished() == false)
	{
		cameraManager->StopCameraShake(canShakeInstance);
	}
	//����ϰ� �ٽ� ����
	canShakeInstance = cameraManager->StartCameraShake(camShakeFactory);

	//�ѽ�� �ִϸ��̼��� ���
	auto anim = Cast<UTPSPlayerAnim>(GetMesh()->GetAnimInstance());

	anim->OnFire();

	//�ѼҸ��� ����
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), fireSound, GetActorLocation(), GetActorRotation());

	//���� �⺻���̶��
	if (bChooseGrenadeGun)
	{
		GetWorldTimerManager().SetTimer(fireTimerHandle, this, &ATPSPlayer::DoFire, fireInterval, true);

		DoFire();
	}
	//�׷��� �ʴٸ�
	else
	{
		FHitResult hitInfo;
		FVector start = cameraComp->GetComponentLocation();
		FVector end = start + cameraComp->GetForwardVector()*100000.0f;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
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

void ATPSPlayer::OnActionFireRelesed()
{
	GetWorldTimerManager().ClearTimer(fireTimerHandle);
}

void ATPSPlayer::DoFire()
{
	FTransform t = gunMeshComp->GetSocketTransform(TEXT("FirePosition"));

	//t.SetRotation(FQuat(GetControlRotation()));

	GetWorld()->SpawnActor<ABulletActor>(bulletFactory, t);
}

void ATPSPlayer::ChooseGun(bool bGrenade)
{
	//���� �ٲٱ� ���� �������۰��̰� �ٲٷ��°� ��ź�̸�
	if (bChooseGrenadeGun == false && bGrenade == true)
	{
		//fov�� 90, cui o, sui x
		cameraComp->SetFieldOfView(90);
		crosshairUI->AddToViewport();
		sniperUI->RemoveFromParent();
	}
	
	bChooseGrenadeGun = bGrenade;

	gunMeshComp->SetVisibility(bGrenade);

	sniperMeshComp->SetVisibility(!bGrenade);

	//3�� ������
	//bool result = bGrenade ? false : true;
}

void ATPSPlayer::OnActionGrenade()
{
	ChooseGun(GRENADE_GUN);
	//ChooseGun(true);
}

void ATPSPlayer::OnActionSniper()
{
	ChooseGun(SNIPER_GUN);
	//ChooseGun(false);
}

void ATPSPlayer::OnActionZoomIn()
{
	//fov Ȯ��
	if (bChooseGrenadeGun != false)
	{
		return;
	}
	
	cameraComp->SetFieldOfView(20);
	crosshairUI->RemoveFromParent();
	sniperUI->AddToViewport();
}

void ATPSPlayer::OnActionZoomOut()
{
	//fov ���
	if (bChooseGrenadeGun != false)
	{
		return;
	}

	cameraComp->SetFieldOfView(90);
	crosshairUI->AddToViewport();
	sniperUI->RemoveFromParent();
}
