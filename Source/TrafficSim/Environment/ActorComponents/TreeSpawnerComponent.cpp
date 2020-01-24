// Property of Dr. Khan, CSUS and Occupied Software.

#include "TreeSpawnerComponent.h"
#include "Engine.h"

// Sets default values for this component's properties
UTreeSpawnerComponent::UTreeSpawnerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	//Attach the spawn area to the center of the component
	spawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	FAttachmentTransformRules spawnAreaAttachRules = FAttachmentTransformRules::KeepRelativeTransform;
	spawnArea->AttachToComponent(this, spawnAreaAttachRules);
}


// Called when the game starts
void UTreeSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	//Add guard against spawning classes when editor starts as it can trigger BeginPlay on blueprint actors.
	if (spawnObjectClasses.Num() < 0) return;

	//Seed random number generator with current unix timestamp
	FMath::RandInit((int32)FDateTime::UtcNow().ToUnixTimestamp());

	//Gets the range to spawn from the center of the component.
	FVector spawnAreaExtent = spawnArea->GetScaledBoxExtent();

	//Attempt to spawn trees
	for (int spawnAttempt = 0; spawnAttempt < maxSpawnCount; spawnAttempt++) {
		//Get a random class to spawn.
		TSubclassOf<AActor> spawnClass = spawnObjectClasses[FMath::RandHelper(spawnObjectClasses.Num() - 1)];

		//Only proceed if this class is actually set.
		if (spawnClass != nullptr) {
			//Only spawn if the randomly selected point is actually empty
			FActorSpawnParameters newActorSpawnparamters;
			newActorSpawnparamters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

			//Attempt to spawn the actor. 
			AActor* newActor = GetWorld()->SpawnActor<AActor>(spawnClass, GetComponentLocation() + FVector(FMath::RandHelper(spawnAreaExtent.X * 2), FMath::RandHelper(spawnAreaExtent.Y * 2), 2) - spawnAreaExtent, FRotator(0, 0, 0), newActorSpawnparamters);

			//If the actor was spawned, add it to the collection of spawned objects.
			if (newActor != nullptr) {
				spawnedObjects.Add(newActor);
			}
		}
	}
	
}

void UTreeSpawnerComponent::BeginDestroy() {
	//Loop through and destroy any spawned objects
	for (AActor* spawnedActor : spawnedObjects) {
		spawnedActor->Destroy();
	}

	//Call parent BeginDestroy. Editor crashes without this line. 
	Super::BeginDestroy();
}

// Called every frame
void UTreeSpawnerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Spawner component doesn't tick, but function is needed as a child of USceneComponent
}

