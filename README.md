# coreUnrealCPP

This is a Sample project to be used with [coreDS Unreal](https://www.ds.tools/products/hla-dis-unreal-engine-4/) and the [Unreal Engine 4 and 5](https://www.unrealengine.com). You can request a free trial on https://www.ds.tools/contact-us/trial-request/

coreDS Unreal must already be installed and activated in order to use this project. The project is compatible with the Unreal engine 4.24+ and 5.0+. Please make sure the coreDS Unreal plugin is enabled.

This sample focuses on using the C++ instead of the BluePrint visual programming language. No matter what you will need the UE4 Editor at some point to manage the configuration file. The base project is the FirstPersonShooter sample, C++ mode, provided by Epic Games.

The Sample project uses the following coreDS concepts:
* Connect
* Disconnect
* Send Update Object (send EntityStatePDU or UpdateAttributeValues, DiscoverObjectInstance)
* Send Message (send a MunitionDetonationPDU or SendInteraction)
* Receive Update Object (Receive EntityStatePDU or UpdateAttributeValues)
* Receive Message (Receive a MunitionDetonationPDU or SendInteraction)
* Delete Objects (EntityStatePDU timeout or RemoveObjectInstance)

## Getting started
First, you need to add the required modules to your application. This is done in the FirstPersonShootCPP.build.cs and adding coreDSActorComponentModule and coreDSSettings to the PublicDependencyModuleNames array. 

<code>
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "coreDSActorComponentModule", "coreDSSettings" });
</code>

You must also add the "coreDSModule" to your .uproject file.

Most functions are included in these header files
<code>
#include "coreDSSettingsClass.h"
#include "coreDSBluePrintBPLibrary.h"
#include "coreDSEngine.h"
</code>

The first step is to configure coreDS Unreal to know which objects, object attributes, messages and message parameters your simulator will support. Keep in mind that the names you define are not related to the distributed simulation protocol you plan on using. These names will only be used internally when using BluePrint or the Mapping interface.

Although you can place the code pretty much where you want, for the sake of simplicity, we added it to the GameModeConstructor (FirstPersonShootCppGameMode.cpp)

<code>
	//coreDS Unreal
	//create our configuration settings. This could be done through the Editor but in case you want to 
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
</code>

In this particular case, we support sending/receiving a GUN object with Location and Orientation property and a BULLET object with a location. The demo also supports a GunFired message, with a Location property. 

The format for Object/Message names is NAME.PROPERTY. The Object/Message name is always the part before the first dot. 

