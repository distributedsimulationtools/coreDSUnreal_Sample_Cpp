// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FirstPersonShootCPPProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

#include "Containers/Array.h"

//coreDS Unreal include
#include "coreDSEngine.h"
#include "coreDS_BPCoordinateConversion.h"

AFirstPersonShootCPPProjectile::AFirstPersonShootCPPProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFirstPersonShootCPPProjectile::OnHit);		// set up a notification for when this component hits something blocking
	
	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

	//coreDS Unreal
	//make this object tickable
	PrimaryActorTick.bCanEverTick = true;

}

void AFirstPersonShootCPPProjectile::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	Engine = GetGameInstance()->GetSubsystem<UcoreDSEngine>();
}

void AFirstPersonShootCPPProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

		Destroy();
	}
}

void AFirstPersonShootCPPProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if the object was dynamically created, just return
	if (ActorHasTag("coreDSCreated"))
	{
		return;
	}

	//coreDS Unreal
	//Send a message on hit
	TArray< FKeyVariantPair > lValues;

	FVector ActorLocation = GetRootComponent()->GetComponentLocation();
	FRotator ActorRotation = GetRootComponent()->GetComponentRotation();

	float x = 0, y = 0, z = 0;
	float psi = 0, theta = 0, phi = 0;
	UcoreDSSettings* lSettings = const_cast<UcoreDSSettings*>(GetDefault<UcoreDSSettings>());
	UCoreDSCoordinateConversion::EnuToEcef_float(ActorLocation.X, ActorLocation.Y, ActorLocation.Z,
		x, y, z,
		psi, theta, phi,
		ActorRotation.Roll, ActorRotation.Pitch, ActorRotation.Yaw,
		lSettings->ReferenceLatitude, lSettings->ReferenceLongitude, lSettings->ReferenceAltitude);

	lValues.Add(FKeyVariantPair("Location.x", x));
	lValues.Add(FKeyVariantPair("Location.y", y));
	lValues.Add(FKeyVariantPair("Location.z", z));

	//The first argument is the object type, followed a unique identifier, then the values
	Engine->updateObject(GetFName().ToString(), "Bullet", lValues);
}

void AFirstPersonShootCPPProjectile::Destroyed()
{
	// If the object was not created by coreDS Unreal (ie, this is a locally created object)
	// Delete it
	if (!ActorHasTag("coreDSCreated"))
	{
		Engine->removeObject(TCHAR_TO_UTF8(*GetFName().ToString()));
	}

	Super::Destroyed();
}