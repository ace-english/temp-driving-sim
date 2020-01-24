// Property of Dr. Khan, CSUS and Occupied Software.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TimeManager.generated.h"

/**
 * 
 */

 //Struct used for in-sim times. For real world times, use Unreal's FDateTime.
USTRUCT(Blueprintable)
struct FSimDateTimeStruct {
	GENERATED_USTRUCT_BODY()

	//Hours of in-sim time. This can hold values of 0-23, with 0 being 1 AM and 23 being midnight.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Date and Time")
	int32 Hour;

	//Minutes of in-sim time. This can hold any value between 0 and 59.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Date and Time")
	int32 Minute;

	//Seconds of in-sim time. This can hold any value between 0 and 59.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Date and Time")
	int32 Second;

	//FInventoryItemStruct is an internal object containing all the information needed to track an item in the inventory
	FSimDateTimeStruct() {
		Hour = 0;
		Minute = 0;
		Second = 0;
	}
};

UCLASS()
class TRAFFICSIM_API UTimeManager : public UObject, public FTickableGameObject
{
	//Changed from GENERATED_BODY() for FTickableGameObject
	GENERATED_UCLASS_BODY()

private:
	TStatId myStatId;

	//Since generic objects don't have a begin play, handle it ourselves in tick
	bool bFirstRun = true;

protected:
	//The current in-sim time
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Date and Time")
	FSimDateTimeStruct CurrentTime;


	//The starting time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Date and Time")
	FSimDateTimeStruct StartTime;

	//The rate time passes in-sim, as a seconds multipler. Value of 1 means game time matches real time. Default of 60 means time passes a minute a second. 
	//WARNING - DO NOT EDIT THIS IN EDITOR AFTER YOU HAVE PLAYED IN EDITOR! Restart the editor first or the editor will crash upon saving/compiling your blueprint!
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Date and Time")
	float TimeSpeedMultiplier = 1.0f;//60.0f;

	//The amount of time since the last update of the CurrentTime struct
	float TimeSinceLastUpdate = 0.0f;

	//Updates the current in-sim time
	void UpdateGameTime();

	//Rolls over excess units into the next higher bracket
	FSimDateTimeStruct RollOverUnits(FSimDateTimeStruct time);

public:
	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	//Returns the current in-sim time
	UFUNCTION(BlueprintCallable, Category = "Time")
	FSimDateTimeStruct GetCurrentSimTime();

	//Sets the current in-sim time.
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetCurrentSimTime(FSimDateTimeStruct newTime);
	void SetCurrentSimTime(float ticksFromBeginTime);

	//Sets the time speed.
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeSpeedMultiplier(float multiplier);

	//Gets the time speed multiplier
	UFUNCTION(BlueprintCallable, Category = "Time")
	float GetTimeSpeedMultiplier();

	//Converts real seconds ito in-sim time
	UFUNCTION(BlueprintCallable, Category = "Time")
	FSimDateTimeStruct GetSimTimeToRealTime(float seconds, float& excessOut);

	//Converts in-sim time relative to current time into real seconds
	UFUNCTION(BlueprintCallable, Category = "Time")
	float GetRealTimeToSimTime(FSimDateTimeStruct gameTime);

	//Adds two in-sim times together and returns the result
	UFUNCTION(BlueprintCallable, Category = "Time")
	FSimDateTimeStruct AddSimTimes(FSimDateTimeStruct time1, FSimDateTimeStruct time2);

	//Returns the later of the two times
	UFUNCTION(BlueprintCallable, Category = "Time")
	FSimDateTimeStruct GetLaterTime(FSimDateTimeStruct time1, FSimDateTimeStruct time2);
};
