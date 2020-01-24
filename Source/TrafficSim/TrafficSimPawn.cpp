// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "TrafficSimPawn.h"
#include "TrafficSimWheelFront.h"
#include "TrafficSimWheelRear.h"
#include "TrafficSimHud.h"
#include "Environment/TimeManager.h"
#include "DataSingleton.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/TextRenderComponent.h"
#include "Materials/Material.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Controller.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Runtime/HeadMountedDisplay/Public/ISpectatorScreenController.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

//UWidget related headers. Figure out what exactly is needed and remove unneeded ones later when cleaning up, moving parts to singleton
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

// Needed for VR Headset
#if HMD_MODULE_INCLUDED
#include "IHeadMountedDisplay.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#endif // HMD_MODULE_INCLUDED

#include "Logging/SaveFolderDialogThread.h"

const FName ATrafficSimPawn::LookUpBinding("LookUp");
const FName ATrafficSimPawn::LookRightBinding("LookRight");

#define LOCTEXT_NAMESPACE "VehiclePawn"

ATrafficSimPawn::ATrafficSimPawn()
{
	// Car mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/Vehicle/Sedan/Sedan_SkelMesh.Sedan_SkelMesh"));
	GetMesh()->SetSkeletalMesh(CarMesh.Object);

	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/Vehicle/Sedan/Sedan_AnimBP"));
	GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);

	// Simulation
	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(GetVehicleMovement());

	check(Vehicle4W->WheelSetups.Num() == 4);

	Vehicle4W->WheelSetups[0].WheelClass = UTrafficSimWheelFront::StaticClass();
	Vehicle4W->WheelSetups[0].BoneName = FName("Wheel_Front_Left");
	Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	Vehicle4W->WheelSetups[1].WheelClass = UTrafficSimWheelFront::StaticClass();
	Vehicle4W->WheelSetups[1].BoneName = FName("Wheel_Front_Right");
	Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 12.f, 0.f);

	Vehicle4W->WheelSetups[2].WheelClass = UTrafficSimWheelRear::StaticClass();
	Vehicle4W->WheelSetups[2].BoneName = FName("Wheel_Rear_Left");
	Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -12.f, 0.f);

	Vehicle4W->WheelSetups[3].WheelClass = UTrafficSimWheelRear::StaticClass();
	Vehicle4W->WheelSetups[3].BoneName = FName("Wheel_Rear_Right");
	Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 12.f, 0.f);

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
	SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	// Create In-Car camera component 
	InternalCameraOriginHMD = FVector(-35.0f, -55.0f, 165.0f);
	InternalCameraOrigin2D = FVector(-35.0, -20.0f, 0);

	//InternalCameraBase is initially set to use InternalCameraOriginHMD as the function call for GetInternalCameraOrigin will not work in the constructor
	InternalCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("InternalCameraBase"));
	InternalCameraBase->SetRelativeLocation(InternalCameraOriginHMD);
	InternalCameraBase->SetupAttachment(GetMesh());

	InternalCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InternalCamera"));
	InternalCamera->bUsePawnControlRotation = false;
	InternalCamera->FieldOfView = 90.f;
	InternalCamera->SetupAttachment(InternalCameraBase);
	
	monitorViewCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MonitorViewCaptureComponent"));
	monitorViewCaptureComponent->SetupAttachment(InternalCamera);
	
	//Setup TextRenderMaterial
	static ConstructorHelpers::FObjectFinder<UMaterial> TextMaterial(TEXT("Material'/Engine/EngineMaterials/AntiAliasedTextMaterialTranslucent.AntiAliasedTextMaterialTranslucent'"));
	
	UMaterialInterface* Material = TextMaterial.Object;

	// Create text render component for in car speed display
	InCarSpeed = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarSpeed"));
	InCarSpeed->SetTextMaterial(Material);
	InCarSpeed->SetRelativeLocation(FVector(70.0f, -75.0f, 99.0f));
	InCarSpeed->SetRelativeRotation(FRotator(18.0f, 180.0f, 0.0f));
	InCarSpeed->SetupAttachment(GetMesh());
	InCarSpeed->SetRelativeScale3D(FVector(1.0f, 0.4f, 0.4f));

	// Create text render component for in car gear display
	InCarGear = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
	InCarGear->SetTextMaterial(Material);
	InCarGear->SetRelativeLocation(FVector(66.0f, -9.0f, 95.0f));	
	InCarGear->SetRelativeRotation(FRotator(25.0f, 180.0f,0.0f));
	InCarGear->SetRelativeScale3D(FVector(1.0f, 0.4f, 0.4f));
	InCarGear->SetupAttachment(GetMesh());
	
	// Colors for the incar gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	// Colors for the in-car gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	bInReverseGear = false;

	//Start with G27 as default inptu
	currentControllerType = EControllerType::CONTROLLER_G920;

	//Start in Drive
	CurrentGear = EVehicleGear::GEAR_DRIVE;
}

void ATrafficSimPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	//PlayerInputComponent->BindAxis("MoveForward", this, &ATrafficSimPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveForwardKeyboard", this, &ATrafficSimPawn::MoveForward_Keyboard);
	PlayerInputComponent->BindAxis("MoveForwardG27", this, &ATrafficSimPawn::MoveForward_G27);
	PlayerInputComponent->BindAxis("MoveForwardG920", this, &ATrafficSimPawn::MoveForward_G920);

	//PlayerInputComponent->BindAxis("MoveRight", this, &ATrafficSimPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveRightKeyboard", this, &ATrafficSimPawn::MoveRight_Keyboard);
	PlayerInputComponent->BindAxis("MoveRightG27", this, &ATrafficSimPawn::MoveRight_G27);
	PlayerInputComponent->BindAxis("MoveRightG920", this, &ATrafficSimPawn::MoveRight_G920);

	PlayerInputComponent->BindAxis("LookUp");
	PlayerInputComponent->BindAxis("LookRight");

	//PlayerInputComponent->BindAxis("Brakes", this, &ATrafficSimPawn::OnBrakePressed);
	PlayerInputComponent->BindAxis("BrakesKeyboard", this, &ATrafficSimPawn::OnBrakePressed_Keyboard);
	PlayerInputComponent->BindAxis("BrakesG27", this, &ATrafficSimPawn::OnBrakePressed_G27);
	PlayerInputComponent->BindAxis("BrakesG920", this, &ATrafficSimPawn::OnBrakePressed_G920);

	PlayerInputComponent->BindAction("ReverseKeyboard", IE_Pressed, this, &ATrafficSimPawn::OnToggleReverse_Keyboard);
	PlayerInputComponent->BindAction("ReverseG27", IE_Pressed, this, &ATrafficSimPawn::OnToggleReverse_G27);
	PlayerInputComponent->BindAction("ReverseG920", IE_Pressed, this, &ATrafficSimPawn::OnToggleReverse_G920);

	//Where is Brake defined in the input properties in the editor?
	//PlayerInputComponent->BindAxis("Brake", this, &ATrafficSimPawn::OnBrakePressed);

	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &ATrafficSimPawn::OnHandbrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &ATrafficSimPawn::OnHandbrakeReleased);
	PlayerInputComponent->BindAction("SwitchCamera", IE_Pressed, this, &ATrafficSimPawn::OnToggleCamera);

	//PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ATrafficSimPawn::OnResetVR); 
	PlayerInputComponent->BindAction("ResetVRKeyboard", IE_Pressed, this, &ATrafficSimPawn::OnResetVR_Keyboard);
	PlayerInputComponent->BindAction("ResetVRG27", IE_Pressed, this, &ATrafficSimPawn::OnResetVR_G27);
	PlayerInputComponent->BindAction("ResetVRG920", IE_Pressed, this, &ATrafficSimPawn::OnResetVR_G920);

	//Window Handlers
	PlayerInputComponent->BindAction("LeftWindowDown", IE_Pressed, this, &ATrafficSimPawn::OnLeftWindowDownPressed);
	PlayerInputComponent->BindAction("LeftWindowDown", IE_Released, this, &ATrafficSimPawn::OnLeftWindowDownReleased);
	PlayerInputComponent->BindAction("LeftWindowUp", IE_Pressed, this, &ATrafficSimPawn::OnLeftWindowUpPressed);
	PlayerInputComponent->BindAction("LeftWindowUp", IE_Released, this, &ATrafficSimPawn::OnLeftWindowUpReleased);

	PlayerInputComponent->BindAction("RightWindowDown", IE_Pressed, this, &ATrafficSimPawn::OnRightWindowDownPressed);
	PlayerInputComponent->BindAction("RightWindowDown", IE_Released, this, &ATrafficSimPawn::OnRightWindowDownReleased);
	PlayerInputComponent->BindAction("RightWindowUp", IE_Pressed, this, &ATrafficSimPawn::OnRightWindowUpPressed);
	PlayerInputComponent->BindAction("RightWindowUp", IE_Released, this, &ATrafficSimPawn::OnRightWindowUpReleased);

	PlayerInputComponent->BindAction("ToggleWipers", IE_Pressed, this, &ATrafficSimPawn::OnToggleWindshieldWipers);
	PlayerInputComponent->BindAction("ToggleWipersG27", IE_Pressed, this, &ATrafficSimPawn::OnToggleWindshieldWipers_G27);
	PlayerInputComponent->BindAction("ToggleWipersG920", IE_Pressed, this, &ATrafficSimPawn::OnToggleWindshieldWipers_G920);

	PlayerInputComponent->BindAction("ToggleHeadlights", IE_Pressed, this, &ATrafficSimPawn::SetHeadlightState);
	PlayerInputComponent->BindAction("ToggleHeadlightsG27", IE_Pressed, this, &ATrafficSimPawn::OnToggleHeadlights_G27);
	PlayerInputComponent->BindAction("ToggleHeadlightsG920", IE_Pressed, this, &ATrafficSimPawn::OnToggleHeadlights_G920);
	PlayerInputComponent->BindAction("ToggleFogLights", IE_Pressed, this, &ATrafficSimPawn::SetFogLightState);
	PlayerInputComponent->BindAction("ToggleFogLightsG27", IE_Pressed, this, &ATrafficSimPawn::OnToggleFoglights_G27);
	PlayerInputComponent->BindAction("ToggleFogLightsG920", IE_Pressed, this, &ATrafficSimPawn::OnToggleFoglights_G920);

	PlayerInputComponent->BindAction("ToggleBlinkerLeftKeyboard", IE_Pressed, this, &ATrafficSimPawn::OnToggleBlinkerLeft_Keyboard);
	PlayerInputComponent->BindAction("ToggleBlinkerLeftG27", IE_Pressed, this, &ATrafficSimPawn::OnToggleBlinkerLeft_G27);
	PlayerInputComponent->BindAction("ToggleBlinkerLeftG920", IE_Pressed, this, &ATrafficSimPawn::OnToggleBlinkerLeft_G920);

	PlayerInputComponent->BindAction("ToggleBlinkerRightKeyboard", IE_Pressed, this, &ATrafficSimPawn::OnToggleBlinkerRight_Keyboard);
	PlayerInputComponent->BindAction("ToggleBlinkerRightG27", IE_Pressed, this, &ATrafficSimPawn::OnToggleBlinkerRight_G27);
	PlayerInputComponent->BindAction("ToggleBlinkerRightG920", IE_Pressed, this, &ATrafficSimPawn::OnToggleBlinkerRight_G920);

	PlayerInputComponent->BindAction("ToggleHazardKeyboard", IE_Pressed, this, &ATrafficSimPawn::OnToggleHazardLights_Keyboard);
	PlayerInputComponent->BindAction("ToggleHazardG27", IE_Pressed, this, &ATrafficSimPawn::OnToggleHazardLights_G27);
	PlayerInputComponent->BindAction("ToggleHazardG920", IE_Pressed, this, &ATrafficSimPawn::OnToggleHazardLights_G920);
}

