// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnManager.h"
#include "Enemy.h"
#include "Spawn.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASpawnManager::ASpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASpawnManager::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawn::StaticClass(), spawnList);

	//랜덤한 시간 후에 적 생성함수 호출
	int32 randTime = FMath::RandRange(minTime, maxTime);
	GetWorld()->GetTimerManager().SetTimer(spawnEnemyTimer, this, &ASpawnManager::MakeEnemy, randTime, false);
	//FTimerManager::SetTimer(spawnEnemyTimer, this, &ASpawnManager::MakeEnemy, 2.0f, false);
}

// Called every frame
void ASpawnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpawnManager::MakeEnemy()
{
	int rIndex = 0;

	rIndex = FMath::RandRange(0, spawnList.Num() - 1);
	//적을 생성
	if(rIndex == preIndex)
	{
		rIndex = (rIndex + 1) % spawnList.Num();
	}
	preIndex = rIndex;

	FVector loc = spawnList[rIndex]->GetActorLocation();
	FRotator rot = spawnList[rIndex]->GetActorRotation();
	GetWorld()->SpawnActor<AEnemy>(enemyFactory, loc, rot);

	int32 randTime = FMath::RandRange(minTime, maxTime);
	GetWorld()->GetTimerManager().SetTimer(spawnEnemyTimer, this, &ASpawnManager::MakeEnemy, randTime, false);
}

