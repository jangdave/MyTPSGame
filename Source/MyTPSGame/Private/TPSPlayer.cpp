// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "BulletActor.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy.h"
#include "TPSPlayerAnim.h"
#include "TPSPlayerFireComponent.h"
#include "TPSPlayerMoveComponent.h"

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

		gunMeshComp->SetRelativeLocationAndRotation(FVector(-9.0f, 3.0f, 5.0f), FRotator(0, 108.0f,-4.52f));
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

	//이동 컴포넌트 생성
	moveComp = CreateDefaultSubobject<UTPSPlayerMoveComponent>(TEXT("moveComp"));

	//발사 컴포넌트 생성
	fireComp = CreateDefaultSubobject<UTPSPlayerFireComponent>(TEXT("fireComp"));
}
// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	HP = MaxHP;
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	setupInputDelegate.Broadcast(PlayerInputComponent);

	//moveComp->SetupPlayerInput(PlayerInputComponent);

	//fireComp->SetupPlayerInput(PlayerInputComponent);
}

void ATPSPlayer::OnHit(int damage)
{
	HP -= damage;

	if(HP <= 0)
	{
		OnMyGameOver();
	}
}
