// Property of Dr. Khan, CSUS and Occupied Software.

#include "LoopingMapManager.h"
#include "Engine.h"


// Sets default values
ALoopingMapManager::ALoopingMapManager(){
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALoopingMapManager::BeginPlay(){
	Super::BeginPlay();

	//Spawn the first map tile
	FActorSpawnParameters newTileSpawnParameters;
	newTileSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ALoopingMapTile* newTile = GetWorld()->SpawnActor<ALoopingMapTile>(loopingMapTileClass, GetActorLocation(), FRotator::ZeroRotator, newTileSpawnParameters);
	newTile->SetLoopingMapManager(this);
	mapTiles.Add(newTile);
}

//Does first frame int, for things that cannot be done on BeginPlay.
void ALoopingMapManager::FirstFrameInit() {
	int spawnedTiles = mapTiles.Num();

	//Spawn forward and backward tiles until we have spawned all the tiles needed.
	//First, back up the current value for whether or not to delete old tiles and then set to false.
	bool bDeleteOldTilesBackup = bDeleteOldTiles;
	bDeleteOldTiles = false;
	while (spawnedTiles < activeTileCount) {
		spawnTileForward(nullptr);
		spawnTileBackward(nullptr);
		spawnedTiles++;
		spawnedTiles++;
	}

	//Restore the original DeleteOldTiles flag
	bDeleteOldTiles = bDeleteOldTilesBackup;

	bFirstFrameInitPerformed = true;
}

// Called every frame
void ALoopingMapManager::Tick(float DeltaTime){
	Super::Tick(DeltaTime);

	if (!bFirstFrameInitPerformed) {
		FirstFrameInit();
	}
}

void ALoopingMapManager::spawnTileForward(ALoopingMapTile* triggeringTile) {
	//Get the tile at the front of the map.
	ALoopingMapTile* forwardTile = mapTiles.Last();

	//Get bounds of the forward tile
	FVector forwardTileOrigin;
	FVector forwardTileBoxExtent;
	forwardTile->GetActorBounds(false, forwardTileOrigin, forwardTileBoxExtent);

	//Spawn the new tile
	FActorSpawnParameters newTileSpawnParameters;
	newTileSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ALoopingMapTile* newTile = GetWorld()->SpawnActor<ALoopingMapTile>(loopingMapTileClass, forwardTile->GetActorLocation() + FVector(forwardTileBoxExtent.X * 2, 0, 0), FRotator::ZeroRotator, newTileSpawnParameters);
	newTile->SetLoopingMapManager(this);
	newTile->SetIgnoreOverlapBegins(true);
	mapTiles.Add(newTile);

	//If the flag to delete old tiles is set, delete the tile in the back of the map
	if (bDeleteOldTiles) {
		//Get a reference to the current tile
		ALoopingMapTile* toDelete = mapTiles[0];
		//Remove it from the array
		mapTiles.Remove(toDelete);
		//Destroy it.
		toDelete->Destroy();
	}
}

void ALoopingMapManager::spawnTileBackward(ALoopingMapTile* triggeringTile) {
	//Get the tile at the back of the map.
	ALoopingMapTile* backTile = mapTiles[0];

	//Get bounds of the forward tile
	FVector backTileOrigin;
	FVector backTileBoxExtent;
	backTile->GetActorBounds(true, backTileOrigin, backTileBoxExtent);

	//Spawn the new tile
	FActorSpawnParameters newTileSpawnParameters;
	newTileSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ALoopingMapTile* newTile = GetWorld()->SpawnActor<ALoopingMapTile>(loopingMapTileClass, backTile->GetActorLocation() - FVector(backTileBoxExtent.X * 2, 0, 0), FRotator::ZeroRotator, newTileSpawnParameters);
	newTile->SetLoopingMapManager(this);
	newTile->SetIgnoreOverlapBegins(true);
	mapTiles.Insert(newTile, 0);

	//If the flag to delete old tiles is set, delete the tile in the back of the map
	if (bDeleteOldTiles) {
		//Get a reference to the current tile
		ALoopingMapTile* toDelete = mapTiles.Last();
		//Remove it from the array
		mapTiles.RemoveSingle(toDelete);
		//Destroy it.
		toDelete->Destroy();
	}
}

void ALoopingMapManager::tileEntered(ALoopingMapTile* triggeringTile) {
	int32 tileIndex = 0;

	if ((triggeringTile != nullptr) && triggeringTile->IsValidLowLevelFast() && mapTiles.Find(triggeringTile, tileIndex) && (tileIndex > 0) && (tileIndex < mapTiles.Num())) {
		mapTiles[tileIndex - 1]->SetIgnoreOverlapBegins(true);
		mapTiles[tileIndex + 1]->SetIgnoreOverlapBegins(true);
	}
}