Next step is to configure an actual connection with a Distributed Simulation protocol. coreDS supports HLA (High-Level Architecture and DIS (Distributed Interaction Simulation). You can do it either from the UnrealEngine editor or at runtime, from your app, by calling

<code>
UcoreDSBluePrintBPLibrary::connect();
</code>

You can open the configuration window either by running the game or by click on the coreDS toolbar button.
![Plugin coreDSToolboxMenu Screenshot](/Doc/Images/coreDSToolboxMenu.png)

This sample comes with some preconfigured settings:
* DIS: DIS v6
* HLA_Player1: HLA 1516e with RPRFOM 2.0
* HLA_Player2: HLA 1516e with RPRFOM 2.0

![Plugin ConfigurationSelection Screenshot](/Doc/Images/ConfigurationSelection.png)

Let's take a look at each configuration.

## DIS
For both configurations, it is important to configure the Configured Network Adapter. Click on the dropbox and select your active Network Adapter.

![Plugin DISConnectionConfiguration Screenshot](/Doc/Images/DISConnectionConfiguration.png)

#### Receiving
Even if DIS does not explicitly support Subscription, coreDS supports incoming filtering. First, we must let coreDS know that we want to receive the EntityStatePDU and the FirePDU.

![Plugin DIS_Receiver_PubSub Screenshot](/Doc/Images/DIS_Receiver_PubSub.png)

Then comes the Mapping configuration. Since we are in a receiver, we care about the "Mapping In"

The first step is to Map a Local Object/Message to a Protocol Object/Message. As you see, the Names you defined during the Plugin configuration are listed in the "+" list. You can then link the Local Object/Message to a Protocol Object/Message by using the drop down m next to the Object/Message name.

![Plugin DISMappingIn_ObjectMapping Screenshot](/Doc/Images/DISMappingIn_ObjectMapping.png)

Then you must map the local properties to the protocol properties. Since we are in a receiving mode, we only care about the values we are interested in. In our case, we want to send back to Unreal the Location and the Orientation.

![Plugin DISMappingIn_WithChoice Screenshot](/Doc/Images/DISMappingIn_WithChoice.png)

Finally, we are receiving coordinates in Geocentric format, which Unreal doesn't like. We could convert the coordinates from within Unreal but by doing so, it will be harder to switch to a different Distributed Simulation Protocol. To keep all the configuration available at runtime, we use an embedded Lua scripting engine to convert from Geocentric to flat coordinates centered around a given Lat/Long.

![Plugin DISMappingIn Screenshot](/Doc/Images/DISMappingIn.png)

Below is the script that converts from Geocentric to local coordinates. Scripts are located in /Config/Scripts

```lua
require("lla2ecef")  -- include functions to convert from Lat/Log to geocentric
require("ReferenceLatLongAlt") -- Include the Center Lat/Long

lastPositionX = 0 -- Last received position, used by the Orientation conversion script
lastPositionY = 0
lastPositionZ = 0

function convertPositionFromDIS()  --same function name as the filename
    lastPositionX = DSimLocal.X
    lastPositionY = DSimLocal.Y
    lastPositionZ = DSimLocal.Z

    -- Since we are working over a fairly small part of the planet, we can assume a flat surface
    --convert lat/long to geocentric
    tempx, tempy, tempz = lla2ecef(referenceOffset_Lat , referenceOffset_Long , referenceOffset_Alt )

    DSimLocal.X = (tempx - DSimLocal.X)  -- offset to the local coordinates
    DSimLocal.Y = (tempy - DSimLocal.Y)
    DSimLocal.Z = (tempz - DSimLocal.Z)
end
```

You might have noticed that nowhere during the configuration process with actually used the Entity Kind to distinguish between a GUN and a BULLET. coreDS supports complexe filtering by leveraging on the LUA scripting engine. In order to filter the GUNs, we've created a simple script, FilterGun.lua, which will check then EntityKind and instruct coreDS to discard the mapping if this is not the correct type.

![Plugin DIS_MappingIn Filter Screenshot](/Doc/Images/DISMappingIn_Filter.png)

```lua
function FilterGun()
    -- Available variables
    -- DSimLocal.Category
    -- DSimLocal.CountryCode
    -- DSimLocal.Domain.Category
    -- DSimLocal.Domain.CountryCode
    -- DSimLocal.Domain.DomainDiscriminant
    -- DSimLocal.EntityKind
    -- DSimLocal.Extra
    -- DSimLocal.On Data Received
    -- DSimLocal.Specific
    -- DSimLocal.Subcategory
    
  if((DSimLocal.EntityKind == "3") ~= true) then
		DeleteValues = 1;
	end
end

```

#### Sending
Then, we must let coreDS know that we want to send the EntityStatePDU and the FirePDU.

![Plugin DIS_Sender_PubSub Screenshot](/Doc/Images/DIS_Sender_PubSub.png)

Then comes the Mapping configuration. Since we are in a Sender, we care about the "Mapping Out"

The first step is to Map a Local Object/Message to a Protocol Object/Message. As you see, the Names you defined during the Plugin configuration are listed in the "+" list. You can then link the Local Object/Message to a Protocol Object/Message by using the dropbox next to the Object/Message name.

Then you must map the local properties to the protocol properties. Since we are in a Sender mode, we must fill the complete structure. Static values can be set at this point. We will map Location and Orientation to local properties.

Finally, we are sending coordinates in local format, which DIS doesn't like. We could convert the coordinates from within Unreal but by doing so, it will be harder to swith to a different Distributed Simulation Protocol. To keep all the configuration available at runtime, we use the embededded Lua scripting engine to convert from flat coordinates, centered around a given Lat/Long, to Geocentric.

![Plugin DISMappingOut Screenshot](/Doc/Images/DISMappingOut.png)

As for outgoing values, you must set a conversion script to convert from the local coordinates to geocentric coordinates. Scripts are located in /Config/Scripts. 

## HLA
For both configurations, it is important to have a valid FOM File. The sample provides an RPRFOM 2.0 compliant FOM file, located in /Config. Altought this sample is configured to use an RPRFOM based FOM file, you can load the FOM file of your choice. 

![Plugin HLAConnectionConfiguration Screenshot](/Doc/Images/HLAConnectionConfiguration.png)

coreDS supports a wide range of RTIs, from the legacy DMSO RTI (HLA 1.3) to HLA 1516e compliant RTI like the Pitch RTI or MAK RTI. The complete list of supported RTI can be found here https://www.ds.tools/products/coreds/detailed-features-list/

If you are using the default configuration, you must select a HLA Evolved compliant RTI. 

![Plugin HLAConnectionRTISettings Screenshot](/Doc/Images/HLAConnectionRTISettings.png)

#### Sending
First, we must let coreDS know that we want to receive the Statial attribute from a LifeForm and the WeaponFire interaction.

![Plugin HLA_Receiver_PubSub Screenshot](/Doc/Images/HLA_Receiver_PubSub.png)

Then comes the Mapping configuration. Since we are in a receiver, we care about the "Mapping In"

The first step is to Map a Local Object/Message to a Protocol Object/Message. As you see, the Names you defined during the Plugin configuration are listed in the "+" list. You can then link the Local Object/Message to a Protocol Object/Message by using the dropbox next to the Object/Message name.

Then you must map the local properties to the protocol properties. Since we are in a receiving mode, we only care about the values we are interested in. In our case, we want to send back to Unreal the Location and the Orientation from the Spatial attribute.

Finally, we are receiving coordinates in Geocentric format, which Unreal doesn't like. We could convert the coordinates from within Unreal but by doing so, it would be harder to switch to a different Distributed Simulation Protocol. To keep all the configuration available at runtime, we use the embedded Lua scripting engine to convert from Geocentric to flat coordinates centered around a given Lat/Long.

![Plugin HLAMappingIn_ObjectMapping Screenshot](/Doc/Images/HLAMappingIn_ObjectMapping.png)

Below is the script that convert from Geocentric to local coordinates. Scripts are located in /Config/Scripts


```lua
angleConversions = require("angleConversions")
require("ecef2lla") -- include functions to convert from Lat/Log to geocentric
require("lla2ecef") -- include functions to convert from geocentric to Lat/Log
require("ReferenceLatLongAlt") --Include the center Lat/Long

function convertPositionFromHLA()  --same function name as the filename

--convert orientation
    latTemp, lonTemp, discard = ecef2lla(DSimLocal.WorldLocation.X,DSimLocal.WorldLocation.Y,DSimLocal.WorldLocation.Z)

    local lat = math.rad(latTemp)  --converting to rad because function requires rad
    local lon = math.rad(lonTemp)

    local psi =  DSimLocal.Orientation.Psi-- roll
    local theta = DSimLocal.Orientation.Theta--pitch
    local phi = DSimLocal.Orientation.Phi --yaw

    DSimLocal.Orientation.Psi =  angleConversions.getOrientationFromEuler(lat, lon, psi, theta)
    DSimLocal.Orientation.Theta = angleConversions.getPitchFromEuler(lat, lon, psi, theta)
    DSimLocal.Orientation.Phi = angleConversions.getRollFromEuler(lat, lon, psi, theta, phi)

--- convert position
    -- Since we are working over a fairly small part of the planet, we can assume a flat surface
    --convert lat/long to geocentric
    tempx, tempy, tempz = lla2ecef(referenceOffset_Lat , referenceOffset_Long , referenceOffset_Alt )

    --Offset the coordinate to the local area
    DSimLocal.WorldLocation.X = (DSimLocal.WorldLocation.X - tempx)
    DSimLocal.WorldLocation.Y = (DSimLocal.WorldLocation.Y - tempy)
    DSimLocal.WorldLocation.Z = (DSimLocal.WorldLocation.Z - tempz)
end

```

#### Receiving
First, we must let coreDS know that we want to send the Statial attribute from a LifeForm and the WeaponFire interaction.

![Plugin DIS_Sender_PubSub Screenshot](/Doc/Images/DIS_Sender_PubSub.png)

Then comes the Mapping configuration. Since we are in a Sender, we care about the "Mapping Out"

The first step is to Map a Local Object/Message to a Protocol Object/Message. As you see, the Names you defined during the Plugin configuration are listed in the "+" list. You can then link the Local Object/Message to a Protocol Object/Message by using the dropbox next to the Object/Message name.

Then you must map the local properties to the protocol properties. Since we are in a Sender mode, we must fill the complete structure. Static values can be set at this point. We will map Location and Orientation to local properties.

Finally, we are sending coordinates in local format, which is not compliant with the RPRFOM Spatial attribute format. We could convert the coordinates from within Unreal but by doing so, it will be harder to swith to a different Distributed Simulation Protocol. To keep all the configuration are runtime, we use the embededded Lua scripting engine to convert from flat coordinates, centered around a given Lat/Long, to Geocentric.

![Plugin HLAMappingOut Screenshot](/Doc/Images/HLAMappingOut.png)

As for outgoing values, you must set a conversion script to convert from the local coordinates to geocentric coordinates. Scripts are located in /Config/Scripts.

## Connect
At some point, you must instruct your simulator to connect to the Distributed Simulation system (either HLA or DIS). When using DIS, a UDP socket will be created.  When using HLA, a connection to the RTI will be attempted. If supported by the HLA version you are using, a call to connect() will be made, followed by a call to createFederationExecution (this call can be disabled from the HLA configuration) and joinFederationExecution. Once we have joined the Federation, we then set the various required states like time management, publish/subscribe, etc.

The easiest way to do that is by calling the connection function

```c++
UcoreDSBluePrintBPLibrary::connect();
```

## Disconnect
When disconnecting, a DIS configuration will close the socket. HLA will call resignFederationExecution and destroyFederationExecution.

```c++
UcoreDSBluePrintBPLibrary::disconnect();
```

## Send Update Object (send EntityStatePDU or UpdateAttributeValues, DiscoverObjectInstance)
Sending an Object Update is fairly simple. You first have to build an array of <Name, Value> pair then uses the SendUpdateObject block. All names must match the names configured in the coreDS Unreal plugin.

When using HLA, if this is the first call using that object type, registerObjectInstance will be called, followed by UpdateObjectAttributeValues.

When using DIS, an EntityStatePDU will be sent.

```c++
	TArray< FKeyVariantPair > lValues;

	lValues.Add(FKeyVariantPair("Location.x", ActorLocation.X));
	lValues.Add(FKeyVariantPair("Location.y", ActorLocation.Y));
	lValues.Add(FKeyVariantPair("Location.z", ActorLocation.Z));

	UcoreDSEngine::updateObject("Bullet", lValues);
```

## Send Message (send a MunitionDetonationPDU or SendInteraction)
Sending an message is fairly simple. You first have to build an array of <Name, Value> pair then use the SendMessage block. All names must match the names configured in the coreDS Unreal plugin.

When using HLA, a call to SendInteraction will be made.

When using DIS, a FirePDU will be sent.

```c++
	// coreDS Unreal
	// Send the WeaponFire message
	TArray< FKeyVariantPair > lValues;

	lValues.Add(FKeyVariantPair("Location.x", ActorLocation.X));
	lValues.Add(FKeyVariantPair("Location.y", ActorLocation.Y));
	lValues.Add(FKeyVariantPair("Location.z", ActorLocation.Z));

	UcoreDSEngine::sendMessage("ShotFired", lValues);
```


## Receive Update Object (Receive EntityStatePDU or UpdateAttributeValues)
If you want to receive Object updates, you must first register a ObjectUpdateHandler. The registration must be done using object name used in the coreDS Unreal plugin configuration:

In C++, this is a three step process

1) Declare the delegate and bind the callback function.
   
