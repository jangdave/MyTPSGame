// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include <Kismet/GameplayStatics.h>
#include "TPSPlayer.h"
#include "Enemy.h"
#include "../MyTPSGame.h"
#include <Components/CapsuleComponent.h>
#include "EnemyAnim.h"
#include "AIController.h"
//#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include "PathManager.h"
#include "MyTPSGame/MyTPSGameGameModeBase.h"

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

	moveSubState = EEnemyMoveSubState::PATROL;

	me = Cast<AEnemy>(GetOwner());

	ai = Cast<AAIController>(me->GetController());

	me->hp = me->maxHP;

	//�¾ �� ���� �������� ���س��� �ʹ�
	UpdateRandomLocation(randomLocationRadius, randomLocation);

	pathManager = Cast<APathManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APathManager::StaticClass()));
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

//������ ��ġ�� ������� �̵��ϰ� �ʹ�
void UEnemyFSM::TickPatrol()
{
	FVector patrolTarget = pathManager->wayPoints[wayIndex]->GetActorLocation();
	auto result = ai->MoveToLocation(patrolTarget);
	//���� ���� ��ġ�� �����ߴٸ�
	if(result == EPathFollowingRequestResult::AlreadyAtGoal || result == EPathFollowingRequestResult::Failed)
	{
		//������ ��ġ�� ���� ��ġ�� ����
		wayIndex++;
		//wayindex�� ���� �ִ�� �������� �ʱ�ȭ
		if(wayIndex >= pathManager->wayPoints.Num())
		{
			wayIndex = 0;
		}

		//wayIndex = (wayIndex + 1) % pathManager->wayPoints.Num();
		//wayIndex = (wayIndex + pathManager->wayPoints.Num() - 1) % pathManager->wayPoints.Num();
	}

	//Ÿ�ٰ��� �Ÿ��� �����ؼ�
	//float dis = FVector::Distance(FVector(target->GetActorLocation()), FVector(me->GetActorLocation()));
	//UKismetSystemLibrary::SphereOverlapActors()
	float dis = me->GetDistanceTo(target);
	//Ÿ���� �����Ÿ� �ȿ� ���Դٸ�
	if(detectDistance >= dis)
	{
		//chase���·� ����
		moveSubState = EEnemyMoveSubState::CHASE;
	}
}

void UEnemyFSM::TickChase()
{
	//�������� ���ؼ� �̵�
	ai->MoveToLocation(target->GetActorLocation());

	//1. �������� ���ϴ� ���� ����
	FVector dir = target->GetActorLocation() - me->GetActorLocation();

	//2. �� �������� �̵�
	//ai->MoveToLocation(target->GetActorLocation());
	//UAIBlueprintHelperLibrary::SimpleMoveToActor()

	//me->AddMovementInput(dir.GetSafeNormal());
	//3. ���ݰ��ɰŸ����� ª�ٸ�
	//float dist = target->GetDistanceTo(me); //GetDistanceTo ���� ���� �Ÿ�
	float dist = dir.Size();
	//float dist = FVector::Dist(target->GetActorLocation(), me->GetActorLocation()); //���� �Ÿ� ���ϱ� ���� Ȱ��
	if (attackRange >= dist)
	{
		//4. �������� ����
		SetState(EEnemyState::ATTACK);
	}
	else if(abandonDistance < dist)//�׷��� �ʰ� ����Ÿ����� �־�����
	{
		moveSubState = EEnemyMoveSubState::PATROL;
	}
}

//�������� ���ؼ� �̵�
//���������� �Ÿ��� ���ݰ��ɰŸ����
//�������� ����
void UEnemyFSM::TickMove()
{
	switch (moveSubState)
	{
		case EEnemyMoveSubState::PATROL:
			TickPatrol();
			break;
		case EEnemyMoveSubState::CHASE:
			TickChase();
			break;
		case EEnemyMoveSubState::OLD_MOVE:
			TickMoveOldMove();
			break;
		default: 
			break;
	}
}

//���� Ÿ�̹�
void UEnemyFSM::TickAttack()
{
	//static float currentTime = 0;
	//1. �ð��� �帣�ٰ�
	currentTime += GetWorld()->GetDeltaSeconds();

	//4. ���ݵ����� �����ٸ�
	if (currentTime > attackDelayTime)
	{
		float dist = target->GetDistanceTo(me);

		target->OnHit(1);
		//5. ��� ������ �Ұ����� �Ǵ��ϰ� �ʹ�
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
	//���� �Ѿ����� �ִϸ��̼��� ������ �ʾҴٸ�
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

//player���� ���ݴ���
void UEnemyFSM::OnDamageProcess(int32 damageAmount)
{
	if(ai != nullptr)
	{
		ai->StopMovement();
	}
	//ü���� �Ҹ��ϰ�
	me->hp -= damageAmount;
	//ü���� 0�� �Ǹ�
	if (me->hp <= 0)
	{
		//Die
		SetState(EEnemyState::DIE);
		me->enemyAnim->bEnemyDieEnd = false;
		//��Ÿ���� die�� �÷���
		me->OnMyDamage(TEXT("Die"));

		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//���Ӹ���� AddExp �Լ��� ȣ���ϰ� �ʹ� 1��
		Cast<AMyTPSGameGameModeBase>(GetWorld()->GetAuthGameMode())->AddExp(1);
	}
	//�׷��� �ʴٸ�
	else
	{
		//Damage�Ѵ�
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

	//3. ������ �Ѵ�(������ ���ݰŸ� �ȿ� �ִ°�)
	if (dist <= attackRange)
	{
		PRINT_LOG(TEXT("Enemyisattack"));
	}
}

void UEnemyFSM::TickMoveOldMove()
{
	//A.���� �� �� �ִ� �� ���� target�� �ִ°�
	UNavigationSystemV1* ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	
	FPathFindingQuery query;
	FAIMoveRequest request;
	request.SetGoalLocation(target->GetActorLocation());
	request.SetAcceptanceRadius(5);
	ai->BuildPathfindingQuery(request, query);

	auto result = ns->FindPathSync(query);

	if (result.Result == ENavigationQueryResult::Success || result.Result == ENavigationQueryResult::Fail)
	{
		//B.�� �� �ִٸ� target������ �̵�
		ai->MoveToLocation(target->GetActorLocation());
	}
	else
	{
		//C.�׷��� �ʴٸ� �������� ��ġ�� �ϳ� �����ؼ� �װ����� ����
		auto r = ai->MoveToLocation(randomLocation);
		if (r == EPathFollowingRequestResult::AlreadyAtGoal || EPathFollowingRequestResult::Failed)
		{
			//D.���� ��ġ�� �����ߴٸ� �ٽ� �������� ��ġ�� �缱��
			UpdateRandomLocation(randomLocationRadius, randomLocation);
		}
	}
}

bool UEnemyFSM::UpdateRandomLocation(float radius, FVector& outLocation)
{
	UNavigationSystemV1* ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	FNavLocation navLoc;
	bool result = ns->GetRandomReachablePointInRadius(me->GetActorLocation(), radius, navLoc);
	if (result)
	{
		outLocation = navLoc.Location;
	}

	return result;
}

void UEnemyFSM::SetState(EEnemyState next)
{
	state = next;
	me->enemyAnim->state = next;
	currentTime = 0;
}

/*int move = 1; 0001

int idle = 2; 0010

int attack = 4; 0100

int die = 8; 1000

int state = 0;

state = state | move;

state = state | idle;

if((state & move) != 0)
{
	TickMove();
}

if((state & idle) != 0)
{
	TickIdle();
}*/