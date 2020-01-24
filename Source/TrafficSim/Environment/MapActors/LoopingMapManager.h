// Property of Dr. Khan, CSUS and Occupied Software.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LoopingMapTile.h"
#include "LoopingMapManager.generated.h"

UCLASS()
class TRAFFICSIM_API ALoopingMapManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALoopingMapManager();

protected:
	//Whether or not old tiles should be deleted when new ones are spawned.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Looping Map")
	bool bDeleteOldTiles = true;

	//Whether or not first frame init has completed
	bool bFirstFrameInitPerformed = false;

	//The class of map tiles to create
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Looping Map")
	TSubclassOf<ALoopingMapTile> loopingMapTileClass;

	//The currently instantiated map tiles. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Looping Map")
	TArray<ALoopingMapTile*> mapTiles;
	
	//Number of active tiles at once. Recommended to keep this value odd, 3 or greater.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Looping Map")
	int32 activeTileCount = 15;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Does first frame int, for things that cannot be done on BeginPlay.
	void FirstFrameInit();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void spawnTileForward(ALoopingMapTile* triggeringTile);

	UFUNCTION(BlueprintCallable)
	void spawnTileBackward(ALoopingMapTile* triggeringTile);
	
	void tileEntered(ALoopingMapTile* triggeringTile);
};