void ATrafficSimPawn::MoveForward(float Val) {
	GasPedalState = Val;
	GetVehicleMovementComponent()->SetThrottleInput(Val * (bInReverseGear ? -1 : 1) * GasPedalEffectiveness);
}

/** Handle pressing forwards on keyboard */
void ATrafficSimPawn::MoveForward_Keyboard(float Val) {
	MoveForward(Val);
}

/** Handle pressing forwards on Logitech G27 */
void ATrafficSimPawn::MoveForward_G27(float Val) {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		//Prevent default input from making car run in circles until input is recieved.
		static bool passedFirstInput = false;
		if (!passedFirstInput && (Val == 0)) {
			return;
		} else if (Val != 0) {
			passedFirstInput = true;
		}

		Val = (Val - 1.0) * -1;

		MoveForward(Val);
	}
}

/** Handle pressing forwards on Logitech G920 */
void ATrafficSimPawn::MoveForward_G920(float Val) {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		MoveForward(Val);
	}
}

void ATrafficSimPawn::MoveRight(float Val) {
	//Update the current steering angle
	//Multiply by 450 as this is the rotation limit of the G27 and G920 in one direction
	CurrentSteeringAngle = Val * 450;

	GetVehicleMovementComponent()->SetSteeringInput((Val * SteeringSensitivity) + (steeringOffset / 450));
}

/** Handle right input from keyboard */
void ATrafficSimPawn::MoveRight_Keyboard(float Val) {
	MoveRight(Val);
}

/** Handle right input from Logitech G27 */
void ATrafficSimPawn::MoveRight_G27(float Val) {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		//Prevent default input from making car run in circles until input is recieved.
		static bool passedFirstInput = false;
		if (!passedFirstInput && (Val == 0)) {
			return;
		} else if (Val != 0) {
			passedFirstInput = true;
		}

		//Kludge fix to get Danny's wheel working.
		//Val -= 1.5;

		Val = (Val - 0.5) * 2.0;

		MoveRight(Val);
	}
}

/** Handle right input from Logitech G920 */
void ATrafficSimPawn::MoveRight_G920(float Val) {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		//G920 reports from -0.5 to 0.5, need to double input.
		//This could be done in the editor's input mapping, but since the G27 requires adjustment that can't be done in the editor
		//this is done in C++ for consistancy.
		Val *= 2;

		MoveRight(Val);
	}
}

void ATrafficSimPawn::OnBrakePressed(float Val) {
	BrakePedalState = Val;

	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, FString::Printf(TEXT("Break input val: %f"), Val));
	if (!bInReverseGear) {
		GetVehicleMovementComponent()->SetBrakeInput(Val * BrakePedalEffectiveness);
	} else {
		//Reverse requires the bReverseOnBrake flag to be set on the vehicle movement component, which breaks brakes. So, use handbrake instead
		if (Val >= 0.25) {
			OnHandbrakePressed();
		} else {
			OnHandbrakeReleased();
		}

	}
}

/** Handle normal brake from keyboard */
void ATrafficSimPawn::OnBrakePressed_Keyboard(float Val) {
	OnBrakePressed(Val);
}

/** Handle normal brake from Logitech G27 */
void ATrafficSimPawn::OnBrakePressed_G27(float Val) {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		//Prevent default input from making car run in circles until input is recieved.
		static bool passedFirstInput = false;
		if (!passedFirstInput && (Val == 0)) {
			return;
		} else if (Val != 0) {
			passedFirstInput = true;
		}

		Val = (Val - 1.0) * -1;

		OnBrakePressed(Val);
	}
}

/** Handle normal brake from Logitech G920 */
void ATrafficSimPawn::OnBrakePressed_G920(float Val) {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		OnBrakePressed(Val);
	}
}


void ATrafficSimPawn::OnHandbrakePressed() {
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void ATrafficSimPawn::OnHandbrakeReleased() {
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

/** Handle toggle reverse */
void ATrafficSimPawn::OnToggleReverse() {
	bInReverseGear = !bInReverseGear;
	//Due to a bug in Unreal, bReverseAsBrake must be enabled to move backwards. However, this limits brake functionality so only enable it in reverse.
	GetVehicleMovementComponent()->bReverseAsBrake = !GetVehicleMovementComponent()->bReverseAsBrake;
	GetVehicleMovementComponent()->SetTargetGear(0, true);

	if (bInReverseGear) {
		CurrentGear = EVehicleGear::GEAR_REVERSE;
	} else {
		CurrentGear = EVehicleGear::GEAR_DRIVE;
	}
}

/** Handle toggle reverse from Keyboard */
void ATrafficSimPawn::OnToggleReverse_Keyboard() {
	OnToggleReverse();
}

/** Handle toggle reverse from Logitech G27 */
void ATrafficSimPawn::OnToggleReverse_G27() {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		OnToggleReverse();
	}
}

/** Handle toggle reverse from G920 */
void ATrafficSimPawn::OnToggleReverse_G920() {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		OnToggleReverse();
	}
}

