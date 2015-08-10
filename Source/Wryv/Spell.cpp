#include "Wryv.h"
#include "Spell.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

ASpell::ASpell(const FObjectInitializer& PCIP) : AGameObject(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;
  attackTarget = 0;
  init = 0;
  caster = 0;
}

void ASpell::Init()
{
  // Get the bounding volume and prox based on it
  TArray<UActorComponent*> ps = GetComponents();
  LOG( "ASpell::ASpell() %d", ps.Num() );
  
  for( int i = 0; i < ps.Num(); i++ )
  {
    if( UShapeComponent* s = Cast<UShapeComponent>( ps[i] ) )
    {
      LOG( "Registering shape component %s", *s->GetName() );
      s->OnComponentBeginOverlap.AddDynamic( this, &ASpell::ProxSpell );
    }
  }

  init = 1;
}

void ASpell::BeginPlay()
{
  Super::BeginPlay();
}

void ASpell::ProxSpell_Implementation( AActor* OtherActor,
  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
  bool bFromSweep, const FHitResult & SweepResult )
{
  LOG( "SPELL: %s collided with %s",
    *Stats.Name, *OtherActor->GetName() );
  
  // deal damage to otheractor (which should be a GameObject
  AGameObject* go = Cast<AGameObject>( OtherActor );
  if( go   &&   go != caster )
  {
    go->Hp -= Stats.AttackDamage;
    // spawn the OnContact spell
    Game->Make( OnContact.GetValue(), Pos, Stats.Team );
    UGameplayStatics::SpawnEmitterAtLocation( GetWorld(), 0, Pos );
    Destroy();
  }
}

void ASpell::Tick( float t )
{
  Super::Tick( t );

  // The init function runs after object construction,
  // For some reason, there's a bug in UE 4.7 that causes 
  // the blueprint to not get loaded.
  if( !init )
  {
    Init();
  }
}

void ASpell::ai( float t )
{
  // The AI for the spell is basically nothing at this point,
  // though we might want to add spin/curl to some spells
}


