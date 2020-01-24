// Property of Dr. Khan, CSUS and Occupied Software.

#include "LoopingMapTile.h"
#include "LoopingMapManager.h"

// Sets default values
ALoopingMapTile::ALoopingMapTile(){
	//mainRoadMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoadMeshComponent"));
	//this->SetRootComponent(mainRoadMeshComponent);

 	// The main map doesn't need to tick.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ALoopingMapTile::BeginPlay(){
	Super::BeginPlay();
	
}

// Called every frame
void ALoopingMapTile::Tick(float DeltaTime){
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ALoopingMapTile::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ALoopingMapTile::SetLoopingMapManager(ALoopingMapManager* manager) {
	tileManager = manager;
}

void ALoopingMapTile::SetIgnoreOverlapBegins(bool ignore) {
	bIgnoreOverlapBegins = ignore;
}

void ALoopingMapTile::SetPawnOverlapping(bool overlapping) {
	bIsPawnOverlapping = overlapping;
}

void ALoopingMapTile::IncrementPawnOverlapCount() {
	pawnComponentOverlapCount++;
}


void ALoopingMapTile::DecrementPawnoverlapCount() {
	pawnComponentOverlapCount--;

	if (pawnComponentOverlapCount < 0) {
		pawnComponentOverlapCount = 0;
	}
}

void ALoopingMapTile::OnPawnOverlapBegin(bool forwardOverlap) {
	IncrementPawnOverlapCount();

	if (!bIgnoreOverlapBegins) {
		//Check if this is the first overlap event for the pawn
		if (pawnComponentOverlapCount == 1) {
			if (forwardOverlap) {
				tileManager->spawnTileForward(this);
			} else {
				tileManager->spawnTileBackward(this);
			}
		}
	}
}

void ALoopingMapTile::OnPawnOverlapEnd(bool forwardOverlap) {
	DecrementPawnoverlapCount();

	if (pawnComponentOverlapCount == 0) {
		bIgnoreOverlapBegins = false;
	}
}


void ALoopingMapTile::OnPawnOverlapInside() {
	if (tileManager != nullptr) {
		tileManager->tileEntered(this);
	}
}