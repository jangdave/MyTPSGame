// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnManager.h"
#include "Enemy.h"
#include "Spawn.h"
#include "Kismet/GameplayStatics.h"
#include "MyTPSGame/MyTPSGameGameModeBase.h"

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

	//랜덤 시간 후에 적 생성
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
	//만든 갯수가 목표 갯수보다 작다면 만들고 싶다
	if(makeCount < makeTargetCount)
	{
		makeCount++;
		
		int rIndex = FMath::RandRange(0, spawnList.Num() - 1);
		
		if(rIndex == preIndex)
		{
			rIndex = (rIndex + 1) % spawnList.Num();
		}
		preIndex = rIndex;

		FVector loc = spawnList[rIndex]->GetActorLocation();
		FRotator rot = spawnList[rIndex]->GetActorRotation();

		int rate = FMath::RandRange(0, 99);
		int levelTargetRate = 50;

		int level = Cast<AMyTPSGameGameModeBase>(GetWorld()->GetAuthGameMode())->level;
		if(level < 5)
		{
			levelTargetRate = 50;
		}
		else
		{
			levelTargetRate = -1;
		}
		if(rate < levelTargetRate)
		{
			GetWorld()->SpawnActor<AEnemy>(enemyFactory, loc, rot);
		}
		
	}
	
	int32 randTime = FMath::RandRange(minTime, maxTime);
	GetWorld()->GetTimerManager().SetTimer(spawnEnemyTimer, this, &ASpawnManager::MakeEnemy, randTime, false);
}

