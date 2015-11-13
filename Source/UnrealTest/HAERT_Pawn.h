// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "HAERT_Pawn.generated.h"

UENUM(BlueprintType)
enum class EPlayerMode : uint8
{
	CarMode		UMETA(DisplayName = "CarMode"),
	MechMode	UMETA(DisplayName = "MechMode")
};

UENUM(BlueprintType)
enum class EStrafeState : uint8
{
	StrafeLeft	UMETA(DisplayName = "StrafeLeft"),
	StrafeNone	UMETA(DisplayName = "StrafeNone"),
	StrafeRight	UMETA(DisplayName = "StrafeRight")
};

UCLASS()
class UNREALTEST_API AHAERT_Pawn : public ACharacter
{
	GENERATED_BODY()


	/** Camera boom positioning the camera behind the character */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;


	float MechAcceleration;
	float CarAcceleration;

public:

	// Sets default values for this character's properties
	AHAERT_Pawn();


	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookRightRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	//virtual void Tick( float DeltaSeconds ) override;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ModeTransformation")
		bool bIsTransforming;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attacks")
		bool bIsShootingPrimary;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attacks")
		bool bIsShootingSecondary;
	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Strafing")
		EStrafeState CurrentStrafeState;

protected:

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	EPlayerMode _CurrentMode;

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	void Strafe(float Value);

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookRightAtRate(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

	UFUNCTION(BlueprintCallable, Category = "Transform")
	void SetTransformModeActionBind();
	void SetTransformMode(EPlayerMode NextMode);

	void FirePrimary();
	void FireSecondary();

	///Car Mode
	void MoveForwardCar(float Value);
	void MoveRightCar(float Value);
	void StrafeCar(float Value);
	void FirePrimaryCar();
	void FireThrusterCar();

	///Mech Mode
	void MoveForwardMech(float Value);
	void MoveRightMech(float Value);
	void StrafeMech(float Value);
	void FirePrimaryMech();
	void FireSecondaryMech();
	
};
