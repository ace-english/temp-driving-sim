// Property of Dr. Khan, CSUS and Occupied Software.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "TreeSpawnerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TRAFFICSIM_API UTreeSpawnerComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTreeSpawnerComponent();

protected:
	//Spawned objects
	TArray<AActor*> spawnedObjects;

	//Classes to use for spawning at random.
	UPROPERTY(Category = "Spawn Properties", EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<AActor>> spawnObjectClasses;

	//The area to spawn the objects within.
	UPROPERTY(Category = "Spawn Properties", EditAnywhere, BlueprintReadWrite)
	UBoxComponent* spawnArea;

	//Number of spawns that will be attempted. Note that this does not mean that there will be this many instances of objects spawned!
	UPROPERTY(Category = "Spawn Properties", EditAnywhere, BlueprintReadWrite)
	int32 maxSpawnCount = 50;

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called when the object is destroyed or the game ends
	virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
