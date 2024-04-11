coreUnrealCPP
=============

This is a Sample project to be used with `coreDS Unreal <https://www.ds.tools/products/hla-dis-unreal-engine-4/>`__ and the `Unreal Engine 4 and 5 <https://www.unrealengine.com>`__. You can request a free trial at https://www.ds.tools/contact-us/trial-request/

coreDS Unreal must already be installed and activated to use this project. The project is compatible with the Unreal engine 4.24+ and 5.0+. Please make sure the coreDS Unreal plugin is enabled.

This sample focuses on using C++ instead of the BluePrint visual programming language. No matter what you will need the UE4 Editor at some point to manage the configuration file. The base project is the FirstPersonShooter sample, C++ mode, provided by Epic Games.

The Sample project uses the following coreDS concepts:
Connect
Disconnect
Send Update Object (send EntityStatePDU or UpdateAttributeValues, DiscoverObjectInstance) 
Send Message (send a MunitionDetonationPDU or SendInteraction)
Receive Update Object (Receive EntityStatePDU or UpdateAttributeValues)
Receive Message (Receive a MunitionDetonationPDU or SendInteraction)
Delete Objects (EntityStatePDU timeout or RemoveObjectInstance)

Getting started
---------------

First, you need to add the required modules to your application. This is done in the FirstPersonShootCPP.build.cs and addingcoreDSActorComponentModule and coreDSSettings to the PublicDependencyModuleNames array.

.. code-block:: c#

   PublicDependencyModuleNames.AddRange(new string[] { 
   “Core”,
   “CoreUObject”,
   “Engine”,
   “InputCore”,
   “coreDSActorComponentModule”,
   “coreDSSettings” });

You must also add the “coreDSModule” to your .uproject file.

Most functions are included in these header files 

.. code-block:: cpp

   #include “coreDSSettingsClass.h”
   #include “coreDSBluePrintBPLibrary.h”
   #include “coreDSEngine.h”

The first step is to configure coreDS Unreal to know which objects, object attributes, messages and message parameters your simulator will support. Keep in mind that the names you define are not related to the distributed simulation protocol you plan on using. These names will only be used internally when using BluePrint or the Mapping interface.

Although you can place the code pretty much where you want, for the sake of simplicity, we added it to the GameModeConstructor (FirstPersonShootCppGameMode.cpp)

.. code-block:: cpp

   // create our configuration settings. This could be done through the Editor but in case you want to
   // manage the configure, you can do it here.
   UcoreDSSettings* lSettings = const_cast<UcoreDSSettings*>(GetDefault());

   //This will overwrite the values - be careful if you are using the Editor
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

In this particular case, we support sending/receiving a GUN object with Location and Orientation property and a BULLET object with a location. The demo also supports a GunFired message, with a Location property.

The format for Object/Message names is NAME.PROPERTY. The Object/Message name is always the part before the first dot.