/** Handle pressing Left Window Down */
void ATrafficSimPawn::OnLeftWindowDownPressed() {
	LeftWindowDirection = -1.0;
}

/** Handle releasing Left Window Down */
void ATrafficSimPawn::OnLeftWindowDownReleased() {
	LeftWindowDirection = 0.0;
}

/** Handle pressing Left Window Up */
void ATrafficSimPawn::OnLeftWindowUpPressed() {
	LeftWindowDirection = 1.0;
}

/** Handle releasing Left Window Up */
void ATrafficSimPawn::OnLeftWindowUpReleased() {
	LeftWindowDirection = 0.0;
}

/** Handle pressing Right Window Down */
void ATrafficSimPawn::OnRightWindowDownPressed() {
	RightWindowDirection = -1.0;
}

/** Handle releasing Right Window Down */
void ATrafficSimPawn::OnRightWindowDownReleased() {
	RightWindowDirection = 0.0;
}

/** Handle pressing Right Window Up */
void ATrafficSimPawn::OnRightWindowUpPressed() {
	RightWindowDirection = 1.0;
}

/** Handle releasing Right Window Up */
void ATrafficSimPawn::OnRightWindowUpReleased() {
	RightWindowDirection = 0.0;
}

/** Handle toggling windshield wipers */
void ATrafficSimPawn::OnToggleWindshieldWipers() {
	bWipersEnabled = !bWipersEnabled;
}

/** Handle togglign windshield wipers from G27 */
void ATrafficSimPawn::OnToggleWindshieldWipers_G27() {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		OnToggleWindshieldWipers();
	}
}

/** Handle toggling windshield wipers from G920 */
void ATrafficSimPawn::OnToggleWindshieldWipers_G920() {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		OnToggleWindshieldWipers();
	}
}

/** Handle changing of blinker state */
void ATrafficSimPawn::SetBlinkerState(EBlinkerState state) {
	currentBlinkerState = state;
	
	//Set the current lit state to on, unless the blinkers are entierly off.
	bBlinkerLit = (state != EBlinkerState::BLINKER_NONE);

	//Reset blinker timer
	BlinkerTimer = 0.0f;
}

/** Handle toggling left blinker from keyboard */
void ATrafficSimPawn::OnToggleBlinkerLeft_Keyboard() {
	if (currentControllerType == EControllerType::CONTROLLER_KEYBOARD) {
		if (currentBlinkerState == EBlinkerState::BLINKER_LEFT) {
			SetBlinkerState(EBlinkerState::BLINKER_NONE);
		} else {
			SetBlinkerState(EBlinkerState::BLINKER_LEFT);
		}
	}
}

/** Handle toggling left blinker from G27 */
void ATrafficSimPawn::OnToggleBlinkerLeft_G27() {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		if (currentBlinkerState == EBlinkerState::BLINKER_LEFT) {
			SetBlinkerState(EBlinkerState::BLINKER_NONE);
		} else {
			SetBlinkerState(EBlinkerState::BLINKER_LEFT);
		}
	}
}

/** Handle toggling left blinker from G920 */
void ATrafficSimPawn::OnToggleBlinkerLeft_G920() {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		if (currentBlinkerState == EBlinkerState::BLINKER_LEFT) {
			SetBlinkerState(EBlinkerState::BLINKER_NONE);
		} else {
			SetBlinkerState(EBlinkerState::BLINKER_LEFT);
		}
	}
}

/** Handle toggling right blinker from keyboard */
void ATrafficSimPawn::OnToggleBlinkerRight_Keyboard() {
	if (currentControllerType == EControllerType::CONTROLLER_KEYBOARD) {
		if (currentBlinkerState == EBlinkerState::BLINKER_RIGHT) {
			SetBlinkerState(EBlinkerState::BLINKER_NONE);
		} else {
			SetBlinkerState(EBlinkerState::BLINKER_RIGHT);
		}
	}
}

/** Handle toggling right blinker frm G27 */
void ATrafficSimPawn::OnToggleBlinkerRight_G27() {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		if (currentBlinkerState == EBlinkerState::BLINKER_RIGHT) {
			SetBlinkerState(EBlinkerState::BLINKER_NONE);
		} else {
			SetBlinkerState(EBlinkerState::BLINKER_RIGHT);
		}
	}
}

/** Handle toggling right blinker from G920 */
void ATrafficSimPawn::OnToggleBlinkerRight_G920() {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		if (currentBlinkerState == EBlinkerState::BLINKER_RIGHT) {
			SetBlinkerState(EBlinkerState::BLINKER_NONE);
		} else {
			SetBlinkerState(EBlinkerState::BLINKER_RIGHT);
		}
	}
}

/** Handle toggling hazard lights from keyboard */
void ATrafficSimPawn::OnToggleHazardLights_Keyboard() {
	if (currentControllerType == EControllerType::CONTROLLER_KEYBOARD) {
		if (currentBlinkerState == EBlinkerState::BLINKER_HAZARD) {
			SetBlinkerState(EBlinkerState::BLINKER_NONE);
		} else {
			SetBlinkerState(EBlinkerState::BLINKER_HAZARD);
		}
	}
}

