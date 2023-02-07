// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyFSM.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8 //0~255
{
	IDLE,
	MOVE,
	ATTACK,
	DAMAGE,
	DIE,
};

UENUM(BlueprintType)
enum class EEnemyMoveSubState : uint8
{
	PATROL,
	CHASE,
	OLD_MOVE,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYTPSGAME_API UEnemyFSM : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyFSM();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	EEnemyState state;

	EEnemyMoveSubState moveSubState;

	UPROPERTY()
	class ATPSPlayer* target; //caching

	UPROPERTY()
	class AEnemy* me;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float attackRange = 200.0f;

	float currentTime;

	bool bAttackPlay;

	UPROPERTY(EditAnywhere)
	float attackDelayTime = 3;

private:
	void TIckIdle();

	void TickPatrol();

	void TickChase();

	void TickMove();

	void TickAttack();

	void TickDamage();

	void TickDie();
	
public:
	void SetState(EEnemyState next);

	//이벤트 함수, callback 함수
	void OnDamageProcess(int32); //자료형만 써도 된다
	
	void OnHitEvent();

	void TickMoveOldMove();

	class AAIController* ai;

	UPROPERTY(EditAnywhere)
	float randomLocationRadius = 500.0f;

	FVector randomLocation;

	bool UpdateRandomLocation(float radius, FVector& outLocation);

	//pathmanager 알고싶다
	UPROPERTY()
	class APathManager* pathManager;

	//pathmanager의 waypoint를 이용해서 목적지 정하기
	int wayIndex;

	//patrol에서 감지 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float detectDistance = 500.0f;

	//chase에서 추적포기 거리 : 감지거리보다 길어야 한다
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float abandonDistance = 800.0f;
};