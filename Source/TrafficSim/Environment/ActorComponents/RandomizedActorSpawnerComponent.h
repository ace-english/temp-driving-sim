// Property of Dr. Khan, CSUS and Occupied Software.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Engine.h"
#include "RandomizedActorSpawnerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TRAFFICSIM_API URandomizedActorSpawnerComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URandomizedActorSpawnerComponent();

	//Actor class to spawn
	UPROPERTY(Category = "Spawn Properties", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> spawnClass;

	//Chance oce spawning
	UPROPERTY(Category = "Spawn Properties", EditAnywhere, BlueprintReadWrite)
	float spawnPercentage = 1.0;

	//Spawn parameters
	UPROPERTY(Category = "Spawn Properties", EditAnywhere, BlueprintReadWrite)
	ESpawnActorCollisionHandlingMethod spawnCollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

protected:
	//Spawned object
	UPROPERTY(Category = "Spawn Properties", BlueprintReadOnly)
	AActor* spawnedObject;

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	
};
