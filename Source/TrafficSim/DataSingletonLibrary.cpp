#include "DataSingletonLibrary.h"
#include "TrafficSim.h"

UDataSingletonLibrary::UDataSingletonLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	//Needed for compile
}

UDataSingleton* UDataSingletonLibrary::GetDataSingleton(bool& IsValid) {
	IsValid = false;
	UDataSingleton* instance = Cast<UDataSingleton>(GEngine->GameSingleton);

	if (instance == nullptr || !instance->IsValidLowLevel()) {
		instance = nullptr;
	} else {
		IsValid = true;
	}

	return instance;
}