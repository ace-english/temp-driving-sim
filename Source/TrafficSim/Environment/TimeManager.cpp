// Property of Dr. Khan, CSUS and Occupied Software.

#include "TimeManager.h"
#include "Engine.h"

UTimeManager::UTimeManager(const FObjectInitializer& ObjectInitializer) {

}

//Updates the current in-sim time
void UTimeManager::UpdateGameTime() {
	FSimDateTimeStruct GameTimeSinceUpdate = GetSimTimeToRealTime(TimeSinceLastUpdate, TimeSinceLastUpdate);
	CurrentTime = AddSimTimes(CurrentTime, GameTimeSinceUpdate);
}

//Rolls over excess units into the next higher bracket
FSimDateTimeStruct UTimeManager::RollOverUnits(FSimDateTimeStruct time) {
	//Convert excess seconds into minutes
	while (time.Second >= 60) {
		time.Second -= 60;
		time.Minute++;
	}

	//Convert excess minutes into hours
	while (time.Minute >= 60) {
		time.Minute -= 60;
		time.Hour++;
	}

	//Roll over excess hours
	while (time.Hour >= 24) {
		time.Hour -= 24;
	}

	return time;
}

// Called every frame
void UTimeManager::Tick(float DeltaSeconds) {
	if (bFirstRun) {
		CurrentTime = StartTime;
		bFirstRun = false;
	}
	FSimDateTimeStruct currentTimeBackup = CurrentTime;
	TimeSinceLastUpdate += DeltaSeconds * TimeSpeedMultiplier;

	UpdateGameTime();
	
	if (TimeSinceLastUpdate >= 1.0f) {
		UpdateGameTime();
	}
}

//Needs to be implemented for FTickableObject?
void UTimeManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

bool UTimeManager::IsTickable() const {
	//Only tick if we've actually been instantiated!
	if (this->IsDefaultSubobject())
		return false;

	return true;
}

TStatId UTimeManager::GetStatId() const {
	//What exactly do I have to do here? It seems we can't actually write any propterties in the id...
	//Documentation, as always, is borderline useless.
	return myStatId;
}

//Returns the current in-sim time
FSimDateTimeStruct UTimeManager::GetCurrentSimTime() {
	return CurrentTime;
}

//Sets the current in-sim time.
void UTimeManager::SetCurrentSimTime(FSimDateTimeStruct newTime) {
	CurrentTime = newTime;
}

//Sets the current in-sim time based on tick
void UTimeManager::SetCurrentSimTime(float ticksFromBeginTime) {
	CurrentTime = StartTime;
	Tick(ticksFromBeginTime);
}

//Sets the time speed.
void UTimeManager::SetTimeSpeedMultiplier(float multiplier) {
	TimeSpeedMultiplier = multiplier;
}

//Gets the time speed multiplier
float UTimeManager::GetTimeSpeedMultiplier() {
	return TimeSpeedMultiplier;
}

//Converts real seconds ito in-sim time. excessOut is the amount of seconds under 1.0f unused since time is kept using integers.
FSimDateTimeStruct UTimeManager::GetSimTimeToRealTime(float seconds, float& excessOut) {
	FSimDateTimeStruct newTime;

	//Add only full seconds in time
	newTime.Second += (int32)seconds;

	//Subtract whole seconds so we don't lose time under a second
	excessOut = seconds - (int32)seconds;

	return RollOverUnits(newTime);
}

//Converts in-sim time relative to current time into real seconds
float UTimeManager::GetRealTimeToSimTime(FSimDateTimeStruct gameTime) {
	return (gameTime.Second + (gameTime.Minute * 60) + (gameTime.Hour * 60 * 60)) / TimeSpeedMultiplier;
}

//Adds two in-sim times together and returns the result
FSimDateTimeStruct UTimeManager::AddSimTimes(FSimDateTimeStruct time1, FSimDateTimeStruct time2) {
	time1.Second += time2.Second;
	time1.Minute += time2.Minute;
	time1.Hour += time2.Hour;

	return RollOverUnits(time1);
}

//Returns the later of the two times
FSimDateTimeStruct UTimeManager::GetLaterTime(FSimDateTimeStruct time1, FSimDateTimeStruct time2) {
	//Assume time1 is larger by default
	FSimDateTimeStruct toReturn = time1;

	if (time2.Hour > time1.Hour) {
		toReturn = time2;
	} else if (time2.Hour == time1.Hour) {
		if (time2.Minute > time1.Minute) {
			toReturn = time2;
		} else if (time2.Minute == time1.Minute) {
			if (time2.Second > time1.Second) {
				toReturn = time2;
			}
		}
	}

	return toReturn;
}



