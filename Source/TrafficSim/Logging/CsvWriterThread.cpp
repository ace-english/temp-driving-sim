#include "CsvWriterThread.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Logging/LoggingStructs.h"

//Needed for Unreal to compile
FCsvWriterThread* FCsvWriterThread::currentInstance = nullptr;

FCsvWriterThread::FCsvWriterThread(FString path, TArray<FSessionTimestampData> data) {
	//Correct directory delimiter since it may be Windows-specific and Unreal expects Unix style paths.
	folderPath = path.Replace(*FString("\\"), *FString("/"));;
	loggedData = data;
	runningThread = FRunnableThread::Create(this, TEXT("FCsvWriterThreadRunnable"), 0, TPri_Normal);
}

FCsvWriterThread::~FCsvWriterThread() {
	//Clear the running thread
	delete runningThread;
	runningThread = nullptr;
}

//The following mus tbe defined for the FRunnable interface
bool FCsvWriterThread::Init() {
	return true;
}

uint32 FCsvWriterThread::Run() {
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.CreateDirectoryTree(*folderPath)) {
		FString userDataPath = folderPath + FString("/userData.csv");
		FString userDataCSV;
		FString vehicleDataPath = folderPath + FString("/vehicleData.csv");
		FString vehicleDataCSV;
		FString envDataPath = folderPath + FString("/envData.csv");
		FString envDataCSV;

		if (PlatformFile.FileExists(*userDataPath)) {
			//Load in previously logged data
			FFileHelper::LoadFileToString(userDataCSV, *userDataPath);
		} else {
			//Write Header
			userDataCSV.Append(FString("User data,\n"));
			userDataCSV.Append(FString("Timestamp,"));
			userDataCSV.Append(FString("Head Location X,Head Location Y,Head Location Z,"));
			userDataCSV.Append(FString("Head Rotation Yaw,Head Rotation Pitch,Head Rotation Roll,"));
			userDataCSV.Append(FString("Left Hand Location X,Left Hand Location Y,Left Hand Location Z,"));
			userDataCSV.Append(FString("Left Hand Rotation Yaw,Left Hand Rotation Pitch,Left Hand Roation Roll,"));
			userDataCSV.Append(FString("Right Hand Location X,Right Hand Location Y,Right Hand Location Z,"));
			userDataCSV.Append(FString("Right Hand Rotation Yaw,Right Hand Rotation Pitch,Right Hand Roation Roll,"));
			userDataCSV.Append(FString("Sight Object,"));
			userDataCSV.Append(FString("Sight Object Location X,Sight Object Location Y,Sight Object Location Z,"));
			userDataCSV.Append(FString("Sight Object Rotation Yaw,Sight Object Rotation Pitch,Sight Object Rotation Roll,"));
			userDataCSV.Append(FString("Sight Object Intersect Location X,Sight Object Intersect Location Y,Sight Object Intersect Location Z,"));
			userDataCSV.Append(FString("Eye Offset Location X,Eye Offset Location Y,Eye Offset Location Z,"));
			userDataCSV.Append(FString("Eye Offset Rotation Yaw,Eye Offset Rotation Pitch,Eye Offset Rotation Roll,"));
			userDataCSV.Append(FString("\n"));
		}
		
		if (PlatformFile.FileExists(*vehicleDataPath)) {
			//Load in previous data
			FFileHelper::LoadFileToString(vehicleDataCSV, *vehicleDataPath);
		} else {
			//Write Header
			vehicleDataCSV.Append(FString("Vehicle data,\n"));
			vehicleDataCSV.Append(FString("Timestamp,"));
			vehicleDataCSV.Append(FString("Vehicle Type,"));
			vehicleDataCSV.Append(FString("Location X,Location Y,Location Z,"));
			vehicleDataCSV.Append(FString("Rotation Yaw,Rotation Pitch,Rotation Roll,"));
			vehicleDataCSV.Append(FString("Velocity,"));
			vehicleDataCSV.Append(FString("Acceleration,"));
			vehicleDataCSV.Append(FString("RPM,"));
			vehicleDataCSV.Append(FString("Gear,"));
			vehicleDataCSV.Append(FString("Gas Pedal Pressed,Brake Pedal Pressed,"));
			vehicleDataCSV.Append(FString("Steering Angle,Wheel Angle,"));
			vehicleDataCSV.Append(FString("Headlights On,Foglights On,"));
			vehicleDataCSV.Append(FString("Blinkers,"));
			vehicleDataCSV.Append(FString("Left Window Position,Right Window Position,Wiper Position,"));
			vehicleDataCSV.Append(FString("Fuel Level,Fuel Capacity,Temperature,"));
			vehicleDataCSV.Append(FString("Mirrors Enabled,"));
			vehicleDataCSV.Append(FString("\n"));
		}

		if (PlatformFile.FileExists(*envDataPath)) {
			//Load in previous data
			FFileHelper::LoadFileToString(envDataCSV, *envDataPath);
		} else {
			//Write Header
			envDataCSV.Append(FString("Environment Data,\n"));
			envDataCSV.Append(FString("Timestamp,"));
			envDataCSV.Append(FString("Sim Type,Map Type,"));
			envDataCSV.Append(FString("Has Time Limit,End Time,"));
			envDataCSV.Append(FString("In-Sim Time,In-Sim Time Passage,"));
			envDataCSV.Append(FString("\n"));
		}

		for (FSessionTimestampData timestampedData : loggedData) {

			//Write user data
			FSessionUserData userData = timestampedData.userData;
			userDataCSV.Append(timestampedData.timeStamp + FString(","));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.headLoc.X, userData.headLoc.Y, userData.headLoc.Z));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.headRot.Yaw, userData.headRot.Pitch, userData.headRot.Roll));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.handLeftLoc.X, userData.handLeftLoc.Y, userData.handLeftLoc.Z));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.handLeftRot.Yaw, userData.handLeftRot.Pitch, userData.handLeftRot.Roll));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.handRightLoc.X, userData.handRightLoc.Y, userData.handRightLoc.Z));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.handRightRot.Yaw, userData.handRightRot.Pitch, userData.handRightRot.Roll));
			userDataCSV.Append(userData.sightObj + FString(","));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.sightObjLoc.X, userData.sightObjLoc.Y, userData.sightObjLoc.Z));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.sightObjRot.Yaw, userData.sightObjRot.Pitch, userData.sightObjRot.Roll));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.sightObjIntersectionLoc.X, userData.sightObjIntersectionLoc.Y, userData.sightObjIntersectionLoc.Z));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.eyeOffsetLoc.X, userData.eyeOffsetLoc.Y, userData.eyeOffsetLoc.Z));
			userDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), userData.eyeOffsetRot.Yaw, userData.eyeOffsetRot.Pitch, userData.eyeOffsetRot.Roll));
			//Append new line
			userDataCSV.Append(FString("\n"));

			//Write env data
			FSessionVehicleData vehicleData = timestampedData.vehicleData;
			vehicleDataCSV.Append(timestampedData.timeStamp + FString(","));
			vehicleDataCSV.Append(vehicleData.vehicleType + FString(","));
			vehicleDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), vehicleData.vehicleLoc.X, vehicleData.vehicleLoc.Y, vehicleData.vehicleLoc.Z));
			vehicleDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), vehicleData.vehicleRot.Yaw, vehicleData.vehicleRot.Pitch, vehicleData.vehicleRot.Roll));
			vehicleDataCSV.Append(FString::Printf(TEXT("%f,"), vehicleData.velocity));
			vehicleDataCSV.Append(FString::Printf(TEXT("%f,"), vehicleData.acceleration));
			vehicleDataCSV.Append(FString::Printf(TEXT("%d,"), vehicleData.vehicleRPM));
			switch (vehicleData.currentGear) {
				case EVehicleGear::GEAR_PARK:
					vehicleDataCSV.Append(FString("Park,"));
					break;
				case EVehicleGear::GEAR_NEUTRAL:
					vehicleDataCSV.Append(FString("Neutral,"));
					break;
				case EVehicleGear::GEAR_REVERSE:
					vehicleDataCSV.Append(FString("Reverse,"));
					break;
				case EVehicleGear::GEAR_DRIVE:
					vehicleDataCSV.Append(FString("Drive,"));
					break;
				default:
					vehicleDataCSV.Append(FString("Unknown,"));
					break;
			}
			vehicleDataCSV.Append(FString::Printf(TEXT("%f,%f,"), vehicleData.gasPedalPressed, vehicleData.brakePedalPressed));
			vehicleDataCSV.Append(FString::Printf(TEXT("%f,%f,"), vehicleData.steeringAngle, vehicleData.wheelAngle));
			vehicleDataCSV.Append(FString(vehicleData.headlightState ? "On" : "Off") + FString(",") + FString(vehicleData.foglightState ? "On" : "Off") + FString(","));
			switch (vehicleData.blinkerState) {
				case EBlinkerState::BLINKER_NONE:
					vehicleDataCSV.Append(FString("Off,"));
					break;
				case EBlinkerState::BLINKER_LEFT:
					vehicleDataCSV.Append(FString("Left,"));
					break;
				case EBlinkerState::BLINKER_RIGHT:
					vehicleDataCSV.Append(FString("Right,"));
					break;
				case EBlinkerState::BLINKER_HAZARD:
					vehicleDataCSV.Append(FString("Hazard,"));
					break;
				default:
					vehicleDataCSV.Append(FString("Unknown,"));
					break;
			}
			vehicleDataCSV.Append(FString::Printf(TEXT("%f,%,%f,"), vehicleData.leftWindowPos, vehicleData.rightWindowPos, vehicleData.wiperLoc));
			vehicleDataCSV.Append(FString::Printf(TEXT("%f,%f,%f,"), vehicleData.fuelLevel, vehicleData.fuelCapacity, vehicleData.temperature));
			vehicleDataCSV.Append(FString(vehicleData.mirrorsEnabled ? "Yes" : "No") + FString(","));
			//Append new line
			vehicleDataCSV.Append(FString("\n"));

			//Write environment data
			FSessionEnvData envData = timestampedData.envData;
			envDataCSV.Append(timestampedData.timeStamp + FString(","));
			envDataCSV.Append(envData.simType + FString(",") + envData.mapType + FString(","));
			envDataCSV.Append(FString(envData.simHasTimeLimit ? "Yes" : "No") + FString(",") + envData.simEndTime.ToString() + FString(","));
			envDataCSV.Append(FString::FromInt(envData.timeOfDay.Hour) + FString(":") + FString::FromInt(envData.timeOfDay.Minute) + FString(":") + FString::FromInt(envData.timeOfDay.Second) + FString(",") + FString(envData.timePassageRate ? "Yes" : "No") + FString(","));
			//Append new line
			envDataCSV.Append("\n");
		}

		//write strings to files
		FFileHelper::SaveStringToFile(userDataCSV, *userDataPath);
		FFileHelper::SaveStringToFile(vehicleDataCSV, *vehicleDataPath);
		FFileHelper::SaveStringToFile(envDataCSV, *envDataPath);
	}

	Stop();
	return 0;
}

void FCsvWriterThread::Stop() {
	bIsFinished = true;
}

//Ensures that the thread has properly stopped
void FCsvWriterThread::EnsureFinished() {
	Stop();
	runningThread->WaitForCompletion();
}

//Starts the thread
FCsvWriterThread* FCsvWriterThread::startThread(FString path, TArray<FSessionTimestampData> data) {
	//Create a new instance of the thread if it doesn't exist
	if (currentInstance == nullptr || currentInstance->IsFinished()){
		currentInstance = new FCsvWriterThread(path, data);
	}

	return currentInstance;
}

//Stop the thread
void FCsvWriterThread::stopThread() {
	if (currentInstance != nullptr) {
		currentInstance->EnsureFinished();
		delete currentInstance;
		currentInstance = nullptr;
	}
}

bool FCsvWriterThread::isThreadFinished() {
	if (currentInstance != nullptr) {
		return currentInstance->IsFinished();
	} else {
		return true;
	}
}