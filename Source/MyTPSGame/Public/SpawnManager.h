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

	//�����ð����� ���� ���� ��ġ�� �����ϰ� �ʹ�
	//- ���� ��ġ�� ���� ��ġ ���
	UPROPERTY(EditAnywhere)
	TArray<AActor*> spawnList;
	//- �ּ� �ִ�ð�
	UPROPERTY(EditAnywhere)
	float minTime = 1.0f;

	UPROPERTY(EditAnywhere)
	float maxTime = 2.0f;

	//������ �ð�
	float createTime;

	//- Ÿ�̸��ڵ�
	FTimerHandle spawnEnemyTimer;

	//- ������
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AEnemy> enemyFactory;

	void MakeEnemy();

	float preIndex = -1;
};
