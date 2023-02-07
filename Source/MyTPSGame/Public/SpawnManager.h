// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnManager.generated.h"

UCLASS()
class MYTPSGAME_API ASpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//랜덤시간마다 적을 랜덤 위치에 생성하고 싶다
	//- 랜덤 위치를 위한 위치 목록
	UPROPERTY(EditAnywhere)
	TArray<AActor*> spawnList;
	//- 최소 최대시간
	UPROPERTY(EditAnywhere)
	float minTime = 1.0f;

	UPROPERTY(EditAnywhere)
	float maxTime = 2.0f;

	//생성할 시간
	float createTime;

	//- 타이머핸들
	FTimerHandle spawnEnemyTimer;

	//- 적공장
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AEnemy> enemyFactory;

	void MakeEnemy();

	float preIndex = -1;
};
