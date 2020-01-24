#pragma once

#include "CoreMinimal.h"
#include "TrafficSimPawn.h"
#include "Environment/TimeManager.h"
#include "LoggingStructs.generated.h"

USTRUCT(Blueprintable)
struct FTrafficSimSessionData {
	GENERATED_USTRUCT_BODY()

	//The name of the session being logged
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FString sessionName;
	//A description for the session being logged
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FString sessionDescription;
	//The name of the person being logged in the headset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FString subjectName;
	//The name of the person administering the simulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FString supervisorName;
};

USTRUCT(Blueprintable)
struct FSessionUserData {
	GENERATED_USTRUCT_BODY()

	//The location of the users head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FVector headLoc;
	//The rotation of the users head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FRotator headRot;
	//The location of the users left hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FVector handLeftLoc;
	//The rotation of the users left hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FRotator handLeftRot;
	//The location of the users right hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FVector handRightLoc;
	//The rotation of the users right hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FRotator handRightRot;
	//The object the user is looking at
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FString sightObj;
	//The world coordinate location of the object being looked at
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FVector sightObjLoc;
	//The world rotatin of the object being looked at
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FRotator sightObjRot;
	//The outermost point on the object where the user's sight intersects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FVector sightObjIntersectionLoc;
	//Unused, reserved for future use. Eye offset location.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FVector eyeOffsetLoc;
	//Unused, reserved for future use. Eye offset rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FRotator eyeOffsetRot;
};

USTRUCT(Blueprintable)
struct FSessionVehicleData {
	GENERATED_USTRUCT_BODY()

	//The type of vehicle (Ex. BrodyCar)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FString vehicleType;
	//Velocity of the vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float velocity;
	//Acceleration of the vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float acceleration;
	//World coordinate location of vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FVector vehicleLoc;
	//World coordinate rotation of vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FRotator vehicleRot;
	//How far the gas pedal is pressed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float gasPedalPressed;
	//How far the brake pedal is pressed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float brakePedalPressed;
	//The angle of the weels.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float wheelAngle;
	//The angle of the steering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float steeringAngle;
	//The height of the left window
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float leftWindowPos;
	//The height of the right window
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float rightWindowPos;
	//Whether headlights are on or not.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	bool headlightState;
	//Whether foglights are on or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	bool foglightState;
	//THe state of the blinkers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	EBlinkerState blinkerState;
	//The current vehicle gear
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	EVehicleGear currentGear;
	//Current vehicle RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	int32 vehicleRPM;
	//Whether mirrors are enabled or not.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	bool mirrorsEnabled;
	//Amount of fuel in the vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float fuelLevel;
	//How much fuel can be in this vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float fuelCapacity;
	//Temperature of the vehicle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float temperature;
	//Location of the windshield wipers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float wiperLoc;
	//For future use. Names of any colliding objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	TArray<FString> collidingObjNames;
};

USTRUCT(Blueprintable)
struct FSessionEnvData {
	GENERATED_USTRUCT_BODY()

	//The type of simulation in use (ex. looping, static...)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FString simType;
	//The type of map in use (ex. normal, looping, db...)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FString mapType;
	//The time of day in the simulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FSimDateTimeStruct timeOfDay;
	//How fast time passes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	float timePassageRate;
	//The start time of the simulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FDateTime simStartTime;
	//The end time of the simulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FDateTime simEndTime;
	//The current time for this data point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FDateTime simCurrentTime;
	//Whether or not this simulation has a time limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	bool simHasTimeLimit;
	//For future use. Any surroudning cars.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	TArray<FSessionVehicleData> surroundingCarData;
};

USTRUCT(Blueprintable)
struct FSessionTimestampData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FString timeStamp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FSessionUserData userData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FSessionVehicleData vehicleData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session Data")
	FSessionEnvData envData;
};