```c++
FObjectUpdateHandler lObjectUpdateHandlerForGuns;
lObjectUpdateHandlerForGuns.BindUFunction(this, "gunUpdated");
```

2) Register the callback to an update object event
```c++	
UcoreDSEngine::registerObjectUpdateHandler("Gun", lObjectUpdateHandlerForGuns);
```

3) Define and fill the "gunUpdated" function. Values are received in a list of value-pair. Each pair consists of the value name, as defined in the mapping and the value as a string. ObjectName is the unique object identifier

```c++	
void  AFirstPersonShootCPPGameMode::gunUpdated(const TArray< FKeyVariantPair > &Values, FString ObjectName)
{
	...
}
```

Then, each time an object of the register type is received, the GunMoved event will be triggered.

In this particular case, we also added some logic to keep a list of discovered object in Unreal. We first look into a map if the discovered object name already exists in the map. If not, a new Actor is spawned, else the Actor instance is updated. That part will most likely need to be updated based on your particular requirement. 


## Receive Message (Receive a MunitionDetonationPDU or SendInteraction)
If you want to receive a message, you must first register a MessageReceivedHandler. The strategy is the same:

```c++
	// WeaponFire message received
	FMessageUpdateHandler lMessageUpdateHandlerWeaponFire;
	lMessageUpdateHandlerWeaponFire.BindUFunction(this, "shotFiredMessageReceived");
```

