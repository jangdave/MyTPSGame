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

	class ATPSPlayer* target; //caching

	class AEnemy* me;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float attackRange = 200.0f;

	float currentTime;

	bool bAttackPlay;

private:
	void TIckIdle();

	void TickMove();

	void TickAttack();

	void TickDamage();

	void TickDie();
	
public:
	int32 hp;

	int32 maxHP = 2;

	//이벤트 함수, callback 함수
	void OnDamageProcess(int32); //자료형만 써도 된다

};
