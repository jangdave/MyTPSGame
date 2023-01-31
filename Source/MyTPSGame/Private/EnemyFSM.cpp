// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include <Kismet/GameplayStatics.h>
#include "TPSPlayer.h"
#include "Enemy.h"
#include "../MyTPSGame.h"
#include <Components/CapsuleComponent.h>
#include "EnemyAnim.h"

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

	state = EEnemyState::IDLE; //�Ͻ��� int a = 10; float b = a; ����� float b = float(a);
	
	me = Cast<AEnemy>(GetOwner());

	hp = maxHP;
}

// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//��ü�� animinstance�� state�� �� state�� �־��ش�
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
		//�ڵ��ϴٰ� ������ ����..
	//}
	//catch (...)
	//{

	//}
}

/// <summary>
/// ���, �÷��̾� ã��->�̵����� ����
/// </summary>
void UEnemyFSM::TIckIdle()
{
	//1. �÷��̾� ã��
	target = Cast<ATPSPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	//2. ���� �÷��̾ ã�Ҵٸ�
	if (target != nullptr)
	{
		//3. �̵����� ����
		SetState(EEnemyState::MOVE);
	}
}

//�������� ���ؼ� �̵�
//���������� �Ÿ��� ���ݰ��ɰŸ����
//�������� ����
void UEnemyFSM::TickMove()
{
	//1. �������� ���ϴ� ���� ����
	FVector dir = target->GetActorLocation() - me->GetActorLocation();
	//2. �� �������� �̵�
	me->AddMovementInput(dir.GetSafeNormal());
	//3. ���ݰ��ɰŸ����� ª�ٸ�
	//float dist = target->GetDistanceTo(me); //GetDistanceTo ���� ���� �Ÿ�
	float dist = dir.Size();
	//float dist = FVector::Dist(target->GetActorLocation(), me->GetActorLocation()); //���� �Ÿ� ���ϱ� ���� Ȱ��
	if (attackRange >= dist)
	{
		//4. �������� ����
		SetState(EEnemyState::ATTACK);
	}
}

//���� Ÿ�̹�
void UEnemyFSM::TickAttack()
{
	//static float currentTime = 0;
	//1. �ð��� �帣�ٰ�
	currentTime += GetWorld()->GetDeltaSeconds();

	float dist = target->GetDistanceTo(me);

	//2. ����ð��� ���ݽð��� �ʰ��ϸ�
	if (bAttackPlay == false && currentTime > 0.1f)
	{
		bAttackPlay = true;
		//3. ������ �Ѵ�(������ ���ݰŸ� �ȿ� �ִ°�)
		if (dist <= attackRange)
		{
			PRINT_LOG(TEXT("Enemyisattack"));
		}
	}
	//4. ���ݵ����� �����ٸ�
	if (currentTime > 2)
	{
		//5. ��� ������ �Ұ����� �Ǵ��ϰ� �ʹ�
		if (dist > attackRange)
		{
			SetState(EEnemyState::MOVE);
		}
		else
		{
			currentTime = 0;
			bAttackPlay = false;
		}
	}
}

//player->enemy ����
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
	currentTime += GetWorld()->GetDeltaSeconds();
	
	FVector p0 = me->GetActorLocation();
	FVector vt = (me->GetActorUpVector() * -1) * 200 * GetWorld()->GetDeltaSeconds();
	me->SetActorLocation(p0 + vt);
	if (currentTime > 1)
	{
		me->Destroy();
	}
}

//player���� ���ݴ���
void UEnemyFSM::OnDamageProcess(int32 damageAmount)
{
	//ü���� �Ҹ��ϰ�
	hp -= damageAmount;
	//ü���� 0�� �Ǹ�
	if (hp <= 0)
	{
		//Die
		SetState(EEnemyState::DIE);

		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	//�׷��� �ʴٸ�
	else
	{
		//Damage�Ѵ�
		SetState(EEnemyState::DAMAGE);
	}
}

void UEnemyFSM::SetState(EEnemyState next)
{
	state = next;
	me->enemyAnim->state = next;
}