/** Handle toggling hazard lights from G27*/
void ATrafficSimPawn::OnToggleHazardLights_G27() {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		if (currentBlinkerState == EBlinkerState::BLINKER_HAZARD) {
			SetBlinkerState(EBlinkerState::BLINKER_NONE);
		} else {
			SetBlinkerState(EBlinkerState::BLINKER_HAZARD);
		}
	}
}

/** Handle togglign hazard lights from G920 */
void ATrafficSimPawn::OnToggleHazardLights_G920() {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		if (currentBlinkerState == EBlinkerState::BLINKER_HAZARD) {
			SetBlinkerState(EBlinkerState::BLINKER_NONE);
		} else {
			SetBlinkerState(EBlinkerState::BLINKER_HAZARD);
		}
	}
}

void ATrafficSimPawn::OnToggleCamera() {
	EnableIncarView(!bInCarCameraActive);
}

void ATrafficSimPawn::EnableIncarView(const bool bState, const bool bForce) {
	if ((bState != bInCarCameraActive) || (bForce == true)) {
		bInCarCameraActive = bState;

		if (bState == true) {
			OnResetVR();
			if (Camera) {
				Camera->Deactivate();
				InternalCamera->Activate();
			}
		} else {
			if (Camera) {
				InternalCamera->Deactivate();
				Camera->Activate();
			}
		}

		InCarSpeed->SetVisibility(bInCarCameraActive);
		InCarGear->SetVisibility(bInCarCameraActive);
	}
}


void ATrafficSimPawn::Tick(float Delta) {
	Super::Tick(Delta);

	// Setup the flag to say we are in reverse gear
	//bInReverseGear = GetVehicleMovement()->GetCurrentGear() < 0;

	// Update the strings used in the hud (incar and onscreen)
	UpdateHUDStrings();

	// Set the string in the incar hud
	SetupInCarHUD();

	bool bHMDActive = false;
#if HMD_MODULE_INCLUDED
	if ((GEngine->HMDDevice.IsValid() == true) && ((GEngine->HMDDevice->IsHeadTrackingAllowed() == true) || (GEngine->IsStereoscopic3D() == true))) {
		bHMDActive = true;
	}

	//Update UI
	if (monitorUIWidget != nullptr && GEngine->HMDDevice.Get() != nullptr && GEngine->HMDDevice->IsHMDConnected()) {
		//Make the UI visible, since we're rendering to the monitor texture.
		FLinearColor uiColor;
		uiColor.A = 255;
		uiColor.R = 255;
		uiColor.G = 255;
		uiColor.B = 255;
		((UUserWidget*)monitorUIWidget)->SetColorAndOpacity(uiColor);

		//Draw the UI
		monitorUIRenderer->DrawWidget(uiRenderTarget, monitorUIWidget->TakeWidget(), FVector2D(/*1600, 900*/1295 * ((float)GSystemResolution.ResX / 1920.0f), 728 * ((float)GSystemResolution.ResY / 1080.0f)), Delta);

		//Render it on the headset
		ISpectatorScreenController* spectatorController = GEngine->HMDDevice->GetSpectatorScreenController();
		spectatorController->SetSpectatorScreenTexture(uiRenderTarget);

		//Make the UI invisible so the viewport doesn't render it in the headset
		uiColor.A = 0;
		((UUserWidget*)monitorUIWidget)->SetColorAndOpacity(uiColor);
	}

#endif // HMD_MODULE_INCLUDED
	if (bHMDActive == false) {
		if ((InputComponent) && (bInCarCameraActive == true)) {
			FRotator HeadRotation = InternalCamera->RelativeRotation;
			HeadRotation.Pitch += InputComponent->GetAxisValue(LookUpBinding);
			HeadRotation.Yaw += InputComponent->GetAxisValue(LookRightBinding);
			InternalCamera->RelativeRotation = HeadRotation;
		}
	}

	//Print position of the box component
	//Depreciated... no longer needed.
	if (SteeringWheelCollision != NULL) {
		FVector relativeCameraLoc = InternalCamera->GetRelativeTransform().GetLocation();
		relativeCameraLoc -= SteeringWheelCollision->GetRelativeTransform().GetLocation();
	}

	//Update Window State
	UpdateWindows(Delta);

	//Update Wiper State
	UpdateWipers(Delta);

	//Update Blinker State
	UpdateBlinkers(Delta);
	//Update sight for logged object
	UpdateSight();

	//Update last wheel angle for next tick
	LastSteeringAngle = CurrentSteeringAngle;
}

