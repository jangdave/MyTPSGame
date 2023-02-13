// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayerMoveComponent.h"
#include "TPSPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UTPSPlayerMoveComponent::UTPSPlayerMoveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
}

// Called when the game starts
void UTPSPlayerMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	me->GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

// Called every frame
void UTPSPlayerMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//direction 방향으로 이동
	FTransform trans(me->GetControlRotation());
	FVector resultDirection = trans.TransformVector(direction);
	resultDirection.Z = 0;
	resultDirection.Normalize();

	me->AddMovementInput(resultDirection);
	//방향 초기화
	direction = FVector::ZeroVector;
}

void UTPSPlayerMoveComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAxis(TEXT("Horizontal"), this, &UTPSPlayerMoveComponent::OnAxisHorizontal);
	PlayerInputComponent->BindAxis(TEXT("Vertical"), this, &UTPSPlayerMoveComponent::OnAxisVertical);
	PlayerInputComponent->BindAxis(TEXT("Turn Right"), this, &UTPSPlayerMoveComponent::OnAxisTurnRight);
	PlayerInputComponent->BindAxis(TEXT("Look Up"), this, &UTPSPlayerMoveComponent::OnAxisLookUp);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &UTPSPlayerMoveComponent::OnActionJump);
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Pressed, this, &UTPSPlayerMoveComponent::OnActionRunPressed);
	PlayerInputComponent->BindAction(TEXT("Run"), IE_Released, this, &UTPSPlayerMoveComponent::OnActionRunReleased);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &UTPSPlayerMoveComponent::OnActionCouchPressed);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &UTPSPlayerMoveComponent::OnActionCouchReleased);
}

void UTPSPlayerMoveComponent::OnAxisHorizontal(float value)
{
	direction.Y = value;
}

void UTPSPlayerMoveComponent::OnAxisVertical(float value)
{
	direction.X = value;
}

void UTPSPlayerMoveComponent::OnAxisLookUp(float value)
{
	//pitch
	me->AddControllerPitchInput(value);
}

void UTPSPlayerMoveComponent::OnAxisTurnRight(float value)
{
	//yaw
	me->AddControllerYawInput(value);
}

void UTPSPlayerMoveComponent::OnActionJump()
{
	me->Jump();
}

void UTPSPlayerMoveComponent::OnActionRunPressed()
{
	me->GetCharacterMovement()->MaxWalkSpeed = runSpeed;
}

void UTPSPlayerMoveComponent::OnActionRunReleased()
{
	me->GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

void UTPSPlayerMoveComponent::OnActionCouchPressed()
{
	me->GetCharacterMovement()->MaxWalkSpeedCrouched = crouchSpeed;
	me->Crouch();
}

void UTPSPlayerMoveComponent::OnActionCouchReleased()
{
	me->GetCharacterMovement()->MaxWalkSpeedCrouched = walkSpeed;
	me->UnCrouch();
}