The next step is to configure an actual connection with a Distributed Simulation protocol. coreDS supports HLA (High-Level Architecture and DIS (Distributed Interaction Simulation). You can do it either from the UnrealEngine editor or at runtime, from your app, by calling

UcoreDSBluePrintBPLibrary::connect();

You can open the configuration window either by running the game or by clicking on the coreDS toolbar button.

This sample comes with some preconfigured settings:
   DIS: DIS v6
   HLA_Player1: HLA 1516e with RPRFOM 2.0
   HLA_Player2: HLA 1516e with RPRFOM 2.0

.. image:: Doc/Images/ConfigurationSelection.png
   :alt: Plugin ConfigurationSelection Screenshot

Let’s take a look at each configuration.

DIS
---

For both configurations, it is important to configure the Configured Network Adapter. Click on the dropbox and select your active Network Adapter.

.. image:: Doc/Images/DISConnectionConfiguration.png
   :alt: Plugin DISConnectionConfiguration Screenshot

Receiving
~~~~~~~~~

Even if DIS does not explicitly support Subscription, coreDS supports incoming filtering. First, we must let coreDS know that we want to receive the EntityStatePDU and the FirePDU.

.. image:: Doc/Images/DIS_Receiver_PubSub.png
   :alt: Plugin DIS_Receiver_PubSub Screenshot

Then comes the Mapping configuration. Since we are in a receiver, we care about the “Mapping In”

The first step is to Map a Local Object/Message to a Protocol Object/Message. As you see, the Names you defined during the Plugin configuration are listed in the “+” list. You can then link the Local Object/Message to a Protocol Object/Message by using the drop-down menu next to the Object/Message name.

.. image:: Doc/Images/DISMappingIn_ObjectMapping.png
   :alt: Plugin DISMappingIn_ObjectMapping Screenshot

Then you must map the local properties to the protocol properties. Since we are in a receiving mode, we only care about the values we are interested in. In our case, we want to send back to Unreal the Location and the Orientation.

.. image:: Doc/Images/DISMappingIn_WithChoice.png
   :alt: Plugin DISMappingIn_WithChoice Screenshot

Finally, we are receiving coordinates in Geocentric format, which Unreal doesn’t like. We could convert the coordinates from within Unreal but by doing so, it will be harder to switch to a different Distributed Simulation Protocol. To keep all the configuration available at runtime, we use an embedded Lua scripting engine to convert from Geocentric to flat coordinates centred around a given Lat/Long.

.. image:: Doc/Images/DISMappingIn.png
   :alt: Plugin DISMappingIn Screenshot

Below is the script that converts from Geocentric to local coordinates. Scripts are located in /Content/coreDS/Scripts

.. code-block:: lua

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
       --Convert lat/long to geocentric
       tempx, tempy, tempz = lla2ecef(referenceOffset_Lat , referenceOffset_Long , referenceOffset_Alt )

       DSimLocal.X = (tempx - DSimLocal.X)  -- offset to the local coordinates
       DSimLocal.Y = (tempy - DSimLocal.Y)
       DSimLocal.Z = (tempz - DSimLocal.Z)
   end

You might have noticed that nowhere during the configuration process with use the Entity Kind to distinguish between a GUN and a BULLET. coreDS supports complex filtering by leveraging the LUA scripting engine. To filter the GUNs, we’ve created a simple script, FilterGun.lua, which will check the EntityKind and instruct coreDS to discard the mapping if this is not the correct type.

.. image:: Doc/Images/DISMappingIn_Filter.png
   :alt: Plugin DIS_MappingIn Filter Screenshot

.. code-block:: lua

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

Sending
~~~~~~~

Then, we must let coreDS know that we want to send the EntityStatePDU and the FirePDU.

.. image:: Doc/Images/DIS_Sender_PubSub.png
   :alt: Plugin DIS_Sender_PubSub Screenshot

Then comes the Mapping configuration. Since we are in a Sender, we care about the “Mapping Out”

The first step is to Map a Local Object/Message to a Protocol Object/Message. As you see, the Names you defined during the Plugin configuration are listed in the “+” list. You can then link the Local Object/Message to a Protocol Object/Message by using the dropbox next to the Object/Message name.

Then you must map the local properties to the protocol properties. Since we are in a Sender mode, we must fill the complete structure. Static values can be set at this point. We will map Location and Orientation to local properties.

Finally, we are sending coordinates in local format, which DIS doesn’t like. We could convert the coordinates from within Unreal but by doing so, it will be harder to switch to a different Distributed Simulation Protocol. To keep all the configuration available at runtime, we use the embedded Lua scripting engine to convert from flat coordinates, centered around a given Lat/Long, to Geocentric.

.. image:: Doc/Images/DISMappingOut.png
   :alt: Plugin DISMappingOut Screenshot

As for outgoing values, you must set a conversion script to convert from the local coordinates to geocentric coordinates. Scripts are located in /Content/coreDS/Scripts.

HLA
---

For both configurations, it is important to have a valid FOM File. The sample provides an RPRFOM 2.0 compliant FOM file, located in /Config. Although this sample is configured to use an RPRFOM-based FOM file, you can load the FOM file of your choice.

.. image:: Doc/Images/HLAConnectionConfiguration.png
   :alt: Plugin HLAConnectionConfiguration Screenshot

coreDS supports a wide range of RTIs, from the legacy DMSO RTI (HLA 1.3) to HLA 1516e compliant RTI like the Pitch RTI or MAK RTI. The complete list of supported RTI can be found here https://www.ds.tools/products/coreds/detailed-features-list/

If you are using the default configuration, you must select an HLA-Evolved compliant RTI.

.. image:: Doc/Images/HLAConnectionRTISettings.png
   :alt: Plugin HLAConnectionRTISettings Screenshot

Sending
~~~~~~~

First, we must let coreDS know that we want to receive the Statial attribute from a LifeForm and the WeaponFire interaction.

.. image:: Doc/Images/HLA_Receiver_PubSub.png
   :alt: Plugin HLA_Receiver_PubSub Screenshot

Then comes the Mapping configuration. Since we are in a receiver, we care about the “Mapping In”

The first step is to Map a Local Object/Message to a Protocol Object/Message. As you see, the Names you defined during the Plugin configuration are listed in the “+” list. You can then link the Local Object/Message to a Protocol Object/Message by using the dropbox next to the Object/Message name.

Then you must map the local properties to the protocol properties. Since we are in a receiving mode, we only care about the values we are interested in. In our case, we want to send back to Unreal the Location and the Orientation from the Spatial attribute.

Finally, we are receiving coordinates in Geocentric format, which Unreal doesn’t like. We could convert the coordinates from within Unreal but by doing so, it would be harder to switch to a different Distributed Simulation Protocol. To keep all the configuration available at runtime, we use the embedded Lua scripting engine to convert from Geocentric to flat coordinates centred around a given Lat/Long.

.. image:: Doc/Images/HLAMappingIn_ObjectMapping.png
   :alt: Plugin HLAMappingIn_ObjectMapping Screenshot

Below is the script that converts from Geocentric to local coordinates. Scripts are located in /Content/coreDS/Scripts

.. code-block:: lua

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
       --Convert lat/long to geocentric
       tempx, tempy, tempz = lla2ecef(referenceOffset_Lat , referenceOffset_Long , referenceOffset_Alt )

       --Offset the coordinate to the local area
       DSimLocal.WorldLocation.X = (DSimLocal.WorldLocation.X - tempx)
       DSimLocal.WorldLocation.Y = (DSimLocal.WorldLocation.Y - tempy)
       DSimLocal.WorldLocation.Z = (DSimLocal.WorldLocation.Z - tempz)
   end

Receiving
~~~~~~~~~

First, we must let coreDS know that we want to send the Statial attribute from a LifeForm and the WeaponFire interaction.

.. image:: Doc/Images/DIS_Sender_PubSub.png
   :alt: Plugin DIS_Sender_PubSub Screenshot

Then comes the Mapping configuration. Since we are in a Sender, we care about the “Mapping Out”

The first step is to Map a Local Object/Message to a Protocol Object/Message. As you see, the Names you defined during the Plugin configuration are listed in the “+” list. You can then link the Local Object/Message to a Protocol Object/Message by using the dropbox next to the Object/Message name.

Then you must map the local properties to the protocol properties. Since we are in a Sender mode, we must fill the complete structure. Static
values can be set at this point. We will map Location and Orientation to local properties.

Finally, we are sending coordinates in local format, which is not compliant with the RPRFOM Spatial attribute format. We could convert the
coordinates from within Unreal but by doing so, it will be harder to switch to a different Distributed Simulation Protocol. To keep all the
configurations editable are runtime, we use the embedded Lua scripting engine to convert from flat coordinates, centred around a given Lat/Long, to
Geocentric.

.. image:: Doc/Images/HLAMappingOut.png
   :alt: Plugin HLAMappingOut Screenshot

As for outgoing values, you must set a conversion script to convert from the local coordinates to geocentric coordinates. Scripts are located in /Content/coreDS/Scripts.

Connect
-------

At some point, you must instruct your simulator to connect to the Distributed Simulation system (either HLA or DIS). When using DIS, a UDP socket will be created. When using HLA, a connection to the RTI will be attempted. If supported by the HLA version you are using, a call to connect() will be made, followed by a call to createFederationExecution (this call can be disabled from the HLA configuration) and joinFederationExecution. Once we have joined the Federation, we then set the various required states like time management, publish/subscribe, etc.

The easiest way to do that is by calling the connection function

.. code-block:: cpp

   UcoreDSBluePrintBPLibrary::connect();

Disconnect
----------

When disconnecting, a DIS configuration will close the socket. HLA will call resignFederationExecution and destroyFederationExecution.

.. code-block:: cpp

   UcoreDSBluePrintBPLibrary::disconnect();

Send Update Object (send EntityStatePDU or UpdateAttributeValues, DiscoverObjectInstance)
-----------------------------------------------------------------------------------------

Sending an Object Update is fairly simple. You first have to build an array of <Name, Value> pairs then use the SendUpdateObject block. All names must match the names configured in the coreDS Unreal plugin.

When using HLA, if this is the first call using that object type, registerObjectInstance will be called, followed by UpdateObjectAttributeValues.

When using DIS, an EntityStatePDU will be sent.

.. code-block:: cpp

       TArray< FKeyVariantPair > lValues;

       lValues.Add(FKeyVariantPair("Location.x", ActorLocation.X));
       lValues.Add(FKeyVariantPair("Location.y", ActorLocation.Y));
       lValues.Add(FKeyVariantPair("Location.z", ActorLocation.Z));

       UcoreDSEngine::updateObject("Bullet", lValues);

Send Message (send a MunitionDetonationPDU or SendInteraction)
--------------------------------------------------------------

Sending a message is fairly simple. You first have to build an array of <Name, Value> pairs then use the SendMessage block. All names must match the names configured in the coreDS Unreal plugin.

When using HLA, a call to SendInteraction will be made.

When using DIS, a FirePDU will be sent.

.. code-block:: cpp

       // coreDS Unreal
       // Send the WeaponFire message
       TArray< FKeyVariantPair > lValues;

       lValues.Add(FKeyVariantPair("Location.x", ActorLocation.X));
       lValues.Add(FKeyVariantPair("Location.y", ActorLocation.Y));
       lValues.Add(FKeyVariantPair("Location.z", ActorLocation.Z));

       UcoreDSEngine::sendMessage("ShotFired", lValues);

Receive Update Object (Receive EntityStatePDU or UpdateAttributeValues)
-----------------------------------------------------------------------

If you want to receive Object updates, you must first register an ObjectUpdateHandler. The registration must be done using the object name used in the coreDS Unreal plugin configuration:

In C++, this is a three-step process

1) Declare the delegate and bind the callback function.