void ATrafficSimPawn::BeginPlay() {
	Super::BeginPlay();
	//Don't reverse with brakes. This actually completely breaks the use of brakes.
	GetVehicleMovementComponent()->bReverseAsBrake = false;

	bool bEnableInCar = false;

	//If this pawn is controlled by an actual user and not AI, set up headset and UI
	if (Cast<APlayerController>(GetController())) {
		//Create UI UMG widget
		monitorUIWidget = CreateWidget<UUserWidget>(Cast<APlayerController>(GetController()), monitorUIWidgetClass);

		//Add UMG widget to viewport. This is needed for input to work. Hiding from the headset will be handled in tick.
		monitorUIWidget->AddToViewport();

		//Force enable HMD
		if ((GEngine->HMDDevice.Get() != nullptr) && GEngine->HMDDevice->IsHMDConnected()) {
			GLog->Log("HMD Device is connected. Turning on...");
			GEngine->HMDDevice->EnableStereo(true);
			
			//Create UI Renderer
			monitorUIRenderer = new FWidgetRenderer(true);

			//Create render target for renderer
			//Size is from maximum values for mouse navigation. Values are scaled from what is known to work on a 1920x1080 display.
			uiRenderTarget = monitorUIRenderer->CreateTargetFor(FVector2D(1295 * ((float)GSystemResolution.ResX/1920.0f), 728 * ((float)GSystemResolution.ResY/1080.0f)), TextureFilter::TF_Default, false);
			
			//Set up spectator mode so we can render UI on monitor
			ISpectatorScreenController* spectatorController = GEngine->HMDDevice->GetSpectatorScreenController();
			spectatorController->SetSpectatorScreenMode(ESpectatorScreenMode::Texture);

		} else {
			GLog->Log("HMD Device is NOT connected!");
		}
	}

#if HMD_MODULE_INCLUDED
	bEnableInCar = UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
#endif // HMD_MODULE_INCLUDED
	EnableIncarView(true, true);

	APlayerController* playerController = Cast<APlayerController>(GetController());
	if (playerController) {
		playerController->bShowMouseCursor = true;
	}

	//Set default mirror state
	SetRenderMirrors(bRenderMirrors);
}


void ATrafficSimPawn::BeginDestroy() {
	Super::BeginDestroy();
	//monitorUIRenderer is not garbage collected by Unreal, so need to manually delete it.
	if (monitorUIRenderer != nullptr) {
		delete monitorUIRenderer;
	}
}


/** Gets the internal camera origin */
FVector ATrafficSimPawn::GetInternalCameraOrigin() {
	FVector toReturn = FVector::ZeroVector;

#if HMD_MODULE_INCLUDED
	if (((GEngine->HMDDevice.Get()) != nullptr) && (GEngine->HMDDevice->IsHMDConnected())) {
		toReturn = InternalCameraOriginHMD;
	} else {
		toReturn = InternalCameraOrigin2D;
	}
#else
	toReturn = InternalCameraOrigin2D;
#endif

	return toReturn;
}

int ATrafficSimPawn::IsLookedAt_Implementation(AActor* observer) {
	//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, FString::Printf(TEXT("Looking at the car!")));
	//Ureal interfaces apparently cannot return void.
	return 0;
}

/** Gets the internal camera origin */
void ATrafficSimPawn::SetInternalCameraOrigin(FVector origin) {
#if HMD_MODULE_INCLUDED
	if (((GEngine->HMDDevice.Get()) != nullptr) && (GEngine->HMDDevice->IsHMDConnected())) {
		InternalCameraOriginHMD = origin;
	} else {
		InternalCameraOrigin2D = origin;
	}
#else
	InternalCameraOrigin2D = origin;
#endif
}

void ATrafficSimPawn::UpdateWindows(float deltaTime) {
	LeftWindowState = FMath::Clamp(LeftWindowState + (LeftWindowDirection * deltaTime), 0.0f, 1.0f);
	RightWindowState = FMath::Clamp(RightWindowState + (RightWindowDirection * deltaTime), 0.0f, 1.0f);
}

void ATrafficSimPawn::UpdateWipers(float deltaTime) {
	//Correct wiper direction if its not correct
	if (WindshieldWiperDirection != -1 && WindshieldWiperDirection != 1.0) {
		WindshieldWiperDirection = 1.0f;
	}

	if (bWipersEnabled || (!bWipersEnabled && (WindshieldWiperState != 0.0f))) {
		WindshieldWiperState = FMath::Clamp(WindshieldWiperState + (WindshieldWiperDirection * deltaTime * WindshieldWiperSpeed), 0.0f, 1.0f);

		if (WindshieldWiperState == 1.0f) {
			WindshieldWiperDirection = -1.0f;
		} else if (WindshieldWiperState == 0.0f) {
			WindshieldWiperDirection = 1.0f;
		}
	}
}

/** Updates the current state of the blinkers */
void ATrafficSimPawn::UpdateBlinkers(float deltaTime) {
	//If the steering angle opposes the current blinker, turn them off.
	if (((currentBlinkerState == EBlinkerState::BLINKER_LEFT) && (CurrentSteeringAngle > LastSteeringAngle)) ||
		((currentBlinkerState == EBlinkerState::BLINKER_RIGHT) && (CurrentSteeringAngle < LastSteeringAngle))) {
		currentBlinkerState = EBlinkerState::BLINKER_NONE;
	}

	//Update blinker toggling
	if (currentBlinkerState != EBlinkerState::BLINKER_NONE) {
		BlinkerTimer += deltaTime;

		//If the timer is over 1 second, toggle the light.
		if (BlinkerTimer >= 0.5f) {
			bBlinkerLit = !bBlinkerLit;
			BlinkerTimer = 0.0f;
		}
	}
}

