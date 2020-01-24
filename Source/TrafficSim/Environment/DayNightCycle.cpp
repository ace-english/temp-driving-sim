// Property of Dr. Khan, CSUS and Occupied Software.

#include "DayNightCycle.h"
#include "TrafficSimGameMode.h"
#include "DataSingleton.h"
#include "Engine.h"

// Set dfeault day / night cycle for compiler.
ADayNightCycle* ADayNightCycle::currentDNC = nullptr;

// Sets default values
ADayNightCycle::ADayNightCycle() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorHiddenInGame(true);

	timeMultiplier = 1.0f;
	bFirstFrameInitPerformed = false;

	skySphere = nullptr;
	sunActor = nullptr;

	bAllowTickBeforeBeginPlay = false;
}

// Called when the game starts or when spawned
void ADayNightCycle::BeginPlay() {
	Super::BeginPlay();

	UDataSingleton* ds = Cast<UDataSingleton>(GEngine->GameSingleton);
	gameTimeManager = ds->GetTimeManager();

	//Update singleton with start time
	gameTimeManager->SetCurrentSimTime(startTime);
	gameTimeManager->SetTimeSpeedMultiplier(timeMultiplier);

	if (!skySphere->FindFunction("UpdateSunDirection")) {
		skySphere = NULL;
	}

	//Keep a reference to the current DNC so we can easily access it anywhere.
	currentDNC = this;
}

// Called every frame
void ADayNightCycle::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if (bFirstFrameInitPerformed && currentDNC && currentDNC->IsValidLowLevelFast()) {
		FSimDateTimeStruct currentTime;

		//Ensure the time manager reference is valid
		UDataSingleton* ds = Cast<UDataSingleton>(GEngine->GameSingleton);
		gameTimeManager = ds->GetTimeManager();

		if (GEngine && gameTimeManager) {
			currentTime = gameTimeManager->GetCurrentSimTime();
		} else return;

		FRotator sunAngle;
		//This code was adapted from a blueprint tutorial, which found dividing the hour by 6 and multiplying by 90 worked.
		//Reduced that to just multiplying 15 for speed (in retrospect, the compiler probably already does this).
		//Applying this to the pitch results in a normal, realistic Day/Night cycle 
		float timeAngle = (((float)currentTime.Hour + (float)currentTime.Minute / 60 + (float)currentTime.Second / 3600) * 15.0f) + 90.0f;

		//Normal Day/Night cycle
		sunAngle.Pitch = timeAngle;
		sunAngle.Yaw = 0;

		if (sunActor && sunActor->IsValidLowLevelFast()) {
			sunActor->SetActorRotation(sunAngle);
		} else {
			//return;
		}

		if (skySphere && skySphere->IsValidLowLevelFast()) {
			FOutputDeviceNull dummy;
			int dummyTerm = 0;
			skySphere->CallFunctionByNameWithArguments(TEXT("UpdateSunDirection"), dummy, this, true);
		} 

		//Updates cycle state enum
		updateCycleState();
	} else {
		FirstFrameInit();
	}
}

void ADayNightCycle::updateCycleState() {
	EDayNightCycleState newCycleState = isNight() ? EDayNightCycleState::EDNCS_Night : EDayNightCycleState::EDNCS_Day;

	if (newCycleState != currentCycleState) {
		//Set the cycle state
		currentCycleState = newCycleState;
	}
}

//Does first frame int, for things that cannot be done on BeginPlay.
void ADayNightCycle::FirstFrameInit() {
	//Mainly used to prevent a race condition where the sunActor or skySphere actor can be null. Don't actually do anything.
	bFirstFrameInitPerformed = true;
}

ADayNightCycle* ADayNightCycle::getCurrentDayNightCycle() {
	return currentDNC;
}

EDayNightCycleState ADayNightCycle::getCurrentCycleState() {
	return currentCycleState;
}

//Gets whether or not it is night
bool ADayNightCycle::isNight() {
	//For now, simply check if sun yaw is above 0
	FRotator sunAngle = sunActor->GetActorRotation().GetNormalized();
	return sunAngle.Pitch > 0.0f;
}
