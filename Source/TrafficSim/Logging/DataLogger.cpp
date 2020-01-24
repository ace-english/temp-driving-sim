// Property of Dr. Khan, CSUS and Occupied Software.

#include "DataLogger.h"
#include "Engine.h"
#include "DataSingleton.h"
#include "WheeledVehicleMovementComponent4W.h"

UDataLogger::UDataLogger() {

}

UDataLogger::~UDataLogger() {
	//Write any unwritten data to disk if running
	//If destructor called, this likely indicates that the program is closing.
	if (bIsRunning) {
		writeData();

		//Busy wait until the write is done. This ensures the data gets written if this was called due to program exit.
		while (!writerThread->isThreadFinished());
	}
}

void UDataLogger::Tick(float DeltaTime) {
	//Only do tick if currently runing
	if (bIsRunning) {

		//Increment the elapsed log time.
		elapsedLogTime += DeltaTime;

		//Check to see if enough time has passed to log data.
		if (elapsedLogTime >= logInterval) {
			//Enough time has passed to log a data point.
			logData();

			//Remove one unit from the elapsed time.
			elapsedLogTime -= logInterval;
		}

		//Increment the elapsed write time
		elapsedWriteTime += DeltaTime;

		//Check to see if enough time has passed to write data
		if (elapsedWriteTime >= writeInterval) {
			//Enough time has passed to write data
			writeData();

			//Remove one unit form the elapsed time
			elapsedWriteTime -= writeInterval;
		}
	}
}

bool UDataLogger::IsTickable() const {
	return true;
}

//Needed to inherit from tickable object
TStatId UDataLogger::GetStatId() const {
	TStatId sid;
	return sid;
}


//Sets how often a data point will be recorded
void UDataLogger::setLogInterval(float interval) {
	logInterval = interval;
}

//Gets how often a data point will be recorded
float UDataLogger::getLogInterval() {
	return logInterval;
}

//Sets how often recorded data points will be written to DB or Disk
void UDataLogger::setWriteInterval(float interval) {
	writeInterval = interval;
}

//Gets how often recorded data points will be written to DB or disk
float UDataLogger::getWriteInterval() {
	return writeInterval;
}

//Sets the path to record data to
void UDataLogger::setLogPath(FString path) {
	logPath = path;
}

//Gets the path where data points will be written to
FString UDataLogger::getLogPath() {
	return logPath;
}

//Sets the traffic sim pawn to log data from
void UDataLogger::setUserPawn(ATrafficSimPawn* pawn) {
	userPawn = pawn;
}

//Starts data logging
void UDataLogger::startLogging() {
	loggingStartTime = FDateTime::Now();
	bIsRunning = true;
}

//Stops data logging
void UDataLogger::stopLogging() {
	bIsRunning = false;
	writeData(); 
}

//Gets whether or not logging is occuring
bool UDataLogger::isLogging() {
	return bIsRunning;
}

//Gets the logging start time
FDateTime UDataLogger::getStartTime() {
	return loggingStartTime;
}

//Record the current state
void UDataLogger::logData() {
	//Create data point struct
	FSessionTimestampData currentData;

	//If the user pawn isn't a traffic sim pawn or isn't properly set, we can't record any data from it.
	if (userPawn == nullptr) return;

	//Get a reference to the current Time Manager
	UTimeManager* timeManager = Cast<UDataSingleton>(GEngine->GameSingleton)->GetTimeManager();

	//Record the current date and time for this point
	currentData.timeStamp = FDateTime::Now().ToString();

	//TODO - implement the functionality that is currently commented out!
	FSessionUserData userData;
	userData.headLoc = userPawn->GetInternalCamera()->GetComponentLocation();
	userData.headRot = userPawn->GetInternalCamera()->GetComponentRotation();
	userData.handLeftLoc = userPawn->LeftHandPosition;
	//userData.handLeftRot = userPawn->LeftHandRotation;
	userData.handRightLoc = userPawn->RightHandPosition;
	//userData.handRightRot = userPawn->RightHandRotation;
	//userData.eyeOffsetLoc = userPawn->GetEyeLocationOffset();
	//userData.eyeOffsetRot = userPawn->GetEyeRotationOffset();
	AActor* sightActor = userPawn->GetCurrentObjectLookedAt().GetActor();
	if (sightActor) {
		userData.sightObj = sightActor->GetName();
		userData.sightObjLoc = sightActor->GetActorLocation();
		userData.sightObjRot = sightActor->GetActorRotation();
		userData.sightObjIntersectionLoc = userPawn->GetCurrentObjectLookedAt().ImpactPoint;
	} else {
		userData.sightObj = FString("None");
	}

	FSessionEnvData envData;
	//TODO - Implement ability to set these currently hard-coded environment attributes
	envData.simType = FString("Static Map");
	envData.mapType = FString("Static Map");
	envData.simHasTimeLimit = false;
	envData.simCurrentTime = FDateTime::Now();
	envData.timeOfDay = timeManager->GetCurrentSimTime();
	envData.timePassageRate = timeManager->GetTimeSpeedMultiplier();

	FSessionVehicleData vehicleData;
	vehicleData.velocity = userPawn->GetVehicleMovement()->GetForwardSpeed();
	vehicleData.vehicleLoc = userPawn->GetActorLocation();
	vehicleData.vehicleRot = userPawn->GetActorRotation();
	vehicleData.vehicleType = userPawn->GetName();
	vehicleData.gasPedalPressed = userPawn->GasPedalState;
	vehicleData.brakePedalPressed = userPawn->BrakePedalState;
	vehicleData.currentGear = userPawn->CurrentGear;
	vehicleData.headlightState = userPawn->headlightsOn;
	vehicleData.foglightState = userPawn->FogLightsOn;
	vehicleData.fuelLevel = userPawn->GasLevel;
	vehicleData.fuelCapacity = userPawn->GasTankSize;
	vehicleData.wiperLoc = userPawn->WindshieldWiperState;
	vehicleData.leftWindowPos = userPawn->LeftWindowState;
	vehicleData.rightWindowPos = userPawn->RightWindowState;
	vehicleData.mirrorsEnabled = userPawn->bRenderMirrors;
	vehicleData.temperature = userPawn->VehicleTemperature;
	vehicleData.steeringAngle = userPawn->CurrentSteeringAngle;
	vehicleData.blinkerState = userPawn->currentBlinkerState;

	//Link subdata structs to main struct
	currentData.userData = userData;
	currentData.envData = envData;
	currentData.vehicleData = vehicleData;

	//Store this data point
	loggedData.Add(currentData);
}

//Write current recorded states to disk and empty the logged data buffer.
void UDataLogger::writeData() {
	//Start Thread and give unwritten data, IF the last thread has finished. Otherwise, pass.
	if (writerThread == nullptr || writerThread->isThreadFinished()) {
		writerThread = FCsvWriterThread::startThread(logPath, loggedData);
		//Empty data point buffer
		loggedData.Empty();
	} else {
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Orange, FString::Printf(TEXT("Warning - CSV Writer Thread is falling behind on saving data!")));
	}
}


