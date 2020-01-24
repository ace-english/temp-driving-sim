// Property of Dr. Khan, CSUS and Occupied Software.

#include "DataSingleton.h"
#include "Logging/SaveFolderDialogThread.h"



UDataSingleton::UDataSingleton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {

}

UTimeManager* UDataSingleton::GetTimeManager() {
	//If the time manager has not been created, create it.
	if (simTimeManager == nullptr) {
		simTimeManager = NewObject<UTimeManager>(GetTransientPackage(), UTimeManager::StaticClass());
	}

	return simTimeManager;
}

UDataLogger* UDataSingleton::GetDataLogger() {
	if (simDataLogger == nullptr) {
		simDataLogger = NewObject<UDataLogger>(GetTransientPackage(), UDataLogger::StaticClass());
	}

	return simDataLogger;
}

void UDataSingleton::ShowSaveDialog() {
	FSaveFolderDialogThread::startThread();
}

FString UDataSingleton::GetCurrentSaveDirectory() {
	FString toReturn;

	//Guard against accessing the folder while the thread still runs. This is needed because the Blueprint UI may constantly call this when updating.
	if (!FSaveFolderDialogThread::isThreadFinished() || FSaveFolderDialogThread::getSelectedFolder().Equals(FString(""))) {
		toReturn = FString("Select a folder to write into...");
	} else {
		toReturn = FSaveFolderDialogThread::getSelectedFolder();
	}

	return toReturn;
}