.. code-block:: cpp

   FObjectUpdateHandler lObjectUpdateHandlerForGuns;
   lObjectUpdateHandlerForGuns.BindUFunction(this, "gunUpdated");

2) Register the callback to an update object event

.. code-block:: cpp

   UcoreDSEngine::registerObjectUpdateHandler("Gun", lObjectUpdateHandlerForGuns);

3) Define and fill the “gunUpdated” function. Values are received in a list of value pairs. Each pair consists of the value name, as defined in the mapping and the value as a string. ObjectName is the unique object identifier

.. code-block:: cpp

   void  AFirstPersonShootCPPGameMode::gunUpdated(const TArray< FKeyVariantPair > &Values, FString ObjectName)
   {
       ...
   }

Then, each time an object of the register type is received, the GunMoved event will be triggered.

In this particular case, we also added some logic to keep a list of discovered objects in Unreal. We first look into a map if the discovered object name already exists on the map. If not, a new Actor is spawned, else the Actor instance is updated. That part will most likely need to be updated based on your particular requirements.

Receive Message (Receive a MunitionDetonationPDU or SendInteraction)
--------------------------------------------------------------------

If you want to receive a message, you must first register a MessageReceivedHandler. The strategy is the same:

.. code-block:: cpp

       // WeaponFire message received
       FMessageUpdateHandler lMessageUpdateHandlerWeaponFire;
       lMessageUpdateHandlerWeaponFire.BindUFunction(this, "shotFiredMessageReceived");

