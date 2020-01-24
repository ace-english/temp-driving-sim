#pragma once

#include "DataSingleton.h"
#include "DataSingletonLibrary.generated.h"

UCLASS()
class TRAFFICSIM_API UDataSingletonLibrary : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UDataSingletonLibrary(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Data Singleton Blueprint Access")
	static UDataSingleton* GetDataSingleton(bool& IsValid);
};