```c++
	///Register a callback to a message is received
	UcoreDSEngine::registerMessageUpdateHandler("ShotFired", lMessageUpdateHandlerWeaponFire);
```

```c++
//Play a sound when the ShotFired message is received
void  AFirstPersonShootCPPGameMode::shotFiredMessageReceived(const  TArray< FKeyVariantPair > &Values)
{
	//find the Character instance so we can play the fire sound
	AFirstPersonShootCPPCharacter* myCharacter = Cast<AFirstPersonShootCPPCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

	// try and play the sound if specified
	if (myCharacter != NULL && myCharacter->FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, myCharacter->FireSound, myCharacter->GetActorLocation());
	}
}
```

## Delete Objects (deleteObjectInstance)
If you are using DIS, at some point, you have to let coreDS know that a given object is no longer active; otherwise coreDS will continue to send the keep-alive packet. For HLA users, the concept is more straight forward since you have to actively delete the Object from the federation.

```c++
UcoreDSBluePrintBPLibrary::deleteObject(<Unique Object Identifier>);
```

## Detect Deleted Objects (EntityStatePDU timeout or RemoveObjectInstance)
At this point, you most likely understood how coreDS Unreal works! Let's add an handler when objects are removed from the Distributed Simulation system. When using DIS, this happen when the EntityStatePDU was not updated for the last 5 seconds (or the configuration value in DIS 7). With HLA, the handler is called when a removeObjectInstance callback is received.

