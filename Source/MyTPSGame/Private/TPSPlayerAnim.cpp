// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayerAnim.h"
#include "TPSPlayer.h"
#include "TPSPlayerFireComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTPSPlayerAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//owner의 velocity를 가져와서
	ATPSPlayer* owner = Cast<ATPSPlayer>(TryGetPawnOwner());
	//rightVelocity, forwordVelocity 를 구한다
	//vector 내적으로 방향값을 구한다
	if(owner == nullptr)
	{
		return;
	}
	FVector velocity = owner->GetVelocity();
	FVector forvelocity = owner->GetActorForwardVector();
	FVector rigvelocity = owner->GetActorRightVector();
	forwordVelocity = FVector::DotProduct(velocity, forvelocity);
	rightVelocity = FVector::DotProduct(velocity, rigvelocity);
	//isInAir를 구한다
	isInAir = owner->GetCharacterMovement()->IsFalling();
	isCrouching = owner->GetCharacterMovement()->IsCrouching();
}

void UTPSPlayerAnim::OnFire(FName sectionName)
{
	ATPSPlayer* owner = Cast<ATPSPlayer>(TryGetPawnOwner());
	owner->PlayAnimMontage(fireMontageFactory, 1, sectionName);
	//Montage_Play(fireMontageFactory);
}

void UTPSPlayerAnim::OnGunReload()
{
	ATPSPlayer* owner = Cast<ATPSPlayer>(TryGetPawnOwner());

	if(owner != nullptr)
	{
		owner->fireComp->ReloadGun();
	}
}

void UTPSPlayerAnim::OnSniperReload()
{
	ATPSPlayer* owner = Cast<ATPSPlayer>(TryGetPawnOwner());

	if(owner != nullptr)
	{
		owner->fireComp->ReloadSniper();
	}
}
