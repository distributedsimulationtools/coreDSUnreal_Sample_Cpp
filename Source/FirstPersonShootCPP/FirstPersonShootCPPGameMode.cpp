// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FirstPersonShootCPPGameMode.h"
#include "FirstPersonShootCPPHUD.h"
#include "FirstPersonShootCPPCharacter.h"
#include "FirstPersonShootCPPProjectile.h"

#include "UObject/ConstructorHelpers.h"

//coreDS Unreal
#include "coreDSSettingsClass.h"
#include "coreDSEngine.h"
#include "coreDSBluePrintBPLibrary.h"

AFirstPersonShootCPPGameMode::AFirstPersonShootCPPGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFirstPersonShootCPPHUD::StaticClass();

	// coreDS Unreal
	// create our configuration settings. This could be done through the Editor but in case you want to 
	// manage the configure, you can do it here.
	UcoreDSSettings* lSettings = const_cast<UcoreDSSettings*>(GetDefault<UcoreDSSettings>());

	//this will overwrite the values - be careful if you are using the Editor
	lSettings->ReferenceLatitude = 46.8298531;
	lSettings->ReferenceLongitude = -71.2540283;
	lSettings->ReferenceAltitude = 5.0;

	//Object your simulator can accept 
	lSettings->SupportedOutputObjects.AddUnique("Gun.Location.x");
	lSettings->SupportedOutputObjects.AddUnique("Gun.Location.y");
	lSettings->SupportedOutputObjects.AddUnique("Gun.Location.z");
	lSettings->SupportedOutputObjects.AddUnique("Gun.Orientation.pitch");
	lSettings->SupportedOutputObjects.AddUnique("Gun.Orientation.yaw");
	lSettings->SupportedOutputObjects.AddUnique("Gun.Orientation.roll");
	lSettings->SupportedOutputObjects.AddUnique("Bullet.Location.x");
	lSettings->SupportedOutputObjects.AddUnique("Bullet.Location.y");
	lSettings->SupportedOutputObjects.AddUnique("Bullet.Location.z");

	//Message your simulator can accept
	lSettings->SupportedOutputMessages.AddUnique("ShotFired.Location.x");
	lSettings->SupportedOutputMessages.AddUnique("ShotFired.Location.y");
	lSettings->SupportedOutputMessages.AddUnique("ShotFired.Location.z");

	//Object your simulator can send 
	lSettings->SupportedInputObjects.AddUnique("Gun.Location.x");
	lSettings->SupportedInputObjects.AddUnique("Gun.Location.y");
	lSettings->SupportedInputObjects.AddUnique("Gun.Location.z");
	lSettings->SupportedInputObjects.AddUnique("Gun.Orientation.pitch");
	lSettings->SupportedInputObjects.AddUnique("Gun.Orientation.yaw");
	lSettings->SupportedInputObjects.AddUnique("Gun.Orientation.roll");
	lSettings->SupportedInputObjects.AddUnique("Bullet.Location.x");
	lSettings->SupportedInputObjects.AddUnique("Bullet.Location.y");
	lSettings->SupportedInputObjects.AddUnique("Bullet.Location.z");

	//Message your simulator can send
	lSettings->SupportedInputMessages.AddUnique("ShotFired.Location.x");
	lSettings->SupportedInputMessages.AddUnique("ShotFired.Location.y");
	lSettings->SupportedInputMessages.AddUnique("ShotFired.Location.z");

	lSettings->SaveConfig();

	// enable tick
	PrimaryActorTick.bCanEverTick = true;

}

void AFirstPersonShootCPPGameMode::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add a callback to be aware when a coreDS based entities is being deleted by Unreal
	//GEngine->OnLevelActorDeleted().Add(&AFirstPersonShootCPPGameMode::objectDeletedFromLevel);
	//GEngine->OnLevelActorDeleted().Add(this, &AFirstPersonShootCPPGameMode::objectDeletedFromLevel);
	OnLevelActorDeletedHandle = GEngine->OnLevelActorDeleted().AddUObject(this, &AFirstPersonShootCPPGameMode::objectDeletedFromLevel);

	// coreDS Unreal
	UcoreDSBluePrintBPLibrary::connect();
	
	//create our delegates that will handle received informations from the distributed simulation backend

	//Error handler
	FErrorReceivedHandler lErrorHandler;
	lErrorHandler.BindUFunction(this, "printErrorDelegate");

	//New gun position received
	FObjectUpdateHandler lObjectUpdateHandlerForGuns;
	lObjectUpdateHandlerForGuns.BindUFunction(this, "gunUpdated");

	//New bullet position received
	FObjectUpdateHandler lObjectUpdateHandlerForBullets;
	lObjectUpdateHandlerForBullets.BindUFunction(this, "bulletUpdated");

	// An object has been removed
	FObjectRemovedHandler lObjectRemovedHandler;
	lObjectRemovedHandler.BindUFunction(this, "objectRemoved");
	
	// WeaponFire message received
	FMessageUpdateHandler lMessageUpdateHandlerWeaponFire;
	lMessageUpdateHandlerWeaponFire.BindUFunction(this, "shotFiredMessageReceived");


	//register the required callbacks to received information for HLA or DIS
	///Register a callback to report an error
	UcoreDSEngine::registerErrorReceivedHandler(lErrorHandler);

	///Register a callback to an update object event
	UcoreDSEngine::registerObjectUpdateHandler("Gun", lObjectUpdateHandlerForGuns);
	UcoreDSEngine::registerObjectUpdateHandler("Bullet", lObjectUpdateHandlerForBullets);

	///Register a callback to an object removed
	UcoreDSEngine::registerObjectRemovedHandler("Gun", lObjectRemovedHandler);
	UcoreDSEngine::registerObjectRemovedHandler("Bullet", lObjectRemovedHandler);

	///Register a callback to a message is received
	UcoreDSEngine::registerMessageUpdateHandler("ShotFired", lMessageUpdateHandlerWeaponFire);
}

void AFirstPersonShootCPPGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Call the base class  
	Super::EndPlay(EndPlayReason);

	// coreDS Unreal
	UcoreDSBluePrintBPLibrary::disconnect();
}

void AFirstPersonShootCPPGameMode::printErrorDelegate(FString Message, int Errorcode)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Message);
	UE_LOG(LogClass, Log, TEXT("coreDS: %s"), *Message);
}

// Values are received in a list of value-pair. Each pair consists of the value name, as defined in the mapping and the value as a string
// ObjectName is the unique object identifier
void  AFirstPersonShootCPPGameMode::gunUpdated(const  TArray< FPairValue > &Values, FString ObjectName)
{
	spawnActorBasedOntype(DefaultPawnClass, Values, ObjectName);
}

void  AFirstPersonShootCPPGameMode::bulletUpdated(const  TArray< FPairValue > &Values, FString ObjectName)
{
	spawnActorBasedOntype(Cast<AFirstPersonShootCPPCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn())->ProjectileClass, Values, ObjectName);
}

void AFirstPersonShootCPPGameMode::objectDeletedFromLevel(AActor* DeletedActor)
{
	FScopeLock lock(&mDiscoveredObjectMutex);

	if (mDiscoveredObjectRev.Contains(DeletedActor))
	{
		mDiscoveredObject.Remove(mDiscoveredObjectRev[DeletedActor]);
		mDiscoveredObjectRev.Remove(DeletedActor);
	}
}

void  AFirstPersonShootCPPGameMode::spawnActorBasedOntype(TSubclassOf<AActor> ActorType, const TArray< FPairValue > &Values, FString ObjectName)
{
	//extract the values
	const FPairValue *lX = Values.FindByKey("Location.x");
	const FPairValue *lY = Values.FindByKey("Location.y");
	const FPairValue *lZ = Values.FindByKey("Location.z");

	const FPairValue *lPitch = Values.FindByKey("Orientation.pitch");
	const FPairValue *lYaw = Values.FindByKey("Orientation.yaw");
	const FPairValue *lRoll = Values.FindByKey("Orientation.roll");

	//check if we have all valid values
	if (!(lX && lY && lZ))
	{
		printErrorDelegate("Missing values when receiving data - please check your mapping", 0);
		return;
	}
	FVector lNewLocation(FCString::Atof(*lX->Value), FCString::Atof(*lY->Value), FCString::Atof(*lZ->Value));

	FRotator lRot(0,0,0);
	if (lPitch && lYaw && lRoll)
	{
		lRot = FRotator(FCString::Atof(*lPitch->Value), FCString::Atof(*lYaw->Value), FCString::Atof(*lRoll->Value));
	}

	FTransform lTransform(lRot, lNewLocation);

	AActor *lActor = NULL;
	
	FScopeLock lock(&mDiscoveredObjectMutex);

	if (!mDiscoveredObject.Contains(ObjectName))
	{
		//not yet discovered
		lActor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ActorType, lTransform));
		if (lActor != nullptr)
		{
			//we need to add the tag before the object is created to prevent a ping-pong effect
			lActor->Tags.Add("coreDSCreated");

			if (IsValid(lActor))
			{
				lActor->SetActorEnableCollision(false);
				lActor->DisableComponentsSimulatePhysics();

				//check if the object was already discovered
				if (!mDiscoveredObject.Contains(ObjectName))
				{
					//spawn the object
					mDiscoveredObject.Emplace(ObjectName, lActor);
					mDiscoveredObjectRev.Emplace(lActor, ObjectName);
				}
			}

			UGameplayStatics::FinishSpawningActor(lActor, lTransform);
		}
	}
	else
	{
		//update the position
		lActor = mDiscoveredObject[ObjectName];

		if (IsValid(lActor))
		{
			//make sure the object is within the scene, otherwise it will get destroyed
			lActor->SetActorLocationAndRotation(lNewLocation, lRot, false, nullptr, ETeleportType::ResetPhysics);
		}
	}
}

//Play a sound when the ShotFired message is received
void  AFirstPersonShootCPPGameMode::shotFiredMessageReceived(const  TArray< FPairValue > &Values)
{
	//find the Character instance so we can play the fire sound
	AFirstPersonShootCPPCharacter* myCharacter = Cast<AFirstPersonShootCPPCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	// try and play the sound if specified
	if (myCharacter != NULL && myCharacter->FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, myCharacter->FireSound, myCharacter->GetActorLocation());
	}
}

void  AFirstPersonShootCPPGameMode::objectRemoved(FString ObjectName)
{
	FScopeLock lock(&mDiscoveredObjectMutex);

	if (mDiscoveredObject.Contains(ObjectName))
	{
		AActor *lActor = mDiscoveredObject[ObjectName];

		//remove the object from the scene
		if (lActor && lActor->IsValidLowLevel() || !lActor->IsActorBeingDestroyed() || !lActor->IsPendingKill() || !lActor->IsPendingKillOrUnreachable()
			|| !lActor->IsPendingKillPending())
		{
			lActor->MarkPendingKill();
		}

		mDiscoveredObject.Remove(ObjectName);
		mDiscoveredObjectRev.Remove(lActor);
	}
}

void  AFirstPersonShootCPPGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UcoreDSBluePrintBPLibrary::step();
}