Same as before, we first declare a delegate
```c++
	// An object has been removed
	FObjectRemovedHandler lObjectRemovedHandler;
	lObjectRemovedHandler.BindUFunction(this, "objectRemoved");
```

Then we bind it to coreDS Unreal, for each given object type
```c++
	///Register a callback to an object removed
	UcoreDSEngine::registerObjectRemovedHandler("Gun", lObjectRemovedHandler);
	UcoreDSEngine::registerObjectRemovedHandler("Bullet", lObjectRemovedHandler);
```

Finally, we can define the function
```c++
void  AFirstPersonShootCPPGameMode::objectRemoved(FString ObjectName)
```

# Blueprint locations
Everything related to receiving objects and messages(interactions) are located in FirstPersonShootCPPGameMode.cpp.

Logic related to sending the GUN location is within FirstPersonShootCPPCharacter.cpp.

Logic related to sending the FIRE message and update the BULLET position is located in the FirstPersonShootCPPProjectile.cpp.

Most changes are prefixed by "coreDS Unreal".

## Overwriting to ErrorMessage handler
This block allows overwriting the default behavior of displaying the errors in the upper left side of the screen. In some cases, you might want to handler errors in a different UI.

1) Deletegate

```c++
FErrorReceivedHandler lErrorHandler;
lErrorHandler.BindUFunction(this, "printErrorDelegate");
```

2) Register the callback to coreDS Unreal

```c++
///Register a callback to report an error
UcoreDSEngine::registerErrorReceivedHandler(lErrorHandler);
```

3) Define the functions

```c++
void AFirstPersonShootCPPGameMode::printErrorDelegate(FString Message, int Errorcode)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Message);
	UE_LOG(LogClass, Log, TEXT("coreDS: %s"), *Message);
}
```

## Setting your origin
The Unreal coordinate system, basically a rectangle, needs to be converted in order to be sent to DIS and HLA (RPR-FOM). In order to do that, coreDS Unreal provides an easy mechanism.

In order to set your project origin coordinates (lat/lon), go to "Project Settings", then "coreDS":

![ConfigurationSceneLatLon Screenshot](/Doc/Images/ConfigurationSceneLatLon.png)

This can also be done via code using 

```c++
	// coreDS Unreal
	// create our configuration settings. This could be done through the Editor but in case you want to 
	// manage the configure, you can do it here.
	UcoreDSSettings* lSettings = const_cast<UcoreDSSettings*>(GetDefault<UcoreDSSettings>());

	//this will overwrite the values - be careful if you are using the Editor
	lSettings->ReferenceLatitude = 46.8298531;
	lSettings->ReferenceLongitude = -71.2540283;
	lSettings->ReferenceAltitude = 5.0;
	```