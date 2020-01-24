// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicle.h"
#include "Components/BoxComponent.h"
#include "Logging/SightLoggable.h"
#include "Runtime/UMG/Public/Slate/WidgetRenderer.h"
#include "TrafficSimPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UTextRenderComponent;
class UInputComponent;

//Type of input device to listen to.
UENUM(BlueprintType)
enum class EControllerType : uint8 {
	CONTROLLER_KEYBOARD = 0 UMETA(DisplayName = "Keyboard or Xbox Controller"),
	CONTROLLER_G920 UMETA(DisplayName = "Logitech G920"),
	CONTROLLER_G27 UMETA(DisplayName = "Logitech G27")
};

//Gears a vehicle can be in.
UENUM(BlueprintType)
enum class EVehicleGear : uint8 {
	GEAR_PARK = 0 UMETA(DisplayName = "Park"),
	GEAR_REVERSE = 1 UMETA(DisplayName = "Reverse"),
	GEAR_NEUTRAL = 2 UMETA(DisplayName = "Neutral"),
	GEAR_DRIVE = 3 UMETA(DisplayName = "Drive")
};

//Blinker States
UENUM(BlueprintType)
enum class EBlinkerState : uint8 {
	BLINKER_NONE = 0 UMETA(DisplayName = "No Blinkers"),
	BLINKER_LEFT UMETA (DisplayName = "Left Blinker"),
	BLINKER_RIGHT UMETA (DisplayName = "Right Blinker"),
	BLINKER_HAZARD UMETA (DisplayName = "Hazard Blinkers")
};

UCLASS(config=Game)
class ATrafficSimPawn : public AWheeledVehicle, public ISightLoggable {
	GENERATED_BODY()

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	/** SCene component for the In-Car view origin */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* InternalCameraBase;

	/** Camera component for the In-Car view */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* InternalCamera;

	/** Text component for the In-Car speed */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* InCarSpeed;

	/** Text component for the In-Car gear */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* InCarGear;

