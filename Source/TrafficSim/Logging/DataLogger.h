// Property of Dr. Khan, CSUS and Occupied Software.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Logging/LoggingStructs.h"
#include "Logging/CsvWriterThread.h"
#include "TrafficSimPawn.h"
#include "DataLogger.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TRAFFICSIM_API UDataLogger : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

	UDataLogger();
	~UDataLogger();

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;	

public:
	//Sets how often a data point will be recorded
	UFUNCTION(BlueprintCallable)
	void setLogInterval(float interval);

	//Gets how often a data point will be recorded
	UFUNCTION(BlueprintCallable)
	float getLogInterval();

	//Sets how often recorded data points will be written to DB or Disk
	UFUNCTION(BlueprintCallable)
	void setWriteInterval(float interval);

	//Gets how often recorded data points will be written to DB or disk
	UFUNCTION(BlueprintCallable)
	float getWriteInterval();

	//Sets the path to record data to
	UFUNCTION(blueprintCallable)
	void setLogPath(FString path);

	//Gets the path where data points will be written to
	UFUNCTION(BlueprintCallable)
	FString getLogPath();

	//Sets the traffic sim pawn to log data from
	UFUNCTION(BlueprintCallable)
	void setUserPawn(ATrafficSimPawn* pawn);

	//Starts data logging
	UFUNCTION(BlueprintCallable)
	void startLogging();

	//Stops data logging
	UFUNCTION(BlueprintCallable)
	void stopLogging();

	//Gets whether or not logging is occuring
	UFUNCTION(BlueprintCallable)
	bool isLogging();

	//Gets the logging start time
	UFUNCTION(BlueprintCallable)
	FDateTime getStartTime();

protected:
	//Header data for the recorded session
	FTrafficSimSessionData headerData;

	//The data points recorded whenever the log interval is hit.
	TArray<FSessionTimestampData> loggedData;
	
	FDateTime loggingStartTime;

	//Whether or not data logging is currently in progress.
	bool bIsRunning = false;
	
	//The interval a data point will be recorded in seconds
	float logInterval = 1.0f;
	
	//The interval data will be written to disk in seconds
	float writeInterval = 30.0f;

	//The elapsed log time for the current interval
	float elapsedLogTime = 0.0f;

	//The elapsed write time for the current interval
	float elapsedWriteTime = 0.0f;

	FString logPath;

	//The thread to use for Csv File Logging
	FCsvWriterThread* writerThread;

	//The traffic sim pawn to log for vehicle and user data
	ATrafficSimPawn* userPawn;

	//Record the current state
	void logData();

	//Write current recorded states to disk and empty the logged data buffer.
	void writeData();
};
