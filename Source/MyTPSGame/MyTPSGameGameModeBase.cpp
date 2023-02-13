// Copyright Epic Games, Inc. All Rights Reserved.


#include "MyTPSGameGameModeBase.h"
#include "MyTPSGame.h"
#include "SpawnManager.h"
#include "TPSPlayer.h"
#include "Kismet/GameplayStatics.h"

AMyTPSGameGameModeBase::AMyTPSGameGameModeBase()
{
	
}

void AMyTPSGameGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	spawnManager = Cast<ASpawnManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ASpawnManager::StaticClass()));

	LevelUp();
}

void AMyTPSGameGameModeBase::LevelUp()
{
	level++;

	//스폰매니져의 makeTargetCount를 현재 레벨로 설정하고 싶다
	spawnManager->makeTargetCount = level;

	spawnManager->makeCount = 0;

	//if(level < 20)
	//{
		
	//}
	//needExp도 설정하고 싶다
	needExp = level;
}

void AMyTPSGameGameModeBase::AddExp(int amount)
{
	exp += amount;

	//경험치가 레벨업에 도달했는가?
	if(exp >= needExp)
	{
		//레벨업 시작
		LevelUpProcess();
	}
}

void AMyTPSGameGameModeBase::LevelUpProcess()
{
	//이펙트 표시
	exp -= needExp;
	LevelUp();

	if(exp >= needExp)
	{
		FTimerHandle th;
		GetWorld()->GetTimerManager().SetTimer(th, this, &AMyTPSGameGameModeBase::LevelUpProcess, 0.2f, false);
	}
}
