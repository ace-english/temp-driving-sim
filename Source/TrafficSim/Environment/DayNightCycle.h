// Property of Dr. Khan, CSUS and Occupied Software.

#pragma once

#include "CoreMinimal.h"
#include "TrafficSim.h"
#include "GameFramework/Actor.h"
#include "Environment/TimeManager.h"
#include "Runtime/Engine/Classes/Engine/DirectionalLight.h"
#include "DayNightCycle.generated.h"

UENUM(BlueprintType)
enum class EDayNightCycleState : uint8 {
	EDNCS_None = 0				UMETA(DisplayName = "No cycle state!"),
	EDNCS_Day					UMETA(DisplayName = "Day time"),
	EDNCS_Night					UMETA(DisplayName = "Night time")
};

UCLASS()
class TRAFFICSIM_API ADayNightCycle : public AActor
{
	GENERATED_BODY()
private:
	static ADayNightCycle* currentDNC;

	//The objects listening for events on this day/night cycle. 
	//These MUST implement IDaytimeObserver!
	UPROPERTY()
	TArray<UObject*> observers;

protected:
	//Whether or not first frame init has completed
	bool bFirstFrameInitPerformed = false;

	UTimeManager* gameTimeManager = NULL;

	//The light that acts as the scene's sun
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
	ADirectionalLight* sunActor = NULL;

	//The skybox blueprint. Type is an actor because Epic's implemention is 100% blueprint. DO NOT ASSIGN A NON-SKYSPHERE TO THIS BOX!
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
	AActor* skySphere = NULL;

	//The current day/night cycle state.
	EDayNightCycleState currentCycleState;

	//Does first frame int, for things that cannot be done on BeginPlay.
	void FirstFrameInit();

	virtual void updateCycleState();

public:
	//THe starting time for this Day/Night cycle. Must be set from editor or before BeginPlay is called.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
	FSimDateTimeStruct startTime;

	//The time scale multiplier for this Day/Night cycle. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
	float timeMultiplier = 1.0f;

	// Sets default values for this actor's properties
	ADayNightCycle();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	//Gets whether or not it is night
	virtual bool isNight();

	virtual EDayNightCycleState getCurrentCycleState();

	static ADayNightCycle* getCurrentDayNightCycle();
};