	//Uses UCHildActorComponent instead of ULeapMotion due to current inability to access leap code from C++
	/** Leap motion hands actor */
	UPROPERTY(Category = LeapMotion, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* LeapHandsChildComponent;
	
public:
	ATrafficSimPawn();

	/** The current speed as a string eg 10 km/h */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText SpeedDisplayString;

	/** The current gear as a string (R,N, 1,2 etc) */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText GearDisplayString;

	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	/** The color of the incar gear text in forward gears */
	FColor	GearDisplayColor;

	/** The color of the incar gear text when in reverse */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FColor	GearDisplayReverseColor;

	/** Are we using incar camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bInCarCameraActive;

	/** Are we in reverse gear */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bInReverseGear;

	/** Are the windshield wipers enabled */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bWipersEnabled = false;

	/** Current left hand position. Updated from blueprint */
	UPROPERTY(Category = LeapHands, EditAnywhere, BlueprintReadWrite)
	FVector LeftHandPosition;

	/** Current right hand position. Updated from blueprint */
	UPROPERTY(Category = LeapHands, EditAnywhere, BlueprintReadWrite)
	FVector RightHandPosition;

	UPROPERTY(Category = SteeringWheel, EditAnywhere, BlueprintReadWrite)
	UBoxComponent* SteeringWheelCollision;

	/** Initial offset of incar camera for headsets */
	UPROPERTY(Category = SteeringWheel, EditAnywhere, BlueprintReadWrite)
	FVector InternalCameraOriginHMD;

	/** Initial offset of incar camera for 2D monitors */
	UPROPERTY(Category = SteeringWheel, EditAnywhere, BlueprintReadWrite)
	FVector InternalCameraOrigin2D;

	/** Temperature of vehicle for display on dashboard */
	UPROPERTY(Category = Dashboard, VisibleDefaultsOnly, BlueprintReadOnly)
	float VehicleTemperature = 75;

	/** Size of gas tank in gallons */
	UPROPERTY(Category = Dashboard, VisibleDefaultsOnly, BlueprintReadOnly)
	float GasTankSize = 16;

	/** Amount of fuel in vehicle */
	UPROPERTY(Category = Dashboard, VisibleDefaultsOnly, BlueprintReadOnly)
	float GasLevel = 12;

	/** Last Frame Steering Angle */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float LastSteeringAngle = 0;

	/** Steering Angle */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float CurrentSteeringAngle = 0;

	/**Steering ANgle Offset */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float steeringOffset = 0.0f;

	/** Gas Pedal Depression */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float GasPedalState = 0;

	/** Brake Pedal Depression */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float BrakePedalState = 0;

	/** Gas Pedal Effectiveness */
	UPROPERTY(Category = Input, VisibleDefaultsOnly, BlueprintReadOnly)
	float GasPedalEffectiveness = 1.0f;

	/** Brake Pedal Effectiveness */
	UPROPERTY(Category = Input, VisibleDefaultsOnly, BlueprintReadOnly)
	float BrakePedalEffectiveness = 1.0f;

	/** Steering Sensitivity */
	UPROPERTY(Category = Input, VisibleDefaultsOnly, BlueprintReadOnly)
	float SteeringSensitivity = 1.0f;

	/** Left Window Position */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float LeftWindowState = 1.0;

	/** Right Window Position */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float RightWindowState = 1.0;

	/** Windshield Wiper Position */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float WindshieldWiperState = 0.0;

	/** Windshield Wiper Speed */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float WindshieldWiperSpeed = 1.0;

	/** Windshield Wiper Direction */
	UPROPERTY(Category = Interior, VisibleDefaultsOnly, BlueprintReadOnly)
	float WindshieldWiperDirection = 1.0f;

	/** Car Gear */
	UPROPERTY(Category = Interior, VisibleAnywhere, BlueprintReadOnly)
	EVehicleGear CurrentGear;

	/** Blinker update timer */
	UPROPERTY(Category = Headlights, VisibleAnywhere, BlueprintReadOnly)
	float BlinkerTimer = 0.0f;

	/** Blinker light state */
	UPROPERTY(Category = Headlights, VisibleAnywhere, BlueprintReadOnly)
	bool bBlinkerLit = false;

	/** Car blinker state */
	UPROPERTY(Category = Headlights, VisibleAnywhere, BlueprintReadOnly)
	EBlinkerState currentBlinkerState = EBlinkerState::BLINKER_NONE;

	/** Car headlights */
	UPROPERTY(Category = Headlights, VisibleAnywhere, BlueprintReadOnly)//---------------------------------------------------------------
	bool headlightsOn = false;

	/** Car fog lights */
	UPROPERTY(Category = Headlights, VisibleAnywhere, BlueprintReadOnly)//---------------------------------------------------------------
	bool FogLightsOn = false;

	/** Whether to render mirrors. Currently this causes too much of a performance impact to be on by default, but is included for future hardware */
	UPROPERTY(Category = Mirrors, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bRenderMirrors = false;

	/** Whether to render hands. The wheel in the current car model is oversized, which becomes more apparent when the hands are shown */
	UPROPERTY(Category = Hands, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bRenderHands = false;

	/** Component for the left hedlight */
	UPROPERTY(Category = HeadLights, EditAnywhere, BlueprintReadWrite)
	USceneComponent* LeftHeadlight;

	/** Component for the right headlight */
	UPROPERTY(Category = HeadLights, EditAnywhere, BlueprintReadWrite)
	USceneComponent* RightHeadlight;

	/** Maps input strings to functions for player input */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/** Runs every frame. Delta is the time in seconds since the last frame */
	virtual void Tick(float Delta) override;

	/** Implementation of the IsLookedAt function of the SightLoggable interface */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Logging")
	int IsLookedAt(AActor* observer);
	virtual int IsLookedAt_Implementation(AActor* observer);

protected:
	/** Array of all components that should be treated by sight as Windows **/
	UPROPERTY(Category = Camera, EditAnywhere, BlueprintReadWrite)
	TArray<UActorComponent*> WindowComponents;

	/** Array of all components that are used for mirror captures **/
	UPROPERTY(Category = Mirrors, EditAnywhere, BlueprintReadWrite)
	TArray<UActorComponent*> MirrorCaptureComponents;/** Controller Type */
	
	UPROPERTY(Category = SteeringWheel, VisibleAnywhere, BlueprintReadOnly)
	EControllerType currentControllerType;

	/** Monitor rendered view **/
	UPROPERTY(Category = UI, VisibleAnywhere, BlueprintReadOnly)
	USceneCaptureComponent2D* monitorViewCaptureComponent = nullptr;

	/**UI Widget class to render with. Note that this must be set before object BeginPlay is called. **/
	UPROPERTY(Category = UI, EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UWidget> monitorUIWidgetClass;

	/** Widget to render in view **/
	UPROPERTY(Category = UI, VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* monitorUIWidget = nullptr;

	//NOTE - Because FWidgetRenderer is created using normal C++ new instead of Unreal New (as it is not an actor or UObject), it is not garbage collected and doesn't need UPROPERTY
	/** UI Widget Renderer **/
	FWidgetRenderer* monitorUIRenderer = nullptr;

	/** UI Widget Render Texture Target **/
	UPROPERTY()
	UTextureRenderTarget2D* uiRenderTarget = nullptr;

	/** Direction of the left window */
	float LeftWindowDirection = 0;

	/** Direction of the right window */
	float RightWindowDirection = 0;

	/**The object currently being looked at */
	FHitResult currentObjectLookedAt;

	/** Called when the simulation in the current map starts */
	virtual void BeginPlay() override;

	/** Called when the simulation in the current map ends */
	virtual void BeginDestroy() override;

	/** Gets the internal camera origin */
	FVector GetInternalCameraOrigin();

	/** Gets the internal camera origin */
	void SetInternalCameraOrigin(FVector origin);

	/** Updates the current state of the windows since the last frame */
	void UpdateWindows(float deltaTime);

	/** Updates the current state of the wipers since the last frame */
	void UpdateWipers(float deltaTime);

	/** Updates the current state of the blinkers */
	void UpdateBlinkers(float deltaTime);

	/** Updates the object the user is looking at this frame */
	void UpdateSight();

public:
	/** Handle pressing forwards */
	void MoveForward(float Val);
	/** Handle pressing forwards on keyboard */
	void MoveForward_Keyboard(float Val);
	/** Handle pressing forwards on Logitech G27 */
	void MoveForward_G27(float Val);
	/** Handle pressing forwards on Logitech G920 */
	void MoveForward_G920(float Val);
	/** Setup the strings used on the hud */
	void SetupInCarHUD();
	/** Update the physics material used by the vehicle mesh */
	void UpdatePhysicsMaterial();
	/** Handle pressing right */
	void MoveRight(float Val);
	/** Handle right input from keyboard */
	void MoveRight_Keyboard(float Val);
	/** Handle right input from Logitech G27 */
	void MoveRight_G27(float Val);
	/** Handle right input from Logitech G920 */
	void MoveRight_G920(float Val);
	/** Handle normal brake press */
	void OnBrakePressed(float Val);
	/** Handle normal brake from keyboard */
	void OnBrakePressed_Keyboard(float Val);
	/** Handle normal brake from Logitech G27 */
	void OnBrakePressed_G27(float Val);
	/** Handle normal brake from Logitech G920 */
	void OnBrakePressed_G920(float Val);
	/** Handle handbrake pressed */
	void OnHandbrakePressed();
	/** Handle handbrake released */
	void OnHandbrakeReleased();

	/** Handle toggle reverse */
	void OnToggleReverse();

	/** Handle toggle reverse from Keyboard */
	void OnToggleReverse_Keyboard();

	/** Handle toggle reverse from Logitech G27 */
	void OnToggleReverse_G27();

	/** Handles toggle reverse from Logitech G920 */
	void OnToggleReverse_G920();

	/** Handle pressing Left Window Down */
	UFUNCTION(Exec)
	void OnLeftWindowDownPressed();

	/** Handle releasing Left Window Down */
	void OnLeftWindowDownReleased();

	/** Handle pressing Left Window Up */
	UFUNCTION(Exec)
	void OnLeftWindowUpPressed();

	/** Handle releasing Left Window Up */
	void OnLeftWindowUpReleased();

	/** Handle pressing Right Window Down */
	UFUNCTION(Exec)
	void OnRightWindowDownPressed();

	/** Handle releasing Right Window Down */
	void OnRightWindowDownReleased();

	/** Handle pressing Right Window Up */
	UFUNCTION(Exec)
	void OnRightWindowUpPressed();

	/** Handle releasing Right Window Up */
	void OnRightWindowUpReleased();

	/** Handle toggling windshield wipers */
	UFUNCTION(Exec)
	void OnToggleWindshieldWipers();
	
	/** Handle togglign windshield wipers from G27 */
	void OnToggleWindshieldWipers_G27();

	/** Handle toggling windshield wipers from G920 */
	void OnToggleWindshieldWipers_G920();

	/** Handle changing blinker state */
	UFUNCTION(Exec, BlueprintCallable)
	void SetBlinkerState(EBlinkerState state);

	/** Handle toggling left blinker from keyboard */
	void OnToggleBlinkerLeft_Keyboard();

	/** Handle toggling left blinker from G27 */
	void OnToggleBlinkerLeft_G27();

	/** Handle toggling left blinker from G920 */
	void OnToggleBlinkerLeft_G920();

	/** Handle toggling right blinker from keyboard */
	void OnToggleBlinkerRight_Keyboard();

	/** Handle toggling right blinker frm G27 */
	void OnToggleBlinkerRight_G27();

	/** Handle toggling right blinker from G920 */
	void OnToggleBlinkerRight_G920();

	/** Handle toggling hazard lights from keyboard */
	void OnToggleHazardLights_Keyboard();

	/** Handle toggling hazard lights from G27*/
	void OnToggleHazardLights_G27();

	/** Handle togglign hazard lights from G920 */
	void OnToggleHazardLights_G920();

	/** Switch between cameras */
	void OnToggleCamera();
	/** Handle reset VR device */
	void OnResetVR();
	/** Handle reset VR from keyboard */
	void OnResetVR_Keyboard();
	/** Handle reset VR from G27 */
	void OnResetVR_G27();
	/** Handle reset VR from G920 */
	void OnResetVR_G920();

	/** Set state of vehicle's headlights */
	UFUNCTION(BlueprintCallable)
	void SetHeadlightState();
	
	/** Sets headlights from G27 */
	void OnToggleHeadlights_G27();

	/** Sets headlights from G920 */
	void OnToggleHeadlights_G920();

	/** Set state of vehicle's fog lights */
	void SetFogLightState();

	/** Set foglights from G27 */
	void OnToggleFoglights_G27();

	/** Sets foglights from G920 */
	void OnToggleFoglights_G920();

	/** Show input mode. Callable from console.*/
	UFUNCTION(exec)
	void ShowInputDevice();

	/** Set input mode. Callable from console.*/
	UFUNCTION(exec, BlueprintCallable)
	void SetInputDevice(EControllerType inputDeviceType);

	/** Updates the sim time. In Pawn since that is the only place console commands can exist */
	UFUNCTION(exec)
	void SetSimTime(int Hour, int Minute, int Second);

	/** Update the sim time multiplier. In Pawn since tha tis the only place console commands can exist */
	UFUNCTION(exec)
	void SetSimTimeMultiplier(float multiplier);

	/** Sets whether mirrors should be rendered */
	UFUNCTION(exec, BlueprintCallable)
	void SetRenderMirrors(bool render);

	/** Sets whether hands should be rendered*/
	UFUNCTION(exec, BlueprintCallable)
	void SetRenderHands(bool render);

	/** Sets the steering offset */
	UFUNCTION(exec, BlueprintCallable)
	void SetSteeringOffset(float offset);

	/** Sets the gas pedal effectiveness */
	UFUNCTION(exec, BlueprintCallable)
	void SetGasPedalEffectiveness(float effectiveness);

	/**Sets the brake pedal effectiveness */
	UFUNCTION(exec, BlueprintCallable)
	void SetBrakePedalEffectiveness(float effectiveness);

	/**Sets the steering sensitivity */
	UFUNCTION(exec, BlueprintCallable)
	void SetSteeringSensitivity(float sensitivity);

	/** Sets the fuel level */
	UFUNCTION(exec, BlueprintCallable)
	void SetFuelLevel(float level);

	/** Sets the fuel capacity */
	UFUNCTION(exec, BlueprintCallable)
	void SetFuelCapacity(float capacity);

	/** Spawns a test thread for the save file dialog **/
	UFUNCTION(exec)
	void ShowSaveDialog();

	/** Starts data logging with the given parameters, with this pawn as the main vehicle **/
	UFUNCTION(exec, BlueprintCallable)
	void StartDataLogging();

	/** Stops data logging **/
	UFUNCTION(exec, BlueprintCallable)
	void StopDataLogging();

	/** Get input mode */
	UFUNCTION(BlueprintCallable)
	EControllerType getInputDevice();

	static const FName LookUpBinding;
	static const FName LookRightBinding;

private:
	/** 
	 * Activate In-Car camera. Enable camera and sets visibility of incar hud display
	 *
	 * @param	bState true will enable in car view and set visibility of various if its doesnt match new state
	 * @param	bForce true will force to always change state
	 */
	void EnableIncarView( const bool bState, const bool bForce = false );

	/** Update the gear and speed strings */
	void UpdateHUDStrings();

	/** Called from blueprint implementation when the leap hands have moved */
	UFUNCTION(BlueprintCallable, Category = LeapHands)
	void handPositionUpdated();

	/* Are we on a 'slippery' surface */
	bool bIsLowFriction;


public:
	/** Returns SpringArm subobject **/
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	/** Returns InternalCamera subobject **/
	FORCEINLINE UCameraComponent* GetInternalCamera() const { return InternalCamera; }
	/** Returns InCarSpeed subobject **/
	FORCEINLINE UTextRenderComponent* GetInCarSpeed() const { return InCarSpeed; }
	/** Returns InCarGear subobject **/
	FORCEINLINE UTextRenderComponent* GetInCarGear() const { return InCarGear; }

	UFUNCTION(BlueprintCallable)
	FHitResult GetCurrentObjectLookedAt();
};
