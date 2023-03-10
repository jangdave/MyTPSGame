// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../MyTPSGame.h"

// Sets default values
ABulletActor::ABulletActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	sphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("sphereComp"));
	SetRootComponent(sphereComp);
	sphereComp->SetSphereRadius(13);
	sphereComp->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

	meshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("meshComp"));
	meshComp->SetupAttachment(sphereComp);
	meshComp->SetRelativeScale3D(FVector(0.25f));
	meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	movementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("movementComp"));
	movementComp->SetUpdatedComponent(sphereComp);
	movementComp->InitialSpeed = 5000.0f;
	movementComp->MaxSpeed = 5000.0f;
	movementComp->bShouldBounce = true;
}

// Called when the game starts or when spawned
void ABulletActor::BeginPlay()
{
	Super::BeginPlay();

	//SetLifeSpan(2);

	//Lambda ?Լ?
	//ĸ??
	//int number = 1;
	//auto myPrint = [number]()->void { int b = number; };
	//auto myPlus = [this](int a, int b)->int {return a + b; };
	//PRINT_LOG(TEXT("%d"), myPlus(20, 10));

	FTimerHandle dieTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(dieTimerHandle, this, &ABulletActor::OnDie, 2.0f, false);
	
	//Lambda??
	//GetWorldTimerManager().SetTimer(dieTimerHandle, FTimerDelegate::CreateLambda([this]()->void {Destroy(); }), 2, false);

}

// Called every frame
void ABulletActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABulletActor::OnDie()
{
	Destroy();
}

