// Property of Dr. Khan, CSUS and Occupied Software.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Environment/TimeManager.h"
#include "Logging/DataLogger.h"
#include "DataSingleton.generated.h"


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TRAFFICSIM_API UDataSingleton : public UObject
{
	GENERATED_BODY()
protected:
	//UPROPERTY macro is needed to prevent Unreal from garbage collecting the time manager.
	UPROPERTY()
	UTimeManager* simTimeManager = nullptr;

	UPROPERTY()
	UDataLogger* simDataLogger = nullptr;

public:
	UDataSingleton(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable)
	UTimeManager* GetTimeManager();

	UFUNCTION(BlueprintCallable)
	UDataLogger* GetDataLogger();

	UFUNCTION(BlueprintCallable)
	void ShowSaveDialog();

	UFUNCTION(BlueprintCallable)
	FString GetCurrentSaveDirectory();
};
