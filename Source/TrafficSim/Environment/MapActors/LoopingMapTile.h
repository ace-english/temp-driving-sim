// Property of Dr. Khan, CSUS and Occupied Software.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LoopingMapTile.generated.h"

//Declare Manager class here instead of including header to avoid recursive inclusion.
class ALoopingMapManager;

UCLASS()
class TRAFFICSIM_API ALoopingMapTile : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ALoopingMapTile();

protected:
	//The manager for this tile
	UPROPERTY(BlueprintReadOnly, Category = "Map Manager")
	ALoopingMapManager* tileManager;

	//Whether to ignore overlap begin events. Used to prevent a vehicle from triggering an event entering a tile's back overlap immediately following the prior tile's enter overlap.
	UPROPERTY(BlueprintReadOnly, Category = "Map Overlaps")
	bool bIgnoreOverlapBegins = false;

	//Whether the player pawn is currently overlapping with one of this tile's triggers
	UPROPERTY(BlueprintReadOnly, Category = "Map Overlaps")
	bool bIsPawnOverlapping = false;

	UPROPERTY(BlueprintReadOnly, Category = "Map Overlaps")
	int32 pawnComponentOverlapCount = 0;

	//The mesh for the main road component
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Mesh")
	//UStaticMeshComponent* mainRoadMeshComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetLoopingMapManager(ALoopingMapManager* manager);
	
	UFUNCTION(BlueprintCallable)
	void SetIgnoreOverlapBegins(bool ignore);

	UFUNCTION(BlueprintCallable)
	void SetPawnOverlapping(bool overlapping);

	UFUNCTION(BlueprintCallable)
	void IncrementPawnOverlapCount();

	UFUNCTION(BlueprintCallable)
	void DecrementPawnoverlapCount();

	UFUNCTION(BlueprintCallable)
	void OnPawnOverlapBegin(bool forwardOverlap);

	UFUNCTION(BlueprintCallable)
	void OnPawnOverlapEnd(bool forwardOverlap);

	UFUNCTION(BlueprintCallable)
	void OnPawnOverlapInside();
};
