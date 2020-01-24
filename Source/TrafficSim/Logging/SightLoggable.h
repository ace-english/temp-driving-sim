// Property of Dr. Khan, CSUS and Occupied Software.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SightLoggable.generated.h"

//Unreal Object interface for SightLoggable, needed for internal reference by Unreal.
UINTERFACE(BlueprintType)
class TRAFFICSIM_API USightLoggable : public UInterface {
	GENERATED_UINTERFACE_BODY()
};

//Actual Sight Loggable Interface
class TRAFFICSIM_API ISightLoggable {
	GENERATED_IINTERFACE_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Logging")
	int IsLookedAt(AActor* observer);
};

