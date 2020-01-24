#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#include "Logging/LoggingStructs.h"

class FCsvWriterThread : public FRunnable {
	//CSV writer thread is a singleton, so we can always access it
	static FCsvWriterThread* currentInstance;

	//The actual thread to run the writer on
	FRunnableThread* runningThread = nullptr;

	//Whether or not the thread is finished writing data
	bool bIsFinished = false;

	//Path to write to
	FString folderPath;

	//Data to write
	TArray<FSessionTimestampData> loggedData;

	FCsvWriterThread(FString path, TArray<FSessionTimestampData> data);
	virtual ~FCsvWriterThread();

	//The following must be defined for the FRunnable interface
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	//Ensure that the thread has properly stopped
	void EnsureFinished();

public:
	//Starts the thread. Staic so we can easilly access it, and that only one writer exists at a time.
	static FCsvWriterThread* startThread(FString path, TArray<FSessionTimestampData> data);

	//Stop the thread. Static so it can easily be called.
	static void stopThread();

	//Check if thread is finished. Static so it can easilly be checked.
	static bool isThreadFinished();

	bool IsFinished() const {
		return bIsFinished;
	}
};