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

	//sudo code 의사코드 -> 알고리즘
	//1. 외관에 해당하는 에셋을 읽어온다 FClassFinder -> 블루프린트 파일 읽을때
	ConstructorHelpers::FObjectFinder<USkeletalMesh> tempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn'"));
	//2. 읽어오는게 성공했다면
	if (tempMesh.Succeeded())
	{
		//3. mesh에 적용하고 싶다
		GetMesh()->SetSkeletalMesh(tempMesh.Object);
		//4. transform을 수정하고 싶다
		GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -90.0f), FRotator(0, -90.0f, 0));
	}
	//스프링암, 카메라 컴포넌트 생성
	//스프링암 root, 카메라 스프링암에 상속
	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("springArmComp"));
	springArmComp->SetupAttachment(RootComponent);
	cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("cameraComp"));
	cameraComp->SetupAttachment(springArmComp);

	springArmComp->SetRelativeLocation(FVector(0, 40.0f, 100.0f));
	springArmComp->TargetArmLength = 250.0f;

	//입력값을 회전에 반영하고 싶다
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

	//UI를 생성하고 싶다
	crosshairUI = CreateWidget(GetWorld(), crosshairFactory);
	sniperUI = CreateWidget(GetWorld(), sniperFactory);
	
	//1. 태어날때 cui를 보이게 하고 싶다
	crosshairUI->AddToViewport();

	ChooseGun(GRENADE_GUN);

	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//direction 방향으로 이동
	FTransform trans(GetControlRotation());
	FVector resultDirection = trans.TransformVector(direction);
	resultDirection.Z = 0;
	resultDirection.Normalize();

	AddMovementInput(resultDirection);
	//방향 초기화
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
	//카메라를 흔들고 싶다
	auto cameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);

	//만약 이미 흔들고 있었다면
	//canShakeInstance가 널이 아니고 흔드는 중이라면
	if(canShakeInstance != nullptr && canShakeInstance->IsFinished() == false)
	{
		cameraManager->StopCameraShake(canShakeInstance);
	}
	//취소하고 다시 흔든다
	canShakeInstance = cameraManager->StartCameraShake(camShakeFactory);

	//총쏘는 애니메이션을 재생
	auto anim = Cast<UTPSPlayerAnim>(GetMesh()->GetAnimInstance());

	anim->OnFire();

	//총소리를 낸다
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), fireSound, GetActorLocation(), GetActorRotation());

	//만약 기본총이라면
	if (bChooseGrenadeGun)
	{
		GetWorldTimerManager().SetTimer(fireTimerHandle, this, &ATPSPlayer::DoFire, fireInterval, true);

		DoFire();
	}
	//그렇지 않다면
	else
	{
		FHitResult hitInfo;
		FVector start = cameraComp->GetComponentLocation();
		FVector end = start + cameraComp->GetForwardVector()*100000.0f;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
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
	//만약 바꾸기 전에 스나이퍼건이고 바꾸려는게 유탄이면
	if (bChooseGrenadeGun == false && bGrenade == true)
	{
		//fov를 90, cui o, sui x
		cameraComp->SetFieldOfView(90);
		crosshairUI->AddToViewport();
		sniperUI->RemoveFromParent();
	}
	
	bChooseGrenadeGun = bGrenade;

	gunMeshComp->SetVisibility(bGrenade);

	sniperMeshComp->SetVisibility(!bGrenade);

	//3항 연산자
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
	//fov 확대
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
	//fov 축소
	if (bChooseGrenadeGun != false)
	{
		return;
	}

	cameraComp->SetFieldOfView(90);
	crosshairUI->AddToViewport();
	sniperUI->RemoveFromParent();
}