void ATrafficSimPawn::UpdateSight() {
	FVector headLocation = GetInternalCamera()->GetComponentLocation();
	FRotator headRotation = GetInternalCamera()->GetComponentRotation();

	FVector end = headLocation + headRotation.Vector() * 99999999999999999;
	FHitResult hitResult(ForceInit);
	FCollisionObjectQueryParams collisionParams;
	collisionParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	collisionParams.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
	collisionParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Vehicle);
	collisionParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
	collisionParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	bool bHitActor;

	TArray<FHitResult> hitObjects;
	bHitActor = GetWorld()->LineTraceMultiByObjectType(hitObjects, headLocation, end, collisionParams);

	//Clear the actor being looked at.
	currentObjectLookedAt = FHitResult::FHitResult();

	if (bHitActor) {
        bool bIsLookingOutWindow = false;
        AActor* closestActor = nullptr;
        FHitResult closestHitResult;
        
		//Loop through results
		for (FHitResult currentResult : hitObjects) {
			AActor* currentActor = currentResult.GetActor();
			bool bIsLookingOutWindow = false;
            
            //Assumes that sight is based on distance, and the first things hit are the closest to the origin of the raycast.
            if(closestActor == nullptr){
                closestActor = currentActor;
                closestHitResult = currentResult;
            }
            
			//Sometimes, actor reference goes null.
			if (currentActor == nullptr) {
				continue;
			} else if (currentActor == this) {
				//If current actor is self, ensure we are looking out a window. Otherwise, stop.
				//Check that the user is looking out a window.
				if (WindowComponents.Contains(currentResult.GetComponent())) {
					//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, FString::Printf(TEXT("Looking out the window")));
					bIsLookingOutWindow = true;
                    
                    //Need to reset the closest actor if we are looking out the window
                    if(closestActor == this){
                        closestActor = nullptr;
                        closestHitResult = FHitResult::FHitResult();
                    }
				}
			}
		}
        
        //See if we can set the actor being looked at.
        //Of course, a typical cast doesn't always work...
        if (closestActor != nullptr && closestActor->GetClass()->ImplementsInterface(USightLoggable::StaticClass())) {
            //However, since at this point we KNOW that the current actor implements the interface we'll do a standard non-Unreal cast.
            //Hopefully this doesn't come back to bite us later. If it does, we'll change currentSightLoggable to an AActor*.
            ISightLoggable* currentSightLoggable = (ISightLoggable*)closestActor;
            currentObjectLookedAt = closestHitResult;
            //Unreal interface functions cannot be called directly. Execute_ prefix is needed, possibly so blueprint implementation can be called if it exists?
            ISightLoggable::Execute_IsLookedAt(closestActor, this);
            //break;
        } else if (closestActor == nullptr){
            currentObjectLookedAt = FHitResult::FHitResult();
        }
	}

}

void ATrafficSimPawn::OnResetVR() {
#if HMD_MODULE_INCLUDED
	if ((GEngine != NULL) && bInCarCameraActive && GEngine->HMDDevice.Get() && GEngine->HMDDevice.IsValid()) {
		//Force view to be inside car.
		EnableIncarView(true);

		GEngine->HMDDevice->ResetOrientationAndPosition();
		InternalCamera->SetRelativeLocation(GetInternalCameraOrigin());
		if (GetController() != NULL) {
			//GetController()->SetControlRotation(FRotator());
		}

		//Attempt to align hands with the wheel
		if (LeapHandsChildComponent != NULL && SteeringWheelCollision != NULL) {
			FVector leapHandsActorLocation = LeapHandsChildComponent->GetRelativeTransform().GetLocation();
			leapHandsActorLocation.X = SteeringWheelCollision->GetRelativeTransform().GetLocation().X;
			LeapHandsChildComponent->SetRelativeLocation(leapHandsActorLocation);
		}
	}
#endif // HMD_MODULE_INCLUDED
	//Moved to be independent of VR so we can see changes without headset attached.
	InternalCamera->SetRelativeLocation(GetInternalCameraOrigin());
}

/** Handle reset VR from keyboard */
void ATrafficSimPawn::OnResetVR_Keyboard() {
	OnResetVR();
}

/** Handle reset VR from G27 */
void ATrafficSimPawn::OnResetVR_G27() {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		OnResetVR();
	}
}

/** Handle reset VR from G920 */
void ATrafficSimPawn::OnResetVR_G920() {
	//Mapped to Button 1, corresponds to A
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		OnResetVR();
	}
}

/**
When headlight key is pressed, this function is called
and sets the state of the vehicle's headlights to either
true or false.
*/
void ATrafficSimPawn::SetHeadlightState() {
	if (headlightsOn)
		headlightsOn = false;
	else
		headlightsOn = true;
}

/** Sets headlights from G27 */
void ATrafficSimPawn::OnToggleHeadlights_G27() {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		SetHeadlightState();
	}
}

/** Sets headlights from G920 */
void ATrafficSimPawn::OnToggleHeadlights_G920() {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		SetHeadlightState();
	}
}

/**
When fog light key is pressed, this function is called
and sets the state of the vehicle's fog lights to either
true or false.
*/
void ATrafficSimPawn::SetFogLightState() {
	if (FogLightsOn)
		FogLightsOn = false;
	else
		FogLightsOn = true;
}

/** Set foglights from G27 */
void ATrafficSimPawn::OnToggleFoglights_G27() {
	if (currentControllerType == EControllerType::CONTROLLER_G27) {
		SetFogLightState();
	}
}

/** Sets foglights from G920 */
void ATrafficSimPawn::OnToggleFoglights_G920() {
	if (currentControllerType == EControllerType::CONTROLLER_G920) {
		SetFogLightState();
	}
}

/** Show input mode*/
void ATrafficSimPawn::ShowInputDevice() {
	switch (currentControllerType) {
	case EControllerType::CONTROLLER_KEYBOARD:
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("Current controller type: keyboard")));
		break;
	case EControllerType::CONTROLLER_G27:
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("Current controller type: Logitech G27")));
		break;
	case EControllerType::CONTROLLER_G920:
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, FString::Printf(TEXT("Current controller type: Logitech G920")));
		break;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, FString::Printf(TEXT("Unknown input device type: %d"), (uint8)currentControllerType));
	}
}