.. code-block:: cpp

       ///Register a callback to a message is received
       UcoreDSEngine::registerMessageUpdateHandler("ShotFired", lMessageUpdateHandlerWeaponFire);

.. code-block:: cpp

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

Delete Objects (deleteObjectInstance)
-------------------------------------

If you are using DIS, at some point, you have to let coreDS know that a given object is no longer active; otherwise, coreDS will continue to send the keep-alive packet. For HLA users, the concept is more straightforward since you have to actively delete the Object from the federation.

.. code-block:: cpp

   UcoreDSBluePrintBPLibrary::deleteObject(<Unique Object Identifier>);

Detect Deleted Objects (EntityStatePDU timeout or RemoveObjectInstance)
-----------------------------------------------------------------------

At this point, you most likely understood how coreDS Unreal works! Let’s add a handler when objects are removed from the Distributed Simulation system. When using DIS, this happens when the EntityStatePDU is not updated for the last 5 seconds (or the configuration value in DIS 7). With HLA, the handler is called when a removeObjectInstance callback is received.

Same as before, we first declare a delegate

.. code-block:: cpp

       // An object has been removed
       FObjectRemovedHandler lObjectRemovedHandler;
       lObjectRemovedHandler.BindUFunction(this, "objectRemoved");

Then we bind it to coreDS Unreal, for each given object type

