// Property of Dr. Khan, CSUS and Occupied Software.

#include "RandomizedActorSpawnerComponent.h"
#include "Engine.h"


// Sets default values for this component's properties
URandomizedActorSpawnerComponent::URandomizedActorSpawnerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void URandomizedActorSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	//Seed random number generator with current unix timestamp
	FMath::RandInit((int32)FDateTime::UtcNow().ToUnixTimestamp());

	FActorSpawnParameters spawnParameters;
	spawnParameters.SpawnCollisionHandlingOverride = spawnCollisionHandling;

	//Only spawn if we generate a random float below the specified percentage
	float spawnCheck = FMath::FRand();
	if (FMath::FRand() <= spawnPercentage){
		//Attempt to spawn the actor. 
		AActor* newActor = GetWorld()->SpawnActor<AActor>(spawnClass, GetComponentLocation(), GetComponentRotation(), spawnParameters);

		//If the actor was spawned, add it to the collection of spawned objects.
		if (newActor != nullptr) {
			spawnedObject = newActor;
		}
	}
	
}

void URandomizedActorSpawnerComponent::BeginDestroy() {
	if (spawnedObject != nullptr) {
		spawnedObject->Destroy();
	}

	Super::BeginDestroy();
}

// Called every frame
void URandomizedActorSpawnerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