/** Set input mode*/
void ATrafficSimPawn::SetInputDevice(EControllerType controller) {
	currentControllerType = controller;
}

/** Updates the sim time. In Pawn since that is the only place console commands can exist */
void ATrafficSimPawn::SetSimTime(int Hour, int Minute, int Second) {
	FSimDateTimeStruct newTime;
	newTime.Hour = Hour;
	newTime.Minute = Minute;
	newTime.Second = Second;

	UDataSingleton* ds = Cast<UDataSingleton>(GEngine->GameSingleton);
	ds->GetTimeManager()->SetCurrentSimTime(newTime);
}

/** Update the sim time multiplier. In Pawn since tha tis the only place console commands can exist */
void ATrafficSimPawn::SetSimTimeMultiplier(float multiplier) {
	UDataSingleton* ds = Cast<UDataSingleton>(GEngine->GameSingleton);
	ds->GetTimeManager()->SetTimeSpeedMultiplier(multiplier);
}

/** Sets whether mirrors should be rendered */
void ATrafficSimPawn::SetRenderMirrors(bool render) {
	bRenderMirrors = render;

	//Loop through all mirror components (typically scene captures) and set their active status
	for (UActorComponent* mirrorComponent : MirrorCaptureComponents) {
		if (bRenderMirrors) {
			mirrorComponent->Activate();
		} else {
			mirrorComponent->Deactivate();
		}
	}
}

/** Sets whether hands should be rendered*/
void ATrafficSimPawn::SetRenderHands(bool render) {
	bRenderHands = render;
}

/** Sets the steering offset */
void ATrafficSimPawn::SetSteeringOffset(float offset) {
	//Set the offset, ensuring that the value is valid.
	steeringOffset = FMath::Clamp<float>(offset, -450, 450);
}

/** Sets the gas pedal effectiveness */
void ATrafficSimPawn::SetGasPedalEffectiveness(float effectiveness) {
	GasPedalEffectiveness = FMath::Abs<float>(effectiveness);
}

/**Sets the brake pedal effectiveness */
void ATrafficSimPawn::SetBrakePedalEffectiveness(float effectiveness) {
	BrakePedalEffectiveness = FMath::Abs<float>(effectiveness);
}

/**Sets the steering sensitivity */
void ATrafficSimPawn::SetSteeringSensitivity(float sensitivity) {
	SteeringSensitivity = FMath::Abs<float>(sensitivity);
}

/** Sets the fuel level */
void ATrafficSimPawn::SetFuelLevel(float level) {
	//Ensure that there is a valid level of fuel.
	GasLevel = FMath::Clamp<float>(level, 0, GasTankSize);
}

/** Sets the fuel capacity */
void ATrafficSimPawn::SetFuelCapacity(float capacity) {
	//Ensure that the tank doesn't hold a negative value
	if (capacity < 0) {
		capacity = 0;
	}

	GasTankSize = capacity;

	//Reset the current fuel level to fit the new tank size if needed.
	SetFuelLevel(GasLevel);
}

/** Spawns a test thread for the save file dialog **/
void ATrafficSimPawn::ShowSaveDialog() {
	UDataSingleton* ds = Cast<UDataSingleton>(GEngine->GameSingleton);
	ds->ShowSaveDialog();
}

/** Starts data logging with the given parameters **/
void ATrafficSimPawn::StartDataLogging() {
	UDataSingleton* ds = Cast<UDataSingleton>(GEngine->GameSingleton);
	ds->GetDataLogger()->setLogPath(FSaveFolderDialogThread::getSelectedFolder());
	ds->GetDataLogger()->setUserPawn(this);
	ds->GetDataLogger()->startLogging();
}

/** Stops data logging **/
void ATrafficSimPawn::StopDataLogging() {
	UDataSingleton* ds = Cast<UDataSingleton>(GEngine->GameSingleton);
	ds->GetDataLogger()->stopLogging();
}

/** Get input mode */
EControllerType ATrafficSimPawn::getInputDevice() {
	return currentControllerType;
}

void ATrafficSimPawn::UpdateHUDStrings() {
	float KPH = FMath::Abs(GetVehicleMovement()->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);
	int32 MPH_int = KPH_int * 0.6213712;
	// Using FText because this is display text that should be localizable
	//SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} km/h"), FText::AsNumber(KPH_int));

	SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} mph"), FText::AsNumber(MPH_int));

	if (bInReverseGear == true) {
		GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
	} else {
		int32 Gear = GetVehicleMovement()->GetCurrentGear();
		GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
	}
}

void ATrafficSimPawn::handPositionUpdated() {
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("Hand position: Left %s, Right %s"), *LeftHandPosition.ToString(), *RightHandPosition.ToString()));
}

void ATrafficSimPawn::SetupInCarHUD() {
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if ((PlayerController != nullptr) && (InCarSpeed != nullptr) && (InCarGear != nullptr)) {
		// Setup the text render component strings
		InCarSpeed->SetText(SpeedDisplayString);
		InCarGear->SetText(GearDisplayString);

		if (bInReverseGear == false) {
			InCarGear->SetTextRenderColor(GearDisplayColor);
		} else {
			InCarGear->SetTextRenderColor(GearDisplayReverseColor);
		}
	}
}

FHitResult ATrafficSimPawn::GetCurrentObjectLookedAt() {
	return currentObjectLookedAt;
}
#undef LOCTEXT_NAMESPACE