.. code-block:: cpp

       ///Register a callback to an object removed
       UcoreDSEngine::registerObjectRemovedHandler("Gun", lObjectRemovedHandler);
       UcoreDSEngine::registerObjectRemovedHandler("Bullet", lObjectRemovedHandler);

Finally, we can define the function

.. code-block:: cpp

   void  AFirstPersonShootCPPGameMode::objectRemoved(FString ObjectName)

Blueprint locations
===================

Everything related to receiving objects and messages(interactions) is located in FirstPersonShootCPPGameMode.cpp.

Logic related to sending the GUN location is within FirstPersonShootCPPCharacter.cpp.

Logic related to sending the FIRE message and updating the BULLET position is located in the FirstPersonShootCPPProjectile.cpp.

Most changes are prefixed by “coreDS Unreal”.

Overwriting to ErrorMessage handler
-----------------------------------

This block allows overwriting the default behaviour of displaying the errors in the upper left side of the screen. In some cases, you might want to handle errors in a different UI.

1) Deletegate

.. code-block:: cpp

   FErrorReceivedHandler lErrorHandler;
   lErrorHandler.BindUFunction(this, "printErrorDelegate");

2) Register the callback to coreDS Unreal

.. code-block:: cpp

   ///Register a callback to report an error
   UcoreDSEngine::registerErrorReceivedHandler(lErrorHandler);

3) Define the functions

.. code-block:: cpp

   void AFirstPersonShootCPPGameMode::printErrorDelegate(FString Message, int Errorcode)
   {
       GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, *Message);
       UE_LOG(LogClass, Log, TEXT("coreDS: %s"), *Message);
   }

Setting your origin
-------------------

The Unreal coordinate system, basically a rectangle, needs to be converted to be sent to DIS and HLA (RPR-FOM). To do that, coreDS Unreal provides an easy mechanism.

To set your project origin coordinates (lat/lon), go to “Project Settings”, then “coreDS”:

.. image:: Doc/Images/ConfigurationSceneLatLon.png
   :alt: ConfigurationSceneLatLon Screenshot

This can also be done via code using

.. code-block:: cpp

   // create our configuration settings. This could be done through the Editor but in case you want to
   // manage the configure, you can do it here.
   UcoreDSSettings* lSettings = const_cast<UcoreDSSettings*>(GetDefault());

   //This will overwrite the values - be careful if you are using the Editor
   lSettings->ReferenceLatitude = 46.8298531;
   lSettings->ReferenceLongitude = -71.2540283;
   lSettings->ReferenceAltitude = 5.0;