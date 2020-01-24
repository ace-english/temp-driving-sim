#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"

class FSaveFolderDialogThread : public FRunnable{
	//Save dialog thread is a singleton, so we can always access it
	static FSaveFolderDialogThread* currentInstance;

	//The actual thread to run the save dialog on
	FRunnableThread* runningThread = nullptr;

	//The folder that the user has selected
	FString selectedFolder;

	//Use a stop counter so we can safely stop the thread... is this needed?
	FThreadSafeCounter stopTaskCounter;

	bool bIsFinished = false;

	FSaveFolderDialogThread();
	virtual ~FSaveFolderDialogThread();

	//The following must be defined for the FRunnable interface
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	//Ensures that the thread has properly stopped
	void EnsureFinished();
public:

	//Starts the thread. Static so we can easilly access it, and that only one dialog exists at a time.
	static FSaveFolderDialogThread* startThread();

	//Stop the thread. Static so it can easily be called
	static void stopThread();

	//Check if the thread is finished from any other thread
	static bool isThreadFinished();

	//Shows the actual save dialog
	void showDialog();

	bool IsFinished() const {
		return bIsFinished;
	}

	static FString getSelectedFolder();
};