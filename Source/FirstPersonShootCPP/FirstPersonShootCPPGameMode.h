// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

//coreDS Unreal
#include "coreDSBluePrintBPLibrary.h"
#include "coreDS_BPVariant.h"

#include "FirstPersonShootCPPGameMode.generated.h"

UCLASS(minimalapi)
class AFirstPersonShootCPPGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFirstPersonShootCPPGameMode();

	//Added for coreDS Unreal
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	//Callback when an object is deleted from the scene by Unreal
	void objectDeletedFromLevel(AActor* DeletedActor);
private:

	//List of discovered entities
	TMap<FString, AActor*> mDiscoveredObject;
	TMap<AActor*, FString> mDiscoveredObjectRev;
	FCriticalSection mDiscoveredObjectMutex;

	UFUNCTION()
		void printErrorDelegate(FString Message, int Errorcode);

	UFUNCTION()
		void gunUpdated(FCoreDSVariant Values, FString ObjectName);

	UFUNCTION()
		void bulletUpdated(FCoreDSVariant Values, FString ObjectName);

	UFUNCTION()
		void shotFiredMessageReceived(FCoreDSVariant Values);

	UFUNCTION()
		void objectRemoved(FString ObjectName);

	//Helper function to spawn objects
	void  spawnActorBasedOntype(TSubclassOf<AActor> ActorType, FCoreDSVariant Values, FString ObjectName);
	FDelegateHandle OnLevelActorDeletedHandle;

	UcoreDSEngine *Engine;
};



