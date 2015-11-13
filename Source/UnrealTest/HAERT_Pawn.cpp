// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealTest.h"
#include "HAERT_Pawn.h"


// Sets default values
AHAERT_Pawn::AHAERT_Pawn()
{
	// set our turn rates for input
	BaseLookRightRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->AttachTo(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

 	// No need for ticks
	PrimaryActorTick.bCanEverTick = false;

	_CurrentMode = EPlayerMode::MechMode;

	bIsTransforming = false;
	bIsShootingPrimary = false;
	bIsShootingSecondary = false;

	MechAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	CarAcceleration = 2.0f * MechAcceleration;
}

// Called when the game starts or when spawned
void AHAERT_Pawn::BeginPlay()
{
	Super::BeginPlay();
	
}

//// Called every frame
//void AHAERT_Pawn::Tick( float DeltaTime )
//{
//	Super::Tick( DeltaTime );
//
//}

// Called to bind functionality to input
void AHAERT_Pawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);
	check(InputComponent);
	InputComponent->BindAxis("MoveForward", this, &AHAERT_Pawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AHAERT_Pawn::MoveRight);
	InputComponent->BindAxis("Strafe", this, &AHAERT_Pawn::Strafe);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("LookRight", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookRightRate", this, &AHAERT_Pawn::LookRightAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AHAERT_Pawn::LookUpAtRate);

	InputComponent->BindAction("Transform", IE_Pressed, this, &AHAERT_Pawn::SetTransformModeActionBind);

	InputComponent->BindAction("FirePrimary", IE_Pressed, this, &AHAERT_Pawn::FirePrimary);
	InputComponent->BindAction("FireSecondary", IE_Pressed, this, &AHAERT_Pawn::FireSecondary);
}

///Movement
void AHAERT_Pawn::MoveForward(float Value)
{
	switch (_CurrentMode)
	{
		case EPlayerMode::CarMode:
			MoveForwardCar(Value);
			break;

		case EPlayerMode::MechMode:
			MoveForwardMech(Value);
			break;

		default:
			break;
	}
}

void AHAERT_Pawn::MoveRight(float Value)
{

	switch (_CurrentMode)
	{
	case EPlayerMode::CarMode:
		MoveRightCar(Value);
		break;

	case EPlayerMode::MechMode:
		MoveRightMech(Value);
		break;

	default:
		break;
	}
}

void AHAERT_Pawn::Strafe(float Value)
{
	CurrentStrafeState = (Value < 0.0f) ? EStrafeState::StrafeLeft : (Value > 0.0f) ? EStrafeState::StrafeRight : EStrafeState::StrafeNone;

	switch (_CurrentMode)
	{
	case EPlayerMode::CarMode:
		StrafeCar(Value);
		break;

	case EPlayerMode::MechMode:
		StrafeMech(Value);
		break;

	default:
		break;
	}
}

void AHAERT_Pawn::LookRightAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	//AddControllerYawInput(Rate * BaseLookRightRate * GetWorld()->GetDeltaSeconds());
}

void AHAERT_Pawn::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	//AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//Transform
void AHAERT_Pawn::SetTransformModeActionBind()
{
	EPlayerMode Next = (_CurrentMode == EPlayerMode::CarMode) ? EPlayerMode::MechMode : EPlayerMode::CarMode;
	SetTransformMode(Next);
}

void AHAERT_Pawn::SetTransformMode(EPlayerMode NextMode)
{
	_CurrentMode = NextMode;
	bIsTransforming = true;
	UE_LOG(LogTemp, Warning, TEXT("TRANSFORMED!"));
}

void AHAERT_Pawn::FirePrimary()
{
	UE_LOG(LogTemp, Warning, TEXT("Primary Fired"));

	switch (_CurrentMode)
	{
	case EPlayerMode::CarMode:
		FirePrimaryCar();
		break;

	case EPlayerMode::MechMode:
		FirePrimaryMech();
		break;

	default:
		break;
	}
}

void AHAERT_Pawn::FireSecondary()
{
	UE_LOG(LogTemp, Warning, TEXT("Secondary Fired"));
	switch (_CurrentMode)
	{
	case EPlayerMode::CarMode:
		FireThrusterCar();
		break;

	case EPlayerMode::MechMode:
		FireSecondaryMech();
		break;

	default:
		break;
	}
}


///
///Car Mode
///
///
void AHAERT_Pawn::MoveForwardCar(float Value)
{

	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AHAERT_Pawn::MoveRightCar(float Value)
{

	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
void AHAERT_Pawn::StrafeCar(float Value)
{}

void AHAERT_Pawn::FirePrimaryCar()
{
}

void AHAERT_Pawn::FireThrusterCar()
{
}

///
///Mech Mode
///
///
void AHAERT_Pawn::MoveForwardMech(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AHAERT_Pawn::MoveRightMech(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AHAERT_Pawn::StrafeMech(float Value)
{

	if ((Controller != NULL) && (Value != 0.0f))
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;


		// find out which way is right
		const FRotator Rotation = this->GetActorRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void AHAERT_Pawn::FirePrimaryMech()
{
}

void AHAERT_Pawn::FireSecondaryMech()
{
}

