// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include <Kismet/GameplayStatics.h>
#include "TPSPlayer.h"
#include "Enemy.h"
#include "../MyTPSGame.h"
#include <Components/CapsuleComponent.h>
#include "EnemyAnim.h"
#include "AIController.h"

// Sets default values for this component's properties
UEnemyFSM::UEnemyFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	state = EEnemyState::IDLE; //암시적 int a = 10; float b = a; 명시적 float b = float(a);
	
	me = Cast<AEnemy>(GetOwner());

	ai = Cast<AAIController>(me->GetController());

	hp = maxHP;
}

// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//본체의 animinstance의 state에 내 state를 넣어준다
	me->enemyAnim->state = this->state;

	switch (state)
	{
	case EEnemyState::IDLE:
		TIckIdle();
		break;
	case EEnemyState::MOVE:
		TickMove();
		break;
	case EEnemyState::ATTACK:
		TickAttack();
		break;
	case EEnemyState::DAMAGE:
		TickDamage();
		break;
	case EEnemyState::DIE:
		TickDie();
		break;
	}

	//try {
		//코딩하다가 에러가 나면..
	//}
	//catch (...)
	//{

	//}
}

/// <summary>
/// 대기, 플레이어 찾기->이동으로 전이
/// </summary>
void UEnemyFSM::TIckIdle()
{
	//1. 플레이어 찾기
	target = Cast<ATPSPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	//2. 만약 플레이어를 찾았다면
	if (target != nullptr)
	{
		//3. 이동으로 전이
		SetState(EEnemyState::MOVE);
	}
}

//목적지를 향해서 이동
//목적지와의 거리가 공격가능거리라면
//공격으로 전이
void UEnemyFSM::TickMove()
{
	//1. 목적지로 향하는 방향 설정
	FVector dir = target->GetActorLocation() - me->GetActorLocation();
	//2. 그 방향으로 이동
	ai->MoveToLocation(target->GetActorLocation());
	//me->AddMovementInput(dir.GetSafeNormal());
	//3. 공격가능거리보다 짧다면
	//float dist = target->GetDistanceTo(me); //GetDistanceTo 상대와 나의 거리
	float dist = dir.Size();
	//float dist = FVector::Dist(target->GetActorLocation(), me->GetActorLocation()); //백터 거리 구하기 공식 활용
	if (attackRange >= dist)
	{
		//4. 공격으로 전이
		SetState(EEnemyState::ATTACK);
	}
}

//공격 타이밍
void UEnemyFSM::TickAttack()
{
	//static float currentTime = 0;
	//1. 시간이 흐르다가
	currentTime += GetWorld()->GetDeltaSeconds();

	//4. 공격동작이 끝났다면
	if (currentTime > attackDelayTime)
	{
		float dist = target->GetDistanceTo(me);

		//5. 계속 공격을 할것인지 판단하고 싶다
		if (dist > attackRange)
		{
			SetState(EEnemyState::MOVE);
		}
		else
		{
			currentTime = 0;
			bAttackPlay = false;
			me->enemyAnim->bAttackPlay = true;
		}
	}
}

//player->enemy 공격
void UEnemyFSM::TickDamage()
{
	currentTime += GetWorld()->GetDeltaSeconds();
	if (currentTime > 1)
	{
		SetState(EEnemyState::MOVE);
		currentTime = 0;
	}
}

void UEnemyFSM::TickDie()
{
	//만약 넘어지는 애니메이션이 끝나지 않았다면
	if(me->enemyAnim->bEnemyDieEnd == false)
	{
		return;
	}

	currentTime += GetWorld()->GetDeltaSeconds();
	
	FVector p0 = me->GetActorLocation();
	FVector vt = (me->GetActorUpVector() * -1) * 200 * GetWorld()->GetDeltaSeconds();
	me->SetActorLocation(p0 + vt);
	if (currentTime > 1)
	{
		me->Destroy();
	}
}

//player에게 공격당함
void UEnemyFSM::OnDamageProcess(int32 damageAmount)
{
	if(ai != nullptr)
	{
		ai->StopMovement();
	}
	//체력을 소모하고
	hp -= damageAmount;
	//체력이 0이 되면
	if (hp <= 0)
	{
		//Die
		SetState(EEnemyState::DIE);
		me->enemyAnim->bEnemyDieEnd = false;
		//몽타주의 die를 플레이
		me->OnMyDamage(TEXT("Die"));

		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	//그렇지 않다면
	else
	{
		//Damage한다
		SetState(EEnemyState::DAMAGE);

		int32 index = FMath::RandRange(0, 1);
		FString sectionName = FString::Printf(TEXT("Damage%d"), index);

		me->OnMyDamage(FName(*sectionName));

		//if(FMath::RandRange(0,100)>50)
		//{
		//	enemy->OnMyDamage(TEXT("Damage0"));
		//}
		//else
		//{
		//	enemy->OnMyDamage(TEXT("Damage1"));
		//}
	}
}

void UEnemyFSM::OnHitEvent()
{
	float dist = target->GetDistanceTo(me);

	me->enemyAnim->bAttackPlay = false;

	//3. 공격을 한다(조건은 공격거리 안에 있는가)
	if (dist <= attackRange)
	{
		PRINT_LOG(TEXT("Enemyisattack"));
	}
}

void UEnemyFSM::SetState(EEnemyState next)
{
	state = next;
	me->enemyAnim->state = next;
	currentTime = 0;
}
