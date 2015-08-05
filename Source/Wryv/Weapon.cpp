#include "Wryv.h"
#include "Weapon.h"

AWeapon::AWeapon( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWeapon::Tick( float t )
{
	Super::Tick( t );
}

