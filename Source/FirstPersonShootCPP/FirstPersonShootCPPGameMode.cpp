// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FirstPersonShootCPPGameMode.h"
#include "FirstPersonShootCPPHUD.h"
#include "FirstPersonShootCPPCharacter.h"
#include "FirstPersonShootCPPProjectile.h"
#include "UObject/ConstructorHelpers.h"
#include "MathUtil.h"

//coreDS Unreal
#include "coreDSSettingsClass.h"
#include "coreDSEngine.h"
#include "coreDS_BPCoordinateConversion.h"

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

	//This needs to me mapped to articalted parts 
	lSettings->SupportedInputObjects.AddUnique("Gun.Parts");

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

	Engine = GetGameInstance()->GetSubsystem<UcoreDSEngine>();
		
#if WITH_EDITOR
	//Add a callback to be aware when a coreDS based entities is being deleted by Unreal
	OnLevelActorDeletedHandle = GEngine->OnLevelActorDeleted().AddUObject(this, &AFirstPersonShootCPPGameMode::objectDeletedFromLevel);
	//create our delegates that will handle received informations from the distributed simulation backend
#endif
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
	Engine->registerErrorReceivedHandler(lErrorHandler);

	///Register a callback to an update object event
	Engine->registerObjectUpdateHandler("Gun", lObjectUpdateHandlerForGuns);
	Engine->registerObjectUpdateHandler("Bullet", lObjectUpdateHandlerForBullets);

	///Register a callback to an object removed
	Engine->registerObjectRemovedHandler("Gun", lObjectRemovedHandler);
	Engine->registerObjectRemovedHandler("Bullet", lObjectRemovedHandler);

	///Register a callback to a message is received
	Engine->registerMessageUpdateHandler("ShotFired", lMessageUpdateHandlerWeaponFire);
}

void AFirstPersonShootCPPGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Call the base class  
	Super::EndPlay(EndPlayReason);

	// coreDS Unreal
	Engine->disconnect();
}

void AFirstPersonShootCPPGameMode::printErrorDelegate(FString Message, int Errorcode)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Message);
	UE_LOG(LogClass, Log, TEXT("coreDS: %s"), *Message);
}

// Values are received in a list of value-pair. Each pair consists of the value name, as defined in the mapping and the value as a string
// ObjectName is the unique object identifier
void  AFirstPersonShootCPPGameMode::gunUpdated(FCoreDSVariant Values, FString ObjectName)
{
	spawnActorBasedOntype(DefaultPawnClass, Values, ObjectName);
}

void  AFirstPersonShootCPPGameMode::bulletUpdated(FCoreDSVariant Values, FString ObjectName)
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

void  AFirstPersonShootCPPGameMode::spawnActorBasedOntype(TSubclassOf<AActor> ActorType, FCoreDSVariant Values, FString ObjectName)
{
	//check if we have all valid values
	if (!Values.exists("Location.x") || !Values.exists("Location.y") || !Values.exists("Location.z"))
	{
		printErrorDelegate("Missing values when receiving data - please check your mapping", 0);
		return;
	}

	FRotator lRot(0,0,0);
	if (Values.exists("Orientation.pitch") && Values.exists("Orientation.yaw") && Values.exists("Orientation.roll"))
	{
		lRot = FRotator(Values["Orientation.pitch"].toDouble(), Values["Orientation.yaw"].toDouble(), Values["Orientation.roll"].toDouble());
	}

	//UE_LOG(LogClass, Log, TEXT("coreDS: Received position %g, %g, %g"), Values["Location.x"].toFloat(), Values["Location.y"].toFloat(), Values["Location.z"].toFloat());

	double xEnu = 0, yEnu = 0, zEnu = 0;
	double pitch = 0, roll = 0, yaw = 0;
	UcoreDSSettings* lSettings = const_cast<UcoreDSSettings*>(GetDefault<UcoreDSSettings>());

	UCoreDSCoordinateConversion::EcefToEnu(Values["Location.x"].toDouble(), Values["Location.y"].toDouble(), Values["Location.z"].toDouble(),
		lSettings->ReferenceLatitude, lSettings->ReferenceLongitude, lSettings->ReferenceAltitude,
		xEnu, yEnu, zEnu);

	xEnu = xEnu / 100;
	yEnu = yEnu / 100;
	zEnu = zEnu / 100;

	UCoreDSCoordinateConversion::EulerToHeadingPitchRoll(
		lSettings->ReferenceLatitude * FMathd::DegToRad,
		lSettings->ReferenceLongitude * FMathd::DegToRad,
		Values["Orientation.pitch"].toDouble(), Values["Orientation.roll"].toDouble(), Values["Orientation.yaw"].toDouble(),
		yaw, pitch, roll);

	roll = roll * FMathd::RadToDeg;
	pitch = pitch * FMathd::RadToDeg;
	yaw = yaw * FMathd::RadToDeg;

	lRot = FRotator(pitch, yaw, roll);
	FVector lNewLocation(xEnu, yEnu, zEnu);

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

			if (IsValid(lActor))
			{
#if WITH_EDITOR
				FString lFinaleName = ObjectName;
				lFinaleName.Append(" (coreDS)");
				lActor->SetActorLabel(lFinaleName);
#endif
			}
			else
			{
				printErrorDelegate("Could not create actor", 0);
				return;
			}
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
		else
		{
			return;
		}
	}

	//from here, we have a valid actor
	if (Values.exists("Parts"))
	{
		unsigned int lNumberOfPats = Values["Parts"].size();

		if (lNumberOfPats > 0)
		{
			for (size_t i = 0; i < lNumberOfPats; i++)
			{
				FCoreDSVariant lSinglePart = Values["Parts"][i];

				//extract the values
				lSinglePart["ParameterValue.Class"].toUInt8();
				//For example, 3072 if for the Landing Gear

				lSinglePart["ParameterValue.TypeMetric"].toUInt8();
				/*0		Not Specified
				1		Position
				2		Position Rate
				3		Extension
				4		Extension Rate
				5		X
				6		X Rate
				7		Y
				8		Y Rate
				9		Z
				10		Z Rate
				11		Azimuth
				12		Azimuth Rate
				13		Elevation
				14		Elevation Rate
				15		Rotation
				16		Rotation Rate*/


				//Actual value
				lSinglePart["ParameterValue.Value"].toFloat();

				//use them to set your actor's part
				FString Message = FString::Printf(TEXT("coreDS: Received part Class: %i, Metric: %i, Value: %g"), lSinglePart["ParameterValue.Class"].toUInt16(),
					lSinglePart["ParameterValue.TypeMetric"].toUInt16(), lSinglePart["ParameterValue.Value"].toFloat());

				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Message);
			}
		}
		//make sure we have valid parts
		//Extract the value for each parts
	}
}

//Play a sound when the ShotFired message is received
void  AFirstPersonShootCPPGameMode::shotFiredMessageReceived(FCoreDSVariant Values)
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
		if (lActor && lActor->IsValidLowLevel() || !lActor->IsActorBeingDestroyed() || !lActor->IsPendingKillPending())
		{
			lActor->MarkAsGarbage();
		}

		mDiscoveredObject.Remove(ObjectName);
		mDiscoveredObjectRev.Remove(lActor);
	}
}

void  AFirstPersonShootCPPGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Engine->step();
}